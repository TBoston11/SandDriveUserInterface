#include "Logs.h"
#include "ui_Logs.h"
#include "../core/DatabaseManager.h"
#include "../core/LogManager.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>

Logs::Logs(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Logs)
{
    ui->setupUi(this);

    connect(ui->backButton, &QPushButton::clicked, this, &Logs::backRequested);
    connect(ui->viewLogButton, &QPushButton::clicked, this, &Logs::onViewLogFile);
    connect(ui->addTestLogButton, &QPushButton::clicked, this, &Logs::onAddTestLog);
    
    refreshLogs();
}

Logs::~Logs()
{
    delete ui;
}

void Logs::refreshLogs()
{
    ui->logListWidget->clear();
    
    QList<QVariantMap> logs = DatabaseManager::instance().getRecentLogs(100);
    
    for (const QVariantMap &log : logs) {
        QString timestamp = log["timestamp"].toString();
        QString user = log["user"].toString();
        QString level = log["level"].toString();
        QString message = log["message"].toString();
        
        QString displayText = QString("[%1] %2 - %3: %4")
            .arg(level)
            .arg(timestamp)
            .arg(user)
            .arg(message);
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, log["filepath"].toString());
        ui->logListWidget->addItem(item);
    }
}

void Logs::onViewLogFile()
{
    QListWidgetItem *selectedItem = ui->logListWidget->currentItem();
    if (!selectedItem) {
        QMessageBox::information(this, "View Log", "Please select a log entry first");
        return;
    }
    
    QString filepath = selectedItem->data(Qt::UserRole).toString();
    if (filepath.isEmpty() || !QFile::exists(filepath)) {
        QMessageBox::warning(this, "View Log", "Log file not found: " + filepath);
        return;
    }
    
    // Emit signal to show log viewer
    emit viewLogFileRequested(filepath);
}

void Logs::onAddTestLog()
{
    // Add a test log entry
    LogManager::instance().log(LogManager::INFO, "admin", "Test log entry created from Logs screen");
    
    QMessageBox::information(this, "Test Log", "Test log entry added successfully");
    refreshLogs();
}
