#ifndef LOGINSCREEN_H
#define LOGINSCREEN_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class LoginScreen; }
QT_END_NAMESPACE

class LoginScreen : public QWidget
{
    Q_OBJECT

public:
    explicit LoginScreen(const QString &username = QString(), QWidget *parent = nullptr);
    ~LoginScreen();

signals:
    void loginSuccessful(const QString &username);
    void backRequested();

private slots:
    void onLoginClicked();
    void onBackClicked();

private:
    Ui::LoginScreen *ui;
    QString m_username;
};

#endif // LOGINSCREEN_H
