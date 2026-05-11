# GitHub 上传步骤

## 1. 上传前检查

确认当前根目录是：

```bash
cd "Database_Course_Experiment"
```

查看将被 Git 管理的文件：

```bash
git status
```

如果还没有初始化 Git：

```bash
git init
```

## 2. 确认忽略构建目录

本项目已经提供 `.gitignore`，会忽略：

```text
build/
.vscode/
*.exe
*.o
*.csv
*.backup
```

可以检查实际会加入 Git 的文件：

```bash
git status --short
```

如果看到 `build/` 出现在待提交列表，说明 `.gitignore` 没生效，需要先执行：

```bash
git rm -r --cached build
```

仅当这些目录已经被 Git 跟踪时才需要执行上面的命令。

## 3. 首次提交

```bash
git add .
git commit -m "Initial genealogy management system framework"
```

## 4. 创建 GitHub 仓库

在 GitHub 网页创建一个新仓库，例如：

```text
genealogy-management-system
```

建议不要勾选自动生成 README，因为本地已经有文档。

## 5. 关联远程仓库

把下面地址替换成你自己的 GitHub 仓库地址：

```bash
git remote add origin https://github.com/你的用户名/genealogy-management-system.git
git branch -M main
git push -u origin main
```

如果后续修改代码：

```bash
git status
git add .
git commit -m "Update project"
git push
```

## 6. 推荐仓库说明

GitHub 仓库描述可以写：

```text
C++17 + Qt6 + PostgreSQL genealogy management system for database course project.
```

## 7. 推荐 README 展示内容

仓库首页建议保留：

- 项目简介
- 技术栈
- 功能模块
- 目录结构
- 环境配置
- 数据库初始化
- 编译运行
- 常见问题

当前详细内容已经拆分到：

- `docs/system_design.md`
- `docs/usage.md`
- `docs/data_and_tests.md`
- `docs/planning_and_work.md`
- `docs/project_structure.md`
- `docs/system_framework.md`
