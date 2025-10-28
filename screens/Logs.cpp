#include "Logs.h"
#include "ui_Logs.h"

Logs::Logs(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Logs)
{
    ui->setupUi(this);

    connect(ui->backButton, &QPushButton::clicked, this, &Logs::backRequested);
}

Logs::~Logs()
{
    delete ui;
}
