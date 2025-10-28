#include "UserSelectScreen.h"
#include "ui_userselectscreen.h"
#include <QListWidgetItem>
#include "../core/DatabaseManager.h"

UserSelectScreen::UserSelectScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserSelectScreen)
{
    ui->setupUi(this);

    // Populate from database
    QStringList users = DatabaseManager::instance().listUsers();
    ui->listWidget->addItems(users);

    connect(ui->listWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem *item){
        emit userSelected(item->text());
    });
}

UserSelectScreen::~UserSelectScreen()
{
    delete ui;
}

// Provide the slot implementation declared in the header to satisfy moc/linker
void UserSelectScreen::onUserClicked()
{
    // Not used currently; selection is handled via the lambda in the constructor.
}
