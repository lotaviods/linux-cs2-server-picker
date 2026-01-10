#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QStatusBar>
#include <QObject>
#include "serverinfo.h"
#include "serverservice.h"
#include "pingservice.h"
#include "firewall/ifirewallservice.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);

private slots:
    void onRefreshServers();
    void onPingAll();
    void onBlockSelected();
    void onUnblockSelected();
    void onBlockAll();
    void onUnblockAll();
    void onToggleCluster();
    void onHeaderClicked(int column);
    void onServerStatusUpdated(int index, ServerStatus status);

private:
    QList<ServerInfo> servers;
    bool isClustered = false;
    bool isAdministrator = false;
    ServerService* serverService;
    PingService* pingService;
    IFirewallService* firewallService;
    QTableWidget* table;
    QStatusBar* statusBar;

    void setupUI();
    void updateTable();
    QString statusToString(ServerStatus status);
    void refreshBlocked();
    void updateServerRow(int index);
};

#endif // MAINWINDOW_H