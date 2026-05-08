#include "GenealogyDao.h"

#include <QSqlQuery>
#include <QVariant>

#include "db/DatabaseManager.h"

std::vector<Genealogy> GenealogyDao::findAccessibleByUser(int userId) const {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "SELECT DISTINCT g.genealogy_id, g.title, g.family_surname, g.revision_time, "
        "       g.creator_user_id, g.description "
        "FROM genealogies g "
        "LEFT JOIN genealogy_collaborators c ON c.genealogy_id = g.genealogy_id "
        "WHERE g.creator_user_id = :user_id OR c.user_id = :user_id "
        "ORDER BY g.genealogy_id");
    query.bindValue(":user_id", userId);

    std::vector<Genealogy> result;
    if (!query.exec()) {
        return result;
    }

    while (query.next()) {
        result.push_back(Genealogy{query.value("genealogy_id").toInt(),
                                   query.value("title").toString(),
                                   query.value("family_surname").toString(),
                                   query.value("revision_time").toDate(),
                                   query.value("creator_user_id").toInt(),
                                   query.value("description").toString()});
    }
    return result;
}

std::optional<Genealogy> GenealogyDao::findById(int genealogyId) const {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "SELECT genealogy_id, title, family_surname, revision_time, creator_user_id, description "
        "FROM genealogies WHERE genealogy_id = :genealogy_id");
    query.bindValue(":genealogy_id", genealogyId);

    if (!query.exec() || !query.next()) {
        return std::nullopt;
    }

    return Genealogy{query.value("genealogy_id").toInt(),
                     query.value("title").toString(),
                     query.value("family_surname").toString(),
                     query.value("revision_time").toDate(),
                     query.value("creator_user_id").toInt(),
                     query.value("description").toString()};
}

bool GenealogyDao::insert(const Genealogy& genealogy) const {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "INSERT INTO genealogies(title, family_surname, revision_time, creator_user_id, description) "
        "VALUES(:title, :family_surname, :revision_time, :creator_user_id, :description)");
    query.bindValue(":title", genealogy.title);
    query.bindValue(":family_surname", genealogy.familySurname);
    query.bindValue(":revision_time", genealogy.revisionTime);
    query.bindValue(":creator_user_id", genealogy.creatorUserId);
    query.bindValue(":description", genealogy.description);
    return query.exec();
}

bool GenealogyDao::update(const Genealogy& genealogy) const {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "UPDATE genealogies "
        "SET title = :title, family_surname = :family_surname, revision_time = :revision_time, "
        "    description = :description "
        "WHERE genealogy_id = :genealogy_id");
    query.bindValue(":genealogy_id", genealogy.genealogyId);
    query.bindValue(":title", genealogy.title);
    query.bindValue(":family_surname", genealogy.familySurname);
    query.bindValue(":revision_time", genealogy.revisionTime);
    query.bindValue(":description", genealogy.description);
    return query.exec();
}

bool GenealogyDao::remove(int genealogyId, int currentUserId) const {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "DELETE FROM genealogies "
        "WHERE genealogy_id = :genealogy_id AND creator_user_id = :creator_user_id");
    query.bindValue(":genealogy_id", genealogyId);
    query.bindValue(":creator_user_id", currentUserId);
    return query.exec();
}

bool GenealogyDao::addCollaboratorByUsername(int genealogyId, const QString& username, const QString& role) const {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "INSERT INTO genealogy_collaborators(genealogy_id, user_id, role) "
        "SELECT :genealogy_id, user_id, :role FROM users WHERE username = :username "
        "ON CONFLICT (genealogy_id, user_id) DO UPDATE SET role = EXCLUDED.role");
    query.bindValue(":genealogy_id", genealogyId);
    query.bindValue(":username", username);
    query.bindValue(":role", role);
    return query.exec();
}
