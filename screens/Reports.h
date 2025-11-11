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
    
    void refreshReports();

signals:
    void backRequested();
    void viewReportRequested(const QString &filepath);

private slots:
    void onGenerateReport();
    void onViewReport();
    void onDeleteReport();

private:
    Ui::Reports *ui;
    QString m_currentUser;
};

#endif // REPORTS_H
