#include "RelationDao.h"

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
