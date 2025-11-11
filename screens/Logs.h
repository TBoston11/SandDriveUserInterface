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
    
    void refreshLogs();

signals:
    void backRequested();
    void viewLogFileRequested(const QString &filepath);

private slots:
    void onViewLogFile();
    void onAddTestLog();

private:
    Ui::Logs *ui;
};

#endif // LOGS_H
