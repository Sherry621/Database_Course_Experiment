#include <QCoreApplication>
#include <QDebug>

#include "db/DatabaseManager.h"
#include "db/GenealogyDao.h"
#include "db/MemberDao.h"
#include "service/AuthService.h"
#include "service/DashboardService.h"

namespace {
int fail(const QString& message) {
    qCritical().noquote() << "FAIL:" << message;
    return 1;
}
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);

    auto& db = DatabaseManager::instance();
    if (!db.connect("localhost", 5432, "genealogy_lab", "genealogy_user", "genealogy_pass")) {
        return fail("database connection failed: " + db.lastError());
    }

    AuthService authService;
    const auto user = authService.login("admin", "123456");
    if (!user.has_value()) {
        return fail("admin login failed");
    }

    GenealogyDao genealogyDao;
    const auto genealogies = genealogyDao.findAccessibleByUser(user->userId);
    if (genealogies.empty()) {
        return fail("no accessible genealogy found for admin");
    }

    const int genealogyId = genealogies.front().genealogyId;

    DashboardService dashboardService;
    const auto stats = dashboardService.loadStats(genealogyId);
    if (stats.totalMembers < 1) {
        return fail(QString("unexpected dashboard stats: total=%1 male=%2 female=%3 maxGeneration=%4 parentChild=%5 marriages=%6")
                        .arg(stats.totalMembers)
                        .arg(stats.maleMembers)
                        .arg(stats.femaleMembers)
                        .arg(stats.maxGeneration)
                        .arg(stats.parentChildRelations)
                        .arg(stats.marriages));
    }

    MemberDao memberDao;
    const auto members = memberDao.findByGenealogy(genealogyId);
    if (members.empty()) {
        return fail(QString("unexpected member list size: %1").arg(members.size()));
    }

    qInfo().noquote() << "PASS: login, genealogy loading, dashboard, and member list are working.";
    qInfo().noquote() << QString("user=%1 genealogy=%2 members=%3")
                             .arg(user->username)
                             .arg(genealogies.front().title)
                             .arg(members.size());
    return 0;
}
