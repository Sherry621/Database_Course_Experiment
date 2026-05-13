#pragma once

#include <QMainWindow>

#include "db/GenealogyDao.h"
#include "db/MemberDao.h"
#include "db/RelationDao.h"
#include "model/User.h"
#include "service/DashboardService.h"
#include "service/TreeService.h"

class QComboBox;
class QGraphicsScene;
class QGraphicsView;
class QLabel;
class QLineEdit;
class QListWidget;
class QStackedWidget;
class QTableWidget;
class QTreeWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(const User& user, QWidget* parent = nullptr);

private slots:
    void switchPage(int row);
    void reloadGenealogies();
    void reloadDashboard();
    void addGenealogy();
    void editGenealogy();
    void deleteGenealogy();
    void inviteCollaborator();
    void reloadMembers();
    void addMember();
    void editMember();
    void deleteMember();
    void showMemberDetail();
    void addParentChildRelation();
    void addMarriageRelation();
    void buildTreePreview();
    void queryAncestors();
    void queryRelationPath();

private:
    void buildUi();
    QWidget* buildDashboardPage();
    QWidget* buildGenealogyPage();
    QWidget* buildMemberPage();
    QWidget* buildRelationManagePage();
    QWidget* buildTreePage();
    QWidget* buildAncestorPage();
    QWidget* buildRelationPage();
    int currentGenealogyId() const;
    int selectedMemberId() const;
    qreal descendantSubtreeWidth(int memberId, int depth, int maxDepth) const;
    qreal drawDescendantNode(const Member& member, qreal left, qreal top, int depth, int maxDepth);
    void drawMemberCard(const Member& member, qreal centerX, qreal top);
    void drawLimitCard(qreal centerX, qreal top);
    void drawRelationPathCard(const Member& member, qreal left, qreal top, bool isStart, bool isEnd);
    void drawRelationArrow(qreal fromX, qreal y, qreal toX);

    User user_;
    GenealogyDao genealogyDao_;
    MemberDao memberDao_;
    RelationDao relationDao_;
    DashboardService dashboardService_;
    TreeService treeService_;

    QListWidget* navigation_ = nullptr;
    QStackedWidget* pages_ = nullptr;
    QComboBox* genealogyCombo_ = nullptr;
    QLabel* statsLabel_ = nullptr;
    QLabel* genealogyInfoLabel_ = nullptr;
    QTableWidget* recentMembersTable_ = nullptr;
    QTableWidget* memberTable_ = nullptr;
    QGraphicsView* descendantTreeView_ = nullptr;
    QGraphicsScene* descendantTreeScene_ = nullptr;
    QLineEdit* memberSearchEdit_ = nullptr;
    QLineEdit* parentIdEdit_ = nullptr;
    QLineEdit* childIdEdit_ = nullptr;
    QComboBox* parentTypeCombo_ = nullptr;
    QLineEdit* spouseAEdit_ = nullptr;
    QLineEdit* spouseBEdit_ = nullptr;
    QLineEdit* marriageYearEdit_ = nullptr;
    QLineEdit* divorceYearEdit_ = nullptr;
    QLineEdit* marriageDescriptionEdit_ = nullptr;
    QLineEdit* treeRootEdit_ = nullptr;
    QLineEdit* treeDepthEdit_ = nullptr;
    QLineEdit* ancestorMemberEdit_ = nullptr;
    QTreeWidget* ancestorTree_ = nullptr;
    QLineEdit* relationAEdit_ = nullptr;
    QLineEdit* relationBEdit_ = nullptr;
    QLabel* relationResultLabel_ = nullptr;
    QGraphicsView* relationPathView_ = nullptr;
    QGraphicsScene* relationPathScene_ = nullptr;
};
