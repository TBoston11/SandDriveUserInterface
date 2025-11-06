#include "DeleteUserScreen.h"
#include "ui_DeleteUserScreen.h"
#include "../core/DatabaseManager.h"
#include <QMessageBox>

DeleteUserScreen::DeleteUserScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DeleteUserScreen)
{
    ui->setupUi(this);
    
    connect(ui->backButton, &QPushButton::clicked, this, &DeleteUserScreen::onBackClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &DeleteUserScreen::onDeleteClicked);
    
    refreshUserList();
}

DeleteUserScreen::~DeleteUserScreen()
{
    delete ui;
}

void DeleteUserScreen::refreshUserList()
{
    ui->userListWidget->clear();
    QStringList users = DatabaseManager::instance().listUsers();
    
    // Filter out the admin user to prevent deletion
    users.removeAll("admin");
    
    ui->userListWidget->addItems(users);
}

void DeleteUserScreen::onDeleteClicked()
{
    QListWidgetItem *selectedItem = ui->userListWidget->currentItem();
    if (!selectedItem) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Delete User");
        msgBox.setText("Please select a user to delete");
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    
    QString username = selectedItem->text();
    
    // Confirm deletion
    QMessageBox confirmBox(this);
    confirmBox.setIcon(QMessageBox::Question);
    confirmBox.setWindowTitle("Confirm Delete");
    confirmBox.setText("Are you sure you want to delete user '" + username + "'?");
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    confirmBox.setDefaultButton(QMessageBox::No);
    
    if (confirmBox.exec() != QMessageBox::Yes) {
        return;
    }
    
    QString err;
    if (!DatabaseManager::instance().deleteUser(username, &err)) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("Delete User");
        msgBox.setText("Failed to delete user: " + err);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
        return;
    }
    
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setWindowTitle("Delete User");
    msgBox.setText("User Deleted.");
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    
    refreshUserList();
    emit userDeleted();
}

void DeleteUserScreen::onBackClicked()
{
    emit backRequested();
}
