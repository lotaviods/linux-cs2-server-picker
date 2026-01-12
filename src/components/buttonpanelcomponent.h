#ifndef BUTTONPANELCOMPONENT_H
#define BUTTONPANELCOMPONENT_H

#include <QWidget>
#include <QList>

class QPushButton;
class AppState;
#include "../actions/appactions.h"

/**
 * ButtonPanelComponent: Encapsulates all control buttons
 * Handles button creation and connects to user actions
 */
class ButtonPanelComponent : public QWidget {
    Q_OBJECT

public:
    explicit ButtonPanelComponent(AppState* appState, AppActions* actions, QWidget* parent = nullptr);

    // Enable/disable buttons based on state
    void setAdministratorMode(bool isAdmin);
    void setOperationInProgress(bool inProgress);

private slots:
    void onAdminStateChanged();
    void onOperationStateChanged();

signals:
    void refreshServersClicked();
    void pingAllClicked();
    void blockSelectedClicked();
    void unblockSelectedClicked();
    void blockAllClicked();
    void unblockAllClicked();
    void toggleClusterClicked();

private:
    AppState* appState;
    AppActions* actions;

    QPushButton* refreshBtn;
    QPushButton* pingBtn;
    QPushButton* blockSelBtn;
    QPushButton* unblockSelBtn;
    QPushButton* blockAllBtn;
    QPushButton* unblockAllBtn;
    QPushButton* toggleBtn;

    void setupButtons();
    void updateButtonStates();
};

#endif // BUTTONPANELCOMPONENT_H
