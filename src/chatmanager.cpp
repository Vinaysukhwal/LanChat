#include "chatmanager.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkInterface>
#include <QDataStream>
#include <QUuid>
#include <QDebug>

ChatManager::ChatManager(QObject *parent)
    : QObject(parent)
    , m_visible(true)
    , m_tcpPort(0)
{
    m_peerId = QUuid::createUuid().toString();

    // grab the system username for a default display name
    QString systemUser = qgetenv("USERNAME");
    if (systemUser.isEmpty()) {
        systemUser = qgetenv("USER");   // linux fallback
    }
    if (systemUser.isEmpty()) {
        systemUser = "User";
    }
    m_username = QString("%1_%2").arg(systemUser).arg(m_peerId.mid(1, 4));

    m_udpSocket = new QUdpSocket(this);
    m_tcpServer = new QTcpServer(this);

    m_heartbeatTimer = new QTimer(this);
    m_timeoutTimer = new QTimer(this);

    connect(m_udpSocket, &QUdpSocket::readyRead, this, &ChatManager::processIncomingBroadcasts);
    connect(m_tcpServer, &QTcpServer::newConnection, this, &ChatManager::onNewTcpConnection);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &ChatManager::broadcastPresence);
    connect(m_timeoutTimer, &QTimer::timeout, this, &ChatManager::checkPeerTimeouts);
}

ChatManager::~ChatManager()
{
    stop();
}

void ChatManager::start()
{
    // bind UDP socket for peer discovery
    // ShareAddress lets multiple instances run on the same machine (useful for testing)
    bool udpBound = m_udpSocket->bind(QHostAddress::AnyIPv4, UDP_PORT,
                                      QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
    if (!udpBound) {
        qWarning() << "Failed to bind UDP socket on port" << UDP_PORT
                    << "-" << m_udpSocket->errorString();
        
        // HACK: on some Windows setups the first bind fails for no good reason,
        // recreating the socket and trying again usually fixes it
        m_udpSocket->close();
        delete m_udpSocket;
        m_udpSocket = new QUdpSocket(this);
        connect(m_udpSocket, &QUdpSocket::readyRead, this, &ChatManager::processIncomingBroadcasts);
        
        udpBound = m_udpSocket->bind(UDP_PORT, QAbstractSocket::ShareAddress | QAbstractSocket::ReuseAddressHint);
        if (!udpBound) {
            qWarning() << "UDP retry failed too:" << m_udpSocket->errorString();
            // app still works for sending, just won't recieve discovery broadcasts
        }
    }
    
    if (udpBound) {
        qDebug() << "UDP bound on port" << UDP_PORT;
    }

    // let the OS pick a free port for TCP
    if (m_tcpServer->listen(QHostAddress::AnyIPv4, 0)) {
        m_tcpPort = m_tcpServer->serverPort();
        qDebug() << "TCP listening on port" << m_tcpPort;
    } else {
        qCritical() << "TCP server failed:" << m_tcpServer->errorString();
        m_tcpPort = 0;
    }

    // heartbeat every 2s, timeout check every 5s
    m_heartbeatTimer->start(2000);
    m_timeoutTimer->start(5000);

    broadcastPresence();
}

void ChatManager::stop()
{
    // tell everyone we're leaving
    sendGoodbye();

    m_heartbeatTimer->stop();
    m_timeoutTimer->stop();

    if (m_tcpServer->isListening()) {
        m_tcpServer->close();
    }
    
    m_udpSocket->close();

    for (auto socket : m_activeConnections) {
        if (socket) {
            socket->disconnect();
            socket->abort();
            socket->deleteLater();
        }
    }
    m_activeConnections.clear();
    m_peers.clear();

    emit peerListUpdated();
}

void ChatManager::setUsername(const QString &username)
{
    if (username.trimmed().isEmpty()) return;
    
    if (m_username != username) {
        m_username = username;
        if (m_udpSocket->isOpen() && m_visible) {
            broadcastPresence();  // broadcast new name right away
        }
    }
}

void ChatManager::setVisibility(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        if (m_visible) {
            if (m_udpSocket->isOpen()) {
                broadcastPresence();
            }
        } else {
            sendGoodbye();
        }
    }
}

QList<PeerInfo> ChatManager::onlinePeers() const
{
    return m_peers.values();
}

PeerInfo ChatManager::peerInfo(const QString &peerId) const
{
    return m_peers.value(peerId, PeerInfo());
}

void ChatManager::sendMessage(const QString &peerId, const QString &text)
{
    if (!m_peers.contains(peerId)) {
        qWarning() << "Peer not found:" << peerId;
        return;
    }

    PeerInfo peer = m_peers[peerId];
    QTcpSocket* socket = m_activeConnections.value(peerId, nullptr);

    if (!socket || socket->state() == QAbstractSocket::UnconnectedState) {
        // need to establish a new connection first
        socket = new QTcpSocket(this);
        setupSocket(socket);
        socket->setProperty("peerId", peerId);

        connect(socket, &QTcpSocket::connected, this, &ChatManager::onTcpSocketConnected);

        m_activeConnections[peerId] = socket;

        // queue the message until connection is ready
        QStringList pending;
        pending.append(text);
        socket->setProperty("pendingMessages", pending);

        qDebug() << "Connecting to" << peer.displayName();
        socket->connectToHost(peer.address, peer.tcpPort);
    } 
    else if (socket->state() == QAbstractSocket::ConnectingState) {
        // still connecting, just queue it
        QStringList pending = socket->property("pendingMessages").toStringList();
        pending.append(text);
        socket->setProperty("pendingMessages", pending);
    } 
    else if (socket->state() == QAbstractSocket::ConnectedState) {
        sendJsonOverSocket(socket, text);
    }
}

void ChatManager::broadcastPresence()
{
    if (!m_visible) return;

    QJsonObject obj;
    obj["type"] = "discovery";
    obj["peerId"] = m_peerId;
    obj["username"] = m_username;
    obj["port"] = m_tcpPort;
    obj["visible"] = true;

    QByteArray datagram = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    // send to general broadcast address
    m_udpSocket->writeDatagram(datagram, QHostAddress::Broadcast, UDP_PORT);

    // also broadcast on each network interface individually
    // (needed on Windows when you have multiple adapters/VPNs)
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            iface.flags().testFlag(QNetworkInterface::CanBroadcast)) {
            
            for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
                QHostAddress bcast = entry.broadcast();
                if (!bcast.isNull()) {
                    m_udpSocket->writeDatagram(datagram, bcast, UDP_PORT);
                }
            }
        }
    }
}

void ChatManager::sendGoodbye()
{
    QJsonObject obj;
    obj["type"] = "offline";
    obj["peerId"] = m_peerId;

    QByteArray datagram = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    m_udpSocket->writeDatagram(datagram, QHostAddress::Broadcast, UDP_PORT);

    // same multi-interface broadcast as above
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            iface.flags().testFlag(QNetworkInterface::CanBroadcast)) {
            
            for (const QNetworkAddressEntry &entry : iface.addressEntries()) {
                QHostAddress bcast = entry.broadcast();
                if (!bcast.isNull()) {
                    m_udpSocket->writeDatagram(datagram, bcast, UDP_PORT);
                }
            }
        }
    }
}

void ChatManager::processIncomingBroadcasts()
{
    while (m_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(static_cast<int>(m_udpSocket->pendingDatagramSize()));
        
        QHostAddress senderAddress;
        quint16 senderPort;
        m_udpSocket->readDatagram(datagram.data(), datagram.size(), &senderAddress, &senderPort);

        QJsonDocument doc = QJsonDocument::fromJson(datagram);
        if (doc.isNull() || !doc.isObject()) continue;

        QJsonObject obj = doc.object();
        QString type = obj["type"].toString();
        QString peerId = obj["peerId"].toString();

        if (peerId == m_peerId) continue; // ignore our own broadcasts

        if (type == "discovery") {
            QString username = obj["username"].toString();
            quint16 tcpPort = static_cast<quint16>(obj["port"].toInt());
            bool visible = obj["visible"].toBool();

            if (!visible) {
                if (m_peers.contains(peerId)) {
                    m_peers.remove(peerId);
                    emit peerListUpdated();
                }
                continue;
            }

            bool changed = false;

            // normalize IPv6-mapped addresses back to IPv4
            QHostAddress normalizedAddress = senderAddress;
            quint32 ipv4 = normalizedAddress.toIPv4Address();
            if (ipv4 != 0) {
                normalizedAddress = QHostAddress(ipv4);
            }

            if (!m_peers.contains(peerId)) {
                PeerInfo peer;
                peer.id = peerId;
                peer.username = username;
                peer.address = normalizedAddress;
                peer.tcpPort = tcpPort;
                peer.lastSeen = QDateTime::currentDateTime();
                m_peers.insert(peerId, peer);
                changed = true;
                qDebug() << "New peer found:" << peer.displayName();
            } else {
                PeerInfo &peer = m_peers[peerId];
                if (peer.username != username || peer.address != normalizedAddress || peer.tcpPort != tcpPort) {
                    peer.username = username;
                    peer.address = normalizedAddress;
                    peer.tcpPort = tcpPort;
                    changed = true;
                }
                peer.lastSeen = QDateTime::currentDateTime();
            }

            if (changed) {
                emit peerListUpdated();
            }
        } 
        else if (type == "offline") {
            if (m_peers.contains(peerId)) {
                qDebug() << "Peer left:" << m_peers[peerId].displayName();
                m_peers.remove(peerId);
                emit peerListUpdated();
            }
        }
    }
}

void ChatManager::checkPeerTimeouts()
{
    // if we havent heard from a peer in 10 seconds, assume they're gone
    QDateTime cutoff = QDateTime::currentDateTime().addSecs(-10);
    bool changed = false;

    auto it = m_peers.begin();
    while (it != m_peers.end()) {
        if (it.value().lastSeen < cutoff) {
            qDebug() << "Peer timed out:" << it.value().displayName();
            it = m_peers.erase(it);
            changed = true;
        } else {
            ++it;
        }
    }

    if (changed) {
        emit peerListUpdated();
    }
}

void ChatManager::onNewTcpConnection()
{
    while (m_tcpServer->hasPendingConnections()) {
        QTcpSocket* socket = m_tcpServer->nextPendingConnection();
        setupSocket(socket);
        // qDebug() << "Incoming TCP from" << socket->peerAddress().toString();
    }
}

void ChatManager::setupSocket(QTcpSocket* socket)
{
    socket->setParent(this);
    connect(socket, &QTcpSocket::readyRead, this, &ChatManager::onTcpDataReady);
    connect(socket, &QTcpSocket::disconnected, this, &ChatManager::onTcpSocketDisconnected);
    connect(socket, &QAbstractSocket::errorOccurred, this, &ChatManager::onTcpSocketError);
}

void ChatManager::onTcpDataReady()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QDataStream in(socket);
    in.setVersion(QDataStream::Qt_5_15);

    // keep reading until we run out of complete frames
    while (true) {
        in.startTransaction();
        QByteArray docBytes;
        in >> docBytes;
        
        if (!in.commitTransaction()) {
            break;  // incomplete packet, wait for more data
        }

        QJsonDocument doc = QJsonDocument::fromJson(docBytes);
        if (doc.isNull() || !doc.isObject()) continue;

        QJsonObject obj = doc.object();
        QString senderName = obj["senderName"].toString();
        QString text = obj["message"].toString();
        QString timestampStr = obj["timestamp"].toString();
        QDateTime timestamp = QDateTime::fromString(timestampStr, Qt::ISODate);
        if (!timestamp.isValid()) {
            timestamp = QDateTime::currentDateTime();
        }

        emit messageReceived(senderName, text, timestamp);
    }
}

void ChatManager::onTcpSocketConnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QString peerId = socket->property("peerId").toString();

    // flush any messages that were queued while we were connecting
    QStringList pending = socket->property("pendingMessages").toStringList();
    for (const QString& msg : pending) {
        sendJsonOverSocket(socket, msg);
    }
    socket->setProperty("pendingMessages", QStringList());
}

void ChatManager::onTcpSocketDisconnected()
{
    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    QString peerId = socket->property("peerId").toString();

    if (!peerId.isEmpty() && m_activeConnections.value(peerId) == socket) {
        m_activeConnections.remove(peerId);
    }

    socket->deleteLater();
}

void ChatManager::onTcpSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);

    QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;

    qWarning() << "TCP error for peer" << socket->property("peerId").toString()
               << ":" << socket->errorString();

    QString peerId = socket->property("peerId").toString();
    if (!peerId.isEmpty() && m_activeConnections.value(peerId) == socket) {
        m_activeConnections.remove(peerId);
    }

    socket->deleteLater();
}

void ChatManager::sendJsonOverSocket(QTcpSocket* socket, const QString &text)
{
    QJsonObject obj;
    obj["senderId"] = m_peerId;
    obj["senderName"] = m_username;
    obj["message"] = text;
    obj["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QByteArray docBytes = QJsonDocument(obj).toJson(QJsonDocument::Compact);

    // wrap in QDataStream framing so the reciever knows where one message ends
    // and the next one begins (TCP doesn't preserve message boundaries)
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_15);
    out << docBytes;

    socket->write(block);
}
