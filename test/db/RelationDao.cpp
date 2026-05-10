#include "RelationDao.h"

#include <algorithm>

#include <QSqlQuery>
#include <QString>
#include <QVariant>

#include "db/DatabaseManager.h"

std::vector<std::pair<int, int>> RelationDao::findParentChildEdges(int genealogyId) const {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "SELECT parent_id, child_id "
        "FROM parent_child_relations "
        "WHERE genealogy_id = :genealogy_id");
    query.bindValue(":genealogy_id", genealogyId);

    std::vector<std::pair<int, int>> edges;
    if (!query.exec()) {
        return edges;
    }

    while (query.next()) {
        edges.emplace_back(query.value("parent_id").toInt(), query.value("child_id").toInt());
    }
    return edges;
}

bool RelationDao::addParentChild(int genealogyId, int parentId, int childId, const QString& relationType) const {
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "INSERT INTO parent_child_relations(genealogy_id, parent_id, child_id, relation_type) "
        "VALUES(:genealogy_id, :parent_id, :child_id, :relation_type)");
    query.bindValue(":genealogy_id", genealogyId);
    query.bindValue(":parent_id", parentId);
    query.bindValue(":child_id", childId);
    query.bindValue(":relation_type", relationType);
    return query.exec();
}

bool RelationDao::addMarriage(int genealogyId,
                              int person1Id,
                              int person2Id,
                              int marriageYear,
                              int divorceYear,
                              const QString& description) const {
    if (person1Id == person2Id) {
        return false;
    }

    const auto [leftPersonId, rightPersonId] = std::minmax(person1Id, person2Id);

    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "INSERT INTO marriages(genealogy_id, person1_id, person2_id, marriage_year, divorce_year, description) "
        "VALUES(:genealogy_id, :person1_id, :person2_id, :marriage_year, :divorce_year, :description)");
    query.bindValue(":genealogy_id", genealogyId);
    query.bindValue(":person1_id", leftPersonId);
    query.bindValue(":person2_id", rightPersonId);
    query.bindValue(":marriage_year", marriageYear == 0 ? QVariant() : QVariant(marriageYear));
    query.bindValue(":divorce_year", divorceYear == 0 ? QVariant() : QVariant(divorceYear));
    query.bindValue(":description", description);
    return query.exec();
}
