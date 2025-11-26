#include "linuxfirewallservice.h"
#include <QProcess>
#include <QDebug>
#include <QtConcurrent>
#include <unistd.h>

const QString LinuxFirewallService::RulePrefix = "CS2ServerPicker_";

LinuxFirewallService::LinuxFirewallService(QObject* parent) : QObject(parent) {}

bool LinuxFirewallService::isAdministrator() {
    return getuid() == 0;
}

QFuture<bool> LinuxFirewallService::blockServerAsync(const QString& ruleName, const QStringList& ipAddresses) {
    return QtConcurrent::run([this, ruleName, ipAddresses]() {
        QString cleanName = ruleName;
        cleanName.replace(" ", "_");
        bool success = true;
        for (const QString& ip : ipAddresses) {
            bool isIpv6 = ip.contains(":");
            QString command = isIpv6 ? ip6tablesPath() : iptablesPath();
            QStringList argsList = QStringList() << "-A" << "OUTPUT" << "-d" << ip << "-j" << "DROP" << "-m" << "comment" << "--comment" << (RulePrefix + cleanName);
            if (!executeCommand(command, argsList)) {
                success = false;
            }
        }
        return success;
    });
}

QFuture<bool> LinuxFirewallService::unblockServerAsync(const QString& ruleName) {
    return QtConcurrent::run([this, ruleName]() {
        QString cleanName = ruleName;
        cleanName.replace(" ", "_");
        QString comment = RulePrefix + cleanName;
        int ipv4Deleted = deleteRulesByComment(iptablesPath(), QStringList{"-L", "OUTPUT", "-n", "--line-numbers"}, comment);
        int ipv6Deleted = deleteRulesByComment(ip6tablesPath(), QStringList{"-L", "OUTPUT", "-n", "--line-numbers"}, comment);
        return (ipv4Deleted + ipv6Deleted) > 0;
    });
}

QFuture<bool> LinuxFirewallService::isServerBlockedAsync(const QString& ruleName) {
    return QtConcurrent::run([this, ruleName]() {
        QString cleanName = ruleName;
        cleanName.replace(" ", "_");
        QString comment = RulePrefix + cleanName;
        QString ipv4Rules = executeCommandWithOutput(iptablesPath(), QStringList{"-L", "OUTPUT", "-n", "--line-numbers"});
        QString ipv6Rules = executeCommandWithOutput(ip6tablesPath(), QStringList{"-L", "OUTPUT", "-n", "--line-numbers"});
        return ipv4Rules.contains(comment) || ipv6Rules.contains(comment);
    });
}

QFuture<bool> LinuxFirewallService::unblockAllServersAsync() {
    return QtConcurrent::run([this]() {
        int ipv4Deleted = deleteRulesByComment(iptablesPath(), QStringList{"-L", "OUTPUT", "-n", "--line-numbers"}, RulePrefix);
        int ipv6Deleted = deleteRulesByComment(ip6tablesPath(), QStringList{"-L", "OUTPUT", "-n", "--line-numbers"}, RulePrefix);
        return (ipv4Deleted + ipv6Deleted) > 0;
    });
}

bool LinuxFirewallService::executeCommand(const QString& command, const QStringList& arguments) {
    QProcess process;
    process.start(command, arguments);
    process.waitForFinished();
    return process.exitCode() == 0;
}

QString LinuxFirewallService::executeCommandWithOutput(const QString& command, const QStringList& arguments) {
    QProcess process;
    process.start(command, arguments);
    process.waitForFinished();
    return process.readAllStandardOutput();
}

int LinuxFirewallService::deleteRulesByComment(const QString& command, const QStringList& listArgs, const QString& commentPrefix) {
    QString output = executeCommandWithOutput(command, listArgs);
    QStringList lines = output.split('\n');
    QList<int> lineNumbers;
    for (const QString& line : lines) {
        if (line.contains(commentPrefix)) {
            QStringList parts = line.trimmed().split(' ', Qt::SkipEmptyParts);
            if (!parts.isEmpty()) {
                bool ok;
                int lineNum = parts.first().toInt(&ok);
                if (ok) {
                    lineNumbers.append(lineNum);
                }
            }
        }
    }
    if (lineNumbers.isEmpty()) {
        return 0;
    }
    std::sort(lineNumbers.begin(), lineNumbers.end(), std::greater<int>());
    int deletedCount = 0;
    for (int lineNum : lineNumbers) {
        QStringList deleteArgs = {"-D", "OUTPUT", QString::number(lineNum)};
        if (executeCommand(command, deleteArgs)) {
            deletedCount++;
        }
    }
    return deletedCount;
}