# 阶段二验收结果：Qt 主工程编译与数据库联通验证

执行时间：2026-05-10

## 1. 构建目录清理

原有构建目录不可复用：

```text
test/build      来自 Windows/MinGW 路径 c:/Users/Sherry Peng/...
test/build-wsl  来自旧 WSL 路径 /mnt/c/Users/Sherry Peng/...
```

这两个目录都是 CMake/Ninja 构建产物，不是源码文件，因此已清理并重新生成当前路径下的标准构建目录：

```text
Database_Course_Experiment/test/build
```

## 2. 编译命令

执行目录：

```text
Database_Course_Experiment/test
```

执行命令：

```bash
cmake -S . -B build -G Ninja
cmake --build build
```

结果：

```text
GenealogySystem 编译成功
Stage2Smoke 编译成功
```

CMake 配置时出现过：

```text
Could NOT find XKB
```

该提示未阻止 Qt Widgets 工程配置和编译。

## 3. 自动 smoke test

为了验证登录、族谱加载、Dashboard 和成员列表不是只靠人工点击判断，新增了一个命令行验收目标：

```text
Stage2Smoke
```

它复用主程序同一套代码：

```text
DatabaseManager
AuthService
GenealogyDao
DashboardService
MemberDao
```

执行命令：

```bash
./build/Stage2Smoke
```

输出：

```text
PASS: login, genealogy loading, dashboard, and member list are working.
user=admin genealogy=张氏族谱 members=7
```

验证内容：

```text
连接 PostgreSQL genealogy_lab 成功。
admin / 123456 登录成功。
admin 能加载“张氏族谱”。
Dashboard 读取统计成功。
成员列表读取到 7 个成员。
```

## 4. GUI 启动验证

执行命令：

```bash
timeout 5s env XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir QT_QPA_PLATFORM=wayland ./build/GenealogySystem
```

结果：

```text
程序可启动，5 秒后被 timeout 正常结束。
```

输出中有 WSLg 运行目录权限提示：

```text
QStandardPaths: wrong permissions on runtime directory /mnt/wslg/runtime-dir, 0777 instead of 0700
```

该提示不影响本次启动验证。

## 5. 手动登录信息

后续手动运行 GUI：

```bash
cd "/home/xsy/mySchoolProject/Database Course lab/Database_Course_Experiment/test"
env XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir QT_QPA_PLATFORM=wayland ./build/GenealogySystem
```

登录账号：

```text
username: admin
password: 123456
```

登录后应能看到：

```text
当前族谱：张氏族谱
Dashboard：总人数 7，男性 4，女性 3，最大代数 4，血缘关系 7，婚姻关系 2
成员管理：7 条成员记录
```

## 6. 阶段二结论

阶段二通过：

```text
Qt 主工程已成功编译。
GUI 主程序可启动。
程序代码路径已验证能连接 PostgreSQL。
登录、族谱加载、Dashboard、成员列表均已通过自动验证。
```
