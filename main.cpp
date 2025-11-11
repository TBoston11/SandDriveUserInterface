#include "ScreenController.h"
#include <QApplication>
#include "core/DatabaseManager.h"
#include "core/LogManager.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Initialize database and ensure default admin user exists
    if (!DatabaseManager::instance().initialize()) {
        qWarning() << "Failed to initialize database";
    } else {
        // ensure an admin user exists
        if (!DatabaseManager::instance().userExists("admin")) {
            QString err;
            DatabaseManager::instance().addUser("admin", "admin", true, &err);
        }
        // add a sample user if none
        if (DatabaseManager::instance().listUsers().size() <= 1) {
            QString err;
            DatabaseManager::instance().addUser("analyst1", "testpass", false, &err);
        }
    }

    // Initialize log manager
    if (!LogManager::instance().initialize()) {
        qWarning() << "Failed to initialize log manager";
    }

    ScreenController controller;
    controller.showFullScreen();

    return app.exec();
}
