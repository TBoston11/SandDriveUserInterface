#include "ScanScreen.h"
#include "ui_ScanScreen.h"

ScanScreen::ScanScreen(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ScanScreen)
{
    ui->setupUi(this);
    connect(ui->backButton, &QPushButton::clicked, this, &ScanScreen::backRequested);
}

ScanScreen::~ScanScreen()
{
    delete ui;
}
