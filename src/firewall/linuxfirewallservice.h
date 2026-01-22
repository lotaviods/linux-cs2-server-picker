#ifndef LINUXFIREWALLSERVICE_H
#define LINUXFIREWALLSERVICE_H

#include "ifirewallservice.h"
#include <QObject>
#include <QtConcurrent>

class LinuxFirewallService : public QObject, public IFirewallService {
    Q_OBJECT
public:
    LinuxFirewallService(QObject* parent = nullptr);

    QFuture<bool> blockServerAsync(const QString& ruleName, const QStringList& ipAddresses) override;
    QFuture<bool> unblockServerAsync(const QString& ruleName) override;
    QFuture<bool> isServerBlockedAsync(const QString& ruleName) override;
    QFuture<bool> unblockAllServersAsync() override;

private:
    static const QString RulePrefix;
    QString iptablesPath() const { return "iptables"; }
    QString ip6tablesPath() const { return "ip6tables"; }

    bool executeCommand(const QString& command, const QStringList& arguments);
    QString executeCommandWithOutput(const QString& command, const QStringList& arguments);
    int deleteRulesByComment(const QString& command, const QStringList& listArgs, const QString& commentPrefix);
};

#endif // LINUXFIREWALLSERVICE_H