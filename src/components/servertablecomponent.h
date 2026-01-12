#ifndef SERVERTABLECOMPONENT_H
#define SERVERTABLECOMPONENT_H

#include <QTableWidget>
#include <QObject>
#include "../serverinfo.h"
#include "../state/appstate.h"

/**
 * ServerTableComponent: Encapsulates the server table display and interactions
 * Manages table rendering and selection, emits user actions
 */
class ServerTableComponent : public QTableWidget {
    Q_OBJECT

public:
    explicit ServerTableComponent(AppState* appState, QWidget* parent = nullptr);

    // Display update methods
    void updateAllRows(const QList<ServerInfo>& servers);
    void updateRow(int index, const ServerInfo& server);
    void clearTable();

    // Selection methods
    QList<int> getSelectedRows() const;
    void clearSelection();

public slots:
    void onServerStatusChanged(int index);

private slots:
    void onServerListChanged();
    void onHeaderClicked(int column);

signals:
    void selectionChanged(const QList<int>& selectedIndices);
    void sortRequested(int column);

private:
    AppState* appState;
    int sortColumn = -1;
    bool sortAscending = true;

    void setupTable();
    void populateRow(int row, const ServerInfo& server);
    QString statusToString(ServerStatus status) const;
    void sortServers(int column);
};

#endif // SERVERTABLECOMPONENT_H
