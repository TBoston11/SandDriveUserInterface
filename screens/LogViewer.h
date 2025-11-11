#ifndef LOGVIEWER_H
#define LOGVIEWER_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class LogViewer; }
QT_END_NAMESPACE

class LogViewer : public QWidget
{
    Q_OBJECT

public:
    explicit LogViewer(QWidget *parent = nullptr);
    ~LogViewer();
    
    void loadLogFile(const QString &filepath);

signals:
    void backRequested();

private:
    Ui::LogViewer *ui;
};

#endif // LOGVIEWER_H
