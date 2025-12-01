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
    
    // Don't display newline here - let the remote output handle it
    // Update prompt position to end
    cursor.movePosition(QTextCursor::End);
    setTextCursor(cursor);
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
    connect(ui->stopVMButton, &QPushButton::clicked, this, &VMTerminal::onToggleVM);
    
    // Connect terminal command signal
    connect(terminal, &InteractiveTerminal::commandEntered, this, &VMTerminal::onCommandEntered);
    
    // Connect VMManager signals
    connect(&VMManager::instance(), &VMManager::vmOutputReady, 
            this, &VMTerminal::appendOutput);
    
    connect(&VMManager::instance(), &VMManager::vmStarted, this, [this]() {
        appendOutput("\n=== VM Started ===\n");
        appendOutput("SSH auto-login enabled (debian/debian).\n\n");
        updateButtonState();
    });
    
    connect(&VMManager::instance(), &VMManager::vmStopped, this, [this]() {
        appendOutput("\n=== VM Stopped ===\n");
        updateButtonState();
    });
    
    connect(&VMManager::instance(), &VMManager::vmError, this, [this](const QString &error) {
        appendOutput("\n=== ERROR ===\n");
        appendOutput(error + "\n");
    });

    // Set initial button state based on VM status
    updateButtonState();

    // Periodically refresh button state to stay in sync
    stateTimer.setInterval(2000);
    connect(&stateTimer, &QTimer::timeout, this, &VMTerminal::updateButtonState);
    stateTimer.start();
}

VMTerminal::~VMTerminal()
{
    delete ui;
}

void VMTerminal::appendOutput(const QString &text)
{
    // Strip ANSI escape codes safely using a robust regex
    QString cleanText = text;

    // General ANSI escape sequence pattern: ESC [ ... final byte in @-~
    QRegularExpression ansiPattern("\x1B\\[[0-9;?]*[ -/]*[@-~]");
    if (ansiPattern.isValid()) {
        cleanText.remove(ansiPattern);
    }

    // Operating System Command (OSC): ESC ] ... BEL
    QRegularExpression oscPattern("\x1B\\][^\x07]*\x07");
    if (oscPattern.isValid()) {
        cleanText.remove(oscPattern);
    }

    // Remove Bell character
    cleanText.remove(QChar('\x07'));

    // Remove other control chars except tab(\t), newline(\n), carriage return(\r)
    QRegularExpression ctrlChars("[\x00-\x06\x0B\x0C\x0E-\x1F]");
    if (ctrlChars.isValid()) {
        cleanText.remove(ctrlChars);
    }
    
    // Filter out command echo if it matches last sent command
    if (!lastCommand.isEmpty() && cleanText.contains(lastCommand)) {
        cleanText.replace(lastCommand, "");
        lastCommand.clear();  // Only filter once
    }
    
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
    QProcess *virshProcess = VMManager::instance().getVirshProcess();
    if (virshProcess && virshProcess->state() == QProcess::Running) {
        // Track command to filter echo
        lastCommand = command;
        
        // If command ends with \t (tab autocomplete), don't add newline
        if (command.endsWith("\t")) {
            virshProcess->write(command.toUtf8());
        } else {
            virshProcess->write((command + "\n").toUtf8());
        }
    } else {
        appendOutput("Error: VM console is not connected\n");
    }
}

void VMTerminal::onToggleVM()
{
    ui->stopVMButton->setEnabled(false);
    bool running = VMManager::instance().isVMRunning();
    if (running) {
        VMManager::instance().stopVM();
        emit stopVMRequested();
    } else {
        VMManager::instance().startVM();
    }
}

void VMTerminal::updateButtonState()
{
    bool running = VMManager::instance().isVMRunning();
    ui->stopVMButton->setText(running ? "Stop VM" : "Start VM");
    
    // Update button color based on state
    if (running) {
        // Red for Stop VM
        ui->stopVMButton->setStyleSheet(
            "QPushButton { background-color: #8B0000; color: white; }"
            "QPushButton:hover { background-color: #A52A2A; }"
            "QPushButton:pressed { background-color: #600000; }"
        );
    } else {
        // Green for Start VM
        ui->stopVMButton->setStyleSheet(
            "QPushButton { background-color: #006400; color: white; }"
            "QPushButton:hover { background-color: #008000; }"
            "QPushButton:pressed { background-color: #004000; }"
        );
    }
    
    ui->stopVMButton->setEnabled(true);
}
