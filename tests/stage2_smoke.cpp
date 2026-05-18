#include <iostream>

#include <QCoreApplication>

#include "db/DatabaseManager.h"
#include "db/GenealogyDao.h"
#include "db/MemberDao.h"
#include "service/AuthService.h"
#include "service/DashboardService.h"

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    auto& db = DatabaseManager::instance();
    if (!db.connect("localhost", 5432, "genealogy_lab", "genealogy_user", "genealogy_pass")) {
        std::cerr << "FAIL: database connection failed: "
                  << db.lastError().toStdString() << '\n';
        return 1;
    }

    AuthService authService;
    const auto user = authService.login("admin", "123456");
    if (!user.has_value()) {
        std::cerr << "FAIL: admin login failed: "
                  << authService.lastError().toStdString() << '\n';
        return 1;
    }

    GenealogyDao genealogyDao;
    const auto genealogies = genealogyDao.findAccessibleByUser(user->userId);
    if (genealogies.empty()) {
        std::cerr << "FAIL: no accessible genealogies found for admin\n";
        return 1;
    }

    const int genealogyId = genealogies.front().genealogyId;
    DashboardService dashboardService;
    const auto stats = dashboardService.loadStats(genealogyId);
    if (stats.totalMembers <= 0) {
        std::cerr << "FAIL: dashboard returned no members\n";
        return 1;
    }

    MemberDao memberDao;
    const auto members = memberDao.findByGenealogy(genealogyId);
    if (members.empty()) {
        std::cerr << "FAIL: member list is empty\n";
        return 1;
    }

    std::cout << "PASS: login, genealogy loading, dashboard, and member list are working.\n"
              << "user=" << user->username.toStdString()
              << " genealogy=" << genealogies.front().title.toStdString()
              << " members=" << members.size() << '\n';
    return 0;
}
