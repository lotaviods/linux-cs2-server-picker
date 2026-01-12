#include <QCoreApplication>
#include <QLocalServer>
#include <QLocalSocket>
#include <QDebug>
#include <QFuture>
#include <QFile>
#include <unistd.h>
#include <sys/stat.h> // for stat
#include <sys/wait.h> // for waitpid
#ifdef __linux__
#include <sys/socket.h>
#include <sys/types.h>
#endif
#include "firewall/linuxfirewallservice.h"
#ifdef __linux__
  #include <sys/prctl.h>
  #include <signal.h>
#endif

const QString SocketPath = "/tmp/CS2ServerPickerDaemon";

int main(int argc, char *argv[]) {
    prctl(PR_SET_PDEATHSIG, SIGTERM);

    if (getppid() == 1)
        return 1;

    QCoreApplication a(argc, argv);
    LinuxFirewallService firewall;
    QLocalServer server;
    server.removeServer(SocketPath);
    if (!server.listen(SocketPath)) {
        qCritical() << "Cannot start daemon:" << server.errorString();
        return 1;
    }
    QFile socketFile(SocketPath);
    socketFile.setPermissions(QFile::ReadOwner | QFile::WriteOwner | QFile::ReadGroup | QFile::WriteGroup | QFile::ReadOther | QFile::WriteOther);
    qDebug() << "Daemon started, listening on" << SocketPath;

    QObject::connect(&server, &QLocalServer::newConnection, [&]() {
        QLocalSocket* socket = server.nextPendingConnection();
#ifdef __linux__
        pid_t peer_pid = -1;
        int fd = socket->socketDescriptor();
        struct ucred cred;
        socklen_t cred_len = sizeof(cred);
        if (fd != -1 && getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cred, &cred_len) == 0) {
            peer_pid = cred.pid;
            qDebug() << "Connection from pid:" << peer_pid;
        } else {
            qWarning() << "Could not obtain peer credentials";
        }

        static pid_t allowed_pid = -1;
        if (peer_pid != -1) {
            if (allowed_pid == -1) {
                allowed_pid = peer_pid;
                qDebug() << "Allowed client PID set to" << allowed_pid;
            } else if (peer_pid != allowed_pid) {
                qWarning() << "Rejected connection from pid" << peer_pid << "(allowed:" << allowed_pid << ")";
                socket->write("error");
                socket->flush();
                socket->disconnectFromServer();
                socket->deleteLater();
                return;
            }
        } else {
            qWarning() << "Peer PID unknown; allowing connection (non-Linux or failure).";
        }
#endif

        QObject::connect(socket, &QLocalSocket::readyRead, [socket, &firewall]() {
            QByteArray data = socket->readAll();
            QString command = QString::fromUtf8(data).trimmed();
            qDebug() << "Daemon received command:" << command;
            QStringList parts = command.split(' ');
            QString cmd = parts[0];
            QString result = "error";
            qDebug() << "Processing command:" << cmd;
            if (cmd == "isAdministrator") {
                result = firewall.isAdministrator() ? "true" : "false";
            } else if (cmd == "block") {
                if (parts.size() >= 2) {
                    QString ruleName = parts[1];
                    QStringList ips = parts.mid(2);
                    result = firewall.blockServerAsync(ruleName, ips).result() ? "ok" : "error";
                }
            } else if (cmd == "unblock") {
                if (parts.size() >= 2) {
                    QString ruleName = parts[1];
                    result = firewall.unblockServerAsync(ruleName).result() ? "ok" : "error";
                }
            } else if (cmd == "unblockAll") {
                result = firewall.unblockAllServersAsync().result() ? "ok" : "error";
            } else if (cmd == "isBlocked") {
                if (parts.size() >= 2) {
                    QString ruleName = parts[1];
                    result = firewall.isServerBlockedAsync(ruleName).result() ? "true" : "false";
                }
            } else if (cmd == "test") {
                result = "ok";
            }
            qDebug() << "Result:" << result;
            socket->write(result.toUtf8());
            socket->flush();
            socket->waitForBytesWritten(1000);
            socket->disconnectFromServer();
        });
    });

    return a.exec();
}
