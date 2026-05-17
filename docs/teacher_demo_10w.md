# 10W 数据验收演示流程

本文档按课程 PPT《第06章 数据库应用实践》第 31-38 页的要求整理，用于向老师演示《寻根溯源族谱管理系统》。演示时不使用小数据脚本，统一使用 10W 条模拟成员数据。

## 1. PPT 要求对照

| PPT 要求 | 当前项目实现 | 完成状态 | 演示/报告材料 |
| --- | --- | --- | --- |
| 多用户登录，用户只能看到自己创建或受邀族谱 | 登录注册、创建者族谱、协作者族谱加载 | 已完成 | Qt 登录窗口、Dashboard 当前用户与可访问族谱 |
| 用户注册制 | 登录窗口支持注册 | 已完成 | 登录窗口点击“注册” |
| Dashboard 显示家族总人数、男女比例 | Dashboard 显示成员总数、男女人数、最大代数、血缘关系数、婚姻关系数 | 已完成 | Qt Dashboard |
| 族谱增删改查 | 族谱管理页支持新增、编辑、删除 | 已完成 | Qt 族谱管理 |
| 族谱邀请他人参与创作 | 族谱管理页支持邀请协作者 | 已完成 | Qt 族谱管理“邀请协作者” |
| 成员增删改查 | 成员管理页支持新增、编辑、删除、详情查看 | 已完成 | Qt 成员管理 |
| 人名模糊查找 | 成员管理页按姓名关键词搜索；数据库使用 `pg_trgm` GIN 索引 | 已完成 | Qt 搜索框、`sql/02_indexes.sql` |
| 树形预览某分支 | 树形预览页按根成员 ID 和显示层数生成后代树 | 已完成 | Qt 树形预览 |
| 输入人物 ID 查询祖先 | 祖先查询页按成员 ID 显示祖先列表 | 已完成 | Qt 祖先查询 |
| 输入两个人 ID 查询亲缘链路 | 亲缘链路页按两个成员 ID 查询通路并显示姓名和 ID | 已完成 | Qt 亲缘链路 |
| ER 图、关系模式、3NF/BCNF 分析 | 已整理报告材料 | 已完成 | `docs/report_materials.md` |
| 主键、外键、CHECK 约束 | 建表脚本包含主外键、性别、年份、代数等 CHECK | 已完成 | `sql/01_schema.sql` |
| 父亲出生年份早于子女等复杂约束 | 触发器检查同族谱、父母性别、出生年、代数、婚姻双方 | 已完成 | `sql/03_triggers.sql` |
| 至少 10 个族谱 | 生成脚本固定生成 10 个族谱 | 已完成 | `tools/generate_data.py`、导入后统计 |
| 至少一个族谱 50000+ 成员 | 第 1 个族谱 55000 成员 | 已完成 | 导入后统计 |
| 系统不少于 100000 条成员数据 | 当前生成 100000 条成员数据 | 已完成 | `generated_data/members.csv`、导入后统计 |
| 每个族谱成员至少与另一个成员有亲缘关系 | 生成脚本为每个族谱按代建立父母子女关系 | 已完成 | `parent_child_relations` 统计 |
| 单个族谱至少 30 代传承 | 第 1 个族谱 35 代 | 已完成 | 导入后 `max_generation=35` |
| COPY 批量导入 CSV | 使用 PostgreSQL `\copy` 导入 6 个 CSV | 已完成 | `sql/05_load_generated_csv.sql` |
| 导出某分支备份文件 | 使用递归 CTE 导出分支成员、分支亲子关系、分支婚姻关系 CSV | 已完成 | `sql/06_export_branch.sql` |
| 核心 SQL：配偶及子女 | `sql/04_core_queries.sql` 第 1 条 | 已完成 | psql 截图 |
| 核心 SQL：递归祖先 | `sql/04_core_queries.sql` 第 2 条 | 已完成 | psql 截图 |
| 核心 SQL：平均寿命最长的一代 | `sql/04_core_queries.sql` 第 3 条 | 已完成 | psql 截图 |
| 核心 SQL：50 岁以上无配偶男性 | `sql/04_core_queries.sql` 第 4 条 | 已完成 | psql 截图 |
| 核心 SQL：早于同代平均出生年份成员 | `sql/04_core_queries.sql` 第 5 条 | 已完成 | psql 截图 |
| 姓名模糊查询索引 | `idx_members_name_trgm` | 已完成 | `sql/02_indexes.sql` |
| 父节点 ID 查询子节点索引 | `idx_parent_child_parent_id` | 已完成 | `sql/02_indexes.sql` |
| 有无索引查询某曾祖父所有曾孙的性能对比与 EXPLAIN | 脚本先删除索引测试，再重建索引测试 | 已完成 | `sql/07_performance_explain.sql` |
| 提交数据生成工具源码 | Python 生成脚本已保留 | 已完成 | `tools/generate_data.py` |
| 提交数据库导出或备份文件 | 分支导出 CSV 已支持 | 已完成 | `generated_data/branch_*.csv` |

结论：PPT 中的实现类要求已经完成。最后需要整理的是最终实验报告截图和现场讲解顺序。

## 2. 演示前准备

进入项目目录：

```bash
cd "/home/xsy/mySchoolProject/Database Course lab/Database_Course_Experiment"
```

确认 PostgreSQL 正常：

```bash
sudo service postgresql start
pg_isready
```

构建程序：

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

如果数据库不是 10W 数据状态，重新生成并导入。项目已删除早期小数据脚本，正式演示只走 10W 数据流程。

```bash
python3 tools/generate_data.py --out generated_data --total-members 100000
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -f sql/05_load_generated_csv.sql
```

确认 10W 数据规模：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -c "SELECT genealogy_id, count(*) AS member_count, max(generation) AS max_generation
      FROM members
      GROUP BY genealogy_id
      ORDER BY member_count DESC;"
```

期望重点结果：

```text
genealogy_id=1  member_count=55000  max_generation=35
genealogy_id=2..10  member_count=5000
```

当前项目已验证的 10W 数据规模：

```text
genealogies: 10
members: 100000
parent_child_relations: 189350
marriages: 12476
isolated_members: 0
```

验证“每个成员至少与另一个成员有亲缘关系”：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -c "SELECT m.genealogy_id,
             count(*) FILTER (
               WHERE NOT EXISTS (
                 SELECT 1
                 FROM parent_child_relations r
                 WHERE r.parent_id = m.member_id OR r.child_id = m.member_id
               )
             ) AS isolated_members,
             count(*) AS total_members
      FROM members m
      GROUP BY m.genealogy_id
      ORDER BY m.genealogy_id;"
```

期望每个族谱的 `isolated_members` 都为 `0`。

推荐演示账号：

```text
用户名：admin
密码：123456
```

推荐演示 ID：

```text
树形预览根成员：168
祖先查询成员：7754
亲缘链路成员 A：45575
亲缘链路成员 B：45584
核心 SQL member_id：1579
核心 SQL genealogy_id：1
```

## 3. 向老师演示的顺序

### 3.1 先说明项目和数据库规模

讲解要点：

```text
我们实现的是一个 Qt6 + C++17 + PostgreSQL 的桌面端族谱管理系统。
数据库当前导入的是 10W 条模拟成员数据，不是小数据。
系统包含 10 个族谱，其中第 1 个族谱有 55000 个成员、35 代传承。
```

可以展示：

- `README.md` 的当前功能列表。
- `sql/README.md` 的 SQL 脚本说明。
- `docs/report_materials.md` 的 ER 图和关系模型。

### 3.2 展示数据库建模和规范化设计

打开 `docs/report_materials.md`，依次展示：

```text
ER 图
实体与联系说明
关系模型
主键、外键和 CHECK 约束
触发器设计
3NF/BCNF 分析
索引设计
```

讲解要点：

```text
成员、族谱、用户是核心实体。
亲子关系和婚姻关系拆成独立关系表，避免在成员表中存列表。
协作者表用复合主键表示用户和族谱的多对多关系。
亲子和婚姻表保留 genealogy_id，方便按族谱过滤和级联管理，并由触发器保证一致性。
```

### 3.3 启动图形化系统

推荐使用脚本启动，保证中文输入环境一致：

```bash
./scripts/run_wsl.sh
```

登录：

```text
用户名：admin
密码：123456
```

### 3.4 Dashboard

操作：

```text
登录后停留在 Dashboard。
选择当前族谱为 赵氏族谱 或第 1 个族谱。
点击“刷新统计”。
```

展示点：

```text
当前登录用户
可访问族谱数量
当前族谱
成员总数
男性人数、女性人数
最大代数
血缘关系数
婚姻关系数
最近新增成员
```

对应 PPT：

```text
Dashboard：显示家族总人数、男女比例。
用户只能看到自己创建或受邀的族谱。
```

### 3.5 族谱管理

操作：

```text
左侧点击“族谱管理”。
展示当前族谱的谱名、姓氏、修谱时间、创建用户 ID 和备注。
点击“新增族谱”，说明可以新增。
点击“编辑族谱”，说明可以修改谱名、姓氏、修谱时间和备注。
点击“邀请协作者”，输入一个已存在用户名，例如 user2。
```

注意：

```text
演示 10W 数据时，不建议现场删除第 1 个大族谱。
删除功能可以说明已实现，但不必真的执行。
```

对应 PPT：

```text
族谱增删改查。
族谱能邀请他人参与创作。
```

### 3.6 成员管理和模糊查询

操作：

```text
左侧点击“成员管理”。
在姓名关键词中输入：赵
点击“搜索”。
再输入：赵磊
点击“搜索”。
选中一行成员，点击“成员详情”。
```

展示点：

```text
成员列表包含 ID、姓名、性别、出生年、死亡年、代数。
姓名模糊查询可以在 10W 数据上执行。
成员详情可以展示生平简介。
```

说明：

```text
为了避免 10W 数据一次性塞满表格，成员列表默认最多展示 500 行。
Dashboard 中的成员总数仍然显示数据库真实总数。
```

对应 PPT：

```text
成员增删改查。
成员查找支持人名模糊查找。
针对姓名模糊查询设计索引。
```

补充说明：

```text
姓名模糊查询使用 PostgreSQL pg_trgm + GIN 索引：idx_members_name_trgm。
```

### 3.7 关系维护

操作：

```text
左侧点击“关系维护”。
展示“血缘关系维护”和“婚姻关系维护”两个区域。
说明亲子关系需要输入父/母成员 ID、子女成员 ID 和关系类型。
说明婚姻关系需要输入成员 A ID、成员 B ID、结婚年份、离婚年份和备注。
```

建议只讲解，不在 10W 数据上随意新增关系，避免影响后续截图结果。

对应 PPT：

```text
记录父子/父女、母子/母女关系。
记录婚姻关系。
父亲出生年份必须早于子女等复杂业务约束。
```

补充说明：

```text
这些复杂约束由 sql/03_triggers.sql 中的触发器实现。
例如父亲必须是男性、母亲必须是女性、父母和子女必须属于同一族谱、父母出生年早于子女。
```

### 3.8 树形预览

操作：

```text
左侧点击“树形预览”。
根成员 ID 输入：168
最大显示层数输入：4
点击“生成后代树”。
```

展示点：

```text
系统以树形结构展示该成员的后代分支。
最大显示层数用于避免 10W 数据下界面一次性展开过大导致卡顿。
```

对应 PPT：

```text
树形预览：以层级结构或缩进列表展示某分支的人物关系树状图。
```

### 3.9 祖先查询

操作：

```text
左侧点击“祖先查询”。
成员 ID 输入：7754
点击“查询祖先”。
```

展示点：

```text
系统显示该成员父辈以上的祖先列表。
每行包含代数、姓名和 ID。
```

对应 PPT：

```text
人物祖先查询：输入一个人物 ID，以树状图显示其父辈以上的祖先。
递归查询使用 Recursive CTE。
```

### 3.10 亲缘链路查询

操作：

```text
左侧点击“亲缘链路”。
成员 A 输入：45575
成员 B 输入：45584
点击“查询链路”。
```

期望展示：

```text
存在亲缘链路：
赵杰0(#45575) -> ... -> 赵明0(#45584)
```

对应 PPT：

```text
输入两个人 ID，查询两者之间是否有亲缘关系连成的通路，如有请展示出来。
```

### 3.11 展示核心 SQL

先打开 `sql/04_core_queries.sql`，说明这里按 PPT 要求写了 5 类核心 SQL。因为第 4、5 条在 10W 数据下可能返回很多行，现场演示建议优先展示 SQL 文件，再执行整份脚本或单条复制后追加 `LIMIT 20` 截图。

执行整份脚本：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -v member_id=1579 \
  -v genealogy_id=1 \
  -f sql/04_core_queries.sql
```

如果担心输出过长，可以把终端输出保存到文件：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -v member_id=1579 \
  -v genealogy_id=1 \
  -f sql/04_core_queries.sql \
  -o generated_data/core_queries_output.txt
```

讲解 5 条 SQL 对应关系：

```text
1. 给定成员 ID 查询配偶及所有子女。
2. Recursive CTE 查询所有祖先。
3. 统计某个家族中平均寿命最长的一代人。
4. 查询年龄超过 50 岁且没有配偶的男性成员。
5. 找出出生年份早于该辈分平均出生年份的成员。
```

### 3.12 展示 COPY 导入

如果现场时间允许，可以重新演示导入命令。注意该脚本会清空并重导入业务表。

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -f sql/05_load_generated_csv.sql
```

讲解要点：

```text
数据由 tools/generate_data.py 生成 CSV。
导入脚本使用 PostgreSQL \copy 批量导入。
导入后执行 ANALYZE 更新统计信息。
```

### 3.13 展示分支导出

执行：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -v root_id=168 \
  -v max_depth=4 \
  -f sql/06_export_branch.sql
```

输出文件：

```text
generated_data/branch_members.csv
generated_data/branch_parent_child_relations.csv
generated_data/branch_marriages.csv
```

讲解要点：

```text
导出使用 Recursive CTE 找到某个根成员的后代分支。
导出的 CSV 不只包含成员，还包含分支内部亲子关系和婚姻关系，因此可以作为该分支的备份材料提交。
```

### 3.14 展示索引和 EXPLAIN 性能对比

执行：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -v root_id=168 \
  -f sql/07_performance_explain.sql
```

讲解要点：

```text
脚本先删除 parent_id/child_id 索引，执行“某曾祖父的所有曾孙”查询并输出 EXPLAIN ANALYZE。
然后重建 idx_parent_child_parent_id 和 idx_parent_child_child_id，再执行同样查询。
对比 Seq Scan 和 Index Scan，以及 Execution Time。
```

对应 PPT：

```text
针对根据父节点 ID 查询子节点设计索引。
记录有无索引情况下查询某曾祖父所有曾孙的执行时间差异，并提交 EXPLAIN 分析。
```

## 4. 最后总结话术

可以这样收尾：

```text
本系统按照 PPT 要求完成了图形化界面、数据库建模、规范化分析、约束和触发器、10W 数据生成、COPY 导入、分支导出、核心 SQL、索引优化和 EXPLAIN 性能对比。
数据库当前包含 10 个族谱、100000 个成员，其中最大族谱 55000 人、35 代。
图形界面可以完成登录注册、族谱管理、成员管理、关系维护、树形预览、祖先查询和亲缘链路查询。
报告材料中已经整理了 ER 图、关系模型、3NF/BCNF 分析、索引和约束说明。
```

## 5. 截图清单

最终报告建议截图：

```text
10W 数据生成命令输出
COPY 导入后的统计结果
Dashboard 统计页面
成员模糊查询页面
族谱管理页面
关系维护页面
树形预览页面
祖先查询页面
亲缘链路查询页面
核心 SQL 执行结果
分支导出 CSV 文件
EXPLAIN ANALYZE 有无索引对比
ER 图和关系模型
```
