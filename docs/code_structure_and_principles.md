# 项目文件结构与关键代码实现原理

本文档用于验收时讲解项目结构、关键文件职责和核心功能的代码实现思路。项目采用 C++17 + Qt6 Widgets + PostgreSQL，实现一个桌面端族谱管理系统。

## 1. 总体结构

当前项目根目录结构如下：

```text
shujuku/
├── CMakeLists.txt
├── README.md
├── src/
│   ├── main.cpp
│   ├── model/
│   ├── db/
│   ├── service/
│   └── ui/
├── sql/
├── tools/
├── tests/
├── scripts/
└── docs/
```

各目录职责：

| 目录 | 作用 |
|---|---|
| `src/model/` | 定义业务数据结构，例如用户、族谱、成员 |
| `src/db/` | 数据访问层，封装 PostgreSQL 查询和增删改查 |
| `src/service/` | 业务逻辑层，封装登录、统计、树查询、亲缘路径 |
| `src/ui/` | Qt 图形界面层，负责窗口、表单、按钮、图形化展示 |
| `sql/` | 数据库建表、索引、触发器、核心查询、导入导出和性能测试 |
| `tools/` | 数据生成工具，用于生成 10 万级 CSV 数据 |
| `tests/` | 自动冒烟测试 |
| `scripts/` | WSL 运行脚本和中文输入法配置脚本 |
| `docs/` | 系统设计、验收资料、报告材料和使用说明 |

## 2. 构建入口

### `CMakeLists.txt`

该文件定义整个 C++/Qt 项目的构建方式。

关键点：

```cmake
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)
```

说明项目依赖 Qt6 的三个模块：

- `Core`：Qt 基础类型，例如 `QString`、`QDate`。
- `Widgets`：桌面图形界面。
- `Sql`：连接 PostgreSQL 数据库。

项目构建两个可执行文件：

```text
GenealogySystem
Stage2Smoke
```

其中：

- `GenealogySystem` 是主程序。
- `Stage2Smoke` 是自动冒烟测试程序，用于验证数据库连接、登录、族谱加载、Dashboard 和成员列表。

## 3. 程序启动流程

### `src/main.cpp`

这是程序入口。

主要流程：

```text
设置 WSL 中文输入环境变量
创建 QApplication
连接数据库
显示 LoginWindow
进入 Qt 事件循环
```

关键实现：

```cpp
qputenv("QT_IM_MODULE", "fcitx");
qputenv("XMODIFIERS", "@im=fcitx");
qputenv("GTK_IM_MODULE", "fcitx");
qputenv("QT_QPA_PLATFORM", "xcb");
```

这部分用于解决 WSLg + Qt6 中中文输入法无法接入的问题。项目同时提供 `scripts/run_wsl.sh`，运行时也会设置这些环境变量。

## 4. 数据模型层

### `src/model/User.h`

定义用户数据结构：

```cpp
struct User {
    int userId;
    QString username;
    QString realName;
    QString email;
};
```

对应数据库中的 `users` 表。

### `src/model/Genealogy.h`

定义族谱数据结构：

```text
genealogyId
title
familySurname
revisionTime
creatorUserId
description
```

对应数据库中的 `genealogies` 表。

### `src/model/Member.h`

定义成员数据结构：

```text
memberId
genealogyId
name
gender
birthYear
deathYear
generation
biography
```

对应数据库中的 `members` 表。

设计原因：

- `memberId` 用于区分同名同姓但不同辈分的人。
- `generation` 用于表示代数。
- `biography` 保存生平简介。

## 5. 数据访问层

数据访问层位于 `src/db/`，主要职责是封装 SQL，不让 UI 层直接拼写复杂数据库语句。

### `DatabaseManager`

文件：

```text
src/db/DatabaseManager.h
src/db/DatabaseManager.cpp
```

作用：

```text
统一管理 PostgreSQL 数据库连接。
```

核心实现：

```cpp
db_ = QSqlDatabase::addDatabase("QPSQL");
db_.setHostName(host);
db_.setPort(port);
db_.setDatabaseName(databaseName);
db_.setUserName(username);
db_.setPassword(password);
db_.open();
```

说明：

Qt 通过 `QSqlDatabase` 连接 PostgreSQL，连接成功后，其他 DAO 通过 `DatabaseManager::instance().database()` 获取同一个数据库连接。

### `UserDao`

文件：

```text
src/db/UserDao.h
src/db/UserDao.cpp
```

主要功能：

```text
根据用户名查用户
根据 ID 查用户
插入新用户
返回最近一次数据库错误
```

它服务于登录注册功能。

### `GenealogyDao`

文件：

```text
src/db/GenealogyDao.h
src/db/GenealogyDao.cpp
```

主要功能：

```text
查询当前用户可访问的族谱
查询族谱详情
新增族谱
编辑族谱
删除族谱
邀请协作者
查询当前用户在某族谱中的角色
```

权限查询核心：

```cpp
QString GenealogyDao::roleForUser(int genealogyId, int userId) const
```

该函数查询 `genealogy_collaborators` 表，返回：

```text
editor
viewer
空字符串
```

如果当前用户是族谱创建者，则 UI 层会把他识别为 `owner`。

### `MemberDao`

文件：

```text
src/db/MemberDao.h
src/db/MemberDao.cpp
```

主要功能：

```text
根据成员 ID 查详情
根据族谱 ID 查询成员列表
按姓名模糊查询成员
查询最近新增成员
查询子女
递归查询祖先
新增、编辑、删除成员
```

祖先查询核心使用 PostgreSQL 递归 CTE：

```sql
WITH RECURSIVE ancestors AS (...)
```

原因：

族谱层级深度不固定，不能用固定数量的 JOIN。递归 CTE 可以不断向上查父母，直到没有上级成员。

### `RelationDao`

文件：

```text
src/db/RelationDao.h
src/db/RelationDao.cpp
```

主要功能：

```text
添加亲子关系
添加婚姻关系
```

亲子关系写入 `parent_child_relations` 表。婚姻关系写入 `marriages` 表。

实际合法性检查主要由数据库触发器兜底，例如：

- 父亲必须是男性。
- 母亲必须是女性。
- 父母和子女必须属于同一族谱。
- 父母出生年份必须早于子女。
- 婚姻双方不能是同一个人。

## 6. 业务服务层

服务层位于 `src/service/`，用于组织 DAO 并封装业务逻辑。

### `AuthService`

文件：

```text
src/service/AuthService.h
src/service/AuthService.cpp
```

主要功能：

```text
登录
注册
密码哈希
```

登录时流程：

```text
读取用户名和密码
对密码做 SHA-256 哈希
根据用户名和哈希值查询 users 表
成功则返回 User
失败则返回空
```

项目小数据中 `admin / 123456` 的密码哈希对应 SHA-256。

### `DashboardService`

文件：

```text
src/service/DashboardService.h
src/service/DashboardService.cpp
```

主要功能：

```text
统计当前族谱总人数
统计男性人数
统计女性人数
统计最大代数
统计血缘关系数
统计婚姻关系数
```

核心 SQL 使用聚合函数：

```sql
COUNT(*)
COUNT(*) FILTER (WHERE gender = 'M')
COUNT(*) FILTER (WHERE gender = 'F')
MAX(generation)
```

Dashboard 页面在 UI 层进一步计算男女比例百分比。

### `TreeService`

文件：

```text
src/service/TreeService.h
src/service/TreeService.cpp
```

主要功能：

```text
查询子女
查询祖先
查询两名成员之间的亲缘链路
```

亲缘链路查询使用 BFS：

```text
从成员 A 出发
把亲子关系和婚姻关系看成图中的边
广度优先搜索
找到成员 B 后回溯 previous 数组生成路径
```

使用 BFS 的原因：

```text
亲缘关系不是简单树结构，婚姻关系会形成额外连接。
BFS 可以在图结构中找到两个成员之间的可达路径。
```

## 7. 图形界面层

UI 层位于 `src/ui/`，使用 Qt Widgets 实现桌面界面。

### `LoginWindow`

文件：

```text
src/ui/LoginWindow.h
src/ui/LoginWindow.cpp
```

主要功能：

```text
输入用户名和密码
登录
注册
数据库连接检查
登录成功后打开 MainWindow
```

核心信号槽：

```cpp
connect(loginButton, &QPushButton::clicked, this, &LoginWindow::handleLogin);
connect(registerButton, &QPushButton::clicked, this, &LoginWindow::handleRegister);
```

Qt 使用信号槽机制处理按钮点击事件。

### `MainWindow`

文件：

```text
src/ui/MainWindow.h
src/ui/MainWindow.cpp
```

这是主窗口，包含系统大部分界面功能。

左侧导航包括：

```text
Dashboard
族谱管理
成员管理
关系维护
树形预览
祖先查询
亲缘链路
```

主界面使用：

```cpp
QListWidget
QStackedWidget
```

实现方式：

```text
左侧 QListWidget 作为菜单。
右侧 QStackedWidget 保存多个页面。
点击不同菜单项时切换页面。
```

#### Dashboard 实现

Dashboard 调用：

```cpp
dashboardService_.loadStats(genealogyId)
```

然后显示：

```text
当前登录用户
可访问族谱数量
当前族谱
成员总数
男性人数和比例
女性人数和比例
最大代数
血缘关系数
婚姻关系数
最近新增成员
```

男女比例在界面层计算：

```cpp
maleMembers * 100.0 / totalMembers
femaleMembers * 100.0 / totalMembers
```

#### 权限控制实现

主窗口根据当前族谱和当前用户计算角色：

```cpp
currentAccessRole()
canManageCurrentGenealogy()
canEditCurrentData()
updatePermissionUi()
```

角色分三类：

| 角色 | 权限 |
|---|---|
| `owner` 创建者 | 管理族谱、邀请协作者、维护成员和关系 |
| `editor` 编辑者 | 维护成员和关系，不能管理族谱 |
| `viewer` 查看者 | 只能浏览和查询 |

按钮禁用逻辑在 `updatePermissionUi()` 中完成。写操作函数内部还会调用 `ensureCanEditCurrentData()` 或 `ensureCanManageCurrentGenealogy()` 做二次检查，防止绕过按钮直接触发操作。

#### 成员管理实现

成员列表使用：

```cpp
QTableWidget
```

关键功能：

```text
姓名模糊搜索
新增成员
编辑成员
删除成员
查看详情
点击表头排序
```

表格排序实现：

```cpp
memberTable_->setSortingEnabled(true);
```

为了让 `ID`、`出生年`、`死亡年`、`代数` 按数字排序，代码使用 `numberItem()` 将数值放入 `Qt::DisplayRole`，避免字符串排序导致 `100` 排在 `20` 前面。

#### 图形化后代树实现

后代树使用：

```cpp
QGraphicsView
QGraphicsScene
```

核心函数：

```cpp
descendantSubtreeWidth()
drawDescendantNode()
drawMemberCard()
```

实现思路：

```text
先递归计算每个子树需要的宽度。
父节点放在该子树宽度的中间。
子节点按照各自子树宽度从左到右排列。
父节点和子节点之间画连线。
```

这样可以实现：

```text
左子树显示在父节点左边
右子树显示在父节点右边
```

点击节点查看详情的实现：

```text
每个图形节点保存 memberId。
节点设置为可选中。
QGraphicsScene::selectionChanged 触发后读取 memberId。
调用 MemberDao::findById 查询完整成员信息。
弹出 QMessageBox 显示详情。
```

#### 祖先查询实现

祖先查询页面输入成员 ID，调用：

```cpp
treeService_.getAncestors(memberId)
```

底层由 `MemberDao::findAncestors()` 使用 Recursive CTE 查询所有祖先。

#### 图形化亲缘链路实现

亲缘链路页面输入两个成员 ID，调用：

```cpp
treeService_.findRelationPath(currentGenealogyId(), memberA, memberB)
```

查询结果是一组成员 ID 路径。UI 层再把每个成员绘制成卡片：

```cpp
drawRelationPathCard()
drawRelationArrow()
```

实现效果：

```text
起点卡片
中间成员卡片
终点卡片
箭头连接
```

### `MemberDialog`

文件：

```text
src/ui/MemberDialog.h
src/ui/MemberDialog.cpp
```

用于新增和编辑成员。

关键设计：

```text
性别使用下拉框：男 / 女
出生年、死亡年、代数做输入校验
姓名不能为空
死亡年不能早于出生年
```

### `GenealogyDialog`

文件：

```text
src/ui/GenealogyDialog.h
src/ui/GenealogyDialog.cpp
```

用于新增和编辑族谱。

字段包括：

```text
谱名
姓氏
修谱时间
备注
```

## 8. SQL 文件说明

### `sql/01_schema.sql`

创建六张核心表：

```text
users
genealogies
genealogy_collaborators
members
parent_child_relations
marriages
```

关键约束：

```text
主键
外键
唯一约束
CHECK 约束
```

例如：

```sql
CHECK (gender IN ('M', 'F'))
CHECK (birth_year IS NULL OR death_year IS NULL OR birth_year <= death_year)
CHECK (role IN ('editor', 'viewer'))
```

### `sql/02_indexes.sql`

创建索引，优化常用查询。

重点索引：

```text
idx_members_name_trgm
idx_parent_child_parent_id
idx_parent_child_child_id
```

说明：

- `idx_members_name_trgm` 用于姓名模糊查询。
- `idx_parent_child_parent_id` 用于根据父节点查询子节点。
- `idx_parent_child_child_id` 用于根据子节点追溯父母或祖先。

### `sql/03_triggers.sql`

定义触发器，保证复杂业务约束。

亲子关系触发器检查：

```text
父母和子女必须属于同一族谱
父亲必须为男
母亲必须为女
父母出生年份必须早于子女
父母代数必须小于子女
```

婚姻关系触发器检查：

```text
夫妻双方必须属于同一族谱
双方不能是同一人
统一 person1_id 和 person2_id 顺序，避免重复关系
```

### `sql/04_core_queries.sql`

保存老师要求的核心 SQL：

```text
查询配偶及所有子女
递归查询所有祖先
统计平均寿命最长的一代
查询 50 岁以上且无配偶男性
查询早于同辈平均出生年份的成员
```

其中祖先查询使用：

```sql
WITH RECURSIVE ancestors AS (...)
```

### `sql/05_seed_small.sql`

小规模测试数据。

用途：

```text
快速初始化数据库
方便登录和界面演示
```

默认账号：

```text
admin / 123456
editor1 / 123456
```

### `sql/07_seed_demo.sql`

演示数据，比小数据更适合展示权限和界面功能。

包含：

```text
admin
editor1
viewer1
多个族谱
成员
亲子关系
婚姻关系
协作者角色
```

### `sql/08_load_generated_csv.sql`

使用 PostgreSQL `COPY` 或 `\copy` 从 CSV 批量导入数据。

导入对象包括：

```text
users.csv
genealogies.csv
genealogy_collaborators.csv
members.csv
parent_child_relations.csv
marriages.csv
```

### `sql/09_export_branch.sql`

导出某个成员的后代分支。

核心实现：

```sql
WITH RECURSIVE descendant_ids AS (...)
```

该脚本会把指定根成员向下若干层的成员导出为：

```text
generated_data/branch_export.csv
```

### `sql/10_performance_explain.sql`

用于索引性能对比。

实现流程：

```text
先删除 parent_id 相关索引
执行四代后代查询 EXPLAIN ANALYZE
再创建 parent_id 和 child_id 索引
再次执行 EXPLAIN ANALYZE
对比执行时间和执行计划
```

## 9. 数据生成工具

### `tools/generate_data.py`

该脚本用于生成大规模模拟数据。

目标：

```text
至少 10 个族谱
整个系统不少于 100000 个成员
至少一个族谱 50000 以上成员
单个族谱至少 30 代
每个族谱内成员至少与其他成员存在亲缘关系
```

输出目录：

```text
generated_data/
```

输出 CSV：

```text
users.csv
genealogies.csv
genealogy_collaborators.csv
members.csv
parent_child_relations.csv
marriages.csv
```

生成后通过 `sql/08_load_generated_csv.sql` 批量导入 PostgreSQL。

## 10. 测试文件

### `tests/stage2_smoke.cpp`

自动冒烟测试。

测试内容：

```text
连接数据库
admin 登录
查询可访问族谱
加载 Dashboard 统计
查询成员列表
```

通过时输出：

```text
PASS: login, genealogy loading, dashboard, and member list are working.
```

它的作用是验收前快速判断核心链路是否正常。

## 11. 脚本文件

### `scripts/run_wsl.sh`

用于在 WSL 中启动 Qt 程序。

主要做两件事：

```text
设置中文输入法相关环境变量
启动 build/GenealogySystem
```

关键变量：

```bash
QT_IM_MODULE=fcitx
GTK_IM_MODULE=fcitx
XMODIFIERS=@im=fcitx
QT_QPA_PLATFORM=xcb
```

### `scripts/setup_fcitx_wsl.sh`

用于配置 WSLg 下的 fcitx5 拼音输入法。

功能：

```text
检查 fcitx5 相关包
写入 ~/.config/fcitx5/profile
把默认输入法设置为 pinyin
启动 fcitx5
```

### `scripts/check_chinese_input.sh`

用于检查中文输入相关环境，辅助排查 Qt 界面无法输入中文的问题。

## 12. 现场讲解建议

验收时可以按这个顺序讲代码：

```text
1. 先讲整体架构：model、db、service、ui、sql 分层。
2. 再讲数据库设计：六张核心表、亲子自关联、婚姻自关联、协作者表。
3. 讲核心难点：递归祖先查询、后代树布局、亲缘链路 BFS。
4. 讲权限控制：owner/editor/viewer，按钮禁用和写操作二次校验。
5. 讲性能优化：姓名模糊查询索引、parent_id 索引、EXPLAIN 对比。
6. 讲数据工程：Python 生成 10 万数据，COPY 导入，分支导出。
7. 最后讲自动测试：Stage2Smoke 验证核心流程。
```

## 13. 项目实现亮点

可以重点说明：

```text
1. 使用 PostgreSQL Recursive CTE 解决族谱层级深度不确定问题。
2. 使用 BFS 查询两个成员之间的亲缘链路。
3. 使用 QGraphicsView 图形化展示后代树和亲缘路径。
4. 使用 role-based 权限控制区分创建者、编辑者和查看者。
5. 使用 pg_trgm + GIN 索引优化姓名模糊查询。
6. 使用 parent_id 索引优化后代查询。
7. 支持 10 万级数据生成、COPY 导入和 EXPLAIN 性能分析。
```
