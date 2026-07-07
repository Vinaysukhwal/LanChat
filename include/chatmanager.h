#ifndef CHATMANAGER_H
#define CHATMANAGER_H

#include <QObject>
#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QMap>
#include <QList>
#include <QDateTime>
#include "peerinfo.h"

/*
 * ChatManager - handles all the networking stuff
 * Uses UDP broadcasts for discovering peers and TCP for actual messages
 */
class ChatManager : public QObject
{
    Q_OBJECT
public:
    explicit ChatManager(QObject *parent = nullptr);
    ~ChatManager() override;

    void start();
    void stop();

    void setUsername(const QString& username);
    QString username() const { return m_username; }

    void setVisibility(bool visible);
    bool isVisible() const { return m_visible; }

    quint16 tcpPort() const { return m_tcpPort; }

    QList<PeerInfo> onlinePeers() const;
    PeerInfo peerInfo(const QString& peerId) const;

    // sends a chat message to the peer with given ID
    void sendMessage(const QString& peerId, const QString& text);

signals:
    void peerListUpdated();

    // emitted when we get a message from someone
    void messageReceived(const QString& senderName, const QString& text, const QDateTime& timestamp);

private slots:
    void broadcastPresence();
    void processIncomingBroadcasts();
    void checkPeerTimeouts();
    void onNewTcpConnection();
    void onTcpDataReady();
    void onTcpSocketConnected();
    void onTcpSocketDisconnected();
    void onTcpSocketError(QAbstractSocket::SocketError socketError);

private:
    void sendGoodbye();
    void sendJsonOverSocket(QTcpSocket* socket, const QString& text);
    void setupSocket(QTcpSocket* socket);

    QString m_peerId;
    QString m_username;
    bool m_visible;
    quint16 m_tcpPort;

    QUdpSocket* m_udpSocket;
    QTcpServer* m_tcpServer;

    QTimer* m_heartbeatTimer;
    QTimer* m_timeoutTimer;

    QMap<QString, PeerInfo> m_peers;                  // peerId -> info
    QMap<QString, QTcpSocket*> m_activeConnections;   // peerId -> socket

    // TODO: maybe make this configurable from the UI?
    const quint16 UDP_PORT = 45454;
};

#endif // CHATMANAGER_H
