#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include "AuthManager.h"

class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    static DatabaseManager &instance();
    bool initialize(const QString &path = QString()); // open/create DB

    bool addUser(const QString &username, const QString &password, bool isAdmin = false, QString *error = nullptr);
    bool authenticate(const QString &username, const QString &password, bool &isAdminOut, QString *error = nullptr);
    bool userExists(const QString &username);
    bool isAdmin(const QString &username);
    QStringList listUsers();

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    bool ensureTables(QString *error = nullptr);

    // non-copyable
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;
};

#endif // DATABASEMANAGER_H
