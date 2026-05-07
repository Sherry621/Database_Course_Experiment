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
