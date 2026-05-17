# SQL 脚本说明

本目录只保留最终按 PPT 10W 数据验收需要的 PostgreSQL 脚本。文件按执行顺序编号，`01` 到 `07` 连续排列。

## 1. 初始化脚本

| 文件 | 用途 | 是否必需 |
| --- | --- | --- |
| `01_schema.sql` | 创建 `users`、`genealogies`、`genealogy_collaborators`、`members`、`parent_child_relations`、`marriages` 六张表，并定义主键、外键和 CHECK 约束。 | 必需 |
| `02_indexes.sql` | 创建姓名模糊查询、族谱筛选、亲子关系、婚姻关系相关索引，并启用 `pg_trgm`。 | 必需 |
| `03_triggers.sql` | 创建亲子关系和婚姻关系触发器，检查同族谱、父母性别、出生年、辈分、婚姻双方等复杂业务约束。 | 必需 |

初始化命令：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/02_indexes.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/03_triggers.sql
```

## 2. 课程核心查询

| 文件 | 用途 | 是否必需 |
| --- | --- | --- |
| `04_core_queries.sql` | PPT 要求的 5 类核心 SQL：配偶及子女、递归祖先、平均寿命最长代、50 岁以上无配偶男性、早于同辈平均出生年份成员。 | 必需 |

`04_core_queries.sql` 使用 `:member_id`、`:genealogy_id` 这类 psql 变量占位。截图时可以传入变量：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -v member_id=1579 \
  -v genealogy_id=1 \
  -f sql/04_core_queries.sql
```

## 3. 10W 数据工程脚本

| 文件 | 用途 | 是否必需 |
| --- | --- | --- |
| `05_load_generated_csv.sql` | 清空业务表并使用 `\copy` 导入 `generated_data/` 下的 10W 级 CSV。 | 必需 |
| `06_export_branch.sql` | 导出某成员后代分支备份，包含分支成员、分支内亲子关系和分支内婚姻关系三份 CSV，支持 `root_id` 和 `max_depth` 变量。 | 必需 |
| `07_performance_explain.sql` | 对比有无 `parent_child_relations(parent_id)` 索引时查询某曾祖父所有曾孙的 `EXPLAIN ANALYZE`。 | 必需 |

10W 数据流程：

```bash
python3 tools/generate_data.py --out generated_data --total-members 100000
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/05_load_generated_csv.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -v root_id=168 -v max_depth=4 -f sql/06_export_branch.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -v root_id=168 -v output_file=generated_data/performance_explain.txt -f sql/07_performance_explain.sql
```

分支导出可控参数：

- `root_id`：要导出的分支根成员 ID。换成其他成员 ID，就会导出另一个成员的后代分支。
- `max_depth`：向下导出的最大后代层数。`0` 只导出根成员，`1` 导出根成员和子女，依此类推。

在同一个数据库状态下，使用相同的 `root_id` 和 `max_depth`，导出的 CSV 内容是确定的；如果重新生成数据、重新导入数据，或选择不同根成员/深度，导出结果会不同。

性能分析输出文件：

- `07_performance_explain.sql` 默认写入 `generated_data/performance_explain.txt`。
- 可以通过 `-v output_file=路径` 改成其他输出文件。

## 4. 清理说明

早期阶段一和小数据 UI 演示脚本已经删除。最终验收只保留 10W 数据生成、导入、导出和性能分析流程。
