#include "ReportViewer.h"
#include "ui_ReportViewer.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileInfo>

ReportViewer::ReportViewer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ReportViewer)
{
    ui->setupUi(this);
    connect(ui->backButton, &QPushButton::clicked, this, &ReportViewer::backRequested);
}

ReportViewer::~ReportViewer()
{
    delete ui;
}

void ReportViewer::loadReportFile(const QString &filepath)
{
    ui->reportTextEdit->clear();
    
    QFile file(filepath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Failed to open report file: " + filepath);
        return;
    }
    
    QTextStream in(&file);
    QString content = in.readAll();
    ui->reportTextEdit->setPlainText(content);
    file.close();
    
    // Set title to show filename
    QFileInfo fileInfo(filepath);
    ui->titleLabel->setText("Report: " + fileInfo.fileName());
}
