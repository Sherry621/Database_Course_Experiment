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

待继续完善：

- 实验报告截图与最终演示材料整理

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
├── tests/
│   └── stage2_smoke.cpp
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
- `docs/data_and_tests.md`：阶段验收、10 万数据、导入导出和性能测试结果。
- `docs/report_materials.md`：ER 图、关系模型、主外键、3NF/BCNF 和报告截图建议。
- `docs/planning_and_work.md`：实现路线、演示顺序和两人分工建议。
- `docs/project_structure.md`：项目目录结构说明。
- `docs/github_upload.md`：GitHub 上传步骤。
- `docs/acceptance_checklist.md`：最终验收检查清单。
- `sql/`：PostgreSQL 数据库脚本。
- `src/`：Qt/C++ 主程序源码。
- `tests/`：自动验收和测试程序。
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
- `sql/08_load_generated_csv.sql`：10 万级 CSV 批量导入
- `sql/09_export_branch.sql`：分支导出备份
- `sql/10_performance_explain.sql`：索引性能对比 EXPLAIN
- `tools/generate_data.py`：模拟数据生成工具

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
  libxkbcommon-dev \
  libxkbcommon-x11-dev \
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

`libxkbcommon-dev` 和 `libxkbcommon-x11-dev` 用于补齐 Qt/xcb 的键盘依赖检测。`fcitx5`、`fcitx5-chinese-addons` 和 `fcitx5-frontend-qt6` 用于 Qt6 中文输入。缺少 fcitx5 相关包时，界面可以显示中文，但输入框中通常无法输入中文。

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

psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/02_indexes.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/03_triggers.sql
```

## 创建测试账号

程序支持在登录窗口直接注册用户。也可以通过 SQL 创建测试账号。

用户名：`admin`

密码：`123456`

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab"
```

```sql
INSERT INTO users(username, password_hash, real_name, email)
VALUES (
  'admin',
  '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92',
  '管理员',
  'admin@example.com'
);

INSERT INTO genealogies(title, family_surname, creator_user_id, description)
VALUES ('张氏族谱', '张', 1, '测试族谱');

\q
```

## 编译运行

进入项目目录：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku"
```

使用当前项目下的 `build/` 构建目录：

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

运行程序。推荐使用脚本启动，它会设置中文输入相关环境变量：

```bash
./scripts/setup_fcitx_wsl.sh
./scripts/run_wsl.sh
```

如果要手动启动并保留中文输入，需要同时设置 fcitx5 相关环境变量：

```bash
env XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir QT_QPA_PLATFORM=xcb \
  QT_IM_MODULE=fcitx GTK_IM_MODULE=fcitx XMODIFIERS=@im=fcitx \
  ./build/GenealogySystem
```

登录：

```text
用户名：admin
密码：123456
```

## 常见问题

### CMakeCache 路径不一致

如果出现 Windows 路径和 WSL 路径冲突，不要共用 `build/` 目录。WSL 下使用：

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

### CMake 提示 Could NOT find XKB

如果配置时出现：

```text
Could NOT find XKB (missing: XKB_LIBRARY XKB_INCLUDE_DIR)
```

说明系统缺少 XKB 开发包。它不是“无法输入中文”的直接原因，但建议补齐，否则 Qt/xcb 的键盘依赖检测不完整：

```bash
sudo apt-get install -y libxkbcommon-dev libxkbcommon-x11-dev
cmake -S . -B build -G Ninja
cmake --build build
```

### 中文显示为方框

安装中文字体：

```bash
sudo apt-get install -y fonts-noto-cjk fonts-wqy-microhei
fc-cache -fv
```

### 无法输入中文

先安装并配置 fcitx5：

```bash
sudo apt-get update
sudo apt-get install -y fcitx5 fcitx5-chinese-addons fcitx5-frontend-qt6
./scripts/setup_fcitx_wsl.sh
./scripts/check_chinese_input.sh
./scripts/run_wsl.sh
```

`check_chinese_input.sh` 输出中应包含 `IM_MODULE_CLASSNAME=fcitx::QFcitxPlatformInputContext`。不要用 Wayland 方式或 IDE 直接启动来测试中文输入；`scripts/run_wsl.sh` 会强制使用 `QT_QPA_PLATFORM=xcb`，在 WSLg/XWayland 下更容易让 fcitx5 接管 Qt 输入框。

### QStandardPaths 权限警告

推荐使用：

```bash
./scripts/run_wsl.sh
```

## 文档索引

- [系统框架说明](docs/system_framework.md)
- [使用与环境配置](docs/usage.md)
- [数据工程与阶段验收](docs/data_and_tests.md)
- [报告材料：ER 图、关系模型与范式分析](docs/report_materials.md)
- [实现路线与分工](docs/planning_and_work.md)
- [文件结构说明](docs/project_structure.md)
- [GitHub 上传步骤](docs/github_upload.md)
- [验收检查清单](docs/acceptance_checklist.md)
- [最终验收资料包](docs/final_acceptance_package.md)
- [项目文件结构与关键代码实现原理](docs/code_structure_and_principles.md)

## 分工建议

本项目适合两人合作完成：

- 成员 A：数据库设计、SQL 查询、数据生成、导入导出、索引优化与性能分析。
- 成员 B：C++/Qt 系统开发、界面设计、登录注册、成员管理、树形预览、祖先查询和亲缘链路查询。

详细分工见 `docs/planning_and_work.md`。

## 课程报告材料

报告建议包含：

- 需求分析
- 系统总体设计
- E-R 图与关系模型
- 范式分析
- 数据库表结构
- 约束与触发器
- 索引设计
- 核心 SQL
- C++ 系统实现
- 数据生成与导入导出
- 性能测试与 EXPLAIN 分析
- 系统运行截图
- 总结
