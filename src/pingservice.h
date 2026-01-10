#ifndef PINGSERVICE_H
#define PINGSERVICE_H

#include <QObject>
#include <QList>
#include <QFuture>
#include "serverinfo.h"

class PingService : public QObject {
    Q_OBJECT
public:
    explicit PingService(QObject* parent = nullptr);
    QFuture<QVariant> pingServerAsync(ServerInfo& server);
    QFuture<void> pingServersAsync(QList<ServerInfo>& servers);

signals:
    void serverStatusUpdated(int index, ServerStatus status);
};

#endif // PINGSERVICE_H