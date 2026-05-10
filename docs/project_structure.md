# 文件结构说明

## 1. 根目录文件

```text
readme.md
```

课程设计总体方案，包含系统目标、技术选型、功能设计、数据库设计、索引设计、性能测试和报告结构。

```text
operation.md
```

原始环境配置说明，重点是 VSCode、WSL2、Qt6 和 PostgreSQL 的安装配置。

```text
work.md
```

两人合作分工方案，区分数据库负责人和 C++/Qt 系统负责人。

```text
.gitignore
```

GitHub 上传时忽略构建产物、IDE 本地配置、二进制文件、数据库备份和生成数据。

## 2. docs 目录

```text
docs/system_framework.md
```

系统分层框架说明，包括 UI 层、Service 层、DAO 层、Database 层和 PostgreSQL 层。

```text
docs/operation_guide.md
```

实际运行操作文档，记录数据库初始化、编译运行、登录测试和常见问题处理。

```text
docs/fcitx_pinyin_guide.md
```

WSL2 + Qt6 中文拼音输入法配置说明。

```text
docs/project_structure.md
```

当前文件结构说明。

```text
docs/github_upload.md
```

GitHub 建仓和上传步骤。

## 3. sql 目录

```text
sql/01_schema.sql
```

创建基础表：

- users
- genealogies
- genealogy_collaborators
- members
- parent_child_relations
- marriages

```text
sql/02_indexes.sql
```

创建索引：

- 成员姓名三元组 GIN 索引
- parent_id 查询子节点索引
- child_id 查询父节点索引
- 族谱成员筛选索引
- 婚姻关系查询索引

```text
sql/03_triggers.sql
```

创建触发器：

- 检查亲子关系成员是否属于同一族谱
- 检查父母出生年份早于子女
- 检查婚姻关系双方是否属于同一族谱

```text
sql/04_core_queries.sql
```

课程要求核心 SQL：

- 查询配偶和子女
- 递归查询祖先
- 统计平均寿命最长的一代
- 查询超过 50 岁且无配偶男性
- 查询早于同辈平均出生年份的成员

## 4. test 目录

```text
test/CMakeLists.txt
```

Qt6 CMake 工程配置。

```text
test/main.cpp
```

程序入口，初始化 QApplication，并设置中文字体 fallback。

```text
test/model/
```

数据模型：

- User
- Genealogy
- Member
- RelationPath

```text
test/db/
```

数据访问层：

- DatabaseManager：数据库连接
- UserDao：用户查询与插入
- GenealogyDao：族谱查询与插入
- MemberDao：成员增删改查、祖先查询、子女查询
- RelationDao：亲子边读取和插入

```text
test/service/
```

业务逻辑层：

- AuthService：注册、登录、密码哈希
- DashboardService：统计数据
- TreeService：后代、祖先、亲缘链路 BFS

```text
test/ui/
```

界面层：

- LoginWindow：登录注册窗口
- MainWindow：主界面
- MemberDialog：新增成员弹窗

## 5. 不应上传的目录

```text
test/build/
test/build-wsl/
```

这些是本地构建产物，不属于源码。
