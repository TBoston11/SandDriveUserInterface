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
    Logs *logsScreen;
    Reports *reportsScreen;

    void setupConnections();
    void setupMainDashboard(const QString &username);
};

#endif // SCREENCONTROLLER_H
