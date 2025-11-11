#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>

class LogManager : public QObject
{
    Q_OBJECT
public:
    static LogManager &instance();
    
    enum LogLevel {
        INFO,
        WARN,
        ERROR
    };
    
    bool initialize();
    void log(LogLevel level, const QString &user, const QString &message);
    QString getLogDirectory() const;
    QString getReportsDirectory() const;
    
private:
    explicit LogManager(QObject *parent = nullptr);
    QString ensureLogDirectory();
    QString ensureReportsDirectory();
    QString currentLogFilePath();
    QString levelToString(LogLevel level);
    
    QString m_logDir;
    QString m_reportsDir;
    
    // non-copyable
    LogManager(const LogManager &) = delete;
    LogManager &operator=(const LogManager &) = delete;
};

#endif // LOGMANAGER_H
