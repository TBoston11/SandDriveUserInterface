#include "UserSelectScreen.h"
#include "ui_userselectscreen.h"
#include <QListWidgetItem>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QIcon>
#include <QStyle>
#include "../core/DatabaseManager.h"

UserSelectScreen::UserSelectScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::UserSelectScreen)
{
    ui->setupUi(this);

    // Get the horizontal layout from the scroll area
    QHBoxLayout *layout = qobject_cast<QHBoxLayout*>(ui->scrollAreaWidgetContents->layout());
    
    // Populate from database
    QStringList users = DatabaseManager::instance().listUsers();
    
    for (const QString &username : users) {
        // Create a container widget for each user
        QWidget *userWidget = new QWidget(ui->scrollAreaWidgetContents);
        QVBoxLayout *userLayout = new QVBoxLayout(userWidget);
        userLayout->setSpacing(15);
        userLayout->setContentsMargins(0, 0, 0, 0);
        
        // Create user button with icon
        QPushButton *userButton = new QPushButton(userWidget);
        userButton->setObjectName("userButton");
        userButton->setCursor(Qt::PointingHandCursor);
        
        // Create a vertical layout for the button content
        QVBoxLayout *buttonLayout = new QVBoxLayout(userButton);
        buttonLayout->setSpacing(10);
        buttonLayout->setContentsMargins(20, 20, 20, 20);
        
        // Add user icon - create a circular styled label
        QLabel *iconLabel = new QLabel("👤", userButton);
        iconLabel->setStyleSheet(
            "font-size: 72pt; "
            "background-color: #505050; "
            "border-radius: 60px; "
            "min-width: 120px; "
            "max-width: 120px; "
            "min-height: 120px; "
            "max-height: 120px; "
            "color: white;"
        );
        iconLabel->setAlignment(Qt::AlignCenter);
        iconLabel->setAttribute(Qt::WA_TransparentForMouseEvents); // Let clicks pass through
        buttonLayout->addWidget(iconLabel, 0, Qt::AlignCenter);
        
        // Add some spacing
        buttonLayout->addStretch();
        
        // Add username label
        QLabel *nameLabel = new QLabel(username, userButton);
        nameLabel->setStyleSheet("font-size: 18pt; color: white; font-weight: bold;");
        nameLabel->setAlignment(Qt::AlignCenter);
        nameLabel->setWordWrap(true);
        nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents); // Let clicks pass through
        buttonLayout->addWidget(nameLabel);
        
        userLayout->addWidget(userButton);
        
        // Connect button click
        connect(userButton, &QPushButton::clicked, this, [this, username]() {
            emit userSelected(username);
        });
        
        layout->addWidget(userWidget);
    }
    
    // Add stretch at the end to push buttons to the left
    layout->addStretch();
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
