#ifndef SCREENCONTROLLER_H
#define SCREENCONTROLLER_H

#include <QWidget>
#include <QStackedWidget>
#include <QLabel>
#include "screens/UserSelectScreen.h"
#include "screens/LoginScreen.h"
#include "screens/MainDashboard.h"
#include "screens/Logs.h"
#include "screens/Reports.h"
#include "screens/CreateUserScreen.h"
#include "screens/DeleteUserScreen.h"
#include "screens/LogViewer.h"
#include "screens/ReportViewer.h"
#include "screens/ScanScreen.h"
#include "screens/VMTerminal.h"
#include "core/BatteryMonitor.h"
#include "core/USBMonitor.h"

class ScreenController : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenController(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QStackedWidget *stack;
    UserSelectScreen *userSelectScreen;
    LoginScreen *loginScreen;
    QWidget *mainDashboard;
    CreateUserScreen *createUserScreen;
    DeleteUserScreen *deleteUserScreen;
    Logs *logsScreen;
    Reports *reportsScreen;
    LogViewer *logViewerScreen;
    ReportViewer *reportViewerScreen;
    ScanScreen *scanScreen;
    VMTerminal *vmTerminalScreen;
    
    // Battery indicator
    QLabel *batteryLabel;
    BatteryMonitor *batteryMonitor;
    
    // USB monitor for auto-passthrough
    USBMonitor *usbMonitor;

    void setupConnections();
    void setupMainDashboard(const QString &username);
    void showLogViewer(const QString &filepath);
    void showReportViewer(const QString &filepath);
    void updateBatteryDisplay(int percentage);
};

#endif // SCREENCONTROLLER_H
