# VSCode + WSL2 + Qt6 + PostgreSQL 环境配置步骤

本文档用于从 0 开始配置本课程实验所需开发环境。

目标环境：

```text
Windows 11
VSCode
WSL2 Ubuntu 24.04
C++17 / C++20
Qt6 Widgets
Qt SQL
PostgreSQL
CMake + Ninja
```

---

# 1. Windows 侧准备

## 1.1 安装 VSCode

安装 Visual Studio Code。

推荐插件：

```text
WSL
C/C++
CMake
CMake Tools
```

可选插件：

```text
Qt tools
SQLTools
```

## 1.2 启用 WSL2

如果尚未安装 WSL，在 PowerShell 中执行：

```powershell
wsl --install
```

建议安装 Ubuntu 24.04。

检查 WSL 版本：

```powershell
wsl -l -v
```

如果不是 WSL2，可执行：

```powershell
wsl --set-version Ubuntu-24.04 2
```

---

# 2. 进入 VSCode WSL 环境

打开 VSCode 后，使用：

```text
Remote Explorer -> WSL -> Ubuntu
```

或者在 WSL 终端中进入项目目录后执行：

```bash
code .
```

之后所有 Linux 命令都应在 VSCode 的 WSL 终端中执行。

---

# 3. 安装 C++ / Qt / PostgreSQL 开发环境

在 WSL 终端中执行：

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
  postgresql-client \
  libpq-dev
```

说明：

```text
build-essential       C++ 编译工具链
cmake                 CMake 构建工具
ninja-build           Ninja 构建后端
gdb                   调试器
pkg-config            包检测工具
qt6-base-dev          Qt6 Core / GUI / Widgets / SQL 开发包
qt6-tools-dev         Qt6 工具开发包
qt6-tools-dev-tools   moc / uic / rcc / designer 等工具
libqt6sql6-psql       Qt PostgreSQL 数据库驱动 QPSQL
postgresql-client     psql 命令行客户端
libpq-dev             PostgreSQL C/C++ 开发库
```

---

# 4. 安装 PostgreSQL 服务端

仅安装 `postgresql-client` 只能使用 `psql` 客户端，不能本地建库。课程实验需要 PostgreSQL 服务端。

执行：

```bash
sudo apt-get install -y postgresql postgresql-16 postgresql-contrib
sudo service postgresql start
```

检查服务是否启动：

```bash
pg_isready
```

正常输出类似：

```text
/var/run/postgresql:5432 - accepting connections
```

如果执行：

```bash
psql -lqt
```

出现：

```text
FATAL: role "你的Linux用户名" does not exist
```

这是正常现象。原因是 PostgreSQL 默认使用当前 Linux 用户名作为数据库角色，但数据库中还没有这个角色。

---

# 5. 创建课程实验数据库

进入 PostgreSQL 管理员账号：

```bash
sudo -u postgres psql
```

在 psql 中执行：

```sql
CREATE USER genealogy_user WITH PASSWORD 'genealogy_pass';
CREATE DATABASE genealogy_lab OWNER genealogy_user;
\q
```

测试连接：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab"
```

如果进入 psql，说明连接成功。

退出：

```sql
\q
```

---

# 6. 验证 Qt / C++ 工具链

执行：

```bash
qmake6 --version
pkg-config --modversion Qt6Core
pkg-config --modversion Qt6Widgets
psql --version
g++ --version
cmake --version
ninja --version
```

当前已验证可用版本示例：

```text
QMake version 3.1
Qt version 6.4.2
Qt6Core 6.4.2
Qt6Widgets 6.4.2
psql 16.13
g++ 13.3.0
cmake 3.28.3
ninja 1.11.1
```

---

# 7. 验证 Qt PostgreSQL 驱动

检查 Qt SQL 插件目录：

```bash
qmake6 -query QT_INSTALL_PLUGINS
ls -la /usr/lib/x86_64-linux-gnu/qt6/plugins/sqldrivers
```

应能看到：

```text
libqsqlpsql.so
```

这说明 Qt 可以使用：

```cpp
QSqlDatabase::addDatabase("QPSQL");
```

连接 PostgreSQL。

---

# 8. 初始化实验数据库表结构

进入项目根目录：

```bash
cd "/home/xsy/mySchoolProject/Database Course lab"
```

执行建表与索引：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/02_indexes.sql
```

导入当前已有 CSV 数据：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -f sql/03_load_csv.sql
```

检查数据量：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab"
```

进入后执行：

```sql
SELECT COUNT(*) FROM genealogies;
SELECT COUNT(*) FROM members;
SELECT COUNT(*) FROM parent_child_relations;
SELECT COUNT(*) FROM marriages;
\q
```

预期数据规模：

```text
genealogies: 10
members: 100000
parent_child_relations: 189350
marriages: 19954
```

---

# 9. Qt 程序连接 PostgreSQL 示例

后续 C++ 项目中可以使用如下连接方式：

```cpp
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
db.setHostName("localhost");
db.setPort(5432);
db.setDatabaseName("genealogy_lab");
db.setUserName("genealogy_user");
db.setPassword("genealogy_pass");

if (!db.open()) {
    qDebug() << "Database connection failed:" << db.lastError().text();
} else {
    qDebug() << "Database connected.";
}
```

---

# 10. WSLg 图形界面注意事项

WSL2 在 Windows 11 下通常自带 WSLg，可以直接运行 Qt GUI。

如果 Qt 程序运行时报 Wayland / xcb 相关错误，可尝试：

```bash
env XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir QT_QPA_PLATFORM=wayland ./your_qt_app
```

如果仍有问题，先确认：

```bash
echo $DISPLAY
echo $WAYLAND_DISPLAY
ls -la /mnt/wslg/runtime-dir
```

正常情况下会看到：

```text
DISPLAY=:0
WAYLAND_DISPLAY=wayland-0
```

---

# 11. Windows Qt 是否需要保留

如果确认后续只使用：

```text
VSCode + WSL2 + Qt6
```

则 Windows 下单独安装的 Qt / Qt Creator 可以卸载。

注意：

```text
Windows Qt 和 WSL Qt 是两套独立环境。
删除 Windows Qt 不会影响 WSL 中的 Qt。
```

建议在完成以下验证后再卸载 Windows Qt：

```text
1. qmake6 / Qt6Core / Qt6Widgets 可用
2. Qt 程序可以编译
3. Qt 程序可以启动 GUI
4. QPSQL 驱动可用
5. Qt 程序可以连接 genealogy_lab 数据库
```

---

# 12. 当前项目注意事项

`DOC/docs/system_design.md` 中部分表名使用单数形式，例如：

```text
member
parent_child_relation
marriage_relation
```

当前 `sql/` 目录中实际表名使用复数形式：

```text
members
parent_child_relations
marriages
```

后续写 C++ DAO 和 SQL 查询时，应优先以 `sql/01_schema.sql` 中的实际表名为准。

当前建议：

```text
沿用 sql/ 目录中的复数表名。
```

因为现有建表、索引、CSV 导入、核心查询、性能测试 SQL 已经全部基于复数表名准备好。
