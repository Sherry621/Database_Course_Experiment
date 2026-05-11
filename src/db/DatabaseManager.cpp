#include "DatabaseManager.h"

#include <QSqlError>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager manager;
    return manager;
}

bool DatabaseManager::connect(const QString& host,
                              int port,
                              const QString& databaseName,
                              const QString& username,
                              const QString& password) {
    if (QSqlDatabase::contains("main")) {
        db_ = QSqlDatabase::database("main");
    } else {
        db_ = QSqlDatabase::addDatabase("QPSQL", "main");
    }

    db_.setHostName(host);
    db_.setPort(port);
    db_.setDatabaseName(databaseName);
    db_.setUserName(username);
    db_.setPassword(password);

    if (!db_.open()) {
        lastError_ = db_.lastError().text();
        return false;
    }

    lastError_.clear();
    return true;
}

QSqlDatabase DatabaseManager::database() const {
    return db_;
}

bool DatabaseManager::isOpen() const {
    return db_.isOpen();
}

QString DatabaseManager::lastError() const {
    return lastError_;
}
