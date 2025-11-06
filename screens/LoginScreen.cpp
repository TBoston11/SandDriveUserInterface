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
        // Ensure there is a readable message
        if (err.isEmpty()) {
            err = "Incorrect Password";
        }
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Login Failed");
        msgBox.setText(err);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
    }
}

void LoginScreen::onBackClicked()
{
    emit backRequested();
}
