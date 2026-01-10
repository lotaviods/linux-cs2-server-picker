#include "pingservice.h"
#include <QProcess>
#include <QRegularExpression>
#include <QtConcurrent>
#include <QDebug>

const int PingTimeout = 3000;

PingService::PingService(QObject* parent) : QObject(parent) {
}

QFuture<QVariant> PingService::pingServerAsync(ServerInfo& server) {
    return QtConcurrent::run([&server]() {
        if (server.isBlocked) {
            server.status = ServerStatus::Blocked;
            return QVariant();
        }
        server.status = ServerStatus::Pinging;
        server.latency = QVariant();
        for (const QString& ip : server.ipAddresses) {
            QProcess process;
            process.start("ping", QStringList() << "-c" << "1" << "-W" << QString::number(PingTimeout / 1000) << ip);
            if (!process.waitForFinished(PingTimeout)) {
                process.terminate();
                process.waitForFinished();
            }
            if (process.exitCode() == 0) {
                QString output = process.readAllStandardOutput();
                QRegularExpression regex("time=(\\d+)");
                QRegularExpressionMatch match = regex.match(output);
                if (match.hasMatch()) {
                    int time = match.captured(1).toInt();
                    server.latency = time;
                    server.status = ServerStatus::Online;
                    return server.latency;
                }
            }
        }
        server.status = ServerStatus::Timeout;
        return QVariant();
    });
}

QFuture<void> PingService::pingServersAsync(QList<ServerInfo>& servers) {
    return QtConcurrent::run([this, &servers]() {
        QList<QFuture<QVariant>> futures;
        for (int i = 0; i < servers.size(); ++i) {
            ServerInfo& server = servers[i];
            futures.append(pingServerAsync(server));
            emit serverStatusUpdated(i, server.status);
        }
        for (auto& future : futures) {
            future.waitForFinished();
        }
    });
}