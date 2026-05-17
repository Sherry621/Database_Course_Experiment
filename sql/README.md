# SQL 脚本说明

本目录保存 PostgreSQL 数据库脚本，文件按执行顺序编号。当前没有必须删除的 SQL；其中 `05`、`06`、`07` 属于小数据、演示和阶段验收辅助脚本，正式 10 万级数据流程主要使用 `08`、`09`、`10`。

## 初始化脚本

| 文件 | 用途 | 是否仍使用 |
| --- | --- | --- |
| `01_schema.sql` | 创建 `users`、`genealogies`、`genealogy_collaborators`、`members`、`parent_child_relations`、`marriages` 六张表。 | 必需 |
| `02_indexes.sql` | 创建姓名模糊查询、族谱筛选、亲子关系、婚姻关系相关索引，并启用 `pg_trgm`。 | 必需 |
| `03_triggers.sql` | 创建亲子关系和婚姻关系触发器，检查同族谱、父母性别、出生年、辈分、婚姻双方等约束。 | 必需 |

初始化命令：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/02_indexes.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/03_triggers.sql
```

## 查询脚本

| 文件 | 用途 | 是否仍使用 |
| --- | --- | --- |
| `04_core_queries.sql` | 配偶及子女、递归祖先、平均寿命最长代、50 岁以上无配偶男性、早于同辈平均出生年份成员。 | 报告使用 |

注意：`04_core_queries.sql` 中使用 `:member_id`、`:genealogy_id` 这类 psql 变量占位。截图时可以在 psql 中传变量，或复制单条 SQL 后替换成具体 ID。

## 小数据和演示脚本

| 文件 | 用途 | 是否仍使用 |
| --- | --- | --- |
| `05_seed_small.sql` | 插入阶段一小规模固定数据，便于验证表结构、触发器和核心查询。 | 辅助保留 |
| `06_stage1_check.sql` | 阶段一验收查询，检查表行数、权限族谱、配偶子女、递归祖先和触发器拒绝错误数据。 | 辅助保留 |
| `07_seed_demo.sql` | 插入较丰富的 UI 演示数据，包括多个用户、多个族谱、协作者、成员、亲子和婚姻关系。 | 演示保留 |

## 10 万级数据工程脚本

| 文件 | 用途 | 是否仍使用 |
| --- | --- | --- |
| `08_load_generated_csv.sql` | 清空业务表并使用 `\copy` 导入 `generated_data/` 下的 10 万级 CSV。 | 正式数据导入 |
| `09_export_branch.sql` | 导出某成员后代分支到 `generated_data/branch_export.csv`，支持 `root_id` 和 `max_depth` 变量。 | 导出备份 |
| `10_performance_explain.sql` | 对比有无 `parent_child_relations(parent_id)` 索引时四代后代查询的 EXPLAIN ANALYZE。 | 性能报告 |

10 万级数据流程：

```bash
python3 tools/generate_data.py --out generated_data --total-members 100000
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/08_load_generated_csv.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -v root_id=1 -v max_depth=6 -f sql/09_export_branch.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -v root_id=1 -f sql/10_performance_explain.sql
```
