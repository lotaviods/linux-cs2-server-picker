#ifndef FIREWALLCLIENT_H
#define FIREWALLCLIENT_H

#include "ifirewallservice.h"
#include <QObject>

class FirewallClient : public QObject, public IFirewallService {
    Q_OBJECT
public:
    FirewallClient(QObject* parent = nullptr);

    QFuture<bool> blockServerAsync(const QString& ruleName, const QStringList& ipAddresses) override;
    QFuture<bool> unblockServerAsync(const QString& ruleName) override;
    QFuture<bool> isServerBlockedAsync(const QString& ruleName) override;
    QFuture<bool> unblockAllServersAsync() override;
    bool isAdministrator() override;

private:
    QString sendCommand(const QString& command);
};

#endif // FIREWALLCLIENT_H