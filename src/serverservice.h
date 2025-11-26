#ifndef SERVERSERVICE_H
#define SERVERSERVICE_H

#include "serverinfo.h"
#include "firewall/ifirewallservice.h"
#include <QMap>
#include <QList>
#include <QFuture>
#include <QVariant>

class ServerService : public QObject {
    Q_OBJECT
public:
    ServerService(IFirewallService* firewallService, QObject* parent = nullptr);

    QFuture<QPair<bool, QVariant>> fetchServerDataAsync();
    QList<ServerInfo> getServers(bool clustered);
    QFuture<bool> blockServerAsync(ServerInfo& server);
    QFuture<bool> unblockServerAsync(ServerInfo& server);
    QFuture<void> refreshBlockedStatusAsync(bool clustered);
    QFuture<bool> unblockAllServersAsync();

private:
    QMap<QString, QList<QString>> clusterDict;
    QMap<QString, ServerInfo> servers;
    QMap<QString, ServerInfo> clusteredServers;
    QVariant serverRevision;
    IFirewallService* firewallService;

    QString getClusteredName(const QString& serverName);
};

#endif // SERVERSERVICE_H