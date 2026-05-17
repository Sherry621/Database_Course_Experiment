# 数据工程与验收

本文档整合最终 10W 数据生成、COPY 导入、分支导出和性能测试结果。

## 1. 数据库初始化

执行：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/02_indexes.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/03_triggers.sql
```

初始化内容：

```text
01_schema.sql    建表、主键、外键、CHECK 约束
02_indexes.sql   姓名模糊查询、亲子关系和统计查询相关索引
03_triggers.sql  亲子关系和婚姻关系业务约束触发器
```

## 2. Qt 编译

执行：

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

## 3. 10 万级数据生成

执行：

```bash
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

亲缘关系覆盖验证：

```text
每个族谱 isolated_members=0
```

## 4. COPY 导入

执行：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -f sql/05_load_generated_csv.sql
```

导入后最大族谱：

```text
genealogy_id=1
member_count=55000
max_generation=35
```

## 5. 分支导出

执行：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -v root_id=168 \
  -v max_depth=4 \
  -f sql/06_export_branch.sql
```

输出：

```text
generated_data/branch_members.csv
generated_data/branch_parent_child_relations.csv
generated_data/branch_marriages.csv
```

其中 `branch_members.csv` 保存分支成员，`branch_parent_child_relations.csv` 保存分支内部亲子边，`branch_marriages.csv` 保存分支内部婚姻关系。这样比单独导出成员列表更接近“分支备份文件”的含义。

导出分支由 `root_id` 和 `max_depth` 控制。同一数据库状态下，相同参数会导出相同内容；更换 `root_id` 可以导出其他成员的后代分支，更换 `max_depth` 可以控制导出层数。

## 6. EXPLAIN 性能对比

执行：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -v root_id=168 \
  -v output_file=generated_data/performance_explain.txt \
  -f sql/07_performance_explain.sql
```

完整输出会保存到：

```text
generated_data/performance_explain.txt
```

无 `parent_id` 索引：

```text
Seq Scan on parent_child_relations
Execution Time: 71.234 ms
Buffers: shared hit=3747
```

有 `idx_parent_child_parent_id` 索引：

```text
Index Scan using idx_parent_child_parent_id
Execution Time: 0.305 ms
Buffers: shared hit=166 read=13
```

结论：PPT 要求的“查询某曾祖父的所有曾孙”对应递归深度 `depth=3`，`parent_child_relations(parent_id)` 索引能显著优化该类父节点查子节点的递归查询。
