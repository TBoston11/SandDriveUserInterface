#include "Reports.h"
#include "ui_Reports.h"

Reports::Reports(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Reports)
{
    ui->setupUi(this);

    connect(ui->backButton, &QPushButton::clicked, this, &Reports::backRequested);
}

Reports::~Reports()
{
    delete ui;
}
