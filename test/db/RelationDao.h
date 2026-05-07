#pragma once

#include <QString>
#include <utility>
#include <vector>

class RelationDao {
public:
    std::vector<std::pair<int, int>> findParentChildEdges(int genealogyId) const;
    bool addParentChild(int genealogyId, int parentId, int childId, const QString& relationType) const;
};
