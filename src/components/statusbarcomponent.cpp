#include "statusbarcomponent.h"
#include "../state/appstate.h"

StatusBarComponent::StatusBarComponent(AppState* appState, QWidget* parent)
    : QStatusBar(parent), appState(appState) {
    // Connect to state changes
    connect(appState, &AppState::statusMessageChanged, this, &StatusBarComponent::onStatusMessageChanged);
    connect(appState, &AppState::operationStateChanged, this, &StatusBarComponent::onOperationStateChanged);
    connect(appState, &AppState::adminStateChanged, this, &StatusBarComponent::onAdminStateChanged);

    // Set initial message
    setStatusMessage(appState->getAdminState().getStatusMessage());
}

void StatusBarComponent::setStatusMessage(const QString& message) {
    showMessage(formatStatusWithProgress(message));
}

void StatusBarComponent::setOperationProgress(int current, int total) {
    const UIState& uiState = appState->getUIState();
    QString progressStr = QString(" [%1/%2]").arg(current).arg(total);
    showMessage(uiState.statusMessage + progressStr);
}

void StatusBarComponent::onStatusMessageChanged(const QString& message) {
    setStatusMessage(message);
}

void StatusBarComponent::onOperationStateChanged() {
    const OperationState& opState = appState->getOperationState();
    const UIState& uiState = appState->getUIState();

    if (opState.operationTotal > 0) {
        QString progressMsg = QString("%1 [%2/%3]")
                             .arg(uiState.statusMessage)
                             .arg(opState.operationProgress)
                             .arg(opState.operationTotal);
        showMessage(progressMsg);
    } else {
        showMessage(uiState.statusMessage);
    }
}

void StatusBarComponent::onAdminStateChanged() {
    const AdminState& adminState = appState->getAdminState();
    if (!adminState.isAdministrator) {
        showMessage(formatAdminStatus());
    }
}

QString StatusBarComponent::formatStatusWithProgress(const QString& baseMessage) const {
    const OperationState& opState = appState->getOperationState();
    if (opState.operationTotal > 0) {
        return QString("%1 [%2/%3]")
               .arg(baseMessage)
               .arg(opState.operationProgress)
               .arg(opState.operationTotal);
    }
    return baseMessage;
}

QString StatusBarComponent::formatAdminStatus() const {
    return appState->getAdminState().getStatusMessage();
}
