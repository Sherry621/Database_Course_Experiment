#pragma once

#include <QSqlDatabase>
#include <QString>

class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool connect(const QString& host,
                 int port,
                 const QString& databaseName,
                 const QString& username,
                 const QString& password);
    QSqlDatabase database() const;
    bool isOpen() const;
    QString lastError() const;

private:
    DatabaseManager() = default;

    QSqlDatabase db_;
    QString lastError_;
};
