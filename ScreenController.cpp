#include <QVBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include "screens/UserSelectScreen.h"
#include "screens/LoginScreen.h"
#include "screens/MainDashboard.h"
#include "screens/CreateUserScreen.h"
#include "screens/DeleteUserScreen.h"
#include "core/DatabaseManager.h"
#include "core/VMManager.h"
#include "core/BatteryMonitor.h"
#include "core/USBMonitor.h"
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
    deleteUserScreen = new DeleteUserScreen(this);
    logViewerScreen = new LogViewer(this);
    reportViewerScreen = new ReportViewer(this);
    scanScreen = new ScanScreen(this);
    vmTerminalScreen = new VMTerminal(this);

    stack->addWidget(userSelectScreen);     // index 0
    stack->addWidget(loginScreen);          // index 1
    stack->addWidget(mainDashboard);        // index 2
    stack->addWidget(logsScreen);           // index 3
    stack->addWidget(reportsScreen);        // index 4
    stack->addWidget(createUserScreen);     // index 5
    stack->addWidget(deleteUserScreen);     // index 6
    stack->addWidget(logViewerScreen);      // index 7
    stack->addWidget(reportViewerScreen);   // index 8
    stack->addWidget(scanScreen);           // index 9
    stack->addWidget(vmTerminalScreen);     // index 10
    stack->setCurrentWidget(userSelectScreen);

    // Create battery monitor and label
    batteryMonitor = new BatteryMonitor(this);
    batteryLabel = new QLabel(this);
    batteryLabel->setStyleSheet(R"(
        QLabel {
            color: #f5f5f7;
            background-color: rgba(0, 0, 0, 0.3);
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 14pt;
            font-weight: 500;
        }
    )");
    batteryLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    
    // Position battery label in top-left corner
    batteryLabel->setParent(this);
    batteryLabel->raise(); // Ensure it's on top
    batteryLabel->move(10, 10); // 10px from left edge
    batteryLabel->setFixedSize(130, 40);
    
    // Connect battery updates
    connect(batteryMonitor, &BatteryMonitor::batteryPercentageChanged, 
            this, &ScreenController::updateBatteryDisplay);
    
    // Set initial battery display
    updateBatteryDisplay(batteryMonitor->getBatteryPercentage());

    auto layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(stack);
    setLayout(layout);

    setupConnections();

    // Handle back button from create user screen
    connect(createUserScreen, &CreateUserScreen::backRequested, this, [this]() {
        stack->setCurrentWidget(mainDashboard);
    });
    
    // Handle back button from delete user screen
    connect(deleteUserScreen, &DeleteUserScreen::backRequested, this, [this]() {
        stack->setCurrentWidget(mainDashboard);
    });
    
    // Handle back button from log viewer
    connect(logViewerScreen, &LogViewer::backRequested, this, [this]() {
        stack->setCurrentWidget(logsScreen);
    });
    
    // Handle back button from report viewer
    connect(reportViewerScreen, &ReportViewer::backRequested, this, [this]() {
        stack->setCurrentWidget(reportsScreen);
    });
    
    // Handle back button from scan screen
    connect(scanScreen, &ScanScreen::backRequested, this, [this]() {
        stack->setCurrentWidget(mainDashboard);
    });
    
    // Handle Open Terminal button from scan screen
    connect(scanScreen, &ScanScreen::openTerminalRequested, this, [this]() {
        // Start VM and show terminal screen
        VMManager::instance().startVM();
        stack->setCurrentWidget(vmTerminalScreen);
    });
    
    // Handle back button from VM terminal
    connect(vmTerminalScreen, &VMTerminal::backRequested, this, [this]() {
        stack->setCurrentWidget(scanScreen);
    });
    
    // Setup USB monitoring for auto-passthrough to VM
    usbMonitor = new USBMonitor(this);
    connect(usbMonitor, &USBMonitor::usbDeviceInserted, this, [](const USBDevice &device) {
        qDebug() << "USB inserted:" << device.description;
        // Auto-attach to VM if running
        if (VMManager::instance().isVMRunning()) {
            VMManager::instance().attachUSBDevice(device.vendorId, device.productId);
        }
    });
    
    connect(usbMonitor, &USBMonitor::usbDeviceRemoved, this, [](const USBDevice &device) {
        qDebug() << "USB removed:" << device.identifier();
        // Auto-detach from VM
        if (VMManager::instance().isVMRunning()) {
            VMManager::instance().detachUSBDevice(device.vendorId, device.productId);
        }
    });
    
    // Start monitoring USB devices
    usbMonitor->startMonitoring();
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
    
    // Handle delete user navigation
    connect(static_cast<MainDashboard*>(mainDashboard), &MainDashboard::deleteUserRequested, this, [this]() {
        deleteUserScreen->refreshUserList();
        stack->setCurrentWidget(deleteUserScreen);
    });

    // Dashboard action buttons
    connect(static_cast<MainDashboard*>(mainDashboard), &MainDashboard::scansRequested, this, [this]() {
        // Show scans screen
        stack->setCurrentWidget(scanScreen);
    });

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
    
    // View log file navigation
    connect(logsScreen, &Logs::viewLogFileRequested, this, &ScreenController::showLogViewer);
    
    // View report file navigation
    connect(reportsScreen, &Reports::viewReportRequested, this, &ScreenController::showReportViewer);
    
    // When a user is deleted, refresh user select screen if needed
    connect(deleteUserScreen, &DeleteUserScreen::userDeleted, this, [this]() {
        // Refresh the user select screen in case we return there
        userSelectScreen->deleteLater();
        userSelectScreen = new UserSelectScreen(this);
        stack->insertWidget(0, userSelectScreen);
        // Will need to reconnect signals when user select screen is shown again
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
            setupMainDashboard(username);
        });
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
}

void ScreenController::showLogViewer(const QString &filepath)
{
    logViewerScreen->loadLogFile(filepath);
    stack->setCurrentWidget(logViewerScreen);
}

void ScreenController::showReportViewer(const QString &filepath)
{
    reportViewerScreen->loadReportFile(filepath);
    stack->setCurrentWidget(reportViewerScreen);
}

void ScreenController::updateBatteryDisplay(int percentage)
{
    QString batteryText = QString("🔋 %1%").arg(percentage);
    batteryLabel->setText(batteryText);
    
    // Update color based on percentage
    QString color;
    if (percentage >= 75) {
        color = "#4ade80"; // Green
    } else if (percentage >= 50) {
        color = "#facc15"; // Yellow
    } else if (percentage >= 25) {
        color = "#fb923c"; // Orange
    } else {
        color = "#f87171"; // Red
    }
    
    batteryLabel->setStyleSheet(QString(R"(
        QLabel {
            color: %1;
            background-color: rgba(0, 0, 0, 0.3);
            border-radius: 8px;
            padding: 8px 16px;
            font-size: 14pt;
            font-weight: 500;
        }
    )").arg(color));
}

void ScreenController::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    
    // Reposition battery label in top-left corner
    if (batteryLabel) {
        batteryLabel->move(10, 10);
    }
}
