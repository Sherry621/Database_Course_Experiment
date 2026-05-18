#include "MemberDao.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

#include "db/DatabaseManager.h"

namespace {
Member readMember(const QSqlQuery& query) {
    return Member{query.value("member_id").toInt(),
                  query.value("genealogy_id").toInt(),
                  query.value("name").toString(),
                  query.value("gender").toChar(),
                  query.value("birth_year").toInt(),
                  query.value("death_year").toInt(),
                  query.value("generation").toInt(),
                  query.value("biography").toString()};
}
}

std::optional<Member> MemberDao::findById(int memberId) const {
    lastError_.clear();
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "SELECT member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography "
        "FROM members WHERE member_id = :member_id");
    query.bindValue(":member_id", memberId);

    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return std::nullopt;
    }
    if (!query.next()) {
        return std::nullopt;
    }
    return readMember(query);
}

std::vector<Member> MemberDao::findByGenealogy(int genealogyId, const QString& keyword) const {
    lastError_.clear();
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "SELECT member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography "
        "FROM members "
        "WHERE genealogy_id = :genealogy_id "
        "  AND (:keyword = '' OR name LIKE :like_keyword) "
        "ORDER BY generation NULLS LAST, birth_year NULLS LAST, member_id "
        "LIMIT 500");
    query.bindValue(":genealogy_id", genealogyId);
    query.bindValue(":keyword", keyword);
    query.bindValue(":like_keyword", "%" + keyword + "%");

    std::vector<Member> result;
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return result;
    }

    while (query.next()) {
        result.push_back(readMember(query));
    }
    return result;
}

std::vector<Member> MemberDao::findRecentByGenealogy(int genealogyId, int limit) const {
    lastError_.clear();
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "SELECT member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography "
        "FROM members "
        "WHERE genealogy_id = :genealogy_id "
        "ORDER BY member_id DESC "
        "LIMIT :limit");
    query.bindValue(":genealogy_id", genealogyId);
    query.bindValue(":limit", limit);

    std::vector<Member> result;
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return result;
    }

    while (query.next()) {
        result.push_back(readMember(query));
    }
    return result;
}

std::vector<Member> MemberDao::findChildren(int memberId) const {
    lastError_.clear();
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "SELECT m.member_id, m.genealogy_id, m.name, m.gender, m.birth_year, m.death_year, "
        "       m.generation, m.biography "
        "FROM parent_child_relations r "
        "JOIN members m ON m.member_id = r.child_id "
        "WHERE r.parent_id = :member_id "
        "ORDER BY m.birth_year NULLS LAST, m.member_id");
    query.bindValue(":member_id", memberId);

    std::vector<Member> result;
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return result;
    }
    while (query.next()) {
        result.push_back(readMember(query));
    }
    return result;
}

std::vector<Member> MemberDao::findAncestors(int memberId) const {
    lastError_.clear();
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "WITH RECURSIVE ancestors AS ("
        "    SELECT parent_id, child_id, 1 AS depth "
        "    FROM parent_child_relations WHERE child_id = :member_id "
        "    UNION ALL "
        "    SELECT p.parent_id, p.child_id, a.depth + 1 "
        "    FROM parent_child_relations p "
        "    JOIN ancestors a ON p.child_id = a.parent_id"
        ") "
        "SELECT m.member_id, m.genealogy_id, m.name, m.gender, m.birth_year, m.death_year, "
        "       m.generation, m.biography "
        "FROM ancestors a "
        "JOIN members m ON m.member_id = a.parent_id "
        "ORDER BY a.depth, m.birth_year NULLS LAST");
    query.bindValue(":member_id", memberId);

    std::vector<Member> result;
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return result;
    }
    while (query.next()) {
        result.push_back(readMember(query));
    }
    return result;
}

bool MemberDao::insert(const Member& member) const {
    lastError_.clear();
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "INSERT INTO members(genealogy_id, name, gender, birth_year, death_year, generation, biography) "
        "VALUES(:genealogy_id, :name, :gender, :birth_year, :death_year, :generation, :biography)");
    query.bindValue(":genealogy_id", member.genealogyId);
    query.bindValue(":name", member.name);
    query.bindValue(":gender", member.gender);
    query.bindValue(":birth_year", member.birthYear == 0 ? QVariant() : QVariant(member.birthYear));
    query.bindValue(":death_year", member.deathYear == 0 ? QVariant() : QVariant(member.deathYear));
    query.bindValue(":generation", member.generation == 0 ? QVariant() : QVariant(member.generation));
    query.bindValue(":biography", member.biography);
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return false;
    }
    return true;
}

bool MemberDao::update(const Member& member) const {
    lastError_.clear();
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare(
        "UPDATE members SET name = :name, gender = :gender, birth_year = :birth_year, "
        "death_year = :death_year, generation = :generation, biography = :biography "
        "WHERE member_id = :member_id");
    query.bindValue(":member_id", member.memberId);
    query.bindValue(":name", member.name);
    query.bindValue(":gender", member.gender);
    query.bindValue(":birth_year", member.birthYear == 0 ? QVariant() : QVariant(member.birthYear));
    query.bindValue(":death_year", member.deathYear == 0 ? QVariant() : QVariant(member.deathYear));
    query.bindValue(":generation", member.generation == 0 ? QVariant() : QVariant(member.generation));
    query.bindValue(":biography", member.biography);
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return false;
    }
    if (query.numRowsAffected() == 0) {
        lastError_ = "未找到要更新的成员。";
        return false;
    }
    return true;
}

bool MemberDao::remove(int memberId) const {
    lastError_.clear();
    QSqlQuery query(DatabaseManager::instance().database());
    query.prepare("DELETE FROM members WHERE member_id = :member_id");
    query.bindValue(":member_id", memberId);
    if (!query.exec()) {
        lastError_ = query.lastError().text();
        return false;
    }
    if (query.numRowsAffected() == 0) {
        lastError_ = "未找到要删除的成员。";
        return false;
    }
    return true;
}

QString MemberDao::lastError() const {
    return lastError_;
}
