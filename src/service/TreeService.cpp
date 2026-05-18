#include "TreeService.h"

#include <algorithm>
#include <queue>
#include <unordered_set>

std::vector<Member> TreeService::getChildren(int memberId) const {
    return memberDao_.findChildren(memberId);
}

std::vector<Member> TreeService::getAncestors(int memberId) const {
    return memberDao_.findAncestors(memberId);
}

std::vector<int> TreeService::findRelationPath(int genealogyId, int memberA, int memberB) const {
    auto graph = buildGraph(genealogyId);
    std::queue<int> queue;
    std::unordered_map<int, int> previous;
    std::unordered_set<int> visited;

    queue.push(memberA);
    visited.insert(memberA);
    previous[memberA] = -1;

    while (!queue.empty()) {
        const int current = queue.front();
        queue.pop();

        if (current == memberB) {
            break;
        }

        for (int next : graph[current]) {
            if (visited.count(next) == 0) {
                visited.insert(next);
                previous[next] = current;
                queue.push(next);
            }
        }
    }

    if (visited.count(memberB) == 0) {
        return {};
    }

    std::vector<int> path;
    for (int current = memberB; current != -1; current = previous[current]) {
        path.push_back(current);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::unordered_map<int, std::vector<int>> TreeService::buildGraph(int genealogyId) const {
    std::unordered_map<int, std::vector<int>> graph;
    for (const auto& [parentId, childId] : relationDao_.findParentChildEdges(genealogyId)) {
        graph[parentId].push_back(childId);
        graph[childId].push_back(parentId);
    }
    return graph;
}
