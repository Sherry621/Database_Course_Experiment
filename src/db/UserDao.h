#pragma once

#include <QString>
#include <optional>

#include "model/User.h"

class UserDao {
public:
    std::optional<User> findByUsername(const QString& username) const;
    std::optional<User> findById(int userId) const;
    bool insert(const QString& username,
                const QString& passwordHash,
                const QString& realName,
                const QString& email) const;
};
