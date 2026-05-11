# 操作文档

本文档记录“寻根溯源”族谱管理系统从环境配置、数据库初始化、编译运行到常见问题处理的完整流程。

## 1. 项目说明

本项目是一个基于 C++17、Qt6 Widgets、Qt SQL 和 PostgreSQL 的族谱管理系统框架。当前框架已经包含：

- 用户注册与登录
- PostgreSQL 数据库连接
- Dashboard 统计
- 成员列表与姓名查询
- 新增成员
- 族谱新增、修改、删除
- 邀请协作者
- 成员编辑、删除和详情查看
- 亲子关系维护
- 婚姻关系维护
- 后代树形预览
- 祖先查询
- 两个成员之间亲缘链路 BFS 查询
- PostgreSQL 建表、索引、触发器和核心 SQL 脚本
- 10 万级模拟数据生成工具
- COPY 批量导入脚本
- 分支导出脚本
- EXPLAIN ANALYZE 性能测试脚本

当前框架还需要继续补充：

- ER 图、关系模型、3NF/BCNF 分析
- 实验报告截图与最终演示材料整理

## 2. 目录结构

```text
./
├── README.md
├── CMakeLists.txt
├── docs/
│   ├── system_design.md      课程方案与系统设计说明
│   ├── operation.md          环境配置原始说明
│   ├── work.md               两人分工说明
│   ├── system_framework.md   系统框架说明
│   ├── operation_guide.md    本操作文档
│   ├── project_structure.md  GitHub 仓库结构说明
│   ├── data_engineering.md   数据工程说明
│   └── github_upload.md      GitHub 上传步骤
├── sql/
│   ├── 01_schema.sql         建表脚本
│   ├── 02_indexes.sql        索引脚本
│   ├── 03_triggers.sql       触发器脚本
│   └── 04_core_queries.sql   核心查询 SQL
├── src/                      Qt/C++ 主程序源码
├── tests/                    自动验收程序
├── tools/                    数据生成工具
└── scripts/                  运行辅助脚本
```

## 3. WSL 环境安装

在 WSL Ubuntu 终端执行：

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

刷新字体缓存：

```bash
fc-cache -fv
```

启动 PostgreSQL：

```bash
sudo service postgresql start
pg_isready
```

## 4. 创建数据库

进入 PostgreSQL 管理员账号：

```bash
sudo -u postgres psql
```

执行：

```sql
CREATE USER genealogy_user WITH PASSWORD 'genealogy_pass';
CREATE DATABASE genealogy_lab OWNER genealogy_user;
\q
```

测试连接：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab"
```

可以进入 psql 就说明连接成功。退出：

```sql
\q
```

## 5. 初始化数据库表

进入项目根目录：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku"
```

执行 SQL 脚本：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/02_indexes.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/03_triggers.sql
```

## 6. 创建可登录用户

当前程序的注册功能可以直接创建用户，推荐方式是先运行程序，在登录窗口输入用户名和密码后点击“注册”。

如果需要直接用 SQL 创建测试用户，可以使用下面示例。用户名是 `admin`，密码是 `123456`。

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab"
```

执行：

```sql
INSERT INTO users(username, password_hash, real_name, email)
VALUES (
  'admin',
  '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92',
  '管理员',
  'admin@example.com'
);
```

给该用户创建一个测试族谱。这里假设 `admin` 的 `user_id` 是 1：

```sql
INSERT INTO genealogies(title, family_surname, creator_user_id, description)
VALUES ('张氏族谱', '张', 1, '测试族谱');
```

如果不确定 `user_id`，先查询：

```sql
SELECT user_id, username FROM users;
```

退出：

```sql
\q
```

## 7. 编译运行

进入 Qt 项目目录：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku"
```

如果 `build/` 目录来自其他路径或 Windows 环境，先删除旧构建缓存，再使用当前项目下的 `build/`：

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

运行：

```bash
./scripts/run_wsl.sh
```

登录测试账号：

```text
用户名：admin
密码：123456
```

## 8. 常见问题

### 8.1 CMakeCache 路径不一致

如果看到类似错误：

```text
CMakeCache.txt is different than the directory ...
Make command was: D:/Qt/Tools/Ninja/ninja.exe
```

原因是 Windows 和 WSL 共用了同一个构建目录。解决方式：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku"
cmake -S . -B build -G Ninja
cmake --build build
```

### 8.2 中文显示为方框

安装中文字体：

```bash
sudo apt-get install -y fonts-noto-cjk fonts-wqy-microhei
fc-cache -fv
```

程序入口 `main.cpp` 已经设置了中文字体 fallback。

如果中文可以显示但不能输入，请参考：

```text
docs/fcitx_pinyin_guide.md
```

推荐运行方式：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku"
./scripts/setup_fcitx_wsl.sh
./scripts/run_wsl.sh
```

### 8.3 QStandardPaths 权限警告

如果看到：

```text
QStandardPaths: wrong permissions on runtime directory /run/user/1000/, 0755 instead of 0700
```

推荐使用 WSLg runtime 运行：

```bash
./scripts/run_wsl.sh
```

也可以修复权限：

```bash
sudo chmod 700 /run/user/1000
```

### 8.4 登录后没有族谱

需要在 `genealogies` 表中为当前用户创建族谱：

```sql
SELECT user_id, username FROM users;

INSERT INTO genealogies(title, family_surname, creator_user_id, description)
VALUES ('张氏族谱', '张', 你的_user_id, '测试族谱');
```

## 9. GitHub 上传前注意

不要上传构建目录和数据库导出文件：

```text
build/
*.csv
*.dump
*.backup
```

这些内容已经写入 `.gitignore`。
