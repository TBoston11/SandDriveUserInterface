#ifndef REPORTS_H
#define REPORTS_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Reports; }
QT_END_NAMESPACE

class Reports : public QWidget
{
    Q_OBJECT

public:
    explicit Reports(QWidget *parent = nullptr);
    ~Reports();

signals:
    void backRequested();

private:
    Ui::Reports *ui;
};

#endif // REPORTS_H
