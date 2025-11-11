#include "LogViewer.h"
#include "ui_LogViewer.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFont>
#include <QFileInfo>

LogViewer::LogViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LogViewer)
{
    ui->setupUi(this);
    connect(ui->backButton, &QPushButton::clicked, this, &LogViewer::backRequested);
}

LogViewer::~LogViewer()
{
    delete ui;
}

void LogViewer::loadLogFile(const QString &filepath)
{
    ui->logTextEdit->clear();
    
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Failed to open log file: " + filepath);
        return;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    ui->logTextEdit->setPlainText(content);
    file.close();
    
    // Set title to show filename
    QFileInfo fileInfo(filepath);
    ui->titleLabel->setText("Log File: " + fileInfo.fileName());
}
