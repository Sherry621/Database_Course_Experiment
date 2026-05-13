#include "MainWindow.h"

#include <algorithm>

#include <QComboBox>
#include <QFormLayout>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QGraphicsView>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "ui/GenealogyDialog.h"
#include "ui/MemberDialog.h"

namespace {
constexpr qreal NodeWidth = 150.0;
constexpr qreal NodeHeight = 56.0;
constexpr qreal HorizontalGap = 36.0;
constexpr qreal VerticalGap = 112.0;
constexpr qreal RelationCardWidth = 156.0;
constexpr qreal RelationCardHeight = 64.0;
constexpr qreal RelationGap = 74.0;

bool parsePositiveInt(const QLineEdit* edit, const QString& fieldName, int& value, QString& error) {
    bool ok = false;
    value = edit->text().trimmed().toInt(&ok);
    if (!ok || value <= 0) {
        error = fieldName + "必须是大于 0 的整数。";
        return false;
    }
    return true;
}

bool parseOptionalYear(const QLineEdit* edit, const QString& fieldName, int& value, QString& error) {
    const QString text = edit->text().trimmed();
    if (text.isEmpty()) {
        value = 0;
        return true;
    }

    bool ok = false;
    value = text.toInt(&ok);
    if (!ok || value < 0) {
        error = fieldName + "必须是非负整数，或者留空。";
        return false;
    }
    return true;
}

QString genderText(QChar gender) {
    return gender == 'F' ? "女" : "男";
}

QString memberLabel(const std::optional<Member>& member, int memberId) {
    if (!member.has_value()) {
        return QString("#%1").arg(memberId);
    }
    return QString("%1(#%2)").arg(member->name).arg(member->memberId);
}
}

MainWindow::MainWindow(const User& user, QWidget* parent) : QMainWindow(parent), user_(user) {
    buildUi();
    reloadGenealogies();
}

void MainWindow::buildUi() {
    setWindowTitle("寻根溯源 - 族谱管理系统");
    resize(1100, 720);

    navigation_ = new QListWidget(this);
    navigation_->addItems({"Dashboard", "族谱管理", "成员管理", "关系维护", "树形预览", "祖先查询", "亲缘链路"});
    navigation_->setFixedWidth(160);
    connect(navigation_, &QListWidget::currentRowChanged, this, &MainWindow::switchPage);

    genealogyCombo_ = new QComboBox(this);
    connect(genealogyCombo_, &QComboBox::currentIndexChanged, this, &MainWindow::reloadDashboard);

    pages_ = new QStackedWidget(this);
    pages_->addWidget(buildDashboardPage());
    pages_->addWidget(buildGenealogyPage());
    pages_->addWidget(buildMemberPage());
    pages_->addWidget(buildRelationManagePage());
    pages_->addWidget(buildTreePage());
    pages_->addWidget(buildAncestorPage());
    pages_->addWidget(buildRelationPage());

    auto* topLayout = new QHBoxLayout();
    topLayout->addWidget(new QLabel("当前族谱", this));
    topLayout->addWidget(genealogyCombo_, 1);

    auto* contentLayout = new QVBoxLayout();
    contentLayout->addLayout(topLayout);
    contentLayout->addWidget(pages_);

    auto* rootLayout = new QHBoxLayout();
    rootLayout->addWidget(navigation_);
    rootLayout->addLayout(contentLayout, 1);

    auto* central = new QWidget(this);
    central->setLayout(rootLayout);
    setCentralWidget(central);

    navigation_->setCurrentRow(0);
}

QWidget* MainWindow::buildDashboardPage() {
    auto* page = new QWidget(this);
    statsLabel_ = new QLabel(page);
    statsLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);
    statsLabel_->setWordWrap(true);

    auto* statsBox = new QGroupBox("系统概览", page);
    auto* statsLayout = new QVBoxLayout(statsBox);
    statsLayout->addWidget(statsLabel_);

    auto* quickBox = new QGroupBox("常用操作", page);
    auto* addMemberButton = new QPushButton("新增成员", quickBox);
    auto* treeButton = new QPushButton("查看族谱树", quickBox);
    auto* relationButton = new QPushButton("亲属关系查询", quickBox);
    auto* refreshButton = new QPushButton("刷新统计", quickBox);
    connect(addMemberButton, &QPushButton::clicked, this, [this]() {
        navigation_->setCurrentRow(2);
        addMember();
    });
    connect(treeButton, &QPushButton::clicked, this, [this]() {
        navigation_->setCurrentRow(4);
    });
    connect(relationButton, &QPushButton::clicked, this, [this]() {
        navigation_->setCurrentRow(6);
    });
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::reloadDashboard);

    auto* quickLayout = new QGridLayout(quickBox);
    quickLayout->addWidget(addMemberButton, 0, 0);
    quickLayout->addWidget(treeButton, 0, 1);
    quickLayout->addWidget(relationButton, 1, 0);
    quickLayout->addWidget(refreshButton, 1, 1);

    recentMembersTable_ = new QTableWidget(page);
    recentMembersTable_->setColumnCount(5);
    recentMembersTable_->setHorizontalHeaderLabels({"ID", "姓名", "性别", "出生年", "代数"});
    recentMembersTable_->horizontalHeader()->setStretchLastSection(true);
    recentMembersTable_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    recentMembersTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    recentMembersTable_->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* recentBox = new QGroupBox("最近新增成员", page);
    auto* recentLayout = new QVBoxLayout(recentBox);
    recentLayout->addWidget(recentMembersTable_);

    auto* layout = new QVBoxLayout(page);
    auto* topLayout = new QHBoxLayout();
    topLayout->addWidget(statsBox, 2);
    topLayout->addWidget(quickBox, 1);
    layout->addLayout(topLayout);
    layout->addWidget(recentBox);
    layout->addStretch();
    return page;
}

QWidget* MainWindow::buildGenealogyPage() {
    auto* page = new QWidget(this);
    genealogyInfoLabel_ = new QLabel(page);
    genealogyInfoLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    auto* addButton = new QPushButton("新增族谱", page);
    auto* editButton = new QPushButton("编辑族谱", page);
    auto* deleteButton = new QPushButton("删除族谱", page);
    auto* inviteButton = new QPushButton("邀请协作者", page);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addGenealogy);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editGenealogy);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteGenealogy);
    connect(inviteButton, &QPushButton::clicked, this, &MainWindow::inviteCollaborator);

    auto* toolbar = new QHBoxLayout();
    toolbar->addWidget(addButton);
    toolbar->addWidget(editButton);
    toolbar->addWidget(deleteButton);
    toolbar->addWidget(inviteButton);
    toolbar->addStretch();

    auto* layout = new QVBoxLayout(page);
    layout->addLayout(toolbar);
    layout->addWidget(genealogyInfoLabel_);
    layout->addStretch();
    return page;
}

QWidget* MainWindow::buildMemberPage() {
    auto* page = new QWidget(this);
    memberSearchEdit_ = new QLineEdit(page);
    memberSearchEdit_->setPlaceholderText("姓名关键词");
    memberSearchEdit_->setAttribute(Qt::WA_InputMethodEnabled, true);

    auto* searchButton = new QPushButton("搜索", page);
    auto* addButton = new QPushButton("新增成员", page);
    auto* editButton = new QPushButton("编辑成员", page);
    auto* detailButton = new QPushButton("成员详情", page);
    auto* deleteButton = new QPushButton("删除成员", page);
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::reloadMembers);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addMember);
    connect(editButton, &QPushButton::clicked, this, &MainWindow::editMember);
    connect(detailButton, &QPushButton::clicked, this, &MainWindow::showMemberDetail);
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::deleteMember);

    auto* toolbar = new QHBoxLayout();
    toolbar->addWidget(memberSearchEdit_);
    toolbar->addWidget(searchButton);
    toolbar->addWidget(addButton);
    toolbar->addWidget(editButton);
    toolbar->addWidget(detailButton);
    toolbar->addWidget(deleteButton);

    memberTable_ = new QTableWidget(page);
    memberTable_->setColumnCount(6);
    memberTable_->setHorizontalHeaderLabels({"ID", "姓名", "性别", "出生年", "死亡年", "代数"});
    memberTable_->horizontalHeader()->setStretchLastSection(true);
    memberTable_->setSelectionBehavior(QAbstractItemView::SelectRows);
    memberTable_->setSelectionMode(QAbstractItemView::SingleSelection);

    auto* layout = new QVBoxLayout(page);
    layout->addLayout(toolbar);
    layout->addWidget(memberTable_);
    return page;
}

QWidget* MainWindow::buildRelationManagePage() {
    auto* page = new QWidget(this);

    parentIdEdit_ = new QLineEdit(page);
    childIdEdit_ = new QLineEdit(page);
    parentTypeCombo_ = new QComboBox(page);
    parentTypeCombo_->addItem("父亲", "father");
    parentTypeCombo_->addItem("母亲", "mother");

    auto* addParentChildButton = new QPushButton("添加亲子关系", page);
    connect(addParentChildButton, &QPushButton::clicked, this, &MainWindow::addParentChildRelation);

    auto* parentForm = new QFormLayout();
    parentForm->addRow("父/母成员 ID", parentIdEdit_);
    parentForm->addRow("子女成员 ID", childIdEdit_);
    parentForm->addRow("关系类型", parentTypeCombo_);
    parentForm->addRow(addParentChildButton);

    spouseAEdit_ = new QLineEdit(page);
    spouseBEdit_ = new QLineEdit(page);
    marriageYearEdit_ = new QLineEdit(page);
    divorceYearEdit_ = new QLineEdit(page);
    marriageDescriptionEdit_ = new QLineEdit(page);
    marriageDescriptionEdit_->setAttribute(Qt::WA_InputMethodEnabled, true);

    auto* addMarriageButton = new QPushButton("添加婚姻关系", page);
    connect(addMarriageButton, &QPushButton::clicked, this, &MainWindow::addMarriageRelation);

    auto* marriageForm = new QFormLayout();
    marriageForm->addRow("成员 A ID", spouseAEdit_);
    marriageForm->addRow("成员 B ID", spouseBEdit_);
    marriageForm->addRow("结婚年份", marriageYearEdit_);
    marriageForm->addRow("离婚年份", divorceYearEdit_);
    marriageForm->addRow("备注", marriageDescriptionEdit_);
    marriageForm->addRow(addMarriageButton);

    auto* layout = new QVBoxLayout(page);
    layout->addWidget(new QLabel("血缘关系维护", page));
    layout->addLayout(parentForm);
    layout->addSpacing(20);
    layout->addWidget(new QLabel("婚姻关系维护", page));
    layout->addLayout(marriageForm);
    layout->addStretch();
    return page;
}

QWidget* MainWindow::buildTreePage() {
    auto* page = new QWidget(this);
    treeRootEdit_ = new QLineEdit(page);
    treeRootEdit_->setPlaceholderText("根成员 ID");
    treeDepthEdit_ = new QLineEdit(page);
    treeDepthEdit_->setPlaceholderText("最大显示层数，默认 5");

    auto* buildButton = new QPushButton("生成后代树", page);
    connect(buildButton, &QPushButton::clicked, this, &MainWindow::buildTreePreview);

    auto* toolbar = new QHBoxLayout();
    toolbar->addWidget(treeRootEdit_);
    toolbar->addWidget(treeDepthEdit_);
    toolbar->addWidget(buildButton);

    descendantTreeScene_ = new QGraphicsScene(page);
    descendantTreeView_ = new QGraphicsView(descendantTreeScene_, page);
    descendantTreeView_->setRenderHint(QPainter::Antialiasing, true);
    descendantTreeView_->setDragMode(QGraphicsView::ScrollHandDrag);
    descendantTreeView_->setAlignment(Qt::AlignCenter);
    descendantTreeView_->setMinimumHeight(420);

    auto* layout = new QVBoxLayout(page);
    layout->addLayout(toolbar);
    layout->addWidget(descendantTreeView_);
    return page;
}

QWidget* MainWindow::buildAncestorPage() {
    auto* page = new QWidget(this);
    ancestorMemberEdit_ = new QLineEdit(page);
    ancestorMemberEdit_->setPlaceholderText("成员 ID");

    auto* queryButton = new QPushButton("查询祖先", page);
    connect(queryButton, &QPushButton::clicked, this, &MainWindow::queryAncestors);

    auto* toolbar = new QHBoxLayout();
    toolbar->addWidget(ancestorMemberEdit_);
    toolbar->addWidget(queryButton);

    ancestorTree_ = new QTreeWidget(page);
    ancestorTree_->setHeaderLabel("祖先列表");

    auto* layout = new QVBoxLayout(page);
    layout->addLayout(toolbar);
    layout->addWidget(ancestorTree_);
    return page;
}

QWidget* MainWindow::buildRelationPage() {
    auto* page = new QWidget(this);
    relationAEdit_ = new QLineEdit(page);
    relationBEdit_ = new QLineEdit(page);
    relationAEdit_->setPlaceholderText("成员 A ID");
    relationBEdit_->setPlaceholderText("成员 B ID");

    auto* queryButton = new QPushButton("查询链路", page);
    connect(queryButton, &QPushButton::clicked, this, &MainWindow::queryRelationPath);

    relationResultLabel_ = new QLabel(page);
    relationResultLabel_->setWordWrap(true);
    relationResultLabel_->setTextInteractionFlags(Qt::TextSelectableByMouse);

    relationPathScene_ = new QGraphicsScene(page);
    relationPathView_ = new QGraphicsView(relationPathScene_, page);
    relationPathView_->setRenderHint(QPainter::Antialiasing, true);
    relationPathView_->setDragMode(QGraphicsView::ScrollHandDrag);
    relationPathView_->setMinimumHeight(260);
    relationPathView_->setAlignment(Qt::AlignCenter);

    auto* form = new QFormLayout();
    form->addRow("成员 A", relationAEdit_);
    form->addRow("成员 B", relationBEdit_);

    auto* layout = new QVBoxLayout(page);
    layout->addLayout(form);
    layout->addWidget(queryButton);
    layout->addWidget(relationResultLabel_);
    layout->addWidget(relationPathView_);
    layout->addStretch();
    return page;
}

void MainWindow::switchPage(int row) {
    pages_->setCurrentIndex(row);
    if (row == 0) {
        reloadDashboard();
    } else if (row == 1) {
        reloadDashboard();
    } else if (row == 2) {
        reloadMembers();
    }
}

void MainWindow::reloadGenealogies() {
    genealogyCombo_->clear();
    for (const auto& genealogy : genealogyDao_.findAccessibleByUser(user_.userId)) {
        genealogyCombo_->addItem(genealogy.title, genealogy.genealogyId);
    }
    reloadDashboard();
}

void MainWindow::reloadDashboard() {
    const int genealogyId = currentGenealogyId();
    if (genealogyId == 0) {
        statsLabel_->setText("当前用户暂无可访问族谱。");
        if (recentMembersTable_) {
            recentMembersTable_->setRowCount(0);
        }
        if (genealogyInfoLabel_) {
            genealogyInfoLabel_->setText("当前用户暂无可访问族谱。");
        }
        return;
    }

    const auto genealogy = genealogyDao_.findById(genealogyId);
    if (genealogy.has_value() && genealogyInfoLabel_) {
        genealogyInfoLabel_->setText(QString(
            "谱名：%1\n姓氏：%2\n修谱时间：%3\n创建用户 ID：%4\n备注：%5")
            .arg(genealogy->title)
            .arg(genealogy->familySurname)
            .arg(genealogy->revisionTime.toString("yyyy-MM-dd"))
            .arg(genealogy->creatorUserId)
            .arg(genealogy->description));
    }

    const auto stats = dashboardService_.loadStats(genealogyId);
    statsLabel_->setText(QString(
        "当前登录用户：%1（ID：%2）\n"
        "可访问族谱数量：%3\n"
        "当前族谱：%4\n"
        "成员总数：%5\n"
        "男性人数：%6\n"
        "女性人数：%7\n"
        "最大代数：%8\n"
        "血缘关系数：%9\n"
        "婚姻关系数：%10")
        .arg(user_.realName.isEmpty() ? user_.username : user_.realName)
        .arg(user_.userId)
        .arg(genealogyCombo_->count())
        .arg(genealogyCombo_->currentText())
        .arg(stats.totalMembers)
        .arg(stats.maleMembers)
        .arg(stats.femaleMembers)
        .arg(stats.maxGeneration)
        .arg(stats.parentChildRelations)
        .arg(stats.marriages));

    if (recentMembersTable_) {
        const auto recentMembers = memberDao_.findRecentByGenealogy(genealogyId, 5);
        recentMembersTable_->setRowCount(static_cast<int>(recentMembers.size()));
        for (int row = 0; row < static_cast<int>(recentMembers.size()); ++row) {
            const auto& member = recentMembers[row];
            recentMembersTable_->setItem(row, 0, new QTableWidgetItem(QString::number(member.memberId)));
            recentMembersTable_->setItem(row, 1, new QTableWidgetItem(member.name));
            recentMembersTable_->setItem(row, 2, new QTableWidgetItem(genderText(member.gender)));
            recentMembersTable_->setItem(row, 3, new QTableWidgetItem(member.birthYear == 0 ? QString() : QString::number(member.birthYear)));
            recentMembersTable_->setItem(row, 4, new QTableWidgetItem(member.generation == 0 ? QString() : QString::number(member.generation)));
        }
    }
}

void MainWindow::addGenealogy() {
    GenealogyDialog dialog(user_.userId, this);
    if (dialog.exec() == QDialog::Accepted) {
        if (!genealogyDao_.insert(dialog.genealogy())) {
            QMessageBox::warning(this, "新增失败", genealogyDao_.lastError());
            return;
        }
        reloadGenealogies();
    }
}

void MainWindow::editGenealogy() {
    const auto genealogy = genealogyDao_.findById(currentGenealogyId());
    if (!genealogy.has_value()) {
        QMessageBox::warning(this, "编辑失败", "请先选择族谱。");
        return;
    }

    GenealogyDialog dialog(*genealogy, this);
    if (dialog.exec() == QDialog::Accepted) {
        if (!genealogyDao_.update(dialog.genealogy())) {
            QMessageBox::warning(this, "编辑失败", genealogyDao_.lastError());
            return;
        }
        reloadGenealogies();
    }
}

void MainWindow::deleteGenealogy() {
    if (currentGenealogyId() == 0) {
        return;
    }

    if (QMessageBox::question(this, "确认删除", "删除族谱会同时删除成员和关系，是否继续？")
        != QMessageBox::Yes) {
        return;
    }

    if (!genealogyDao_.remove(currentGenealogyId(), user_.userId)) {
        QMessageBox::warning(this, "删除失败", genealogyDao_.lastError());
        return;
    }
    reloadGenealogies();
}

void MainWindow::inviteCollaborator() {
    bool ok = false;
    const QString username = QInputDialog::getText(this, "邀请协作者", "请输入用户名：", QLineEdit::Normal, {}, &ok);
    if (!ok || username.trimmed().isEmpty()) {
        return;
    }

    if (!genealogyDao_.addCollaboratorByUsername(currentGenealogyId(), username.trimmed(), "editor")) {
        QMessageBox::warning(this, "邀请失败", genealogyDao_.lastError());
        return;
    }
    QMessageBox::information(this, "邀请成功", "协作者已加入当前族谱。");
}

void MainWindow::reloadMembers() {
    const auto members = memberDao_.findByGenealogy(currentGenealogyId(), memberSearchEdit_->text().trimmed());
    memberTable_->setRowCount(static_cast<int>(members.size()));

    for (int row = 0; row < static_cast<int>(members.size()); ++row) {
        const auto& member = members[row];
        memberTable_->setItem(row, 0, new QTableWidgetItem(QString::number(member.memberId)));
        memberTable_->setItem(row, 1, new QTableWidgetItem(member.name));
        memberTable_->setItem(row, 2, new QTableWidgetItem(genderText(member.gender)));
        memberTable_->setItem(row, 3, new QTableWidgetItem(QString::number(member.birthYear)));
        memberTable_->setItem(row, 4, new QTableWidgetItem(QString::number(member.deathYear)));
        memberTable_->setItem(row, 5, new QTableWidgetItem(QString::number(member.generation)));
    }
}

void MainWindow::addMember() {
    if (currentGenealogyId() == 0) {
        QMessageBox::warning(this, "新增失败", "请先创建或选择族谱。");
        return;
    }

    MemberDialog dialog(currentGenealogyId(), this);
    if (dialog.exec() == QDialog::Accepted) {
        if (!memberDao_.insert(dialog.member())) {
            QMessageBox::warning(this, "新增失败", memberDao_.lastError());
            return;
        }
        reloadMembers();
        reloadDashboard();
    }
}

void MainWindow::editMember() {
    const int memberId = selectedMemberId();
    const auto member = memberDao_.findById(memberId);
    if (!member.has_value()) {
        QMessageBox::warning(this, "编辑失败", "请先选择成员。");
        return;
    }

    MemberDialog dialog(*member, this);
    if (dialog.exec() == QDialog::Accepted) {
        if (!memberDao_.update(dialog.member())) {
            QMessageBox::warning(this, "编辑失败", memberDao_.lastError());
            return;
        }
        reloadMembers();
        reloadDashboard();
    }
}

void MainWindow::deleteMember() {
    const int memberId = selectedMemberId();
    if (memberId == 0) {
        QMessageBox::warning(this, "删除失败", "请先选择成员。");
        return;
    }

    if (QMessageBox::question(this, "确认删除", "删除成员会同时删除关联关系，是否继续？")
        != QMessageBox::Yes) {
        return;
    }

    if (!memberDao_.remove(memberId)) {
        QMessageBox::warning(this, "删除失败", memberDao_.lastError());
        return;
    }
    reloadMembers();
    reloadDashboard();
}

void MainWindow::showMemberDetail() {
    const int memberId = selectedMemberId();
    const auto member = memberDao_.findById(memberId);
    if (!member.has_value()) {
        QMessageBox::warning(this, "查看失败", "请先选择成员。");
        return;
    }

    QMessageBox::information(this, "成员详情", QString(
        "ID：%1\n姓名：%2\n性别：%3\n出生年：%4\n死亡年：%5\n代数：%6\n生平简介：%7")
        .arg(member->memberId)
        .arg(member->name)
        .arg(genderText(member->gender))
        .arg(member->birthYear)
        .arg(member->deathYear)
        .arg(member->generation)
        .arg(member->biography));
}

void MainWindow::addParentChildRelation() {
    if (currentGenealogyId() == 0) {
        QMessageBox::warning(this, "添加失败", "请先选择族谱。");
        return;
    }

    QString error;
    int parentId = 0;
    int childId = 0;
    if (!parsePositiveInt(parentIdEdit_, "父/母成员 ID", parentId, error) ||
        !parsePositiveInt(childIdEdit_, "子女成员 ID", childId, error)) {
        QMessageBox::warning(this, "输入错误", error);
        return;
    }

    if (parentId == childId) {
        QMessageBox::warning(this, "输入错误", "父母成员和子女成员不能是同一个人。");
        return;
    }

    if (!relationDao_.addParentChild(currentGenealogyId(),
                                     parentId,
                                     childId,
                                     parentTypeCombo_->currentData().toString())) {
        QMessageBox::warning(this, "添加失败", relationDao_.lastError());
        return;
    }

    QMessageBox::information(this, "添加成功", "亲子关系已添加。");
    reloadDashboard();
}

void MainWindow::addMarriageRelation() {
    if (currentGenealogyId() == 0) {
        QMessageBox::warning(this, "添加失败", "请先选择族谱。");
        return;
    }

    QString error;
    int spouseAId = 0;
    int spouseBId = 0;
    int marriageYear = 0;
    int divorceYear = 0;
    if (!parsePositiveInt(spouseAEdit_, "成员 A ID", spouseAId, error) ||
        !parsePositiveInt(spouseBEdit_, "成员 B ID", spouseBId, error) ||
        !parseOptionalYear(marriageYearEdit_, "结婚年份", marriageYear, error) ||
        !parseOptionalYear(divorceYearEdit_, "离婚年份", divorceYear, error)) {
        QMessageBox::warning(this, "输入错误", error);
        return;
    }

    if (spouseAId == spouseBId) {
        QMessageBox::warning(this, "输入错误", "婚姻关系双方不能是同一个人。");
        return;
    }

    if (marriageYear > 0 && divorceYear > 0 && divorceYear < marriageYear) {
        QMessageBox::warning(this, "输入错误", "离婚年份不能早于结婚年份。");
        return;
    }

    if (!relationDao_.addMarriage(currentGenealogyId(),
                                  spouseAId,
                                  spouseBId,
                                  marriageYear,
                                  divorceYear,
                                  marriageDescriptionEdit_->text().trimmed())) {
        QMessageBox::warning(this, "添加失败", relationDao_.lastError());
        return;
    }

    QMessageBox::information(this, "添加成功", "婚姻关系已添加。");
    reloadDashboard();
}

void MainWindow::buildTreePreview() {
    descendantTreeScene_->clear();
    QString error;
    int rootId = 0;
    if (!parsePositiveInt(treeRootEdit_, "根成员 ID", rootId, error)) {
        QMessageBox::warning(this, "输入错误", error);
        return;
    }

    int maxDepth = 5;
    if (!treeDepthEdit_->text().trimmed().isEmpty() &&
        !parsePositiveInt(treeDepthEdit_, "最大显示层数", maxDepth, error)) {
        QMessageBox::warning(this, "输入错误", error);
        return;
    }

    const auto root = memberDao_.findById(rootId);
    if (!root.has_value()) {
        QMessageBox::warning(this, "查询失败", "未找到根成员。");
        return;
    }

    const qreal treeWidth = descendantSubtreeWidth(rootId, 0, maxDepth);
    drawDescendantNode(*root, 0.0, 20.0, 0, maxDepth);
    descendantTreeScene_->setSceneRect(descendantTreeScene_->itemsBoundingRect().adjusted(-80, -60, 80, 80));
    descendantTreeView_->fitInView(descendantTreeScene_->sceneRect(), Qt::KeepAspectRatio);
    descendantTreeView_->centerOn(treeWidth / 2.0, 20.0);
}

void MainWindow::queryAncestors() {
    ancestorTree_->clear();
    QString error;
    int memberId = 0;
    if (!parsePositiveInt(ancestorMemberEdit_, "成员 ID", memberId, error)) {
        QMessageBox::warning(this, "输入错误", error);
        return;
    }

    for (const auto& ancestor : treeService_.getAncestors(memberId)) {
        ancestorTree_->addTopLevelItem(new QTreeWidgetItem(QStringList(
            QString("第%1代 %2 ID:%3").arg(ancestor.generation).arg(ancestor.name).arg(ancestor.memberId))));
    }
}

void MainWindow::queryRelationPath() {
    relationPathScene_->clear();
    QString error;
    int memberA = 0;
    int memberB = 0;
    if (!parsePositiveInt(relationAEdit_, "成员 A ID", memberA, error) ||
        !parsePositiveInt(relationBEdit_, "成员 B ID", memberB, error)) {
        QMessageBox::warning(this, "输入错误", error);
        return;
    }

    const auto path = treeService_.findRelationPath(
        currentGenealogyId(),
        memberA,
        memberB);

    if (path.empty()) {
        relationResultLabel_->setText("未找到亲缘链路。");
        auto* text = relationPathScene_->addText("未找到两名成员之间的亲缘链路");
        QFont font = text->font();
        font.setPointSize(12);
        font.setBold(true);
        text->setFont(font);
        text->setDefaultTextColor(QColor("#52606D"));
        text->setPos(24.0, 24.0);
        relationPathScene_->setSceneRect(relationPathScene_->itemsBoundingRect().adjusted(-40, -40, 40, 40));
        return;
    }

    QStringList parts;
    std::vector<Member> members;
    for (int memberId : path) {
        const auto member = memberDao_.findById(memberId);
        parts << memberLabel(member, memberId);
        if (member.has_value()) {
            members.push_back(*member);
        }
    }
    relationResultLabel_->setText(QString("存在亲缘链路，共 %1 个节点：\n%2")
                                      .arg(path.size())
                                      .arg(parts.join(" -> ")));

    qreal left = 20.0;
    const qreal top = 56.0;
    for (int i = 0; i < static_cast<int>(members.size()); ++i) {
        drawRelationPathCard(members[i],
                             left,
                             top,
                             i == 0,
                             i == static_cast<int>(members.size()) - 1);
        if (i < static_cast<int>(members.size()) - 1) {
            drawRelationArrow(left + RelationCardWidth + 10.0,
                              top + RelationCardHeight / 2.0,
                              left + RelationCardWidth + RelationGap - 10.0);
        }
        left += RelationCardWidth + RelationGap;
    }

    relationPathScene_->setSceneRect(relationPathScene_->itemsBoundingRect().adjusted(-40, -50, 40, 50));
    relationPathView_->fitInView(relationPathScene_->sceneRect(), Qt::KeepAspectRatio);
}

int MainWindow::currentGenealogyId() const {
    return genealogyCombo_->currentData().toInt();
}

int MainWindow::selectedMemberId() const {
    if (!memberTable_ || memberTable_->currentRow() < 0 || !memberTable_->item(memberTable_->currentRow(), 0)) {
        return 0;
    }
    return memberTable_->item(memberTable_->currentRow(), 0)->text().toInt();
}

qreal MainWindow::descendantSubtreeWidth(int memberId, int depth, int maxDepth) const {
    if (depth >= maxDepth) {
        return NodeWidth;
    }

    const auto children = treeService_.getChildren(memberId);
    if (children.empty()) {
        return NodeWidth;
    }

    qreal width = 0.0;
    for (const auto& child : children) {
        if (width > 0.0) {
            width += HorizontalGap;
        }
        width += descendantSubtreeWidth(child.memberId, depth + 1, maxDepth);
    }
    return std::max(NodeWidth, width);
}

qreal MainWindow::drawDescendantNode(const Member& member, qreal left, qreal top, int depth, int maxDepth) {
    const qreal subtreeWidth = descendantSubtreeWidth(member.memberId, depth, maxDepth);
    const qreal centerX = left + subtreeWidth / 2.0;
    drawMemberCard(member, centerX, top);

    const auto children = treeService_.getChildren(member.memberId);
    if (children.empty()) {
        return subtreeWidth;
    }

    const qreal childTop = top + VerticalGap;
    if (depth >= maxDepth) {
        drawLimitCard(centerX, childTop);
        descendantTreeScene_->addLine(centerX,
                                      top + NodeHeight,
                                      centerX,
                                      childTop,
                                      QPen(QColor("#7A8794"), 2));
        return subtreeWidth;
    }

    qreal childLeft = left;
    for (const auto& child : children) {
        const qreal childWidth = descendantSubtreeWidth(child.memberId, depth + 1, maxDepth);
        const qreal childCenterX = childLeft + childWidth / 2.0;
        descendantTreeScene_->addLine(centerX,
                                      top + NodeHeight,
                                      childCenterX,
                                      childTop,
                                      QPen(QColor("#7A8794"), 2));
        drawDescendantNode(child, childLeft, childTop, depth + 1, maxDepth);
        childLeft += childWidth + HorizontalGap;
    }

    return subtreeWidth;
}

void MainWindow::drawMemberCard(const Member& member, qreal centerX, qreal top) {
    const QRectF rect(centerX - NodeWidth / 2.0, top, NodeWidth, NodeHeight);
    const QColor border = member.gender == 'F' ? QColor("#C94F7C") : QColor("#2F6FAE");
    const QColor fill = member.gender == 'F' ? QColor("#FFF1F6") : QColor("#EEF6FF");

    QPainterPath path;
    path.addRoundedRect(rect, 8, 8);
    descendantTreeScene_->addPath(path, QPen(border, 2), QBrush(fill));

    auto* nameText = descendantTreeScene_->addText(member.name);
    QFont nameFont = nameText->font();
    nameFont.setBold(true);
    nameFont.setPointSize(10);
    nameText->setFont(nameFont);
    nameText->setDefaultTextColor(QColor("#1F2933"));
    nameText->setTextWidth(NodeWidth - 16.0);
    nameText->setPos(rect.left() + 8.0, rect.top() + 6.0);

    auto* detailText = descendantTreeScene_->addText(
        QString("ID:%1  %2  第%3代")
            .arg(member.memberId)
            .arg(genderText(member.gender))
            .arg(member.generation == 0 ? QString("-") : QString::number(member.generation)));
    QFont detailFont = detailText->font();
    detailFont.setPointSize(8);
    detailText->setFont(detailFont);
    detailText->setDefaultTextColor(QColor("#4B5563"));
    detailText->setTextWidth(NodeWidth - 16.0);
    detailText->setPos(rect.left() + 8.0, rect.top() + 31.0);
}

void MainWindow::drawLimitCard(qreal centerX, qreal top) {
    const QRectF rect(centerX - 58.0, top, 116.0, 34.0);
    QPainterPath path;
    path.addRoundedRect(rect, 8, 8);
    descendantTreeScene_->addPath(path,
                                  QPen(QColor("#9AA5B1"), 1, Qt::DashLine),
                                  QBrush(QColor("#F5F7FA")));
    auto* text = descendantTreeScene_->addText("还有下级成员");
    text->setDefaultTextColor(QColor("#52606D"));
    text->setTextWidth(rect.width());
    text->setPos(rect.left() + 8.0, rect.top() + 6.0);
}

void MainWindow::drawRelationPathCard(const Member& member, qreal left, qreal top, bool isStart, bool isEnd) {
    const QRectF rect(left, top, RelationCardWidth, RelationCardHeight);
    const QColor border = isStart ? QColor("#1D8A5A") : (isEnd ? QColor("#B45309") : QColor("#52606D"));
    const QColor fill = isStart ? QColor("#ECFDF3") : (isEnd ? QColor("#FFF7ED") : QColor("#F8FAFC"));

    QPainterPath path;
    path.addRoundedRect(rect, 8, 8);
    relationPathScene_->addPath(path, QPen(border, 2), QBrush(fill));

    const QString badge = isStart ? "起点" : (isEnd ? "终点" : "中间成员");
    auto* badgeText = relationPathScene_->addText(badge);
    QFont badgeFont = badgeText->font();
    badgeFont.setPointSize(8);
    badgeText->setFont(badgeFont);
    badgeText->setDefaultTextColor(border);
    badgeText->setPos(rect.left() + 8.0, rect.top() + 5.0);

    auto* nameText = relationPathScene_->addText(member.name);
    QFont nameFont = nameText->font();
    nameFont.setBold(true);
    nameFont.setPointSize(10);
    nameText->setFont(nameFont);
    nameText->setDefaultTextColor(QColor("#1F2933"));
    nameText->setTextWidth(RelationCardWidth - 16.0);
    nameText->setPos(rect.left() + 8.0, rect.top() + 24.0);

    auto* detailText = relationPathScene_->addText(
        QString("ID:%1  %2  第%3代")
            .arg(member.memberId)
            .arg(genderText(member.gender))
            .arg(member.generation == 0 ? QString("-") : QString::number(member.generation)));
    QFont detailFont = detailText->font();
    detailFont.setPointSize(8);
    detailText->setFont(detailFont);
    detailText->setDefaultTextColor(QColor("#4B5563"));
    detailText->setTextWidth(RelationCardWidth - 16.0);
    detailText->setPos(rect.left() + 8.0, rect.top() + 45.0);
}

void MainWindow::drawRelationArrow(qreal fromX, qreal y, qreal toX) {
    const QColor color("#64748B");
    relationPathScene_->addLine(fromX, y, toX, y, QPen(color, 2));

    QPolygonF arrowHead;
    arrowHead << QPointF(toX, y)
              << QPointF(toX - 10.0, y - 6.0)
              << QPointF(toX - 10.0, y + 6.0);
    relationPathScene_->addPolygon(arrowHead, QPen(color, 2), QBrush(color));
}
