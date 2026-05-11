# 数据工程与性能测试阶段结果

执行时间：2026-05-10

## 1. 10 万级数据生成

执行命令：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku"
python3 tools/generate_data.py --out generated_data --total-members 100000
```

生成结果：

```text
users=5
genealogies=10
members=100000
parent_child_relations=189350
marriages=12476
```

## 2. COPY 导入

执行命令：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -f sql/08_load_generated_csv.sql
```

导入结果：

```text
genealogies=10
members=100000
parent_child_relations=189350
marriages=12476
```

最大族谱：

```text
genealogy_id=1
member_count=55000
max_generation=35
```

## 3. 分支导出

执行命令：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -v root_id=1 \
  -v max_depth=6 \
  -f sql/09_export_branch.sql
```

导出文件：

```text
generated_data/branch_export.csv
```

本次导出：

```text
34 条成员记录
35 行 CSV，包含表头
```

## 4. EXPLAIN 性能对比

执行命令：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -v root_id=1 \
  -f sql/10_performance_explain.sql
```

无 `parent_id` 索引：

```text
Seq Scan on parent_child_relations
Execution Time: 86.564 ms
Buffers: shared hit=4888
```

有 `idx_parent_child_parent_id` 索引：

```text
Index Scan using idx_parent_child_parent_id
Execution Time: 0.298 ms
Buffers: shared hit=39 read=6
```

结论：

```text
parent_child_relations(parent_id) 索引能显著优化从父节点查子节点的递归后代查询。
```
