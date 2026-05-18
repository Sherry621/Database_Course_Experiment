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
- 族谱新增、编辑、删除
- 邀请协作者
- 成员编辑、删除和详情查看
- 亲子关系维护
- 婚姻关系维护
- 后代树形预览
- 祖先递归查询
- 两个成员之间亲缘链路 BFS 查询
- 数据库建表、索引、触发器和核心查询 SQL
- 10 万级模拟数据生成工具
- COPY 批量导入脚本
- 分支导出脚本
- EXPLAIN ANALYZE 性能测试脚本
- ER 图、关系模型、3NF/BCNF 分析

## 目录结构

```text
.
├── README.md
├── CMakeLists.txt
├── docs/
│   ├── system_design.md
│   ├── system_framework.md
│   ├── usage.md
│   ├── data_and_tests.md
│   ├── report_materials.md
│   ├── planning_and_work.md
│   ├── project_structure.md
│   ├── github_upload.md
│   └── acceptance_checklist.md
├── sql/
│   ├── 01_schema.sql
│   ├── 02_indexes.sql
│   ├── 03_triggers.sql
│   ├── 04_core_queries.sql
│   └── ...
├── src/
│   ├── main.cpp
│   ├── db/
│   ├── model/
│   ├── service/
│   └── ui/
├── tools/
│   └── generate_data.py
└── scripts/
    ├── run_wsl.sh
    └── setup_fcitx_wsl.sh
```

说明：

- `README.md`：GitHub 仓库首页说明。
- `docs/system_design.md`：课程设计完整方案说明。
- `docs/usage.md`：环境配置、数据库初始化、编译运行和中文输入说明。
- `docs/chinese_input_guide.md`：Qt 图形界面中文显示与拼音输入完整配置方案。
- `docs/data_and_tests.md`：10 万数据、导入导出和性能测试结果。
- `docs/report_materials.md`：ER 图、关系模型、主外键、3NF/BCNF 和报告截图建议。
- `docs/planning_and_work.md`：实现路线、演示顺序和两人分工建议。
- `docs/project_structure.md`：项目目录结构说明。
- `docs/github_upload.md`：GitHub 上传步骤。
- `docs/acceptance_checklist.md`：最终验收检查清单。
- `sql/`：PostgreSQL 数据库脚本。
- `src/`：Qt/C++ 主程序源码。
- `tools/`：数据生成工具。
- `scripts/`：WSL 运行和输入法辅助脚本。

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
- `sql/05_load_generated_csv.sql`：10 万级 CSV 批量导入
- `sql/06_export_branch.sql`：分支导出备份
- `sql/07_performance_explain.sql`：索引性能对比 EXPLAIN
- `tools/generate_data.py`：模拟数据生成工具

# 使用与环境配置

本文档整合环境安装、数据库初始化、编译运行和中文输入配置。

## 1. 环境依赖

WSL Ubuntu 24.04 下安装：

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
  fonts-wqy-microhei \
  fcitx5 \
  fcitx5-chinese-addons \
  fcitx5-frontend-qt6
```

中文输入需要 `fcitx5`、`fcitx5-chinese-addons` 和 `fcitx5-frontend-qt6`。如果缺少这些包，Qt6 程序中可以显示中文，但无法输入中文。

## 2. 数据库初始化

启动 PostgreSQL：

```bash
sudo service postgresql start
pg_isready
```

创建用户和数据库：

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
cd "/home/xsy/mySchoolProject/Database Course lab/Database_Course_Experiment"
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/02_indexes.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/03_triggers.sql
```

导入 10 万级数据：

```bash
python3 tools/generate_data.py --out generated_data --total-members 100000
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/05_load_generated_csv.sql
```

## 3. 编译运行

在项目根目录：

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

运行 GUI：

```bash
./scripts/run_wsl.sh
```

也可以手动运行：

```bash
env XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir QT_QPA_PLATFORM=xcb \
  QT_IM_MODULE=fcitx GTK_IM_MODULE=fcitx XMODIFIERS=@im=fcitx \
  ./build/GenealogySystem
```

登录账号：

```text
admin / 123456
```

## 4. 中文输入

第一次使用前执行：

```bash
./scripts/setup_fcitx_wsl.sh
./scripts/run_wsl.sh
```

如果脚本提示缺包，先执行：

```bash
sudo apt-get update
sudo apt-get install -y fcitx5 fcitx5-chinese-addons fcitx5-frontend-qt6
```

验证 Qt6 输入法插件：

```bash
./scripts/check_chinese_input.sh
```

输出中应包含：

```text
IM_MODULE_CLASSNAME=fcitx::QFcitxPlatformInputContext
```

如果只看到 `QComposeInputContext`，说明当前启动环境没有让 Qt6 接入 fcitx。此时不要直接运行 `./build/GenealogySystem`，需要用 `./scripts/run_wsl.sh` 启动。

## 5. 常见问题

### CMakeCache 路径不一致

删除旧构建目录后重新构建：

```bash
rm -rf build
cmake -S . -B build -G Ninja
cmake --build build
```

### QStandardPaths 权限警告

WSLg 下可能出现：

```text
QStandardPaths: wrong permissions on runtime directory /mnt/wslg/runtime-dir
```

通常不影响程序启动。建议继续用 `scripts/run_wsl.sh` 运行。