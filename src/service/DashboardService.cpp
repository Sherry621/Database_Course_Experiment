#include "DashboardService.h"

#include <QSqlQuery>
#include <QVariant>

#include "db/DatabaseManager.h"

DashboardStats DashboardService::loadStats(int genealogyId) const {
    DashboardStats stats;
    QSqlQuery query(DatabaseManager::instance().database());

    query.prepare(
        "SELECT COUNT(*) AS total_members, "
        "       COUNT(*) FILTER (WHERE gender = 'M') AS male_members, "
        "       COUNT(*) FILTER (WHERE gender = 'F') AS female_members, "
        "       COALESCE(MAX(generation), 0) AS max_generation "
        "FROM members WHERE genealogy_id = :genealogy_id");
    query.bindValue(":genealogy_id", genealogyId);
    if (query.exec() && query.next()) {
        stats.totalMembers = query.value("total_members").toInt();
        stats.maleMembers = query.value("male_members").toInt();
        stats.femaleMembers = query.value("female_members").toInt();
        stats.maxGeneration = query.value("max_generation").toInt();
    }

    query.prepare("SELECT COUNT(*) FROM parent_child_relations WHERE genealogy_id = :genealogy_id");
    query.bindValue(":genealogy_id", genealogyId);
    if (query.exec() && query.next()) {
        stats.parentChildRelations = query.value(0).toInt();
    }

    query.prepare("SELECT COUNT(*) FROM marriages WHERE genealogy_id = :genealogy_id");
    query.bindValue(":genealogy_id", genealogyId);
    if (query.exec() && query.next()) {
        stats.marriages = query.value(0).toInt();
    }

    return stats;
}
