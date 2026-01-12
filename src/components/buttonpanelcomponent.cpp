#include "buttonpanelcomponent.h"
#include <QPushButton>
#include <QHBoxLayout>
#include "../state/appstate.h"
#include "../actions/appactions.h"

ButtonPanelComponent::ButtonPanelComponent(AppState* appState, AppActions* actions, QWidget* parent)
    : QWidget(parent), appState(appState), actions(actions) {
    setupButtons();

    // Connect state changes to update button states
    connect(appState, &AppState::adminStateChanged, this, &ButtonPanelComponent::onAdminStateChanged);
    connect(appState, &AppState::operationStateChanged, this, &ButtonPanelComponent::onOperationStateChanged);

    // Initial state update
    updateButtonStates();
}

void ButtonPanelComponent::setupButtons() {
    QHBoxLayout* layout = new QHBoxLayout(this);

    // Refresh button
    refreshBtn = new QPushButton("Refresh Servers");
    connect(refreshBtn, &QPushButton::clicked, actions, &AppActions::refreshServers);
    connect(refreshBtn, &QPushButton::clicked, this, &ButtonPanelComponent::refreshServersClicked);
    layout->addWidget(refreshBtn);

    // Ping button
    pingBtn = new QPushButton("Ping All");
    connect(pingBtn, &QPushButton::clicked, actions, &AppActions::pingAllServers);
    connect(pingBtn, &QPushButton::clicked, this, &ButtonPanelComponent::pingAllClicked);
    layout->addWidget(pingBtn);

    // Block selected button
    blockSelBtn = new QPushButton("Block Selected");
    connect(blockSelBtn, &QPushButton::clicked, this, [this]() {
        emit blockSelectedClicked();
        actions->blockSelectedServers(appState->getUIState().selectedServerIndices);
    });
    layout->addWidget(blockSelBtn);

    // Unblock selected button
    unblockSelBtn = new QPushButton("Unblock Selected");
    connect(unblockSelBtn, &QPushButton::clicked, this, [this]() {
        emit unblockSelectedClicked();
        actions->unblockSelectedServers(appState->getUIState().selectedServerIndices);
    });
    layout->addWidget(unblockSelBtn);

    // Block all button
    blockAllBtn = new QPushButton("Block All");
    connect(blockAllBtn, &QPushButton::clicked, actions, &AppActions::blockAllServers);
    connect(blockAllBtn, &QPushButton::clicked, this, &ButtonPanelComponent::blockAllClicked);
    layout->addWidget(blockAllBtn);

    // Unblock all button
    unblockAllBtn = new QPushButton("Unblock All");
    connect(unblockAllBtn, &QPushButton::clicked, actions, &AppActions::unblockAllServers);
    connect(unblockAllBtn, &QPushButton::clicked, this, &ButtonPanelComponent::unblockAllClicked);
    layout->addWidget(unblockAllBtn);

    // Toggle cluster button
    toggleBtn = new QPushButton("Toggle Cluster");
    connect(toggleBtn, &QPushButton::clicked, actions, &AppActions::toggleClusterMode);
    connect(toggleBtn, &QPushButton::clicked, this, &ButtonPanelComponent::toggleClusterClicked);
    layout->addWidget(toggleBtn);

    setLayout(layout);
}

void ButtonPanelComponent::setAdministratorMode(bool isAdmin) {
    blockSelBtn->setEnabled(isAdmin);
    unblockSelBtn->setEnabled(isAdmin);
    blockAllBtn->setEnabled(isAdmin);
    unblockAllBtn->setEnabled(isAdmin);
    toggleBtn->setEnabled(isAdmin);
}

void ButtonPanelComponent::setOperationInProgress(bool inProgress) {
    refreshBtn->setEnabled(!inProgress);
    pingBtn->setEnabled(!inProgress);
    blockSelBtn->setEnabled(!inProgress);
    unblockSelBtn->setEnabled(!inProgress);
    blockAllBtn->setEnabled(!inProgress);
    unblockAllBtn->setEnabled(!inProgress);
    toggleBtn->setEnabled(!inProgress);
}

void ButtonPanelComponent::onAdminStateChanged() {
    updateButtonStates();
}

void ButtonPanelComponent::onOperationStateChanged() {
    updateButtonStates();
}

void ButtonPanelComponent::updateButtonStates() {
    const AdminState& adminState = appState->getAdminState();
    const OperationState& opState = appState->getOperationState();

    setAdministratorMode(adminState.isAdministrator && !opState.isBusy());
    setOperationInProgress(opState.isBusy());
}
