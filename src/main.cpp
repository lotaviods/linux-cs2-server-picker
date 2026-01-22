#include <QApplication>
#include <QDebug>
#include <QCoreApplication>
#include <QLocalSocket>
#include <QFile>
#include <unistd.h>
#include <sys/wait.h>
#include "mainwindow.h"
#include "firewall/firewallclient.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    const QString appDir = QCoreApplication::applicationDirPath();
    const QString daemonPath = appDir + "/CS2ServerPickerDaemon";
    const QString socketPath = "/tmp/CS2ServerPickerDaemon";
    QFile::remove(socketPath);

    qDebug() << "Starting daemon via pkexec...";
    pid_t originalPid = getpid();
    pid_t pid = fork();

    if (pid == 0)
    {

        execl("/usr/bin/pkexec",
              "pkexec",
              daemonPath.toStdString().c_str(),
              "--replace",
              QString::number(originalPid).toStdString().c_str(),
              nullptr);
        _exit(1);
    }
    else if (pid < 0)
    {
        qCritical() << "fork failed";
        return 1;
    }

    FirewallClient client;
    int timeout = 60;
    bool ready = false;
    while (timeout > 0)
    {
        QString response = client.sendCommand("alive");
        if (response == "true")
        {
            ready = true;
            break;
        }
        sleep(1);
        timeout--;
    }

    if (!ready)
    {
        qCritical() << "Daemon did not become ready";
        return 1;
    }

    qDebug() << "Daemon is ready";

    MainWindow window;
    window.show();

    return app.exec();
}
