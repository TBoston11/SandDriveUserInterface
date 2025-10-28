#ifndef LOGS_H
#define LOGS_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Logs; }
QT_END_NAMESPACE

class Logs : public QWidget
{
    Q_OBJECT

public:
    explicit Logs(QWidget *parent = nullptr);
    ~Logs();

signals:
    void backRequested();

private:
    Ui::Logs *ui;
};

#endif // LOGS_H
