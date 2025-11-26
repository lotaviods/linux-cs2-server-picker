#ifndef SERVERINFO_H
#define SERVERINFO_H

#include <QString>
#include <QStringList>
#include <QVariant>

enum class ServerStatus {
    Unknown,
    Pinging,
    Online,
    Timeout,
    Blocked
};

struct ServerInfo {
    QString name;
    QString region;
    QStringList ipAddresses;
    QVariant latency; // int or null
    bool isBlocked = false;
    ServerStatus status = ServerStatus::Unknown;
};

#endif // SERVERINFO_H