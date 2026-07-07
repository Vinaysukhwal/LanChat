#ifndef PEERINFO_H
#define PEERINFO_H

#include <QString>
#include <QHostAddress>
#include <QDateTime>

// holds info about a discovered peer on the network
struct PeerInfo {
    QString id;             // UUID
    QString username;
    QHostAddress address;
    quint16 tcpPort;
    QDateTime lastSeen;
    // QString status; // might use this later for away/busy

    QString displayName() const {
        return QString("%1 (%2:%3)")
            .arg(username.isEmpty() ? "Anonymous" : username)
            .arg(address.toString())
            .arg(tcpPort);
    }
};

#endif // PEERINFO_H
