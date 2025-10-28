#ifndef AUTHMANAGER_H
#define AUTHMANAGER_H

#include <QString>

class AuthManager {
public:
    static QString hashPassword(const QString &password);
    static bool verifyPassword(const QString &inputPassword, const QString &hashedPassword);
};

#endif // AUTHMANAGER_H
