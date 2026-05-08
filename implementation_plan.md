# 数据库课程实验逐步实现路线

本文档用于指导后续一步一步完成《“寻根溯源”族谱管理系统设计与实现》。目标不是一次性写完所有代码，而是每一步都能产出可验证结果，最后能支撑实验报告和现场演示。

## 1. 当前仓库判断

当前项目里有三类内容：

1. `DOC/`
   实验说明、环境检查、分工和设计文档。PPT 的核心验收点已经可以归纳为：界面功能、数据库建模、10 万级数据、核心 SQL、索引性能、报告截图。

2. 根目录的 `sql/`、`data/`、`scripts/`
   这里已经有大数据生成、CSV、导入、核心查询、性能测试脚本，适合作为数据库实验材料来源。

3. `Database_Course_Experiment/`
   这里是队友搭好的 C++/Qt 主工程，包括 Qt 界面、DAO、Service、PostgreSQL 脚本。它更适合作为最终运行和演示的主工程。

建议采用以下主线：

```text
以 Database_Course_Experiment/ 作为主工程
以它的 C++/Qt 代码和 SQL 字段命名为准
把根目录 scripts/sql/data 中可用的内容迁移或改写进主工程
```

原因是 C++ 代码已经大量使用 `member_id`、`genealogy_id`、`generation`、`person1_id`、`person2_id` 等字段。如果改成根目录 `sql/01_schema.sql` 的 `id`、`generation_no`、`spouse1_id` 风格，需要改很多 DAO 和 Service，风险更高。

## 2. 必须先统一的数据库契约

正式写 C++ 功能前，先确认一套表结构。建议使用 `Database_Course_Experiment/sql/01_schema.sql` 作为标准：

```text
users(user_id, username, password_hash, real_name, email, created_at)
genealogies(genealogy_id, title, family_surname, revision_time, creator_user_id, description, created_at)
genealogy_collaborators(genealogy_id, user_id, role, invited_at)
members(member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography, created_at)
parent_child_relations(relation_id, genealogy_id, parent_id, child_id, relation_type)
marriages(marriage_id, genealogy_id, person1_id, person2_id, marriage_year, divorce_year, description)
```

后续所有 C++ 查询、核心 SQL、数据生成、导入导出、报告截图都围绕这一套字段命名。这样可以避免“SQL 能跑但程序查不到字段”的问题。

## 3. 阶段一：数据库能跑通

目标：先让 PostgreSQL 中有表、有约束、有少量可检查数据。

要做：

1. 安装并启动 PostgreSQL 服务端。
2. 创建数据库用户和数据库：

```bash
sudo service postgresql start
sudo -u postgres psql
```

```sql
CREATE USER genealogy_user WITH PASSWORD 'genealogy_pass';
CREATE DATABASE genealogy_lab OWNER genealogy_user;
```

3. 在主工程里执行建表、索引和触发器：

```bash
cd "/home/xsy/mySchoolProject/Database Course lab/Database_Course_Experiment"
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/02_indexes.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/03_triggers.sql
```

4. 插入一个测试用户、一个族谱、几名成员、几条亲子关系和婚姻关系：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/05_seed_small.sql
```

5. 执行阶段一验收查询：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/06_stage1_check.sql
```

验收标准：

```text
psql 可以 SELECT 到用户、族谱、成员。
触发器能阻止父母出生年份晚于子女的错误关系。
核心查询 SQL 至少能在小数据上得到正确结果。
```

## 4. 阶段二：Qt 主工程能编译并连接数据库

目标：先跑起桌面程序，不急着补全所有按钮。

要做：

```bash
cd "/home/xsy/mySchoolProject/Database Course lab/Database_Course_Experiment/test"
cmake -S . -B build-wsl -G Ninja
cmake --build build-wsl
env XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir QT_QPA_PLATFORM=wayland ./build-wsl/GenealogySystem
```

重点检查：

```text
LoginWindow 是否能注册和登录。
MainWindow 是否能加载当前用户可访问的族谱。
成员列表是否能显示。
Dashboard 是否能显示人数和关系数量。
```

如果登录失败，优先检查 `DatabaseManager` 的连接参数和 PostgreSQL 是否启动。

## 5. 阶段三：补齐基础 CRUD

目标：完成 PPT 中“族谱、成员管理增删改查”和“邀请协作”。

优先级：

1. 成员编辑和删除
   当前已有新增成员和成员列表，下一步补 `编辑成员`、`删除成员` 按钮。对应 `MemberDao::update` 和 `MemberDao::remove` 已存在。

2. 族谱新增、编辑、删除
   当前 `GenealogyDao::insert` 已有雏形，需要补 `update/remove`，UI 上增加族谱管理页或在顶部族谱下拉框旁加按钮。

3. 协作者邀请
   新增 DAO 函数：按用户名查用户，把 `(genealogy_id, user_id, role)` 插入 `genealogy_collaborators`。

4. 亲子关系和婚姻关系维护
   增加简单表单：输入父母 ID、子女 ID、关系类型；输入两名成员 ID、结婚年份。优先保证功能可演示，不必追求复杂交互。

验收标准：

```text
界面上能完成成员和族谱的增删改查。
用户只能看到自己创建或被邀请的族谱。
成员支持姓名模糊查询。
```

## 6. 阶段四：核心查询和树形展示

目标：把数据库课程的查询能力通过界面展示出来。

当前框架已具备：

```text
TreeService::getChildren
TreeService::getAncestors
TreeService::findRelationPath
MainWindow 的树形预览、祖先查询、亲缘链路页面
```

下一步要完善：

1. 后代树形预览
   现在是逐层查子女并递归构造 `QTreeWidgetItem`。小数据没问题，大数据演示时要限制展开层数，避免一次展开几万人。

2. 祖先查询
   使用 PostgreSQL `WITH RECURSIVE` 查询所有祖先。报告中重点解释：族谱深度不确定，所以不能固定 JOIN 多少次。

3. 亲缘链路查询
   C++ 中用 BFS。报告中重点解释：亲子关系可以看成无向图，BFS 找到的是边数最短的亲缘通路。

4. 核心 SQL 文件
   `Database_Course_Experiment/sql/04_core_queries.sql` 已覆盖 PPT 的五个 SQL 需求，后续只需要在数据库中跑出截图并贴到报告。

验收标准：

```text
能输入成员 ID 查询后代树。
能输入成员 ID 查询所有祖先。
能输入两个成员 ID 输出亲缘链路。
核心 SQL 在 psql 中可单独执行并截图。
```

## 7. 阶段五：10 万级数据、COPY 导入导出

目标：满足 PPT 的 Data Engineering 要求。

根目录 `scripts/generate_data.py` 已经能生成 10 个族谱和 10 万成员，但它现在匹配的是根目录 SQL 字段，不完全匹配主工程 SQL。下一步建议复制或改写成：

```text
Database_Course_Experiment/tools/generate_data.py
Database_Course_Experiment/sql/05_load_csv.sql
Database_Course_Experiment/sql/06_export_branch.sql
```

改写重点：

```text
id -> user_id / genealogy_id / member_id / marriage_id
name/surname -> title/family_surname
owner_user_id -> creator_user_id
generation_no -> generation
spouse1_id/spouse2_id -> person1_id/person2_id
married_year/ended_year -> marriage_year/divorce_year
parent_child_relations 需要包含 genealogy_id
```

验收标准：

```text
至少 10 个族谱。
全系统 members 不少于 100000。
至少 1 个族谱 members 超过 50000。
单个族谱至少 30 代。
每个族谱内成员至少有亲缘关系。
可以用 COPY 或 \copy 批量导入。
可以导出某个分支的 CSV 备份文件。
```

## 8. 阶段六：索引和性能对比

PPT 要求两个索引方向：

```text
根据姓名模糊查询
根据父节点 ID 查询子节点
```

主工程已有：

```sql
CREATE EXTENSION IF NOT EXISTS pg_trgm;
CREATE INDEX IF NOT EXISTS idx_members_name_trgm ON members USING gin (name gin_trgm_ops);
CREATE INDEX IF NOT EXISTS idx_parent_child_parent_id ON parent_child_relations(parent_id);
```

性能实验建议固定一个“四代查询”：

```text
查询某曾祖父的所有曾孙
```

截图流程：

1. 无索引：删除相关索引，执行 `EXPLAIN (ANALYZE, BUFFERS)`，保存时间和执行计划。
2. 有索引：重建索引并 `ANALYZE`，再次执行同一 SQL。
3. 报告中对比执行时间、扫描方式、Buffer 使用情况。

验收标准：

```text
报告里有无索引和有索引两张 EXPLAIN 截图。
能解释为什么 parent_id 索引对递归后代查询有效。
能解释为什么 pg_trgm 适合 LIKE '%关键词%' 这种模糊查询。
```

## 9. 阶段七：报告和演示材料

报告需要覆盖：

```text
系统需求分析
E-R 图
关系模型
3NF 或 BCNF 分析
主键、外键、CHECK、触发器
RDBMS 名称和版本
数据生成方法
COPY 导入导出
核心 SQL 及执行截图
索引设计及性能对比
应用界面截图
个人工作说明
```

演示顺序建议：

```text
登录/注册
查看可访问族谱
Dashboard
成员新增、修改、删除、模糊查询
族谱协作邀请
树形预览
祖先查询
亲缘链路查询
展示 10 万数据统计
展示 EXPLAIN 性能对比
```

## 10. 下一次最合适的实际编码任务

建议下一步从下面两件事中选一个开始：

1. 数据库优先
   统一并初始化 `Database_Course_Experiment/sql`，插入小测试数据，保证所有核心 SQL 都能跑通。

2. 程序优先
   编译运行 `Database_Course_Experiment/test`，修复连接和字段问题，先让登录、族谱加载、成员列表可用。

更推荐先做数据库优先。原因是 C++/Qt 所有界面都依赖稳定表结构，如果表结构还没定，后面 DAO 会反复改。
