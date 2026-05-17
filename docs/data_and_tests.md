# 数据工程与阶段验收

本文档整合阶段验收、10 万级数据生成、COPY 导入、分支导出和性能测试结果。

## 1. 阶段一：数据库初始化

执行：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/02_indexes.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/03_triggers.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/05_seed_small.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/06_stage1_check.sql
```

小数据验证结果：

```text
users: 2
genealogies: 1
genealogy_collaborators: 1
members: 7
parent_child_relations: 7
marriages: 2
```

触发器已验证能拒绝女性成员作为 `father` 的错误亲子关系。

## 2. 阶段二：Qt 编译与连接

执行：

```bash
cmake -S . -B build -G Ninja
cmake --build build
./build/Stage2Smoke
```

通过输出：

```text
PASS: login, genealogy loading, dashboard, and member list are working.
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
  -f sql/08_load_generated_csv.sql
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
  -f sql/09_export_branch.sql
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

结论：`parent_child_relations(parent_id)` 索引能显著优化父节点查子节点的递归后代查询。
