#ifndef PINGSERVICE_H
#define PINGSERVICE_H

#include "serverinfo.h"
#include <QList>
#include <QFuture>

class PingService {
public:
    QFuture<QVariant> pingServerAsync(ServerInfo& server);
    QFuture<void> pingServersAsync(QList<ServerInfo>& servers);
};

#endif // PINGSERVICE_H