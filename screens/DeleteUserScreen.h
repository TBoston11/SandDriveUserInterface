#ifndef DELETEUSERSCREEN_H
#define DELETEUSERSCREEN_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class DeleteUserScreen; }
QT_END_NAMESPACE

class DeleteUserScreen : public QWidget
{
    Q_OBJECT

public:
    explicit DeleteUserScreen(QWidget *parent = nullptr);
    ~DeleteUserScreen();
    
    void refreshUserList();

signals:
    void backRequested();
    void userDeleted();

private slots:
    void onDeleteClicked();
    void onBackClicked();

private:
    Ui::DeleteUserScreen *ui;
};

#endif // DELETEUSERSCREEN_H
