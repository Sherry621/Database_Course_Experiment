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
  -v root_id=1 \
  -v max_depth=6 \
  -f sql/06_export_branch.sql
```

输出：

```text
generated_data/branch_export.csv
```

本次测试导出 34 条成员记录，CSV 共 35 行含表头。

## 6. EXPLAIN 性能对比

执行：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
  -v ON_ERROR_STOP=1 \
  -v root_id=1 \
  -f sql/07_performance_explain.sql
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

结论：`parent_child_relations(parent_id)` 索引能显著优化父节点查子节点的递归后代查询。
