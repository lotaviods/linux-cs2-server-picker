#include "appactions.h"
#include "../state/appstate.h"
#include "../serverservice.h"
#include "../pingservice.h"
#include "../firewall/ifirewallservice.h"
#include <QFutureWatcher>

AppActions::AppActions(AppState* appState, ServerService* serverService, PingService* pingService, IFirewallService* firewallService, QObject* parent)
    : QObject(parent), appState(appState), serverService(serverService), pingService(pingService), firewallService(firewallService) {
}

void AppActions::refreshServers() {
    appState->setFetching(true);
    appState->setStatusMessage("Fetching server data...");
    
    QFuture<QPair<bool, QVariant>> future = serverService->fetchServerDataAsync();
    QFutureWatcher<QPair<bool, QVariant>>* watcher = new QFutureWatcher<QPair<bool, QVariant>>(this);
    
    connect(watcher, &QFutureWatcher<QPair<bool, QVariant>>::finished, [this, watcher]() {
        QPair<bool, QVariant> result = watcher->result();
        if (result.first) {
            QList<ServerInfo> servers = serverService->getServers(appState->getServerListState().isClustered);
            appState->updateServers(servers);
            appState->setStatusMessage("Server data refreshed successfully.");
        } else {
            appState->setStatusMessage("Failed to refresh server data.");
        }
        appState->setFetching(false);
        watcher->deleteLater();
    });
    
    watcher->setFuture(future);
}

void AppActions::pingAllServers() {
    appState->setPinging(true);
    appState->setStatusMessage("Pinging all servers...");
    
    QList<ServerInfo> servers = appState->getServerListState().servers;
    QFuture<void> future = pingService->pingServersAsync(servers);
    QFutureWatcher<void>* watcher = new QFutureWatcher<void>(this);
    
    connect(watcher, &QFutureWatcher<void>::finished, [this, watcher]() {
        appState->setStatusMessage("Ping completed.");
        appState->setPinging(false);
        watcher->deleteLater();
    });
    
    watcher->setFuture(future);
}

void AppActions::blockSelectedServers(const QList<int>& selectedIndices) {
    if (selectedIndices.isEmpty()) return;
    
    appState->setBlocking(true);
    appState->setStatusMessage("Blocking selected servers...");
    
    QList<ServerInfo> servers = appState->getServerListState().servers;
    for (int index : selectedIndices) {
        if (index >= 0 && index < servers.size()) {
            QFuture<bool> future = serverService->blockServerAsync(servers[index]);
            QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);
            
            connect(watcher, &QFutureWatcher<bool>::finished, [this, watcher, index]() {
                if (watcher->result()) {
                    appState->setStatusMessage("Server blocked successfully.");
                } else {
                    appState->setStatusMessage("Failed to block server.");
                }
                appState->setBlocking(false);
                watcher->deleteLater();
            });
            
            watcher->setFuture(future);
        }
    }
}

void AppActions::unblockSelectedServers(const QList<int>& selectedIndices) {
    if (selectedIndices.isEmpty()) return;
    
    appState->setBlocking(true);
    appState->setStatusMessage("Unblocking selected servers...");
    
    QList<ServerInfo> servers = appState->getServerListState().servers;
    for (int index : selectedIndices) {
        if (index >= 0 && index < servers.size()) {
            QFuture<bool> future = serverService->unblockServerAsync(servers[index]);
            QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);
            
            connect(watcher, &QFutureWatcher<bool>::finished, [this, watcher, index]() {
                if (watcher->result()) {
                    appState->setStatusMessage("Server unblocked successfully.");
                } else {
                    appState->setStatusMessage("Failed to unblock server.");
                }
                appState->setBlocking(false);
                watcher->deleteLater();
            });
            
            watcher->setFuture(future);
        }
    }
}

void AppActions::blockAllServers() {
    appState->setBlocking(true);
    appState->setStatusMessage("Blocking all servers...");
    
    QFuture<bool> future = firewallService->unblockAllServersAsync();
    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);
    
    connect(watcher, &QFutureWatcher<bool>::finished, [this, watcher]() {
        if (watcher->result()) {
            appState->setStatusMessage("All servers blocked successfully.");
        } else {
            appState->setStatusMessage("Failed to block all servers.");
        }
        appState->setBlocking(false);
        watcher->deleteLater();
    });
    
    watcher->setFuture(future);
}

void AppActions::unblockAllServers() {
    appState->setBlocking(true);
    appState->setStatusMessage("Unblocking all servers...");
    
    QFuture<bool> future = firewallService->unblockAllServersAsync();
    QFutureWatcher<bool>* watcher = new QFutureWatcher<bool>(this);
    
    connect(watcher, &QFutureWatcher<bool>::finished, [this, watcher]() {
        if (watcher->result()) {
            appState->setStatusMessage("All servers unblocked successfully.");
        } else {
            appState->setStatusMessage("Failed to unblock all servers.");
        }
        appState->setBlocking(false);
        watcher->deleteLater();
    });
    
    watcher->setFuture(future);
}

void AppActions::toggleClusterMode() {
    bool isClustered = !appState->getServerListState().isClustered;
    appState->setClusteredMode(isClustered);
    appState->setStatusMessage(isClustered ? "Cluster mode enabled." : "Cluster mode disabled.");
    
    // Refresh servers to apply clustering
    refreshServers();
}

void AppActions::updateServerSelection(const QList<int>& selectedIndices) {
    appState->setSelectedServers(selectedIndices);
}