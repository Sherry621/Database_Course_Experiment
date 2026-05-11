# 数据工程脚本说明

本文档说明 10 万级数据生成、COPY 导入、分支导出和 EXPLAIN 性能测试的执行方式。

## 1. 生成 10 万级 CSV

执行目录：

```bash
cd Database_Course_Experiment
```

生成默认 100000 名成员：

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

脚本保证：

```text
10 个族谱
全系统不少于 100000 名成员
第 1 个族谱超过 50000 名成员
第 1 个族谱有 35 代
其余族谱有 12 代
每个族谱都有亲缘关系
```

## 2. COPY 导入

导入会清空现有业务表，然后加载 `generated_data/` 下的 CSV：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -f sql/08_load_generated_csv.sql
```

导入后脚本会输出主要表行数和最大族谱规模。

## 3. 导出分支备份

默认导出 `member_id = 1` 的后代分支：

```bash
mkdir -p generated_data
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -f sql/09_export_branch.sql
```

指定根成员和最大导出层数：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v root_id=1 \
  -v max_depth=6 \
  -f sql/09_export_branch.sql
```

导出文件固定为：

```text
generated_data/branch_export.csv
```

## 4. 性能对比

比较“查询某曾祖父所有曾孙”的四代递归查询在有无索引时的执行计划：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v root_id=1 \
  -f sql/10_performance_explain.sql
```

报告中需要截取两段输出：

```text
four-generation query WITHOUT parent_id index
four-generation query WITH parent_id index
```

重点比较：

```text
Execution Time
Buffers
是否使用 idx_parent_child_parent_id
```
