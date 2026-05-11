#pragma once

#include <unordered_map>
#include <vector>

#include "db/MemberDao.h"
#include "db/RelationDao.h"
#include "model/Member.h"

class TreeService {
public:
    std::vector<Member> getChildren(int memberId) const;
    std::vector<Member> getAncestors(int memberId) const;
    std::vector<int> findRelationPath(int genealogyId, int memberA, int memberB) const;

private:
    std::unordered_map<int, std::vector<int>> buildGraph(int genealogyId) const;

    MemberDao memberDao_;
    RelationDao relationDao_;
};
