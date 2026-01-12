#ifndef STATUSBARCOMPONENT_H
#define STATUSBARCOMPONENT_H

#include <QStatusBar>
#include <QObject>

class AppState;

/**
 * StatusBarComponent: Encapsulates the status bar display
 * Shows status messages, operation progress, and application state
 */
class StatusBarComponent : public QStatusBar {
    Q_OBJECT

public:
    explicit StatusBarComponent(AppState* appState, QWidget* parent = nullptr);

    // Update methods
    void setStatusMessage(const QString& message);
    void setOperationProgress(int current, int total);

private slots:
    void onStatusMessageChanged(const QString& message);
    void onOperationStateChanged();
    void onAdminStateChanged();

private:
    AppState* appState;

    QString formatStatusWithProgress(const QString& baseMessage) const;
    QString formatAdminStatus() const;
};

#endif // STATUSBARCOMPONENT_H
