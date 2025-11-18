#include "MainDashboard.h"
#include "ui_MainDashboard.h"
#include <QMessageBox>

MainDashboard::MainDashboard(const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MainDashboard)
    , m_username(username)
{
    ui->setupUi(this);

    // Display username
    ui->welcomeLabel->setText("Welcome, " + m_username);

    // Connect logout button
    connect(ui->logoutButton, &QPushButton::clicked, this, &MainDashboard::logoutRequested);

    // Connect action buttons
    connect(ui->scansButton, &QPushButton::clicked, this, [this]() { emit scansRequested(); });
    connect(ui->logsButton, &QPushButton::clicked, this, [this]() { emit logsRequested(); });
    connect(ui->reportsButton, &QPushButton::clicked, this, [this]() { emit reportsRequested(); });

    // Create User button (hidden by default)
    if (ui->createUserButton) {
        ui->createUserButton->setVisible(false);
        connect(ui->createUserButton, &QPushButton::clicked, this, &MainDashboard::createUserRequested);
    }
    
    // Delete User button (hidden by default)
    if (ui->deleteUserButton) {
        ui->deleteUserButton->setVisible(false);
        connect(ui->deleteUserButton, &QPushButton::clicked, this, &MainDashboard::deleteUserRequested);
    }

    // Connect control buttons
    connect(ui->sleepButton, &QPushButton::clicked, this, &MainDashboard::onSleepClicked);
    connect(ui->shutdownButton, &QPushButton::clicked, this, &MainDashboard::onShutdownClicked);
}

MainDashboard::~MainDashboard()
{
    delete ui;
}

void MainDashboard::onSleepClicked()
{
    QMessageBox::information(this, "Sleep", "Sleep button clicked!\n\nThis will put the host machine to sleep.");
}

void MainDashboard::onShutdownClicked()
{
    QMessageBox::information(this, "Shutdown", "Shutdown button clicked!\n\nThis will shutdown the host machine.");
}

void MainDashboard::setIsAdmin(bool isAdmin)
{
    if (ui->createUserButton) ui->createUserButton->setVisible(isAdmin);
    if (ui->deleteUserButton) ui->deleteUserButton->setVisible(isAdmin);
}
