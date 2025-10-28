#ifndef CREATEUSERSCREEN_H
#define CREATEUSERSCREEN_H

#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class CreateUserScreen; }
QT_END_NAMESPACE

class CreateUserScreen : public QWidget
{
    Q_OBJECT
public:
    explicit CreateUserScreen(QWidget *parent = nullptr);
    ~CreateUserScreen();

signals:
    void backRequested();
    void userCreated();

private slots:
    void onCreateClicked();
    void onBackClicked();

private:
    Ui::CreateUserScreen *ui;
};

#endif // CREATEUSERSCREEN_H
