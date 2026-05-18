#include "AuthService.h"

#include <QCryptographicHash>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "db/DatabaseManager.h"

std::optional<User> AuthService::login(const QString& username, const QString& password) const {
    lastError_.clear();
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "SELECT user_id, username, real_name, email "
        "FROM users "
        "WHERE username = :username AND password_hash = :password_hash");
    query.bindValue(":username", username);
    query.bindValue(":password_hash", hashPassword(password));

    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return std::nullopt;
    }
    if (!query.next()) {
        return std::nullopt;
    }

    return User{query.value("user_id").toInt(),
                query.value("username").toString(),
                query.value("real_name").toString(),
                query.value("email").toString()};
}

bool AuthService::registerUser(const QString& username,
                               const QString& password,
                               const QString& realName,
                               const QString& email) const {
    lastError_.clear();
    if (!userDao_.insert(username, hashPassword(password), realName, email)) {
        lastError_ = userDao_.lastError();
        return false;
    }
    return true;
}

QString AuthService::hashPassword(const QString& password) const {
    return QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex();
}

QString AuthService::lastError() const {
    return lastError_;
}
