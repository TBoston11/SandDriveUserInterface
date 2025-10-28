#include <QVBoxLayout>
#include <QStackedWidget>
#include "screens/UserSelectScreen.h"
#include "screens/LoginScreen.h"
#include "screens/MainDashboard.h"
#include "screens/CreateUserScreen.h"
#include "core/DatabaseManager.h"
#include "ScreenController.h"

ScreenController::ScreenController(QWidget *parent)
    : QWidget(parent)
{
    stack = new QStackedWidget(this);

    userSelectScreen = new UserSelectScreen(this);
    loginScreen = new LoginScreen("", this);
    mainDashboard = new MainDashboard("", this);
    logsScreen = new Logs(this);
    reportsScreen = new Reports(this);
    createUserScreen = new CreateUserScreen(this);

    stack->addWidget(userSelectScreen); // index 0
    stack->addWidget(loginScreen);      // index 1
    stack->addWidget(mainDashboard);    // index 2
    stack->addWidget(logsScreen);       // index 3
    stack->addWidget(reportsScreen);    // index 4
    stack->addWidget(createUserScreen); // index 5
    stack->setCurrentWidget(userSelectScreen);

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(stack);
    setLayout(layout);

    setupConnections();

    // Handle back button from create user screen
    connect(createUserScreen, &CreateUserScreen::backRequested, this, [this]() {
        stack->setCurrentWidget(mainDashboard);
    });

    // Handle back button from create user screen
    connect(createUserScreen, &CreateUserScreen::backRequested, this, [this]() {
        stack->setCurrentWidget(mainDashboard);
    });
}

void ScreenController::setupMainDashboard(const QString &username)
{
    // Recreate dashboard dynamically for the logged-in user
    static_cast<MainDashboard*>(mainDashboard)->deleteLater();
    mainDashboard = new MainDashboard(username, this);
    stack->insertWidget(2, mainDashboard);
    stack->setCurrentWidget(mainDashboard);

    // Logout button → back to user select
    connect(static_cast<MainDashboard*>(mainDashboard), &MainDashboard::logoutRequested, this, [this]() {
        stack->setCurrentWidget(userSelectScreen);
    });

    // If logged in user is admin, show create user button
    bool isAdmin = DatabaseManager::instance().isAdmin(username);
    static_cast<MainDashboard*>(mainDashboard)->setIsAdmin(isAdmin);

    // Handle create user navigation
    connect(static_cast<MainDashboard*>(mainDashboard), &MainDashboard::createUserRequested, this, [this]() {
        stack->setCurrentWidget(createUserScreen);
    });

    // Dashboard action buttons
    connect(static_cast<MainDashboard*>(mainDashboard), &MainDashboard::logsRequested, this, [this]() {
        // Show logs screen
        stack->setCurrentWidget(logsScreen);
    });

    connect(static_cast<MainDashboard*>(mainDashboard), &MainDashboard::reportsRequested, this, [this]() {
        // Show reports screen
        stack->setCurrentWidget(reportsScreen);
    });

    // Back navigation from Logs and Reports -> return to dashboard
    connect(logsScreen, &Logs::backRequested, this, [this]() {
        stack->setCurrentWidget(mainDashboard);
    });

    connect(reportsScreen, &Reports::backRequested, this, [this]() {
        stack->setCurrentWidget(mainDashboard);
    });
}

void ScreenController::setupConnections(){
    // When user clicks on a name → go to login screen
    connect(userSelectScreen, &UserSelectScreen::userSelected, this, [this](const QString &username) {
        // Recreate login screen dynamically with the chosen username
        loginScreen->deleteLater();
        loginScreen = new LoginScreen(username, this);
        stack->insertWidget(1, loginScreen);
        stack->setCurrentWidget(loginScreen);

        // Back button → return to user select
        connect(loginScreen, &LoginScreen::backRequested, this, [this]() {
            stack->setCurrentWidget(userSelectScreen);
        });

        // Successful login → go to main dashboard
        connect(loginScreen, &LoginScreen::loginSuccessful, this, [this, username]() {
            // Recreate dashboard dynamically for the logged-in user
            static_cast<MainDashboard*>(mainDashboard)->deleteLater();
            mainDashboard = new MainDashboard(username, this);
            stack->insertWidget(2, mainDashboard);
            stack->setCurrentWidget(mainDashboard);

            // Logout button → back to user select
            connect(static_cast<MainDashboard*>(mainDashboard), &MainDashboard::logoutRequested, this, [this]() {
                stack->setCurrentWidget(userSelectScreen);
            });

            // If logged in user is admin, show create user button
            bool isAdmin = DatabaseManager::instance().isAdmin(username);
            static_cast<MainDashboard*>(mainDashboard)->setIsAdmin(isAdmin);

            // Handle create user navigation
            connect(static_cast<MainDashboard*>(mainDashboard), &MainDashboard::createUserRequested, this, [this]() {
                stack->setCurrentWidget(createUserScreen);
            });

            // When a user is created, refresh the user list and go back to user select
            connect(createUserScreen, &CreateUserScreen::userCreated, this, [this]() {
                // refresh user select
                userSelectScreen->deleteLater();
                userSelectScreen = new UserSelectScreen(this);
                stack->insertWidget(0, userSelectScreen);
                // Reconnect the user selection signal
                connect(userSelectScreen, &UserSelectScreen::userSelected, this, [this](const QString &username) {
                    // Recreate login screen dynamically with the chosen username
                    loginScreen->deleteLater();
                    loginScreen = new LoginScreen(username, this);
                    stack->insertWidget(1, loginScreen);
                    stack->setCurrentWidget(loginScreen);

                    // Back button → return to user select
                    connect(loginScreen, &LoginScreen::backRequested, this, [this]() {
                        stack->setCurrentWidget(userSelectScreen);
                    });

                    // Successful login → go to main dashboard
                    connect(loginScreen, &LoginScreen::loginSuccessful, this, [this, username]() {
                        setupMainDashboard(username);
                    });
                });
                stack->setCurrentWidget(userSelectScreen);
            });

            // Dashboard action buttons
            connect(static_cast<MainDashboard*>(mainDashboard), &MainDashboard::logsRequested, this, [this]() {
                // Show logs screen
                stack->setCurrentWidget(logsScreen);
            });

            connect(static_cast<MainDashboard*>(mainDashboard), &MainDashboard::reportsRequested, this, [this]() {
                // Show reports screen
                stack->setCurrentWidget(reportsScreen);
            });

            // Back navigation from Logs and Reports -> return to dashboard
            connect(logsScreen, &Logs::backRequested, this, [this]() {
                stack->setCurrentWidget(mainDashboard);
            });

            connect(reportsScreen, &Reports::backRequested, this, [this]() {
                stack->setCurrentWidget(mainDashboard);
            });
        });
    });
}
