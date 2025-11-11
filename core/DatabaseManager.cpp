#include "DatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
}

bool DatabaseManager::initialize(const QString &path)
{
    QString dbPath = path;
    if (dbPath.isEmpty()) {
        // Use QStandardPaths to get a consistent data location
        QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir dir;
        if (!dir.exists(dataDir)) {
            dir.mkpath(dataDir);
        }
        dbPath = dataDir + QDir::separator() + "sanddrive_users.db";
    }

    if (QSqlDatabase::contains("sandrive_connection")) {
        QSqlDatabase::database("sandrive_connection").close();
        QSqlDatabase::removeDatabase("sandrive_connection");
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "sandrive_connection");
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        qWarning("Failed to open database: %s", qPrintable(db.lastError().text()));
        return false;
    }

    return ensureTables(nullptr);
}

bool DatabaseManager::ensureTables(QString *error)
{
    QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
    if (!db.isOpen()) {
        if (error) *error = "Database not open";
        return false;
    }

    QSqlQuery q(db);
    // users table: id, username unique, password, is_admin
    if (!q.exec("CREATE TABLE IF NOT EXISTS users (id INTEGER PRIMARY KEY AUTOINCREMENT, username TEXT UNIQUE NOT NULL, password TEXT NOT NULL, is_admin INTEGER DEFAULT 0)")) {
        if (error) *error = q.lastError().text();
        return false;
    }

    // logs table: metadata for log entries
    if (!q.exec("CREATE TABLE IF NOT EXISTS logs (id INTEGER PRIMARY KEY AUTOINCREMENT, timestamp TEXT NOT NULL, user TEXT NOT NULL, level TEXT NOT NULL, message TEXT, filepath TEXT)")) {
        if (error) *error = q.lastError().text();
        return false;
    }

    // reports table: metadata for report files
    if (!q.exec("CREATE TABLE IF NOT EXISTS reports (id INTEGER PRIMARY KEY AUTOINCREMENT, created_at TEXT NOT NULL, user TEXT NOT NULL, title TEXT NOT NULL, filepath TEXT NOT NULL, format TEXT)")) {
        if (error) *error = q.lastError().text();
        return false;
    }

    return true;
}

bool DatabaseManager::addUser(const QString &username, const QString &password, bool isAdmin, QString *error)
{
    if (username.isEmpty() || password.isEmpty()) {
        if (error) *error = "username or password empty";
        return false;
    }

    QString hashedPassword = AuthManager::hashPassword(password);
    QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
    QSqlQuery q(db);
    q.prepare("INSERT INTO users (username, password, is_admin) VALUES (:u, :p, :a)");
    q.bindValue(":u", username);
    q.bindValue(":p", hashedPassword);
    q.bindValue(":a", isAdmin ? 1 : 0);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteUser(const QString &username, QString *error)
{
    if (username.isEmpty()) {
        if (error) *error = "Username is empty";
        return false;
    }

    // Prevent deleting the last admin user
    if (isAdmin(username)) {
        QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
        QSqlQuery q(db);
        q.prepare("SELECT COUNT(1) FROM users WHERE is_admin = 1");
        if (q.exec() && q.next()) {
            int adminCount = q.value(0).toInt();
            if (adminCount <= 1) {
                if (error) *error = "Cannot delete the last admin user";
                return false;
            }
        }
    }

    QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
    QSqlQuery q(db);
    q.prepare("DELETE FROM users WHERE username = :u");
    q.bindValue(":u", username);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    
    if (q.numRowsAffected() == 0) {
        if (error) *error = "User not found";
        return false;
    }
    
    return true;
}

bool DatabaseManager::authenticate(const QString &username, const QString &password, bool &isAdminOut, QString *error)
{
    QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
    QSqlQuery q(db);
    q.prepare("SELECT password, is_admin FROM users WHERE username = :u");
    q.bindValue(":u", username);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    if (q.next()) {
        QString storedHash = q.value(0).toString();
        int is_admin = q.value(1).toInt();
        if (AuthManager::verifyPassword(password, storedHash)) {
            isAdminOut = (is_admin != 0);
            return true;
        }
    if (error) *error = "Incorrect Password";
        return false;
    }
    if (error) *error = "user not found";
    return false;
}

bool DatabaseManager::userExists(const QString &username)
{
    QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
    QSqlQuery q(db);
    q.prepare("SELECT COUNT(1) FROM users WHERE username = :u");
    q.bindValue(":u", username);
    if (!q.exec()) return false;
    if (q.next()) return q.value(0).toInt() > 0;
    return false;
}

bool DatabaseManager::isAdmin(const QString &username)
{
    QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
    QSqlQuery q(db);
    q.prepare("SELECT is_admin FROM users WHERE username = :u");
    q.bindValue(":u", username);
    if (!q.exec()) return false;
    if (q.next()) return q.value(0).toInt() != 0;
    return false;
}

QStringList DatabaseManager::listUsers()
{
    QStringList out;
    QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
    QSqlQuery q(db);
    if (!q.exec("SELECT username FROM users ORDER BY username")) return out;
    while (q.next()) {
        out << q.value(0).toString();
    }
    return out;
}

bool DatabaseManager::addLogEntry(const QString &timestamp, const QString &user, const QString &level, const QString &message, const QString &filepath, QString *error)
{
    QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
    QSqlQuery q(db);
    q.prepare("INSERT INTO logs (timestamp, user, level, message, filepath) VALUES (:ts, :u, :lvl, :msg, :fp)");
    q.bindValue(":ts", timestamp);
    q.bindValue(":u", user);
    q.bindValue(":lvl", level);
    q.bindValue(":msg", message);
    q.bindValue(":fp", filepath);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    return true;
}

QList<QVariantMap> DatabaseManager::getRecentLogs(int limit)
{
    QList<QVariantMap> logs;
    QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
    QSqlQuery q(db);
    q.prepare("SELECT id, timestamp, user, level, message, filepath FROM logs ORDER BY id DESC LIMIT :limit");
    q.bindValue(":limit", limit);
    
    if (!q.exec()) return logs;
    
    while (q.next()) {
        QVariantMap log;
        log["id"] = q.value(0).toInt();
        log["timestamp"] = q.value(1).toString();
        log["user"] = q.value(2).toString();
        log["level"] = q.value(3).toString();
        log["message"] = q.value(4).toString();
        log["filepath"] = q.value(5).toString();
        logs.append(log);
    }
    return logs;
}

bool DatabaseManager::addReport(const QString &title, const QString &user, const QString &filepath, const QString &format, QString *error)
{
    QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
    QSqlQuery q(db);
    QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODate);
    q.prepare("INSERT INTO reports (created_at, user, title, filepath, format) VALUES (:ts, :u, :t, :fp, :fmt)");
    q.bindValue(":ts", timestamp);
    q.bindValue(":u", user);
    q.bindValue(":t", title);
    q.bindValue(":fp", filepath);
    q.bindValue(":fmt", format);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    return true;
}

QList<QVariantMap> DatabaseManager::listReports()
{
    QList<QVariantMap> reports;
    QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
    QSqlQuery q(db);
    
    if (!q.exec("SELECT id, created_at, user, title, filepath, format FROM reports ORDER BY created_at DESC")) {
        return reports;
    }
    
    while (q.next()) {
        QVariantMap report;
        report["id"] = q.value(0).toInt();
        report["created_at"] = q.value(1).toString();
        report["user"] = q.value(2).toString();
        report["title"] = q.value(3).toString();
        report["filepath"] = q.value(4).toString();
        report["format"] = q.value(5).toString();
        reports.append(report);
    }
    return reports;
}

bool DatabaseManager::deleteReport(int reportId, QString *error)
{
    QSqlDatabase db = QSqlDatabase::database("sandrive_connection");
    QSqlQuery q(db);
    q.prepare("DELETE FROM reports WHERE id = :id");
    q.bindValue(":id", reportId);
    if (!q.exec()) {
        if (error) *error = q.lastError().text();
        return false;
    }
    return q.numRowsAffected() > 0;
}
