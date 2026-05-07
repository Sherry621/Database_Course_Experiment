#include "MainWindow.h"

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

#include "ui/MemberDialog.h"

MainWindow::MainWindow(const User& user, QWidget* parent) : QMainWindow(parent), user_(user) {
    buildUi();
    reloadGenealogies();
}

void MainWindow::buildUi() {
    setWindowTitle("寻根溯源 - 族谱管理系统");
    resize(1100, 720);

    navigation_ = new QListWidget(this);
    navigation_->addItems({"Dashboard", "成员管理", "树形预览", "祖先查询", "亲缘链路"});
    navigation_->setFixedWidth(160);
    connect(navigation_, &QListWidget::currentRowChanged, this, &MainWindow::switchPage);

    genealogyCombo_ = new QComboBox(this);
    connect(genealogyCombo_, &QComboBox::currentIndexChanged, this, &MainWindow::reloadDashboard);

    pages_ = new QStackedWidget(this);
    pages_->addWidget(buildDashboardPage());
    pages_->addWidget(buildMemberPage());
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

QWidget* MainWindow::buildMemberPage() {
    auto* page = new QWidget(this);
    memberSearchEdit_ = new QLineEdit(page);
    memberSearchEdit_->setPlaceholderText("姓名关键词");

    auto* searchButton = new QPushButton("搜索", page);
    auto* addButton = new QPushButton("新增成员", page);
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::reloadMembers);
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addMember);

    auto* toolbar = new QHBoxLayout();
    toolbar->addWidget(memberSearchEdit_);
    toolbar->addWidget(searchButton);
    toolbar->addWidget(addButton);

    memberTable_ = new QTableWidget(page);
    memberTable_->setColumnCount(6);
    memberTable_->setHorizontalHeaderLabels({"ID", "姓名", "性别", "出生年", "死亡年", "代数"});
    memberTable_->horizontalHeader()->setStretchLastSection(true);

    auto* layout = new QVBoxLayout(page);
    layout->addLayout(toolbar);
    layout->addWidget(memberTable_);
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
        return;
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
    MemberDialog dialog(currentGenealogyId(), this);
    if (dialog.exec() == QDialog::Accepted && memberDao_.insert(dialog.member())) {
        reloadMembers();
        reloadDashboard();
    }
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

void MainWindow::appendTreeChildren(QTreeWidgetItem* parentItem, int memberId) {
    for (const auto& child : treeService_.getChildren(memberId)) {
        auto* item = new QTreeWidgetItem(QStringList(
            QString("%1 ID:%2 第%3代").arg(child.name).arg(child.memberId).arg(child.generation)));
        parentItem->addChild(item);
        appendTreeChildren(item, child.memberId);
    }
}
