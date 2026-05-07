#pragma once

#include <vector>

#include "model/Genealogy.h"

class GenealogyDao {
public:
    std::vector<Genealogy> findAccessibleByUser(int userId) const;
    bool insert(const Genealogy& genealogy) const;
};
