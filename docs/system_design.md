# 系统设计说明

本文档说明《寻根溯源族谱管理系统》的总体设计。详细 ER 图、关系模型、主外键约束和 3NF/BCNF 分析见 `docs/report_materials.md`。

## 1. 系统目标

系统面向族谱资料的录入、维护、查询和展示，核心目标包括：

```text
用户注册与登录
族谱创建、编辑、删除和协作访问
成员信息维护
亲子关系和婚姻关系维护
Dashboard 统计展示
后代树形预览
祖先递归查询
成员亲缘链路查询
大规模数据导入、分支导出和索引性能分析
```

## 2. 技术架构

```text
Qt Widgets UI
    |
Service 业务层
    |
DAO 数据访问层
    |
Qt SQL / PostgreSQL
    |
SQL 表结构、索引、触发器、递归查询
```

技术栈：

- C++17
- Qt6 Widgets
- Qt SQL
- PostgreSQL
- CMake + Ninja
- WSL2 Ubuntu 24.04

## 3. 目录结构

```text
.
├── README.md
├── CMakeLists.txt
├── docs/
├── sql/
├── src/
├── tools/
└── scripts/
```

关键目录职责：

- `src/db/`：数据库连接管理和 DAO。
- `src/model/`：用户、族谱、成员等数据模型。
- `src/service/`：认证、统计、树形结构和亲缘链路业务逻辑。
- `src/ui/`：Qt Widgets 界面。
- `sql/`：建表、索引、触发器、核心查询、导入导出和性能分析脚本。
- `tools/`：10 万级模拟数据生成工具。
- `scripts/`：WSL 启动和中文输入法辅助脚本。

## 4. 数据库设计概览

主要数据表：

- `users`：系统用户。
- `genealogies`：族谱基本信息。
- `genealogy_collaborators`：族谱协作者关系。
- `members`：族谱成员。
- `parent_child_relations`：亲子关系。
- `marriages`：婚姻关系。

设计原则：

- 用户、族谱、成员使用代理主键，便于程序引用。
- 协作者表使用复合主键 `(genealogy_id, user_id)` 表达多对多关系。
- 亲子关系和婚姻关系拆为独立关系表，避免在成员表中存储非原子列表。
- 亲子、婚姻表保留 `genealogy_id`，便于按族谱过滤和级联管理，并通过触发器保证与成员所属族谱一致。
- SQL 标准以 `sql/01_schema.sql` 为准。

## 5. 业务模块

### 用户与权限

用户可以注册和登录。系统加载当前用户创建的族谱，以及协作者表中授权访问的族谱。协作者角色分为 `editor` 和 `viewer`，用于支持后续更细粒度的权限控制。

### 族谱管理

族谱模块支持新增、编辑、删除和邀请协作者。删除族谱时，数据库通过外键级联删除相关成员、关系和协作者记录。

### 成员管理

成员模块支持新增、编辑、删除、详情查看和姓名模糊查询。数据库约束保证性别、出生年份、死亡年份和世代字段的基本合法性。

### 关系维护

亲子关系表记录父亲或母亲关系，触发器检查父母性别、世代、出生年份和族谱一致性。婚姻关系表记录夫妻关系，触发器统一双方 ID 顺序，避免重复。

### 查询与展示

系统支持 Dashboard 统计、后代树形预览、祖先递归查询和两个成员之间的亲缘链路查询。递归查询和链路查询体现数据库课程实验中对层次数据和关系路径的处理。

## 6. 数据工程

项目提供 `tools/generate_data.py` 生成 10 万级 CSV 数据，并通过 `sql/05_load_generated_csv.sql` 使用 PostgreSQL `COPY` 批量导入。

导出和性能脚本：

- `sql/06_export_branch.sql`：按成员根节点导出分支数据。
- `sql/07_performance_explain.sql`：对核心查询执行 `EXPLAIN ANALYZE`，用于索引效果分析。

## 7. 构建与运行

从项目根目录构建：

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

运行程序：

```bash
./scripts/run_wsl.sh
```

完整环境配置、数据库初始化和中文输入说明见 `docs/usage.md`。
