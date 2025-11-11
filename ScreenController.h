#ifndef SCREENCONTROLLER_H
#define SCREENCONTROLLER_H

#include <QWidget>
#include <QStackedWidget>
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

class ScreenController : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenController(QWidget *parent = nullptr);

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

    void setupConnections();
    void setupMainDashboard(const QString &username);
    void showLogViewer(const QString &filepath);
    void showReportViewer(const QString &filepath);
};

#endif // SCREENCONTROLLER_H
