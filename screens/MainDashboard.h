#ifndef MAINDASHBOARD_H
#define MAINDASHBOARD_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainDashboard; }
QT_END_NAMESPACE

class MainDashboard : public QWidget
{
    Q_OBJECT

public:
    explicit MainDashboard(const QString &username = QString(), QWidget *parent = nullptr);
    ~MainDashboard();

    // Show or hide admin-only functionality
    void setIsAdmin(bool isAdmin);

signals:
    void logoutRequested();
    void launchVmRequested();
    void logsRequested();
    void reportsRequested();
    void createUserRequested();

private:
    Ui::MainDashboard *ui;
    QString m_username;
};

#endif // MAINDASHBOARD_H
