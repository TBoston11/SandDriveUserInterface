#ifndef USERSELECTSCREEN_H
#define USERSELECTSCREEN_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class UserSelectScreen; }
QT_END_NAMESPACE

class UserSelectScreen : public QWidget
{
    Q_OBJECT

public:
    explicit UserSelectScreen(QWidget *parent = nullptr);
    ~UserSelectScreen();

signals:
    void userSelected(const QString &username);

private slots:
    void onUserClicked();  // Example slot for handling user selection

private:
    Ui::UserSelectScreen *ui;
};

#endif // USERSELECTSCREEN_H
