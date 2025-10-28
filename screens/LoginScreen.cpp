#include "LoginScreen.h"
#include "ui_LoginScreen.h"
#include <QMessageBox>
#include "../core/DatabaseManager.h"

LoginScreen::LoginScreen(const QString &username, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginScreen)
    , m_username(username)
{
    ui->setupUi(this);

    // Display the username on the screen
    ui->usernameLabel->setText(m_username);

    // Connect buttons
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginScreen::onLoginClicked);
    connect(ui->backButton, &QPushButton::clicked, this, &LoginScreen::onBackClicked);
    // Allow pressing Enter/Return in the password field to trigger login
    connect(ui->passwordEdit, &QLineEdit::returnPressed, this, &LoginScreen::onLoginClicked);
}

LoginScreen::~LoginScreen()
{
    delete ui;
}

void LoginScreen::onLoginClicked()
{
    QString password = ui->passwordEdit->text();
    bool isAdmin = false;
    QString err;
    if (DatabaseManager::instance().authenticate(m_username, password, isAdmin, &err)) {
        Q_UNUSED(isAdmin);
        emit loginSuccessful(m_username);
    } else {
        // If the authentication failure is due to incorrect password, show a concise test message
        if (err.isEmpty() || err.toLower() == "incorrect password") {
            QMessageBox::warning(this, "Login Failed", "Incorrect Password");
        } else {
            QMessageBox::warning(this, "Login Failed", err);
        }
    }
}

void LoginScreen::onBackClicked()
{
    emit backRequested();
}
