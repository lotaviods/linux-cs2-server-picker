#include "servertablecomponent.h"
#include <QHeaderView>
#include <QTableWidgetItem>

ServerTableComponent::ServerTableComponent(AppState* appState, QWidget* parent)
    : QTableWidget(parent), appState(appState) {
    setupTable();
    
    // Connect signals
    connect(appState, &AppState::serverListChanged, this, &ServerTableComponent::onServerListChanged);
    connect(appState, &AppState::serverStatusChanged, this, &ServerTableComponent::onServerStatusChanged);
    connect(horizontalHeader(), &QHeaderView::sectionClicked, this, &ServerTableComponent::onHeaderClicked);
}

void ServerTableComponent::setupTable() {
    setColumnCount(5);
    QStringList headers = {"Name", "Address", "Status", "Ping", "Players"};
    setHorizontalHeaderLabels(headers);
    horizontalHeader()->setSortIndicatorShown(true);
    horizontalHeader()->setSectionsClickable(true);
    
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void ServerTableComponent::updateAllRows(const QList<ServerInfo>& servers) {
    setRowCount(servers.size());
    for (int i = 0; i < servers.size(); ++i) {
        populateRow(i, servers[i]);
    }
}

void ServerTableComponent::updateRow(int index, const ServerInfo& server) {
    if (index >= 0 && index < rowCount()) {
        populateRow(index, server);
    }
}

void ServerTableComponent::clearTable() {
    setRowCount(0);
}

QList<int> ServerTableComponent::getSelectedRows() const {
    QList<int> selectedRows;
    QModelIndexList selectedIndexes = selectionModel()->selectedRows();
    for (const QModelIndex& index : selectedIndexes) {
        selectedRows.append(index.row());
    }
    return selectedRows;
}

void ServerTableComponent::clearSelection() {
    selectionModel()->clearSelection();
}

void ServerTableComponent::onServerListChanged() {
    updateAllRows(appState->getServerListState().servers);
}

void ServerTableComponent::onServerStatusChanged(int index) {
    updateRow(index, appState->getServerListState().servers[index]);
}

void ServerTableComponent::onHeaderClicked(int column) {
    if (column == sortColumn) {
        sortAscending = !sortAscending;
    } else {
        sortColumn = column;
        sortAscending = true;
    }
    sortServers(column);
    emit sortRequested(column);
}

void ServerTableComponent::populateRow(int row, const ServerInfo& server) {
    setItem(row, 0, new QTableWidgetItem(server.name));
    setItem(row, 1, new QTableWidgetItem(server.region));
    setItem(row, 2, new QTableWidgetItem(statusToString(server.status)));
    setItem(row, 3, new QTableWidgetItem(server.latency.isValid() ? QString::number(server.latency.toInt()) : "N/A"));
    setItem(row, 4, new QTableWidgetItem(server.isBlocked ? "Blocked" : "Unblocked"));
}

QString ServerTableComponent::statusToString(ServerStatus status) const {
    switch (status) {
        case ServerStatus::Online: return "Online";
        case ServerStatus::Pinging: return "Pinging";
        case ServerStatus::Timeout: return "Timeout";
        case ServerStatus::Blocked: return "Blocked";
        case ServerStatus::Unknown:
        default: return "Unknown";
    }
}

void ServerTableComponent::sortServers(int column) {
    // Implementation for sorting servers
}