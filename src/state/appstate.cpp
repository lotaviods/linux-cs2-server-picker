#include "appstate.h"

AppState::AppState(QObject* parent) : QObject(parent) {}

void AppState::updateServers(const QList<ServerInfo>& newServers, int revision) {
    serverListState.servers = newServers;
    if (revision >= 0) {
        serverListState.serverRevision = revision;
    }
    emit serverListChanged();
}

void AppState::updateServerStatus(int index, ServerStatus status) {
    if (index >= 0 && index < serverListState.servers.size()) {
        serverListState.servers[index].status = status;
        emit serverStatusChanged(index);
    }
}

void AppState::setAdministratorStatus(bool isAdmin, bool daemonConnected) {
    adminState.isAdministrator = isAdmin;
    adminState.daemonConnected = daemonConnected;
    emit adminStateChanged();
}

void AppState::setStatusMessage(const QString& message) {
    uiState.statusMessage = message;
    emit statusMessageChanged(message);
}

void AppState::setSelectedServers(const QList<int>& indices) {
    uiState.selectedServerIndices = indices;
    emit selectionChanged();
}

void AppState::clearSelection() {
    uiState.selectedServerIndices.clear();
    emit selectionChanged();
}

void AppState::setClusteredMode(bool clustered) {
    serverListState.isClustered = clustered;
    emit clusteredModeChanged(clustered);
}

void AppState::setPinging(bool isPinging) {
    operationState.isPinging = isPinging;
    emit operationStateChanged();
}

void AppState::setFetching(bool isFetching) {
    operationState.isFetching = isFetching;
    emit operationStateChanged();
}

void AppState::setBlocking(bool isBlocking) {
    operationState.isBlocking = isBlocking;
    emit operationStateChanged();
}

void AppState::setOperationProgress(int progress, int total) {
    operationState.operationProgress = progress;
    operationState.operationTotal = total;
    emit operationStateChanged();
}
