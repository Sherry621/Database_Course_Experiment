#include "MainWindow.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "ui/GenealogyDialog.h"
#include "ui/MemberDialog.h"

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

    auto* refreshButton = new QPushButton("刷新统计", page);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::reloadDashboard);

    auto* layout = new QVBoxLayout(page);
    layout->addWidget(refreshButton);
    layout->addWidget(statsLabel_);
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

    auto* buildButton = new QPushButton("生成后代树", page);
    connect(buildButton, &QPushButton::clicked, this, &MainWindow::buildTreePreview);

    auto* toolbar = new QHBoxLayout();
    toolbar->addWidget(treeRootEdit_);
    toolbar->addWidget(buildButton);

    treeWidget_ = new QTreeWidget(page);
    treeWidget_->setHeaderLabel("后代分支");

    auto* layout = new QVBoxLayout(page);
    layout->addLayout(toolbar);
    layout->addWidget(treeWidget_);
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

    auto* form = new QFormLayout();
    form->addRow("成员 A", relationAEdit_);
    form->addRow("成员 B", relationBEdit_);

    auto* layout = new QVBoxLayout(page);
    layout->addLayout(form);
    layout->addWidget(queryButton);
    layout->addWidget(relationResultLabel_);
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
        "家族总人数：%1\n男性人数：%2\n女性人数：%3\n最大代数：%4\n血缘关系数：%5\n婚姻关系数：%6")
        .arg(stats.totalMembers)
        .arg(stats.maleMembers)
        .arg(stats.femaleMembers)
        .arg(stats.maxGeneration)
        .arg(stats.parentChildRelations)
        .arg(stats.marriages));
}

void MainWindow::addGenealogy() {
    GenealogyDialog dialog(user_.userId, this);
    if (dialog.exec() == QDialog::Accepted && genealogyDao_.insert(dialog.genealogy())) {
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
    if (dialog.exec() == QDialog::Accepted && genealogyDao_.update(dialog.genealogy())) {
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
        QMessageBox::warning(this, "删除失败", "只有族谱创建者可以删除族谱。");
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
        QMessageBox::warning(this, "邀请失败", "请确认用户存在且当前族谱可访问。");
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
        memberTable_->setItem(row, 2, new QTableWidgetItem(QString(member.gender)));
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
    if (dialog.exec() == QDialog::Accepted && memberDao_.insert(dialog.member())) {
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
    if (dialog.exec() == QDialog::Accepted && memberDao_.update(dialog.member())) {
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

    if (memberDao_.remove(memberId)) {
        reloadMembers();
        reloadDashboard();
    }
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
        .arg(member->gender)
        .arg(member->birthYear)
        .arg(member->deathYear)
        .arg(member->generation)
        .arg(member->biography));
}

void MainWindow::addParentChildRelation() {
    if (!relationDao_.addParentChild(currentGenealogyId(),
                                     parentIdEdit_->text().toInt(),
                                     childIdEdit_->text().toInt(),
                                     parentTypeCombo_->currentData().toString())) {
        QMessageBox::warning(this, "添加失败", "请检查成员 ID、族谱归属、出生年份和父母唯一性约束。");
        return;
    }

    QMessageBox::information(this, "添加成功", "亲子关系已添加。");
    reloadDashboard();
}

void MainWindow::addMarriageRelation() {
    if (!relationDao_.addMarriage(currentGenealogyId(),
                                  spouseAEdit_->text().toInt(),
                                  spouseBEdit_->text().toInt(),
                                  marriageYearEdit_->text().toInt(),
                                  divorceYearEdit_->text().toInt(),
                                  marriageDescriptionEdit_->text().trimmed())) {
        QMessageBox::warning(this, "添加失败", "请检查成员 ID、族谱归属和婚姻年份。");
        return;
    }

    QMessageBox::information(this, "添加成功", "婚姻关系已添加。");
    reloadDashboard();
}

void MainWindow::buildTreePreview() {
    treeWidget_->clear();
    const int rootId = treeRootEdit_->text().toInt();
    const auto root = memberDao_.findById(rootId);
    if (!root.has_value()) {
        QMessageBox::warning(this, "查询失败", "未找到根成员。");
        return;
    }

    auto* rootItem = new QTreeWidgetItem(QStringList(root->name + QString(" ID:%1").arg(root->memberId)));
    treeWidget_->addTopLevelItem(rootItem);
    appendTreeChildren(rootItem, rootId);
    treeWidget_->expandAll();
}

void MainWindow::queryAncestors() {
    ancestorTree_->clear();
    for (const auto& ancestor : treeService_.getAncestors(ancestorMemberEdit_->text().toInt())) {
        ancestorTree_->addTopLevelItem(new QTreeWidgetItem(QStringList(
            QString("第%1代 %2 ID:%3").arg(ancestor.generation).arg(ancestor.name).arg(ancestor.memberId))));
    }
}

void MainWindow::queryRelationPath() {
    const auto path = treeService_.findRelationPath(
        currentGenealogyId(),
        relationAEdit_->text().toInt(),
        relationBEdit_->text().toInt());

    if (path.empty()) {
        relationResultLabel_->setText("未找到亲缘链路。");
        return;
    }

    QStringList parts;
    for (int memberId : path) {
        parts << QString::number(memberId);
    }
    relationResultLabel_->setText("存在亲缘链路：\n" + parts.join(" -> "));
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

void MainWindow::appendTreeChildren(QTreeWidgetItem* parentItem, int memberId) {
    for (const auto& child : treeService_.getChildren(memberId)) {
        auto* item = new QTreeWidgetItem(QStringList(
            QString("%1 ID:%2 第%3代").arg(child.name).arg(child.memberId).arg(child.generation)));
        parentItem->addChild(item);
        appendTreeChildren(item, child.memberId);
    }
}
