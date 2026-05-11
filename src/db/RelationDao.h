#pragma once

#include <QString>
#include <utility>
#include <vector>

class RelationDao {
public:
    std::vector<std::pair<int, int>> findParentChildEdges(int genealogyId) const;
    bool addParentChild(int genealogyId, int parentId, int childId, const QString& relationType) const;
    bool addMarriage(int genealogyId,
                     int person1Id,
                     int person2Id,
                     int marriageYear,
                     int divorceYear,
                     const QString& description) const;
    QString lastError() const;

private:
    mutable QString lastError_;
};
