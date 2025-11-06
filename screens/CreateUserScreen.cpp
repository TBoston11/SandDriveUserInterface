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
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Create User");
        msgBox.setText("Username and password must not be empty");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    if (DatabaseManager::instance().userExists(username)) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Create User");
        msgBox.setText("User already exists");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    if (!DatabaseManager::instance().addUser(username, password, false, &err)) {
        if (err.isEmpty()) err = "Failed to create user";
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Create User");
        msgBox.setText("Failed to create user: " + err);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    // Show a concise confirmation message for tests
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("Create User");
    msgBox.setText("New User Added.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
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
