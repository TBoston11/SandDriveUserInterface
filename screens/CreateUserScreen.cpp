#include "CreateUserScreen.h"
#include "ui_CreateUserScreen.h"
#include "../core/DatabaseManager.h"
#include <QMessageBox>

CreateUserScreen::CreateUserScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CreateUserScreen)
{
    ui->setupUi(this);
    connect(ui->backButton, &QPushButton::clicked, this, &CreateUserScreen::onBackClicked);
    connect(ui->createButton, &QPushButton::clicked, this, &CreateUserScreen::onCreateClicked);
}

CreateUserScreen::~CreateUserScreen()
{
    delete ui;
}

void CreateUserScreen::onCreateClicked()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();
    QString err;
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "Create User", "Username and password must not be empty");
        return;
    }
    if (DatabaseManager::instance().userExists(username)) {
        QMessageBox::warning(this, "Create User", "User already exists");
        return;
    }
    if (!DatabaseManager::instance().addUser(username, password, false, &err)) {
        QMessageBox::warning(this, "Create User", "Failed to create user: " + err);
        return;
    }
    // Show a concise confirmation message for tests
    QMessageBox::information(this, "Create User", "New User Added.");
    // Clear the input fields
    ui->usernameEdit->clear();
    ui->passwordEdit->clear();
    emit userCreated();
}

void CreateUserScreen::onBackClicked()
{
    // Clear the input fields before going back
    ui->usernameEdit->clear();
    ui->passwordEdit->clear();
    emit backRequested();
}
