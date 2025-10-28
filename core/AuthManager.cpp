#include "AuthManager.h"
#include <QCryptographicHash>
#include <QString>

QString AuthManager::hashPassword(const QString &password)
{
    // Use SHA-256 for password hashing
    QByteArray passwordBytes = password.toUtf8();
    QByteArray hashedPassword = QCryptographicHash::hash(passwordBytes, QCryptographicHash::Sha256);
    return QString(hashedPassword.toHex());
}

bool AuthManager::verifyPassword(const QString &inputPassword, const QString &hashedPassword)
{
    QString hashedInput = hashPassword(inputPassword);
    return hashedInput == hashedPassword;
}
