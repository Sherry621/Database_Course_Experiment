#pragma once

#include <QString>
#include <vector>

struct RelationPathNode {
    int memberId = 0;
    QString name;
};

using RelationPath = std::vector<RelationPathNode>;
