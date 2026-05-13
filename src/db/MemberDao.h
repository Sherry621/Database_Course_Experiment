#pragma once

#include <optional>
#include <vector>

#include <QString>

#include "model/Member.h"

class MemberDao {
public:
    std::optional<Member> findById(int memberId) const;
    std::vector<Member> findByGenealogy(int genealogyId, const QString& keyword = {}) const;
    std::vector<Member> findRecentByGenealogy(int genealogyId, int limit = 5) const;
    std::vector<Member> findChildren(int memberId) const;
    std::vector<Member> findAncestors(int memberId) const;
    bool insert(const Member& member) const;
    bool update(const Member& member) const;
    bool remove(int memberId) const;
    QString lastError() const;

private:
    mutable QString lastError_;
};
