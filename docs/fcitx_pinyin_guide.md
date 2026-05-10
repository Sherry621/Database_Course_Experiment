# WSL2 + Qt6 中文拼音输入法配置指南

本文档用于解决 Qt 程序在 WSL2 / WSLg 环境下无法输入中文的问题。

适用场景：

```text
系统：Windows 11 + WSL2 Ubuntu
界面：Qt6 Widgets
输入法：fcitx5 + pinyin
程序：test/build-wsl/GenealogySystem
```

## 1. 问题表现

常见现象：

```text
Qt 程序可以显示中文，但输入框里无法切换中文输入法。
fcitx5 进程存在，但输入法只有 keyboard-us。
echo $QT_IM_MODULE 输出为空。
echo $XMODIFIERS 输出为空。
fcitx5-diagnose 显示 DefaultIM=keyboard-us。
```

如果 `~/.config/fcitx5/profile` 中只有：

```text
DefaultIM=keyboard-us
Name=keyboard-us
```

说明 fcitx5 没有启用拼音输入法。

## 2. 安装 fcitx5 拼音组件

在 WSL 终端执行：

```bash
sudo apt-get update
sudo apt-get install -y \
  fcitx5 \
  fcitx5-chinese-addons \
  fcitx5-frontend-qt6 \
  fcitx5-frontend-qt5 \
  fcitx5-config-qt
```

如果中文显示为方框，还需要安装中文字体：

```bash
sudo apt-get install -y fonts-noto-cjk fonts-wqy-microhei
fc-cache -fv
```

## 3. 配置拼音输入法

项目已经提供脚本：

```text
test/setup_fcitx_wsl.sh
```

执行：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku/test"
chmod +x setup_fcitx_wsl.sh run_wsl.sh
./setup_fcitx_wsl.sh
```

脚本会完成：

```text
1. 停止已有 fcitx5 进程
2. 写入 ~/.config/fcitx5/profile
3. 设置默认输入法为 pinyin
4. 写入 ~/.xprofile 环境变量
5. 重新启动 fcitx5
```

## 4. 检查配置是否成功

执行：

```bash
cat ~/.config/fcitx5/profile
```

正确结果应包含：

```text
DefaultIM=pinyin
```

并且包含：

```text
[Groups/0/Items/1]
Name=pinyin
```

如果仍然是：

```text
DefaultIM=keyboard-us
```

说明配置没有生效。重新执行：

```bash
./setup_fcitx_wsl.sh
```

## 5. 加载输入法环境变量

当前 shell 中加载：

```bash
source ~/.xprofile
```

检查：

```bash
echo $QT_IM_MODULE
echo $XMODIFIERS
```

正确输出应为：

```text
fcitx
@im=fcitx
```

检查 fcitx5 是否运行：

```bash
pgrep -a fcitx5
```

正常会看到类似：

```text
3610 fcitx5 -d
```

## 6. 编译并运行 Qt 程序

不要直接运行：

```bash
./build-wsl/GenealogySystem
```

应该使用项目提供的启动脚本：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku/test"
cmake --build build-wsl
./run_wsl.sh
```

`run_wsl.sh` 会在启动程序前设置：

```bash
export QT_IM_MODULE=fcitx
export GTK_IM_MODULE=fcitx
export XMODIFIERS=@im=fcitx
export QT_QPA_PLATFORM=xcb
export XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir
```

`xcb` 通常比原生 `wayland` 更容易让 Qt 程序接入 fcitx5。

## 7. 在程序中切换中文

打开程序后，点击输入框，尝试：

```text
Ctrl + Space
```

如果没有反应，再尝试：

```text
Ctrl + Shift
```

或者：

```text
Super + Space
```

## 8. 手动启动方式

如果不使用 `run_wsl.sh`，可以手动执行：

```bash
QT_QPA_PLATFORM=xcb \
QT_IM_MODULE=fcitx \
XMODIFIERS=@im=fcitx \
GTK_IM_MODULE=fcitx \
XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir \
./build-wsl/GenealogySystem
```

## 9. 诊断命令

如果仍然不能输入中文，执行：

```bash
fcitx5-diagnose
```

重点检查：

```text
Fcitx State -> process 是否存在
Qt -> qt6 是否找到 fcitx5 im module
Input Methods -> profile 是否包含 pinyin
Xim -> XMODIFIERS 是否为 @im=fcitx
Qt -> QT_IM_MODULE 是否为 fcitx
```

## 10. 常见错误

### 10.1 把 PowerShell 路径写进 WSL

错误写法：

```bash
cd & 'C:\Users\Sherry Peng\OneDrive\桌面\shujuku'
```

这是 PowerShell 写法，不能在 WSL bash 中使用。

WSL 正确写法：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku/test"
```

### 10.2 profile 被 fcitx5 覆盖回 keyboard-us

如果写入后又变回：

```text
DefaultIM=keyboard-us
```

先停止 fcitx5，再写配置：

```bash
pkill -x fcitx5 || true
./setup_fcitx_wsl.sh
```

### 10.3 当前 shell 环境变量为空

如果：

```bash
echo $QT_IM_MODULE
echo $XMODIFIERS
```

输出为空，执行：

```bash
source ~/.xprofile
```

但即使当前 shell 为空，只要用 `./run_wsl.sh` 启动程序，脚本也会为 Qt 程序设置这些变量。
