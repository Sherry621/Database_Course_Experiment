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
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku"
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/02_indexes.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/03_triggers.sql
```

导入小测试数据：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/05_seed_small.sql
```

导入 10 万级数据：

```bash
python3 tools/generate_data.py --out generated_data --total-members 100000
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/08_load_generated_csv.sql
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

## 5. 自动验收

```bash
./build/Stage2Smoke
```

通过时输出：

```text
PASS: login, genealogy loading, dashboard, and member list are working.
```

## 6. 常见问题

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
