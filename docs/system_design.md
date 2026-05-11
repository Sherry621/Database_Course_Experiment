# 寻根溯源族谱管理系统

本项目是数据库课程设计项目《“寻根溯源”族谱管理系统设计与实现》的系统框架实现，采用 C++17、Qt6 Widgets 和 PostgreSQL 构建桌面端族谱管理应用。

系统围绕族谱管理场景设计，支持用户登录注册、族谱数据管理、成员管理、树形预览、祖先查询、亲缘链路查询、递归 SQL、索引优化和课程报告材料整理。

## 技术栈

- C++17
- Qt6 Widgets
- Qt SQL
- PostgreSQL
- CMake
- Ninja
- WSL2 Ubuntu 24.04

## 当前功能

已完成系统基础框架：

- 用户注册与登录
- PostgreSQL 数据库连接
- 登录后加载当前用户可访问的族谱
- Dashboard 基础统计
- 成员列表展示
- 成员姓名模糊查询
- 新增成员
- 后代树形预览
- 祖先递归查询
- 两个成员之间亲缘链路 BFS 查询
- 数据库建表、索引、触发器和核心查询 SQL

待继续完善：

- 族谱新增、编辑、删除界面
- 成员编辑、删除界面
- 亲子关系维护界面
- 婚姻关系维护界面
- 10 万级模拟数据生成工具
- 数据导入导出脚本
- EXPLAIN ANALYZE 性能测试截图整理

## 目录结构

```text
.
├── README.md
├── readme.md
├── operation.md
├── work.md
├── docs/
│   ├── system_framework.md
│   ├── operation_guide.md
│   ├── project_structure.md
│   └── github_upload.md
├── sql/
│   ├── 01_schema.sql
│   ├── 02_indexes.sql
│   ├── 03_triggers.sql
│   └── 04_core_queries.sql
└── test/
    ├── CMakeLists.txt
    ├── main.cpp
    ├── db/
    ├── model/
    ├── service/
    └── ui/
```

说明：

- `README.md`：GitHub 仓库首页说明。
- `readme.md`：课程设计完整方案说明。
- `operation.md`：原始环境配置说明。
- `work.md`：两人合作分工方案。
- `docs/`：运行、结构、上传和系统框架文档。
- `sql/`：PostgreSQL 数据库脚本。
- `test/`：Qt/C++ 项目源码。

## 数据库设计

当前 SQL 使用复数表名，主要表包括：

- `users`：用户表
- `genealogies`：族谱表
- `genealogy_collaborators`：族谱协作者表
- `members`：成员表
- `parent_child_relations`：亲子关系表
- `marriages`：婚姻关系表

数据库脚本：

- `sql/01_schema.sql`：建表脚本
- `sql/02_indexes.sql`：索引脚本
- `sql/03_triggers.sql`：触发器脚本
- `sql/04_core_queries.sql`：课程要求核心查询

## 环境准备

在 WSL Ubuntu 中安装依赖：

```bash
sudo apt-get update
sudo apt-get install -y \
  build-essential \
  cmake \
  ninja-build \
  gdb \
  pkg-config \
  qt6-base-dev \
  qt6-tools-dev \
  qt6-tools-dev-tools \
  libqt6sql6-psql \
  postgresql \
  postgresql-16 \
  postgresql-contrib \
  libpq-dev \
  fonts-noto-cjk \
  fonts-wqy-microhei
```

启动 PostgreSQL：

```bash
sudo service postgresql start
pg_isready
```

## 初始化数据库

创建数据库用户和数据库：

```bash
sudo -u postgres psql
```

```sql
CREATE USER genealogy_user WITH PASSWORD 'genealogy_pass';
CREATE DATABASE genealogy_lab OWNER genealogy_user;
\q
```

执行初始化脚本：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku"

cd "Database_Course_Experiment"

cmake -S . -B build -G Ninja
cmake --build build
./build/GenealogySystem
./scripts/run_wsl.sh
