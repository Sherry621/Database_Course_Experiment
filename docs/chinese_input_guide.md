# Qt 图形界面中文显示与中文输入配置方案

本文档说明在 WSL2 + WSLg + Qt6 环境下，如何让本项目窗口正常显示中文，并在输入框中使用拼音输入中文。

## 1. 问题现象

常见问题分为两类：

1. 中文显示为方框或乱码。
2. 窗口能显示中文，但输入框无法切换到中文输入法。

第一类通常是中文字体或文件编码问题。第二类通常是 Qt 没有加载 fcitx5 输入法模块，或者运行程序时没有设置输入法环境变量。

## 2. 推荐环境

本项目推荐在 WSL2 Ubuntu 中运行 Qt 程序，并使用 fcitx5 拼音输入法。

需要安装的软件包：

```bash
sudo apt-get update
sudo apt-get install -y \
  fonts-noto-cjk \
  locales \
  fcitx5 \
  fcitx5-chinese-addons \
  fcitx5-frontend-qt6
```

其中：

- `fonts-noto-cjk`：解决中文显示为方框的问题。
- `locales`：提供 UTF-8 语言环境支持。
- `fcitx5`：输入法框架。
- `fcitx5-chinese-addons`：提供拼音输入法。
- `fcitx5-frontend-qt6`：让 Qt6 程序能接入 fcitx5。

## 3. 一次性配置拼音输入法

在项目根目录执行：

```bash
chmod +x scripts/setup_fcitx_wsl.sh scripts/run_wsl.sh
./scripts/setup_fcitx_wsl.sh
```

该脚本会完成以下工作：

- 检查 `fcitx5`、`fcitx5-chinese-addons`、`fcitx5-frontend-qt6` 是否安装。
- 写入 `~/.config/fcitx5/profile`。
- 将默认输入法设置为 `pinyin`。
- 在 `~/.xprofile` 中写入输入法环境变量。
- 启动 fcitx5 后台进程。

配置完成后，可以查看 profile：

```bash
cat ~/.config/fcitx5/profile
```

正常情况下应包含：

```text
DefaultIM=pinyin
Name=keyboard-us
Name=pinyin
```

## 4. 每次启动程序的正确方式

不要直接双击程序，也不要直接运行：

```bash
./build/GenealogySystem
```

推荐从项目根目录使用脚本启动：

```bash
./scripts/run_wsl.sh
```

该脚本会在启动 Qt 程序前设置：

```bash
export QT_IM_MODULE=fcitx
export GTK_IM_MODULE=fcitx
export XMODIFIERS=@im=fcitx
export QT_QPA_PLATFORM=xcb
export XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir
```

这些变量的作用：

- `QT_IM_MODULE=fcitx`：让 Qt 使用 fcitx 输入法模块。
- `GTK_IM_MODULE=fcitx`：保持 GTK 程序输入法环境一致。
- `XMODIFIERS=@im=fcitx`：让 X11/XWayland 程序能找到 fcitx。
- `QT_QPA_PLATFORM=xcb`：在 WSLg 中优先走 XWayland，通常比原生 Wayland 更容易接入 fcitx5。
- `XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir`：使用 WSLg 提供的运行目录。

## 5. 完整运行流程

第一次配置：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku"
sudo apt-get update
sudo apt-get install -y fonts-noto-cjk locales fcitx5 fcitx5-chinese-addons fcitx5-frontend-qt6
chmod +x scripts/setup_fcitx_wsl.sh scripts/run_wsl.sh
./scripts/setup_fcitx_wsl.sh
```

编译项目：

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

启动程序：

```bash
./scripts/run_wsl.sh
```

## 6. 诊断命令

如果仍然不能输入中文，先检查环境变量：

```bash
echo $QT_IM_MODULE
echo $GTK_IM_MODULE
echo $XMODIFIERS
```

使用 `scripts/run_wsl.sh` 启动时，程序内部环境应等价于：

```text
QT_IM_MODULE=fcitx
GTK_IM_MODULE=fcitx
XMODIFIERS=@im=fcitx
```

检查 fcitx5 是否正在运行：

```bash
pgrep -a fcitx5
```

正常情况下应能看到类似：

```text
3610 fcitx5 -d
```

检查 Qt6 fcitx 插件是否存在：

```bash
find /usr/lib -path '*platforminputcontexts*' -type f | sort
```

正常情况下应能看到类似：

```text
/lib/x86_64-linux-gnu/qt6/plugins/platforminputcontexts/libfcitx5platforminputcontextplugin.so
```

运行 fcitx5 诊断：

```bash
fcitx5-diagnose
```

重点看以下几项：

- `XMODIFIERS` 是否为 `@im=fcitx`。
- `QT_IM_MODULE` 是否为 `fcitx`。
- Qt6 input method module 是否能找到 fcitx5 插件。
- `~/.config/fcitx5/profile` 中是否有 `pinyin`。

## 7. 常见问题处理

### 7.1 中文显示成方框

安装中文字体：

```bash
sudo apt-get install -y fonts-noto-cjk
```

重新启动程序：

```bash
./scripts/run_wsl.sh
```

### 7.2 可以显示中文，但不能输入中文

先确认 fcitx5 和 Qt6 前端是否安装：

```bash
sudo apt-get install -y fcitx5 fcitx5-chinese-addons fcitx5-frontend-qt6
```

重新配置：

```bash
./scripts/setup_fcitx_wsl.sh
```

重新启动：

```bash
./scripts/run_wsl.sh
```

### 7.3 `fcitx5-diagnose` 显示 `QT_IM_MODULE` 为空

这说明当前 shell 没有设置 Qt 输入法环境变量。

不要直接运行：

```bash
./build/GenealogySystem
```

应使用：

```bash
./scripts/run_wsl.sh
```

如果要临时手动启动，也可以执行：

```bash
QT_IM_MODULE=fcitx \
GTK_IM_MODULE=fcitx \
XMODIFIERS=@im=fcitx \
QT_QPA_PLATFORM=xcb \
XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir \
./build/GenealogySystem
```

### 7.4 `~/.config/fcitx5/profile` 中只有 `keyboard-us`

说明拼音输入法没有加入 fcitx5 配置。重新执行：

```bash
./scripts/setup_fcitx_wsl.sh
```

然后确认：

```bash
cat ~/.config/fcitx5/profile
```

应看到：

```text
DefaultIM=pinyin
Name=pinyin
```

### 7.5 运行脚本出现 `bash\r`

错误示例：

```text
/usr/bin/env: 'bash\r': No such file or directory
```

这是脚本被 Windows 换行符 CRLF 修改导致的。处理方式：

```bash
sudo apt-get install -y dos2unix
dos2unix scripts/*.sh
chmod +x scripts/*.sh
```

然后重新运行：

```bash
./scripts/run_wsl.sh
```

### 7.6 `QStandardPaths: wrong permissions on runtime directory`

示例：

```text
QStandardPaths: wrong permissions on runtime directory /run/user/1000/, 0755 instead of 0700
```

在 WSLg 中这通常不影响程序启动。推荐继续通过：

```bash
./scripts/run_wsl.sh
```

因为脚本会设置：

```bash
XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir
```

## 8. 项目代码中的配合

项目中已经做了两层配合：

1. `scripts/run_wsl.sh` 在启动前设置输入法环境变量。
2. `src/main.cpp` 在检测到 WSL 环境且变量为空时，会补充设置 `QT_IM_MODULE`、`GTK_IM_MODULE` 和 `XMODIFIERS`。

因此正常情况下，只需要执行：

```bash
./scripts/setup_fcitx_wsl.sh
./scripts/run_wsl.sh
```

如果这两个脚本都执行正确，Qt 窗口中的文本框、搜索框、备注框应可以使用拼音输入中文。
