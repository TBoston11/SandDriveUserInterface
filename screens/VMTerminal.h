#ifndef VMTERMINAL_H
#define VMTERMINAL_H

#include <QWidget>
#include <QTextEdit>
#include <QKeyEvent>

namespace Ui {
class VMTerminal;
}

class InteractiveTerminal : public QTextEdit
{
    Q_OBJECT
public:
    explicit InteractiveTerminal(QWidget *parent = nullptr);
    
    int promptPosition;
    
signals:
    void commandEntered(const QString &command);
    
protected:
    void keyPressEvent(QKeyEvent *event) override;
    
private:
    QString currentCommand;
    QStringList commandHistory;
    int historyIndex;
    
    void executeCommand();
    void navigateHistory(bool up);
};

class VMTerminal : public QWidget
{
    Q_OBJECT

public:
    explicit VMTerminal(QWidget *parent = nullptr);
    ~VMTerminal();
    
    void appendOutput(const QString &text);
    void clear();

signals:
    void backRequested();
    void stopVMRequested();

private slots:
    void onStopVM();
    void onCommandEntered(const QString &command);

private:
    Ui::VMTerminal *ui;
    InteractiveTerminal *terminal;
};

#endif // VMTERMINAL_H
