# 最终验收资料包

本文档用于最终提交和现场演示前整理材料。建议把本文档作为验收总索引，配合 `docs/report_materials.md`、`docs/data_and_tests.md`、`docs/code_structure_and_principles.md`、`docs/sql_code_analysis.md`、`docs/experiment_report_submission_guide.md`、`docs/acceptance_q_and_a.md` 和实际截图完成实验报告。

## 1. 项目基本信息

项目名称：

```text
“寻根溯源”族谱管理系统设计与实现
```

开发工具：

```text
C++17
Qt6 Widgets
PostgreSQL
CMake
Ninja
Python 数据生成脚本
```

RDBMS：

```text
PostgreSQL
```

建议验收前截图记录版本：

```bash
psql --version
qmake6 --version
cmake --version
ninja --version
```

## 2. 功能完成对照

| 老师要求 | 项目完成情况 | 对应文件或界面 |
|---|---|---|
| 用户登录 | 已完成 | 登录窗口 |
| 用户注册 | 已完成 | 登录窗口注册入口 |
| 用户只能看到自己创建或受邀族谱 | 已完成 | `GenealogyDao::findAccessibleByUser` |
| Dashboard 总人数、男女比例 | 已完成 | Dashboard 页面 |
| 族谱增删改查 | 已完成 | 族谱管理页面 |
| 邀请他人参与族谱 | 已完成 | 族谱管理页面 |
| 协作者权限 | 已完成 | 创建者、编辑者、查看者 |
| 成员增删改查 | 已完成 | 成员管理页面 |
| 姓名模糊查找 | 已完成 | 成员管理页面 |
| 成员表格排序 | 已完成 | 成员管理页面 |
| 树形预览 | 已完成 | 图形化后代树 |
| 树节点详情 | 已完成 | 点击后代树节点 |
| 祖先查询 | 已完成 | 祖先查询页面 |
| 亲缘关系查询 | 已完成 | 图形化亲缘链路 |
| ER 图 | 已完成 | `docs/report_materials.md` |
| 关系模式 | 已完成 | `docs/report_materials.md` |
| 3NF/BCNF 分析 | 已完成 | `docs/report_materials.md` |
| 主键、外键、CHECK | 已完成 | `sql/01_schema.sql` |
| 父母出生年份早于子女 | 已完成 | `sql/03_triggers.sql` |
| 10 万级数据生成 | 已完成 | `tools/generate_data.py` |
| COPY 批量导入 | 已完成 | `sql/08_load_generated_csv.sql` |
| 分支导出 | 已完成 | `sql/09_export_branch.sql` |
| 核心 SQL 查询 | 已完成 | `sql/04_core_queries.sql` |
| 索引设计 | 已完成 | `sql/02_indexes.sql` |
| EXPLAIN 性能对比 | 已完成 | `sql/10_performance_explain.sql` |

## 3. 验收前运行命令

从 WSL 进入项目根目录：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku"
```

初始化小数据：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/02_indexes.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/03_triggers.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/05_seed_small.sql
```

编译：

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

自动检查：

```bash
./build/Stage2Smoke
```

运行图形界面：

```bash
./scripts/run_wsl.sh
```

测试账号：

```text
admin / 123456
editor1 / 123456
viewer1 / 123456
```

如需完整演示权限角色，建议先导入演示数据：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/07_seed_demo.sql
```

## 4. 大数据与性能材料

生成 10 万级 CSV：

```bash
python3 tools/generate_data.py --out generated_data --total-members 100000
```

导入：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/08_load_generated_csv.sql
```

分支导出：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -v root_id=1 -v max_depth=6 -f sql/09_export_branch.sql
```

性能对比：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -v root_id=1 -f sql/10_performance_explain.sql
```

数据库备份：

```bash
pg_dump "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -Fc -f generated_data/genealogy_lab.backup
```

## 5. 报告截图清单

建议按下面顺序截图：

```text
1. PostgreSQL / Qt / CMake 版本截图
2. 建表、索引、触发器执行成功截图
3. ER 图截图
4. 登录界面截图
5. 注册界面截图
6. Dashboard 截图，重点包含总人数和男女比例
7. 族谱管理截图
8. 邀请协作者与权限选择截图
9. 成员管理截图，展示模糊查询和表头排序
10. 成员新增或编辑截图
11. 关系维护截图，展示亲子关系和婚姻关系
12. 图形化后代树截图
13. 点击后代树节点查看详情截图
14. 祖先查询截图
15. 图形化亲缘链路截图
16. 10 万数据生成截图
17. COPY 导入结果截图
18. 分支导出 CSV 截图
19. 核心 SQL 查询执行结果截图
20. EXPLAIN ANALYZE 有无索引对比截图
21. 自动冒烟测试 PASS 截图
```

## 6. 演示顺序

现场演示建议控制在 5 到 8 分钟：

```text
1. 介绍项目目标：多用户族谱管理、成员关系、递归查询和性能优化。
2. 登录 admin，展示 Dashboard 总人数、男女比例、最近成员和快捷入口。
3. 展示族谱管理：新增/编辑族谱、邀请协作者并选择编辑者或查看者。
4. 展示成员管理：姓名模糊搜索、按出生年或代数排序、查看详情。
5. 展示关系维护：添加亲子关系或婚姻关系。
6. 展示图形化后代树，点击节点查看成员详情。
7. 展示祖先查询。
8. 展示图形化亲缘链路。
9. 展示大数据生成、COPY 导入、分支导出和 EXPLAIN 性能对比材料。
10. 总结数据库设计：ER 图、关系模式、3NF/BCNF、索引和触发器。
```

## 7. 口头说明要点

### 7.1 数据库设计

可以这样说明：

```text
系统使用 users、genealogies、genealogy_collaborators、members、parent_child_relations、marriages 六张核心表。
成员之间的父子/母子关系通过 parent_child_relations 表表示，是 members 表的自关联。
婚姻关系通过 marriages 表表示，也是 members 表的自关联。
用户与族谱之间通过 genealogy_collaborators 表支持多用户协作。
```

### 7.2 递归查询

可以这样说明：

```text
祖先查询和分支导出使用 PostgreSQL Recursive CTE。
由于族谱层级深度不固定，递归 CTE 比固定层数 JOIN 更适合处理祖先和后代查询。
```

### 7.3 索引优化

可以这样说明：

```text
姓名模糊查询使用 pg_trgm + GIN 索引。
父节点查询子节点使用 parent_child_relations(parent_id) 索引。
性能脚本通过删除和重建索引，对四代后代查询执行 EXPLAIN ANALYZE，对比有无索引的执行计划和耗时。
```

### 7.4 权限控制

可以这样说明：

```text
创建者拥有族谱管理、协作者管理和数据维护权限。
编辑者可以维护成员和关系，但不能删除族谱或邀请协作者。
查看者只能浏览 Dashboard、成员、树形预览、祖先查询和亲缘链路。
界面按钮会根据权限自动禁用，写操作函数内部也有二次权限判断。
```

## 8. 提交文件建议

建议最终提交内容：

```text
src/
sql/
docs/
tools/
tests/
scripts/
CMakeLists.txt
README.md
```

需要额外提交给老师的材料：

```text
实验报告 PDF 或 Word
数据库备份文件 generated_data/genealogy_lab.backup
分支导出文件 generated_data/branch_export.csv
数据生成工具源码 tools/generate_data.py
关键 SQL 脚本 sql/*.sql
演示截图
```

不建议提交到 GitHub 的本地构建产物：

```text
build/
build-wsl/
generated_data/
*.backup
*.csv
```

## 9. 最终检查

提交前执行：

```bash
git status --short
cmake --build build
./build/Stage2Smoke
```

确认：

```text
程序能编译。
自动测试 PASS。
GUI 能启动。
报告截图齐全。
数据库备份和分支导出文件已生成。
```
