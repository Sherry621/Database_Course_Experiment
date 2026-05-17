# 实验报告提交材料说明

本文档针对老师要求的实验报告内容，逐项说明本项目已经准备好的材料、需要截图的位置、需要执行的命令，以及最终应提交的文件。

## 1. 实验报告应包含的内容

老师要求：

```text
E-R 图
关系模型
对关系模型是 3NF 的分析说明
创建的索引以及约束等的说明
数据的生成方法
所使用的 RDBMS 名称和版本说明
查询对应的 SQL 语句
在数据库中执行 SQL 语句后得到的结果截图
个人所做工作说明
自编数据生成工具源码
数据库导出或备份文件
```

本项目对应材料主要位于：

```text
docs/report_materials.md
docs/sql_code_analysis.md
docs/data_and_tests.md
docs/code_structure_and_principles.md
sql/
tools/generate_data.py
generated_data/
```

## 2. E-R 图

对应文档：

```text
docs/report_materials.md
```

对应章节：

```text
## 2. ER 图
```

操作方法：

1. 打开 `docs/report_materials.md`。
2. 复制 Mermaid ER 图代码。
3. 打开 Mermaid 在线编辑器：

```text
https://mermaid.live/
```

4. 粘贴 Mermaid 代码。
5. 导出 PNG，或直接截图放入实验报告。

报告中需要说明的实体：

```text
users
genealogies
genealogy_collaborators
members
parent_child_relations
marriages
```

联系类型说明：

```text
users 1:N genealogies
users M:N genealogies，通过 genealogy_collaborators 实现
genealogies 1:N members
genealogies 1:N parent_child_relations
genealogies 1:N marriages
members 1:N parent_child_relations，作为 parent
members 1:N parent_child_relations，作为 child
members M:N members，通过 marriages 表表示婚姻关系
members M:N members，通过 parent_child_relations 表表示亲子自关联
```

## 3. 关系模型

对应文档：

```text
docs/report_materials.md
```

对应章节：

```text
## 4. 关系模型
```

报告中可写：

```text
USERS(user_id PK, username UK, password_hash, real_name, email UK, created_at)

GENEALOGIES(genealogy_id PK, title, family_surname, revision_time, creator_user_id FK, description, created_at)

GENEALOGY_COLLABORATORS(genealogy_id PK/FK, user_id PK/FK, role, invited_at)

MEMBERS(member_id PK, genealogy_id FK, name, gender, birth_year, death_year, generation, biography, created_at)

PARENT_CHILD_RELATIONS(relation_id PK, genealogy_id FK, parent_id FK, child_id FK, relation_type)

MARRIAGES(marriage_id PK, genealogy_id FK, person1_id FK, person2_id FK, marriage_year, divorce_year, description)
```

## 4. 3NF 分析说明

对应文档：

```text
docs/report_materials.md
```

对应章节：

```text
## 7. 3NF 与 BCNF 分析
```

报告中可写：

```text
本系统各关系模式均满足第一范式。各字段均为原子值，不存在重复组或多值属性。

本系统各关系模式满足第二范式。对于单主键表，例如 users、genealogies、members、parent_child_relations、marriages，非主属性均完全依赖主键。对于 genealogy_collaborators，该表使用复合主键 (genealogy_id, user_id)，role 和 invited_at 完全依赖整个复合主键，不依赖其中某一部分。

本系统各关系模式满足第三范式。各表中的非主属性不依赖其他非主属性。例如用户真实姓名不决定邮箱，成员姓名不决定出生年份，婚姻年份不决定离婚年份。非主属性均直接依赖主键或候选键，不存在传递依赖。

因此，本系统关系模型至少满足 3NF。结合唯一约束和候选键分析，主要表也基本满足 BCNF。
```

## 5. 索引说明

对应文件：

```text
sql/02_indexes.sql
```

对应说明文档：

```text
docs/sql_code_analysis.md
```

报告中重点说明：

### 5.1 姓名模糊查询索引

```sql
CREATE EXTENSION IF NOT EXISTS pg_trgm;

CREATE INDEX IF NOT EXISTS idx_members_name_trgm
ON members USING GIN (name gin_trgm_ops);
```

作用：

```text
优化成员姓名模糊查询，例如 name LIKE '%张%'。
```

### 5.2 父节点查询子节点索引

```sql
CREATE INDEX IF NOT EXISTS idx_parent_child_parent_id
ON parent_child_relations(parent_id);
```

作用：

```text
优化根据父节点 ID 查询子女节点的操作，用于后代树、分支导出和四代后代查询。
```

### 5.3 子节点查询父节点索引

```sql
CREATE INDEX IF NOT EXISTS idx_parent_child_child_id
ON parent_child_relations(child_id);
```

作用：

```text
优化根据子节点向上追溯父母和祖先的操作。
```

其他索引：

```text
idx_genealogies_creator
idx_members_genealogy
idx_members_genealogy_generation
idx_parent_child_genealogy
idx_marriages_person1
idx_marriages_person2
```

## 6. 约束说明

对应文件：

```text
sql/01_schema.sql
sql/03_triggers.sql
```

### 6.1 主键

```text
users.user_id
genealogies.genealogy_id
members.member_id
parent_child_relations.relation_id
marriages.marriage_id
genealogy_collaborators(genealogy_id, user_id)
```

### 6.2 外键

```text
genealogies.creator_user_id -> users.user_id
genealogy_collaborators.genealogy_id -> genealogies.genealogy_id
genealogy_collaborators.user_id -> users.user_id
members.genealogy_id -> genealogies.genealogy_id
parent_child_relations.genealogy_id -> genealogies.genealogy_id
parent_child_relations.parent_id -> members.member_id
parent_child_relations.child_id -> members.member_id
marriages.genealogy_id -> genealogies.genealogy_id
marriages.person1_id -> members.member_id
marriages.person2_id -> members.member_id
```

### 6.3 CHECK 约束

```text
role IN ('editor', 'viewer')
gender IN ('M', 'F')
birth_year <= death_year
generation >= 1
relation_type IN ('father', 'mother')
parent_id <> child_id
person1_id <> person2_id
marriage_year <= divorce_year
```

### 6.4 触发器约束

对应文件：

```text
sql/03_triggers.sql
```

触发器 `trg_check_parent_child_relation` 检查：

```text
父母和子女必须属于同一个族谱。
father 关系要求父节点性别为男性。
mother 关系要求父节点性别为女性。
父母出生年份必须早于子女出生年份。
父母辈分必须早于子女辈分。
```

触发器 `trg_check_marriage_relation` 检查：

```text
婚姻双方不能是同一个人。
婚姻双方必须属于同一个族谱。
统一 person1_id 和 person2_id 顺序，避免重复婚姻记录。
```

## 7. 数据生成方法

本项目采用自编 Python 工具生成数据。

源码文件：

```text
tools/generate_data.py
```

生成命令：

```bash
python3 tools/generate_data.py --out generated_data --total-members 100000
```

生成结果：

```text
generated_data/users.csv
generated_data/genealogies.csv
generated_data/genealogy_collaborators.csv
generated_data/members.csv
generated_data/parent_child_relations.csv
generated_data/marriages.csv
```

满足要求：

```text
至少 10 个族谱。
整个系统不少于 100000 条成员数据。
至少一个族谱拥有 50000 以上成员。
单个族谱模拟至少 30 代传承关系。
每个族谱内成员至少与另一个成员存在亲缘关系。
```

导入命令：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -f sql/08_load_generated_csv.sql
```

## 8. RDBMS 名称和版本说明

本项目使用：

```text
PostgreSQL
```

验收前执行：

```bash
psql --version
```

把输出截图放入报告。例如：

```text
psql (PostgreSQL) xx.x
```

报告中写：

```text
本项目采用 PostgreSQL 作为关系数据库管理系统，使用 psql 客户端执行建表、导入导出、核心查询和性能分析脚本。
```

## 9. 查询对应的 SQL 语句

对应文件：

```text
sql/04_core_queries.sql
```

包含五个核心 SQL：

```text
1. 给定成员 ID，查询其配偶及所有子女。
2. 输入成员 ID，使用 Recursive CTE 查询所有历代祖先。
3. 统计某个家族中平均寿命最长的一代人。
4. 查询年龄超过 50 岁且没有配偶的男性成员。
5. 找出出生年份早于该辈分平均出生年份的成员。
```

报告中可直接复制 `sql/04_core_queries.sql` 中的 SQL，并配合执行结果截图。

## 10. SQL 执行结果截图

建议执行并截图：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v member_id=1 \
  -v genealogy_id=1 \
  -f sql/04_core_queries.sql
```

如果因为 `:member_id` 或 `:genealogy_id` 变量不能按预期替换，可以把 `sql/04_core_queries.sql` 中的变量临时替换为具体数字后执行截图。

建议截图内容：

```text
配偶及子女查询结果
递归祖先查询结果
平均寿命最长代查询结果
50 岁以上无配偶男性查询结果
早于同辈平均出生年份成员查询结果
```

## 11. 性能对比与 EXPLAIN 截图

对应文件：

```text
sql/10_performance_explain.sql
```

执行：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -v root_id=1 \
  -f sql/10_performance_explain.sql \
  > generated_data/performance_explain.txt
```

截图或提交：

```text
generated_data/performance_explain.txt
```

报告中说明：

```text
无 parent_id/child_id 索引时，四代后代查询耗时较高。
创建 parent_id/child_id 索引后，查询时间下降。
```

你之前实测结果：

```text
无索引：约 1.509 ms
有索引：约 0.519 ms
```

## 12. 数据库导出或备份文件

老师要求使用 RDBMS 提供的数据导出或备份工具导出数据库。

推荐使用 PostgreSQL 的 `pg_dump`：

```bash
mkdir -p generated_data

pg_dump "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -Fc \
  -f generated_data/genealogy_lab.backup
```

说明：

```text
-Fc 表示 PostgreSQL custom 格式备份。
generated_data/genealogy_lab.backup 是最终需要提交的数据库备份文件。
```

检查：

```bash
ls -lh generated_data/genealogy_lab.backup
```

如果需要导出 SQL 文本格式，也可以执行：

```bash
pg_dump "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -f generated_data/genealogy_lab.sql
```

## 13. 自编工具源码提交

需要提交：

```text
tools/generate_data.py
```

报告中说明：

```text
本项目使用 Python 编写数据生成工具，自动生成用户、族谱、协作者、成员、亲子关系和婚姻关系 CSV 文件，并通过 PostgreSQL \copy 命令批量导入数据库。
```

## 14. 个人所做工作说明

可按你的实际工作写：

```text
本人主要完成了族谱管理系统的数据库设计、关系模式转换、主外键和约束设计、核心 SQL 查询编写、索引设计、数据生成工具设计、COPY 批量导入、分支导出、EXPLAIN 性能对比，以及 Qt 图形界面中的登录注册、族谱管理、成员管理、关系维护、Dashboard、树形预览、祖先查询和亲缘链路查询功能。
```

如果是两人分工，可以写：

```text
成员 A：数据库设计、SQL 查询、数据生成、导入导出、索引优化和性能分析。
成员 B：Qt 图形界面、DAO/Service 封装、登录注册、族谱和成员管理、树形预览、祖先查询和亲缘链路查询。
```

## 15. 最终提交文件清单

建议提交：

```text
实验报告 PDF 或 Word
ER 图图片
SQL 执行结果截图
EXPLAIN 性能截图或 performance_explain.txt
数据库备份文件 generated_data/genealogy_lab.backup
分支导出文件 generated_data/branch_export.csv
数据生成源码 tools/generate_data.py
项目源码 src/
数据库脚本 sql/
运行说明 README.md
```

GitHub 仓库中不建议提交：

```text
build/
build-wsl/
generated_data/
*.backup
*.csv
```

但提交给老师的压缩包中可以单独包含：

```text
generated_data/genealogy_lab.backup
generated_data/branch_export.csv
generated_data/performance_explain.txt
```
