#pragma once

#include <optional>
#include <vector>

#include "model/Genealogy.h"

class GenealogyDao {
public:
    std::vector<Genealogy> findAccessibleByUser(int userId) const;
    std::optional<Genealogy> findById(int genealogyId) const;
    bool insert(const Genealogy& genealogy) const;
    bool update(const Genealogy& genealogy) const;
    bool remove(int genealogyId, int currentUserId) const;
    bool addCollaboratorByUsername(int genealogyId, const QString& username, const QString& role) const;
    QString roleForUser(int genealogyId, int userId) const;
    QString lastError() const;

private:
    mutable QString lastError_;
};
