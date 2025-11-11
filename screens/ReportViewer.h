#ifndef REPORTVIEWER_H
#define REPORTVIEWER_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class ReportViewer; }
QT_END_NAMESPACE

class ReportViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ReportViewer(QWidget *parent = nullptr);
    ~ReportViewer();
    
    void loadReportFile(const QString &filepath);

signals:
    void backRequested();

private:
    Ui::ReportViewer *ui;
};

#endif // REPORTVIEWER_H
