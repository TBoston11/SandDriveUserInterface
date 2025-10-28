#include "DatabaseManager.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QFile>
#include <QDir>

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
        // store in application directory
        dbPath = QDir::currentPath() + QDir::separator() + "sanddrive_users.db";
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
