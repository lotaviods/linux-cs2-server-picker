#ifndef APPSTATE_H
#define APPSTATE_H

#include <QString>
#include <QList>
#include <QObject>
#include "../serverinfo.h"

/**
 * ServerListState: Manages server data and their properties
 */
struct ServerListState {
    QList<ServerInfo> servers;
    bool isClustered = false;
    int serverRevision = 0;

    bool isEmpty() const { return servers.isEmpty(); }
    int count() const { return servers.size(); }
    
    void clear() {
        servers.clear();
        serverRevision = 0;
    }
};

/**
 * AdminState: Manages administrator and firewall-related state
 */
struct AdminState {
    bool isAdministrator = false;
    bool daemonConnected = false;

    QString getStatusMessage() const {
        if (!isAdministrator) {
            return "Daemon not connected.";
        }
        if (daemonConnected) {
            return "Connected";
        }
        return "Disconnected";
    }
};

/**
 * OperationState: Tracks ongoing operations
 */
struct OperationState {
    bool isPinging = false;
    bool isFetching = false;
    bool isBlocking = false;
    int operationProgress = 0;
    int operationTotal = 0;

    bool isBusy() const { return isPinging || isFetching || isBlocking; }
    
    QString getProgressText() const {
        if (operationTotal == 0) return "";
        return QString("%1/%2").arg(operationProgress).arg(operationTotal);
    }
};

/**
 * UIState: Manages UI-specific state like status messages and selection
 */
struct UIState {
    QString statusMessage;
    QList<int> selectedServerIndices;
    int sortColumn = -1;
    bool sortAscending = true;

    bool hasSelection() const { return !selectedServerIndices.isEmpty(); }
    void clearSelection() { selectedServerIndices.clear(); }
};

/**
 * AppState: Complete application state
 * All state changes should be reflected through this unified structure
 */
class AppState : public QObject {
    Q_OBJECT

public:
    explicit AppState(QObject* parent = nullptr);

    // State getters
    const ServerListState& getServerListState() const { return serverListState; }
    const AdminState& getAdminState() const { return adminState; }
    const OperationState& getOperationState() const { return operationState; }
    const UIState& getUIState() const { return uiState; }

    // State mutators
    void updateServers(const QList<ServerInfo>& newServers, int revision = -1);
    void updateServerStatus(int index, ServerStatus status);
    void setAdministratorStatus(bool isAdmin, bool daemonConnected);
    void setStatusMessage(const QString& message);
    void setSelectedServers(const QList<int>& indices);
    void clearSelection();
    void setClusteredMode(bool clustered);
    void setPinging(bool isPinging);
    void setFetching(bool isFetching);
    void setBlocking(bool isBlocking);
    void setOperationProgress(int progress, int total);

signals:
    void serverListChanged();
    void serverStatusChanged(int index);
    void adminStateChanged();
    void statusMessageChanged(const QString& message);
    void selectionChanged();
    void clusteredModeChanged(bool clustered);
    void operationStateChanged();

private:
    ServerListState serverListState;
    AdminState adminState;
    OperationState operationState;
    UIState uiState;
};

#endif // APPSTATE_H
