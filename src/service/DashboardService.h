#pragma once

struct DashboardStats {
    int totalMembers = 0;
    int maleMembers = 0;
    int femaleMembers = 0;
    int maxGeneration = 0;
    int parentChildRelations = 0;
    int marriages = 0;
};

class DashboardService {
public:
    DashboardStats loadStats(int genealogyId) const;
};
