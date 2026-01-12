#ifndef APPACTIONS_H
#define APPACTIONS_H

#include <QObject>
#include <QList>

class AppState;
class ServerService;
class PingService;
class IFirewallService;

class AppActions : public QObject {
    Q_OBJECT

public:
    AppActions(AppState* appState, ServerService* serverService, PingService* pingService, IFirewallService* firewallService, QObject* parent = nullptr);

public slots:
    void refreshServers();
    void pingAllServers();
    void blockSelectedServers(const QList<int>& selectedIndices);
    void unblockSelectedServers(const QList<int>& selectedIndices);
    void blockAllServers();
    void unblockAllServers();
    void toggleClusterMode();
    void updateServerSelection(const QList<int>& selectedIndices);

private:
    AppState* appState;
    ServerService* serverService;
    PingService* pingService;
    IFirewallService* firewallService;
};

#endif // APPACTIONS_H