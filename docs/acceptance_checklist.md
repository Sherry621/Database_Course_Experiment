# 验收检查清单

本文档用于在提交代码、截图或现场演示前快速确认项目可复现、可运行、可说明。

## 1. 环境检查

在 WSL 终端执行：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku"
qmake6 --version
cmake --version
ninja --version
psql --version
pg_isready
```

预期：

```text
Qt6、CMake、Ninja、PostgreSQL 客户端可用。
PostgreSQL 服务返回 accepting connections。
```

## 2. 数据库初始化检查

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/02_indexes.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/03_triggers.sql
```

预期：

```text
表结构创建成功。
索引创建成功。
触发器创建成功。
```

## 3. C++ 构建检查

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

预期生成：

```text
build/GenealogySystem
```

## 4. GUI 运行检查

如果需要中文输入法，先执行：

```bash
./scripts/setup_fcitx_wsl.sh
```

启动 GUI：

```bash
./scripts/run_wsl.sh
```

测试账号：

```text
username: admin
password: 123456
```

界面检查：

```text
可以登录。
可以看到可访问族谱。
Dashboard 有统计数据。
成员列表可以显示。
成员搜索可以使用。
成员新增、编辑、删除可用。
族谱新增、编辑、删除可用。
亲子关系和婚姻关系维护可用。
后代树形预览可用。
祖先查询可用。
亲缘链路查询可用。
```

## 6. 数据工程检查

生成 10 万级数据：

```bash
python3 tools/generate_data.py --out generated_data --total-members 100000
```

导入：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/05_load_generated_csv.sql
```

分支导出：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -v root_id=1 -v max_depth=6 -f sql/06_export_branch.sql
```

性能测试：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -v root_id=1 -f sql/07_performance_explain.sql
```

报告截图至少应包含：

```text
10 万级数据导入结果。
最大族谱成员数和最大代数。
分支导出 CSV 文件。
无 parent_id 索引的 EXPLAIN ANALYZE。
有 parent_id 索引的 EXPLAIN ANALYZE。
```

## 7. Git 提交前检查

```bash
git status --short
```

不应提交：

```text
build/
generated_data/
*.csv
*.backup
```

确认无误后：

```bash
git add .
git commit -m "Prepare acceptance-ready genealogy system"
```
