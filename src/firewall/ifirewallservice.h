#ifndef IFIREWALLSERVICE_H
#define IFIREWALLSERVICE_H

#include <QString>
#include <QStringList>
#include <QFuture>

class IFirewallService {
public:
    virtual ~IFirewallService() = default;
    virtual QFuture<bool> blockServerAsync(const QString& ruleName, const QStringList& ipAddresses) = 0;
    virtual QFuture<bool> unblockServerAsync(const QString& ruleName) = 0;
    virtual QFuture<bool> isServerBlockedAsync(const QString& ruleName) = 0;
    virtual QFuture<bool> unblockAllServersAsync() = 0;
};

#endif // IFIREWALLSERVICE_H