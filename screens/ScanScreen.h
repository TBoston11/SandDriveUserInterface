#ifndef SCANSCREEN_H
#define SCANSCREEN_H

#include <QWidget>

namespace Ui {
class ScanScreen;
}

class ScanScreen : public QWidget
{
    Q_OBJECT

public:
    explicit ScanScreen(QWidget *parent = nullptr);
    ~ScanScreen();

signals:
    void backRequested();

private:
    Ui::ScanScreen *ui;
};

#endif // SCANSCREEN_H
