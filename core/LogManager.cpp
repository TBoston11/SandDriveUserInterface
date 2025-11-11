#include "LogManager.h"
#include "DatabaseManager.h"
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QDebug>

LogManager &LogManager::instance()
{
    static LogManager inst;
    return inst;
}

LogManager::LogManager(QObject *parent)
    : QObject(parent)
{
}

bool LogManager::initialize()
{
    m_logDir = ensureLogDirectory();
    m_reportsDir = ensureReportsDirectory();
    return !m_logDir.isEmpty() && !m_reportsDir.isEmpty();
}

QString LogManager::ensureLogDirectory()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString logDir = dataDir + QDir::separator() + "logs";
    QDir dir;
    if (!dir.exists(logDir)) {
        if (!dir.mkpath(logDir)) {
            qWarning() << "Failed to create log directory:" << logDir;
            return QString();
        }
    }
    return logDir;
}

QString LogManager::ensureReportsDirectory()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString reportsDir = dataDir + QDir::separator() + "reports";
    QDir dir;
    if (!dir.exists(reportsDir)) {
        if (!dir.mkpath(reportsDir)) {
            qWarning() << "Failed to create reports directory:" << reportsDir;
            return QString();
        }
    }
    return reportsDir;
}

QString LogManager::currentLogFilePath()
{
    if (m_logDir.isEmpty()) return QString();
    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    return m_logDir + QDir::separator() + date + ".log";
}

QString LogManager::levelToString(LogLevel level)
{
    switch (level) {
        case INFO: return "INFO";
        case WARN: return "WARN";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void LogManager::log(LogLevel level, const QString &user, const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    QString levelStr = levelToString(level);
    QString logFilePath = currentLogFilePath();
    
    if (logFilePath.isEmpty()) {
        qWarning() << "Log directory not initialized";
        return;
    }
    
    // Write to file
    QFile file(logFilePath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << timestamp << " [" << levelStr << "] " << user << ": " << message << "\n";
        file.close();
    } else {
        qWarning() << "Failed to open log file:" << logFilePath;
    }
    
    // Store metadata in database
    QString err;
    if (!DatabaseManager::instance().addLogEntry(timestamp, user, levelStr, message, logFilePath, &err)) {
        qWarning() << "Failed to add log entry to database:" << err;
    }
}

QString LogManager::getLogDirectory() const
{
    return m_logDir;
}

QString LogManager::getReportsDirectory() const
{
    return m_reportsDir;
}
