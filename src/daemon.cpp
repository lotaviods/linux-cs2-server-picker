#include <QCoreApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDebug>
#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <memory>
#include <optional>

#ifdef __linux__
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#endif

#include "firewall/linuxfirewallservice.h"

namespace
{

    const QString SOCKET_PATH = "/tmp/CS2ServerPickerDaemon";
    constexpr const char *ALLOWED_COMMAND_PREFIX = "CS2ServerPickerDaemon"; // used in pgrep

    // Small helper for safer system() usage
    bool safeSystem(const QString &cmd)
    {
        int ret = system(cmd.toUtf8().constData());
        return WIFEXITED(ret) && WEXITSTATUS(ret) == 0;
    }

} // namespace

class Daemon final : public QObject
{
    Q_OBJECT
public:
    explicit Daemon(pid_t guiPid = -1)
        : QObject(), guiPid_(guiPid)
    {
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [this]()
                {
    if (guiPid_ > 0 && kill(guiPid_, 0) != 0) {
        qInfo() << "GUI process is gone, exiting daemon";
        QCoreApplication::quit();
    } });
        timer->start(1000);
    }

    ~Daemon() = default;

    bool start()
    {
        // Clean up possible stale socket
        server_.removeServer(SOCKET_PATH);

        if (!server_.listen(SOCKET_PATH))
        {
            qCritical() << "Failed to listen on local socket:" << server_.errorString();
            return false;
        }

        // Make socket accessible (careful with permissions!)
        QFile socketFile(SOCKET_PATH);
        if (!socketFile.setPermissions(
                QFile::ReadOwner | QFile::WriteOwner |
                QFile::ReadGroup | QFile::WriteGroup |
                QFile::ReadOther | QFile::WriteOther))
        {
            qWarning() << "Failed to set socket permissions";
        }

        QObject::connect(&server_, &QLocalServer::newConnection, this, &Daemon::handleNewConnection);

        qInfo() << "Daemon listening on" << SOCKET_PATH
                << "(started by GUI PID:" << guiPid_ << ")";

        return true;
    }

private:
    void handleNewConnection()
    {
        QLocalSocket *client = server_.nextPendingConnection();
        if (!client)
            return;

        qInfo() << "New client connection established";

        QObject::connect(client, &QLocalSocket::readyRead,
                         this, [this, client]()
                         { handleClientMessage(client); });
    }

    void handleClientMessage(QLocalSocket *client)
    {
        QByteArray raw = client->readAll();
        if (raw.isEmpty())
            return;

        QString command = QString::fromUtf8(raw).trimmed();
        qDebug() << "Received command:" << command;

        QStringList parts = command.split(' ', Qt::SkipEmptyParts);
        if (parts.isEmpty())
        {
            sendResponse(client, "error");
            return;
        }

        QString cmd = parts[0];
        QString result = "error";

        if (cmd == "alive")
        {
            result = "true";
        }
        else if (cmd == "block" && parts.size() >= 2)
        {
            QString serverIp = parts[1];
            QStringList ports = parts.mid(2);
            result = firewall_.blockServerAsync(serverIp, ports).result() ? "ok" : "error";
        }
        else if (cmd == "unblock" && parts.size() >= 2)
        {
            result = firewall_.unblockServerAsync(parts[1]).result() ? "ok" : "error";
        }
        else if (cmd == "unblockAll")
        {
            result = firewall_.unblockAllServersAsync().result() ? "ok" : "error";
        }
        else if (cmd == "isBlocked" && parts.size() >= 2)
        {
            bool blocked = firewall_.isServerBlockedAsync(parts[1]).result();
            result = blocked ? "true" : "false";
        }
        else if (cmd == "test")
        {
            result = "ok";
        }

        sendResponse(client, result);
    }

    static void sendResponse(QLocalSocket *client, const QString &msg)
    {
        if (!client->isWritable())
            return;

        client->write(msg.toUtf8());
        client->flush();
        qInfo() << "Sending response:" << msg;
    }

private:
    QLocalServer server_;
    LinuxFirewallService firewall_;
    const pid_t guiPid_ = -1;

#ifdef __linux__
    std::optional<pid_t> allowedClientPid_;
#endif
};

#include "daemon.moc"

int main(int argc, char *argv[])
{
    // Replace previous instance if requested
    if (argc > 1 && QString(argv[1]) == "--replace")
    {
        pid_t mypid = getpid();
        QString cmd = QStringLiteral(
                          "pgrep -f %1 | grep -v %2 | xargs -r kill -TERM 2>/dev/null || true")
                          .arg(ALLOWED_COMMAND_PREFIX)
                          .arg(mypid);

        safeSystem(cmd);
    }

    pid_t guiPid = -1;
    if (argc >= 3)
    {
        bool ok = false;
        guiPid = QString(argv[2]).toInt(&ok);
        if (!ok)
            guiPid = -1;
    }

    QCoreApplication app(argc, argv);

    Daemon daemon(guiPid);

    if (!daemon.start())
        return 1;

    return app.exec();
}