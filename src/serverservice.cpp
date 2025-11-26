#include "serverservice.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QEventLoop>
#include <QtConcurrent>

ServerService::ServerService(IFirewallService* firewallService, QObject* parent)
    : QObject(parent), firewallService(firewallService) {
    clusterDict = {
        {"China", {"Perfect", "Hong Kong", "Alibaba", "Tencent"}},
        {"Japan", {"Tokyo"}},
        {"Stockholm (Sweden)", {"Stockholm"}},
        {"India", {"Chennai", "Mumbai"}}
    };
}

QFuture<QPair<bool, QVariant>> ServerService::fetchServerDataAsync() {
    return QtConcurrent::run([this]() -> QPair<bool, QVariant> {
        QNetworkAccessManager manager;
        QNetworkRequest request(QUrl("https://api.steampowered.com/ISteamApps/GetSDRConfig/v1/?appid=730"));
        QNetworkReply* reply = manager.get(request);
        QEventLoop loop;
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();
        if (reply->error() != QNetworkReply::NoError) {
            reply->deleteLater();
            return {false, QVariant()};
        }
        QByteArray data = reply->readAll();
        reply->deleteLater();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (!doc.isObject()) return {false, QVariant()};
        QJsonObject root = doc.object();
        if (!root.contains("revision")) return {false, QVariant()};
        serverRevision = root["revision"].toInt();
        if (!root.contains("pops")) return {false, QVariant()};
        QJsonObject pops = root["pops"].toObject();
        servers.clear();
        clusteredServers.clear();
        for (auto it = pops.begin(); it != pops.end(); ++it) {
            QJsonObject serverObj = it.value().toObject();
            if (!serverObj.contains("relays") || !serverObj.contains("desc")) continue;
            QString serverName = QString("%1 (%2)").arg(serverObj["desc"].toString(), it.key());
            QStringList ipAddresses;
            QJsonArray relays = serverObj["relays"].toArray();
            for (const QJsonValue& relay : relays) {
                QJsonObject relayObj = relay.toObject();
                if (relayObj.contains("ipv4")) {
                    QString ip = relayObj["ipv4"].toString();
                    if (!ip.isEmpty()) ipAddresses.append(ip);
                }
            }
            if (ipAddresses.isEmpty()) continue;
            // unclustered
            if (servers.contains(serverName)) {
                servers[serverName].ipAddresses.append(ipAddresses);
            } else {
                servers[serverName] = {serverName, serverName, ipAddresses};
            }
            // clustered
            QString clusteredName = getClusteredName(serverName);
            if (!clusteredName.isEmpty()) {
                if (clusteredServers.contains(clusteredName)) {
                    clusteredServers[clusteredName].ipAddresses.append(ipAddresses);
                } else {
                    clusteredServers[clusteredName] = {clusteredName, clusteredName, ipAddresses};
                }
            } else {
                if (clusteredServers.contains(serverName)) {
                    clusteredServers[serverName].ipAddresses.append(ipAddresses);
                } else {
                    clusteredServers[serverName] = {serverName, serverName, ipAddresses};
                }
            }
        }
        return {true, serverRevision};
    });
}

QList<ServerInfo> ServerService::getServers(bool clustered) {
    auto& dict = clustered ? clusteredServers : servers;
    return dict.values();
}

QFuture<bool> ServerService::blockServerAsync(ServerInfo& server) {
    return QtConcurrent::run([this, &server]() {
        bool success = firewallService->blockServerAsync(server.region, server.ipAddresses).result();
        if (success) server.isBlocked = true;
        return success;
    });
}

QFuture<bool> ServerService::unblockServerAsync(ServerInfo& server) {
    return QtConcurrent::run([this, &server]() {
        bool success = firewallService->unblockServerAsync(server.region).result();
        if (success) server.isBlocked = false;
        return success;
    });
}

QFuture<void> ServerService::refreshBlockedStatusAsync(bool clustered) {
    return QtConcurrent::run([this, clustered]() {
        auto& dict = clustered ? clusteredServers : servers;
        for (auto& pair : dict) {
            pair.isBlocked = firewallService->isServerBlockedAsync(pair.region).result();
        }
    });
}

QFuture<bool> ServerService::unblockAllServersAsync() {
    return QtConcurrent::run([this]() {
        bool success = firewallService->unblockAllServersAsync().result();
        if (success) {
            for (auto& server : servers) server.isBlocked = false;
            for (auto& server : clusteredServers) server.isBlocked = false;
        }
        return success;
    });
}

QString ServerService::getClusteredName(const QString& serverName) {
    for (auto it = clusterDict.begin(); it != clusterDict.end(); ++it) {
        for (const QString& value : it.value()) {
            if (serverName.contains(value)) return it.key();
        }
    }
    return QString();
}