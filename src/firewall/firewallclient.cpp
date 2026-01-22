#include "firewallclient.h"
#include <QLocalSocket>
#include <QDir>
#include <QDebug>
#include <QtConcurrent>

const QString SocketPath = "/tmp/CS2ServerPickerDaemon";

FirewallClient::FirewallClient(QObject *parent) : QObject(parent) {}

QFuture<bool> FirewallClient::blockServerAsync(const QString &ruleName, const QStringList &ipAddresses)
{
    QString command = "block " + ruleName;
    for (const QString &ip : ipAddresses)
    {
        command += " " + ip;
    }
    QString response = sendCommand(command);
    return QtConcurrent::run([response]()
                             { return response == "ok"; });
}

QFuture<bool> FirewallClient::unblockServerAsync(const QString &ruleName)
{
    QString command = "unblock " + ruleName;
    QString response = sendCommand(command);
    return QtConcurrent::run([response]()
                             { return response == "ok"; });
}

QFuture<bool> FirewallClient::isServerBlockedAsync(const QString &ruleName)
{
    QString command = "isBlocked " + ruleName;
    QString response = sendCommand(command);
    return QtConcurrent::run([response]()
                             { return response == "true"; });
}

QFuture<bool> FirewallClient::unblockAllServersAsync()
{
    QString response = sendCommand("unblockAll");
    return QtConcurrent::run([response]()
                             { return response == "ok"; });
}

QString FirewallClient::sendCommand(const QString &command)
{
    QLocalSocket socket;
    socket.connectToServer(SocketPath);
    if (!socket.waitForConnected(1000)) {
        qWarning() << "Cannot connect to daemon";
        return "error";
    }
    qInfo() << "Successfully connected to daemon for command:" << command;
    socket.write(command.toUtf8());
    socket.waitForBytesWritten(30 * 1000);
    if (!socket.waitForReadyRead(30 * 1000))
    {
        qWarning() << "Timeout waiting for daemon response";
        return "error";
    }
    QByteArray response = socket.readAll();
    socket.disconnectFromServer();
    return QString::fromUtf8(response).trimmed();
}