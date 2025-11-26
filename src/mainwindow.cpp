#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QCheckBox>
#include <QFutureWatcher>
#include <QApplication>
#include <algorithm>
#include <climits>
#include "firewall/firewallclient.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    firewallService = new FirewallClient(this);
    pingService = new PingService();
    serverService = new ServerService(firewallService, this);
    isAdministrator = firewallService->isAdministrator();
    setupUI();
    if (!isAdministrator) {
        statusBar->showMessage("Daemon not connected. Please start with: sudo ./CS2ServerPicker");
    }
    updateTable();
}

void MainWindow::setupUI() {
    setWindowTitle("CS2 Server Picker");
    QWidget* central = new QWidget;
    setCentralWidget(central);
    QVBoxLayout* layout = new QVBoxLayout(central);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    QPushButton* refreshBtn = new QPushButton("Refresh Servers");
    connect(refreshBtn, &QPushButton::clicked, this, &MainWindow::onRefreshServers);
    buttonLayout->addWidget(refreshBtn);

    QPushButton* pingBtn = new QPushButton("Ping All");
    connect(pingBtn, &QPushButton::clicked, this, &MainWindow::onPingAll);
    buttonLayout->addWidget(pingBtn);

    QPushButton* blockSelBtn = new QPushButton("Block Selected");
    connect(blockSelBtn, &QPushButton::clicked, this, &MainWindow::onBlockSelected);
    buttonLayout->addWidget(blockSelBtn);

    QPushButton* unblockSelBtn = new QPushButton("Unblock Selected");
    connect(unblockSelBtn, &QPushButton::clicked, this, &MainWindow::onUnblockSelected);
    buttonLayout->addWidget(unblockSelBtn);

    QPushButton* blockAllBtn = new QPushButton("Block All");
    connect(blockAllBtn, &QPushButton::clicked, this, &MainWindow::onBlockAll);
    buttonLayout->addWidget(blockAllBtn);

    QPushButton* unblockAllBtn = new QPushButton("Unblock All");
    connect(unblockAllBtn, &QPushButton::clicked, this, &MainWindow::onUnblockAll);
    buttonLayout->addWidget(unblockAllBtn);

    QPushButton* toggleBtn = new QPushButton("Toggle Cluster");
    connect(toggleBtn, &QPushButton::clicked, this, &MainWindow::onToggleCluster);
    buttonLayout->addWidget(toggleBtn);

    layout->addLayout(buttonLayout);

    table = new QTableWidget;
    table->setColumnCount(6);
    table->setHorizontalHeaderLabels({"Select", "Name", "Region", "Latency", "Status", "Blocked"});
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSortingEnabled(false);
    connect(table->horizontalHeader(), &QHeaderView::sectionClicked, this, &MainWindow::onHeaderClicked);
    layout->addWidget(table);

    statusBar = new QStatusBar;
    setStatusBar(statusBar);
}

void MainWindow::updateTable() {
    table->setRowCount(servers.size());
    for (int i = 0; i < servers.size(); ++i) {
        const ServerInfo& server = servers[i];
        QCheckBox* check = new QCheckBox;
        table->setCellWidget(i, 0, check);
        table->setItem(i, 1, new QTableWidgetItem(server.name));
        table->setItem(i, 2, new QTableWidgetItem(server.region));
        QString latencyStr = server.latency.isValid() ? QString::number(server.latency.toInt()) : "-";
        table->setItem(i, 3, new QTableWidgetItem(latencyStr));
        table->setItem(i, 4, new QTableWidgetItem(statusToString(server.status)));
        table->setItem(i, 5, new QTableWidgetItem(server.isBlocked ? "Yes" : "No"));
    }
}

QString MainWindow::statusToString(ServerStatus status) {
    switch (status) {
    case ServerStatus::Unknown: return "Unknown";
    case ServerStatus::Pinging: return "Pinging";
    case ServerStatus::Online: return "Online";
    case ServerStatus::Timeout: return "Timeout";
    case ServerStatus::Blocked: return "Blocked";
    }
    return "Unknown";
}

void MainWindow::onRefreshServers() {
    auto watcher = new QFutureWatcher<QPair<bool, QVariant>>(this);
    connect(watcher, &QFutureWatcher<QPair<bool, QVariant>>::finished, this, [this, watcher]() {
        auto result = watcher->result();
        if (result.first) {
            servers = serverService->getServers(isClustered);
            refreshBlocked();
            updateTable();
            statusBar->showMessage(QString("Servers refreshed. Revision: %1").arg(result.second.toInt()));
        } else {
            statusBar->showMessage("Failed to fetch server data.");
        }
        watcher->deleteLater();
    });
    watcher->setFuture(serverService->fetchServerDataAsync());
}

void MainWindow::refreshBlocked() {
    auto watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this, [this, watcher]() {
        servers = serverService->getServers(isClustered);
        updateTable();
        watcher->deleteLater();
    });
    watcher->setFuture(serverService->refreshBlockedStatusAsync(isClustered));
}

void MainWindow::onPingAll() {
    if (servers.isEmpty()) {
        statusBar->showMessage("No servers loaded. Please refresh first.");
        return;
    }
    auto refreshWatcher = new QFutureWatcher<void>(this);
    connect(refreshWatcher, &QFutureWatcher<void>::finished, this, [this, refreshWatcher]() {
        refreshWatcher->deleteLater();
        auto pingWatcher = new QFutureWatcher<void>(this);
        connect(pingWatcher, &QFutureWatcher<void>::finished, this, [this, pingWatcher]() {
            updateTable();
            statusBar->showMessage("Ping completed.");
            pingWatcher->deleteLater();
        });
        pingWatcher->setFuture(pingService->pingServersAsync(servers));
    });
    refreshWatcher->setFuture(serverService->refreshBlockedStatusAsync(isClustered));
}

void MainWindow::onBlockSelected() {
    if (!isAdministrator) {
        statusBar->showMessage("Administrator privileges required.");
        return;
    }
    QList<int> selected;
    for (int i = 0; i < table->rowCount(); ++i) {
        QCheckBox* check = qobject_cast<QCheckBox*>(table->cellWidget(i, 0));
        if (check && check->isChecked()) {
            selected.append(i);
        }
    }
    if (selected.isEmpty()) {
        statusBar->showMessage("No servers selected.");
        return;
    }
    for (int idx : selected) {
        auto watcher = new QFutureWatcher<bool>(this);
        connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]() {
            watcher->deleteLater();
        });
        watcher->setFuture(serverService->blockServerAsync(servers[idx]));
    }
    refreshBlocked();
    statusBar->showMessage("Blocking selected servers...");
}

void MainWindow::onUnblockSelected() {
    if (!isAdministrator) {
        statusBar->showMessage("Administrator privileges required.");
        return;
    }
    QList<int> selected;
    for (int i = 0; i < table->rowCount(); ++i) {
        QCheckBox* check = qobject_cast<QCheckBox*>(table->cellWidget(i, 0));
        if (check && check->isChecked()) {
            selected.append(i);
        }
    }
    if (selected.isEmpty()) {
        statusBar->showMessage("No servers selected.");
        return;
    }
    for (int idx : selected) {
        auto watcher = new QFutureWatcher<bool>(this);
        connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]() {
            watcher->deleteLater();
        });
        watcher->setFuture(serverService->unblockServerAsync(servers[idx]));
    }
    refreshBlocked();
    statusBar->showMessage("Unblocking selected servers...");
}

void MainWindow::onBlockAll() {
    if (!isAdministrator) {
        statusBar->showMessage("Administrator privileges required.");
        return;
    }
    if (servers.isEmpty()) {
        statusBar->showMessage("No servers loaded. Please refresh first.");
        return;
    }
    for (ServerInfo& server : servers) {
        if (!server.isBlocked) {
            auto watcher = new QFutureWatcher<bool>(this);
            connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]() {
                watcher->deleteLater();
            });
            watcher->setFuture(serverService->blockServerAsync(server));
        }
    }
    refreshBlocked();
    statusBar->showMessage("Blocking all servers...");
}

void MainWindow::onUnblockAll() {
    if (!isAdministrator) {
        statusBar->showMessage("Administrator privileges required.");
        return;
    }
    auto watcher = new QFutureWatcher<bool>(this);
    connect(watcher, &QFutureWatcher<bool>::finished, this, [this, watcher]() {
        if (watcher->result()) {
            statusBar->showMessage("All servers unblocked successfully.");
        } else {
            statusBar->showMessage("Failed to unblock all servers.");
        }
        updateTable();
        watcher->deleteLater();
    });
    watcher->setFuture(serverService->unblockAllServersAsync());
}

void MainWindow::onToggleCluster() {
    if (!isAdministrator) {
        statusBar->showMessage("Administrator privileges required. Please unblock all servers first.");
        return;
    }
    auto unblockWatcher = new QFutureWatcher<bool>(this);
    connect(unblockWatcher, &QFutureWatcher<bool>::finished, this, [this, unblockWatcher]() {
        if (unblockWatcher->result()) {
            isClustered = !isClustered;
            servers = serverService->getServers(isClustered);
            refreshBlocked();
            updateTable();
            statusBar->showMessage(QString("Servers %1 successfully.").arg(isClustered ? "clustered" : "unclustered"));
        } else {
            statusBar->showMessage("Failed to unblock all servers.");
        }
        unblockWatcher->deleteLater();
    });
    unblockWatcher->setFuture(serverService->unblockAllServersAsync());
}

void MainWindow::onHeaderClicked(int column) {
    if (column == 0) return;
    std::sort(servers.begin(), servers.end(), [column](const ServerInfo& a, const ServerInfo& b) {
        switch (column) {
        case 1: return a.name < b.name;
        case 2: return a.region < b.region;
        case 3: {
            int latA = a.latency.isValid() ? a.latency.toInt() : INT_MAX;
            int latB = b.latency.isValid() ? b.latency.toInt() : INT_MAX;
            return latA < latB;
        }
        case 4: return static_cast<int>(a.status) < static_cast<int>(b.status);
        case 5: return a.isBlocked < b.isBlocked;
        default: return false;
        }
    });
    updateTable();
}