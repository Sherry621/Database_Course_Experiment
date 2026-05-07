#include "UserDao.h"

#include <QSqlQuery>
#include <QVariant>

#include "db/DatabaseManager.h"

std::optional<User> UserDao::findByUsername(const QString& username) const {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT user_id, username, real_name, email FROM users WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return User{query.value("user_id").toInt(),
                query.value("username").toString(),
                query.value("real_name").toString(),
                query.value("email").toString()};
}

std::optional<User> UserDao::findById(int userId) const {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("SELECT user_id, username, real_name, email FROM users WHERE user_id = :user_id");
    query.bindValue(":user_id", userId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return User{query.value("user_id").toInt(),
                query.value("username").toString(),
                query.value("real_name").toString(),
                query.value("email").toString()};
}

bool UserDao::insert(const QString& username,
                     const QString& passwordHash,
                     const QString& realName,
                     const QString& email) const {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "INSERT INTO users(username, password_hash, real_name, email) "
        "VALUES(:username, :password_hash, :real_name, :email)");
    query.bindValue(":username", username);
    query.bindValue(":password_hash", passwordHash);
    query.bindValue(":real_name", realName);
    query.bindValue(":email", email);
    return query.exec();
}
