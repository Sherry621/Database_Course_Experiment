# “寻根溯源”族谱管理系统框架

## 1. 技术栈

- 开发语言：C++17
- 图形界面：Qt6 Widgets
- 数据库访问：Qt SQL，驱动使用 QPSQL
- 数据库：PostgreSQL
- 构建工具：CMake + Ninja

## 2. 当前目录结构

```text
src/
├── main.cpp
├── db/          数据库连接与 DAO
├── model/       User、Genealogy、Member 等数据模型
├── service/     认证、统计、族谱树、亲缘链路等业务逻辑
└── ui/          登录窗口、主窗口、成员弹窗

sql/
├── 01_schema.sql        建表脚本
├── 02_indexes.sql       索引脚本
├── 03_triggers.sql      触发器脚本
├── 04_core_queries.sql  课程要求核心查询
├── 05_load_generated_csv.sql
├── 06_export_branch.sql
└── 07_performance_explain.sql
```

## 3. 分层说明

```text
UI 层
  LoginWindow / MainWindow / MemberDialog
  负责窗口、按钮、表格、树形控件与用户交互。

Service 层
  AuthService / DashboardService / TreeService
  负责登录注册、统计汇总、祖先查询、后代树、亲缘链路 BFS。

DAO 层
  UserDao / GenealogyDao / MemberDao / RelationDao
  负责封装 SQL，统一使用复数表名。

Database 层
  DatabaseManager
  负责 PostgreSQL 连接。

PostgreSQL 层
  users / genealogies / members / parent_child_relations / marriages
  负责数据存储、约束、索引、触发器、递归 SQL。
```

## 4. 已预留核心功能

- 用户注册、登录
- 登录后加载当前用户可访问族谱
- Dashboard 统计
- 成员列表、姓名模糊查询、新增成员
- 后代树形预览
- 祖先递归查询
- 两成员亲缘链路 BFS 查询

## 5. 最终验收流程

1. 执行 `sql/01_schema.sql`、`sql/02_indexes.sql`、`sql/03_triggers.sql` 初始化数据库。
2. 基于 `tools/generate_data.py` 生成 10 万级 CSV，并使用 `sql/05_load_generated_csv.sql` 执行 `COPY` 导入。
3. 编译并运行 Qt 项目，展示登录、Dashboard、族谱、成员、关系维护、树形预览、祖先查询和亲缘链路。
4. 使用 `sql/04_core_queries.sql` 展示 PPT 要求的核心 SQL。
5. 使用 `sql/06_export_branch.sql` 导出分支备份。
6. 使用 `sql/07_performance_explain.sql` 截图整理 EXPLAIN ANALYZE 性能对比，用于课程报告。
