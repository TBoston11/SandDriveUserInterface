#include "VMTerminal.h"
#include "ui_VMTerminal.h"
#include "../core/VMManager.h"
#include <QScrollBar>
#include <QRegularExpression>

// InteractiveTerminal implementation
InteractiveTerminal::InteractiveTerminal(QWidget *parent)
    : QTextEdit(parent)
    , promptPosition(0)
    , historyIndex(-1)
{
    setReadOnly(false);
}

void InteractiveTerminal::keyPressEvent(QKeyEvent *event)
{
    // Get cursor position
    QTextCursor cursor = textCursor();
    int cursorPos = cursor.position();
    
    // Handle Tab key - send it to the VM for autocomplete
    if (event->key() == Qt::Key_Tab) {
        // Get current command
        cursor.setPosition(promptPosition);
        cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
        QString command = cursor.selectedText();
        
        // Send tab character to VM
        emit commandEntered(command + "\t");
        event->accept();
        return;
    }
    
    // Don't allow editing before the prompt
    if (cursorPos < promptPosition && event->key() != Qt::Key_Up && 
        event->key() != Qt::Key_Down && event->key() != Qt::Key_Return) {
        cursor.movePosition(QTextCursor::End);
        setTextCursor(cursor);
        return;
    }
    
    // Handle special keys
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        executeCommand();
        event->accept();
        return;
    }
    else if (event->key() == Qt::Key_Up) {
        navigateHistory(true);
        event->accept();
        return;
    }
    else if (event->key() == Qt::Key_Down) {
        navigateHistory(false);
        event->accept();
        return;
    }
    else if (event->key() == Qt::Key_Backspace) {
        // Don't delete past the prompt
        if (cursorPos <= promptPosition) {
            event->accept();
            return;
        }
    }
    
    // Allow normal text input
    QTextEdit::keyPressEvent(event);
}

void InteractiveTerminal::executeCommand()
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    
    // Get the command (everything after prompt position)
    cursor.setPosition(promptPosition);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    QString command = cursor.selectedText();
    
    if (!command.isEmpty()) {
        // Add to history
        commandHistory.append(command);
        historyIndex = commandHistory.size();
        
        // Emit signal to send command
        emit commandEntered(command);
    }
    
    // Move to end and add newline
    cursor.movePosition(QTextCursor::End);
    cursor.insertText("\n");
    setTextCursor(cursor);
    
    // Update prompt position
    promptPosition = cursor.position();
}

void InteractiveTerminal::navigateHistory(bool up)
{
    if (commandHistory.isEmpty()) {
        return;
    }
    
    if (up) {
        if (historyIndex > 0) {
            historyIndex--;
        }
    } else {
        if (historyIndex < commandHistory.size() - 1) {
            historyIndex++;
        } else {
            // Clear current line
            QTextCursor cursor = textCursor();
            cursor.setPosition(promptPosition);
            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            cursor.removeSelectedText();
            return;
        }
    }
    
    // Replace current command with history
    QTextCursor cursor = textCursor();
    cursor.setPosition(promptPosition);
    cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(commandHistory[historyIndex]);
}

// VMTerminal implementation
VMTerminal::VMTerminal(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::VMTerminal)
{
    ui->setupUi(this);
    
    // Replace the terminalOutput QTextEdit with our InteractiveTerminal
    terminal = new InteractiveTerminal(this);
    terminal->setStyleSheet(ui->terminalOutput->styleSheet());
    
    // Replace in layout
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(this->layout());
    if (layout) {
        int index = layout->indexOf(ui->terminalOutput);
        if (index >= 0) {
            layout->removeWidget(ui->terminalOutput);
            ui->terminalOutput->deleteLater();
            layout->insertWidget(index, terminal);
        }
    }
    
    // Connect buttons
    connect(ui->backButton, &QPushButton::clicked, this, &VMTerminal::backRequested);
    connect(ui->stopVMButton, &QPushButton::clicked, this, &VMTerminal::onStopVM);
    
    // Connect terminal command signal
    connect(terminal, &InteractiveTerminal::commandEntered, this, &VMTerminal::onCommandEntered);
    
    // Connect VMManager signals
    connect(&VMManager::instance(), &VMManager::vmOutputReady, 
            this, &VMTerminal::appendOutput);
    
    connect(&VMManager::instance(), &VMManager::vmStarted, this, [this]() {
        appendOutput("\n=== VM Started ===\n");
        appendOutput("SSH: ssh -p 2222 kali@localhost\n");
        appendOutput("Default credentials: kali/kali\n\n");
    });
    
    connect(&VMManager::instance(), &VMManager::vmStopped, this, [this]() {
        appendOutput("\n=== VM Stopped ===\n");
    });
    
    connect(&VMManager::instance(), &VMManager::vmError, this, [this](const QString &error) {
        appendOutput("\n=== ERROR ===\n");
        appendOutput(error + "\n");
    });
}

VMTerminal::~VMTerminal()
{
    delete ui;
}

void VMTerminal::appendOutput(const QString &text)
{
    // Strip ANSI escape codes more comprehensively
    QString cleanText = text;
    
    // Remove all ESC sequences: ESC followed by [ and parameters
    cleanText.remove(QRegularExpression("\x1B\\[[^m]*m")); // Color codes
    cleanText.remove(QRegularExpression("\x1B\\[[0-9;?]*[A-Za-z]")); // Cursor movement, modes
    cleanText.remove(QRegularExpression("\x1B\\][^\x07]*\x07")); // Operating system commands
    cleanText.remove(QRegularExpression("\x1B[=>]")); // Keypad modes
    cleanText.remove(QRegularExpression("\x1B\\([B0]")); // Character sets
    cleanText.remove(QRegularExpression("\x1B.")); // Any other ESC sequences
    
    // Remove specific problematic sequences
    cleanText.remove(QRegularExpression("\\[\\?2004[hl]")); // Bracketed paste mode
    cleanText.remove(QRegularExpression("\\[\\?[0-9]+[hl]")); // Private mode sequences
    cleanText.remove(QRegularExpression("\\[[0-9]+;[0-9]+H")); // Cursor position
    cleanText.remove(QRegularExpression("\\[K")); // Erase line
    cleanText.remove(QRegularExpression("\\[J")); // Erase display
    cleanText.remove(QRegularExpression("\\[6n")); // Cursor position report
    
    // Remove Bell character
    cleanText.remove(QChar('\x07'));
    
    // Remove control characters except tab, newline, carriage return
    // This includes backspace (\x08) which causes display issues
    cleanText.remove(QRegularExpression("[\x00-\x06\x0B\x0C\x0E-\x1F]"));
    
    QTextCursor cursor = terminal->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(cleanText);
    terminal->setTextCursor(cursor);
    
    // Auto-scroll to bottom
    terminal->verticalScrollBar()->setValue(terminal->verticalScrollBar()->maximum());
    
    // Update prompt position
    terminal->promptPosition = cursor.position();
}

void VMTerminal::clear()
{
    terminal->clear();
    terminal->promptPosition = 0;
}

void VMTerminal::onCommandEntered(const QString &command)
{
    QProcess *qemuProcess = VMManager::instance().getQemuProcess();
    if (qemuProcess && qemuProcess->state() == QProcess::Running) {
        // If command ends with \t (tab autocomplete), don't add newline
        if (command.endsWith("\t")) {
            qemuProcess->write(command.toUtf8());
        } else {
            qemuProcess->write((command + "\n").toUtf8());
        }
    } else {
        appendOutput("Error: VM is not running\n");
    }
}

void VMTerminal::onStopVM()
{
    VMManager::instance().stopVM();
    emit stopVMRequested();
}
