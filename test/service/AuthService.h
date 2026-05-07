#pragma once

#include <optional>

#include <QString>

#include "db/UserDao.h"
#include "model/User.h"

class AuthService {
public:
    std::optional<User> login(const QString& username, const QString& password) const;
    bool registerUser(const QString& username,
                      const QString& password,
                      const QString& realName,
                      const QString& email) const;

private:
    QString hashPassword(const QString& password) const;

    UserDao userDao_;
};
