# 验收常见问题与参考回答

本文档用于最终验收答辩前准备。问题按主题分类，回答尽量使用本项目中的实际设计、文件和实现方式。

## 1. 项目整体问题

### Q1：你这个系统主要解决什么问题？

参考回答：

```text
本系统实现的是“寻根溯源”族谱管理系统，主要解决多用户环境下族谱、成员、亲子关系和婚姻关系的管理问题。
系统支持用户登录注册，每个用户可以创建自己的族谱，也可以邀请其他用户作为编辑者或查看者参与族谱维护。
核心功能包括族谱管理、成员管理、关系维护、Dashboard 统计、后代树形预览、祖先查询和亲缘链路查询。
数据库层面重点解决层级关系深度不确定、同名成员区分、递归祖先查询、后代分支导出和索引优化等问题。
```

### Q2：项目使用了哪些技术？

参考回答：

```text
前端界面使用 C++17 和 Qt6 Widgets 实现。
数据库使用 PostgreSQL。
数据库访问使用 Qt SQL 模块中的 QSqlDatabase 和 QSqlQuery。
构建工具使用 CMake 和 Ninja。
大规模数据生成使用 Python 脚本 tools/generate_data.py。
数据库导入导出使用 PostgreSQL 的 \copy、pg_dump 和 psql 脚本。
```

### Q3：项目的目录结构是怎么划分的？

参考回答：

```text
src/model 保存数据结构，例如 User、Genealogy、Member。
src/db 是数据访问层，封装对 PostgreSQL 的增删改查。
src/service 是业务逻辑层，例如登录认证、Dashboard 统计、祖先查询和亲缘链路查询。
src/ui 是 Qt 界面层，实现登录窗口、主窗口、成员对话框和族谱对话框。
sql 保存建表、索引、触发器、核心查询、导入导出和性能分析脚本。
tools 保存数据生成脚本。
tests 保存自动冒烟测试。
docs 保存设计文档、报告材料和验收资料。
```

### Q4：为什么选择 PostgreSQL？

参考回答：

```text
PostgreSQL 支持 Recursive CTE，适合处理族谱这种层级深度不确定的数据。
PostgreSQL 也支持触发器、CHECK 约束、外键、EXPLAIN ANALYZE、COPY 批量导入和 pg_trgm 模糊查询索引。
这些能力正好对应本实验中递归查询、约束设计、数据导入导出和索引性能分析的要求。
```

## 2. 数据库建模问题

### Q5：你的系统中有哪些实体？

参考回答：

```text
主要实体包括 users、genealogies、members。
users 表示系统用户。
genealogies 表示族谱。
members 表示族谱中的成员。
另外 genealogy_collaborators 是用户和族谱之间的协作关系表。
parent_child_relations 是成员之间的亲子关系表。
marriages 是成员之间的婚姻关系表。
```

### Q6：为什么要单独设计 genealogy_collaborators 表？

参考回答：

```text
因为用户和族谱之间除了创建关系，还有受邀协作关系。
一个用户可以参与多个族谱，一个族谱也可以邀请多个用户，所以这是多对多关系。
因此使用 genealogy_collaborators 表来拆解 M:N 关系。
该表的主键是 (genealogy_id, user_id)，并带有 role 字段区分 editor 和 viewer。
```

### Q7：为什么亲子关系不用直接放在 members 表中？

参考回答：

```text
如果在 members 表中直接保存 father_id 和 mother_id，可以表示基本父母关系，但扩展性较弱。
本项目使用 parent_child_relations 表保存亲子关系，使成员之间的父子/母子关系成为独立关系。
这样可以明确记录 relation_type，也方便做递归查询、索引优化和触发器约束。
同时 parent_id 和 child_id 都引用 members 表，这是典型的自关联设计。
```

### Q8：为什么婚姻关系要单独建 marriages 表？

参考回答：

```text
婚姻关系是成员与成员之间的关系，属于 members 表的自关联。
如果把配偶字段直接放在 members 表中，难以记录婚姻年份、离婚年份、备注，也不方便处理双向关系。
因此单独建立 marriages 表，person1_id 和 person2_id 都引用 members(member_id)。
```

### Q9：ER 图中有哪些联系类型？

参考回答：

```text
users 与 genealogies 是 1:N，一个用户可以创建多个族谱。
users 与 genealogies 通过 genealogy_collaborators 构成 M:N 协作关系。
genealogies 与 members 是 1:N，一个族谱包含多个成员。
genealogies 与 parent_child_relations 是 1:N。
genealogies 与 marriages 是 1:N。
members 与 parent_child_relations 是自关联，一个成员可以作为 parent，也可以作为 child。
members 与 marriages 也是自关联，表示两个成员之间的婚姻关系。
```

### Q10：为什么成员姓名不设为唯一？

参考回答：

```text
族谱中可能出现同名同姓但不同辈分的人。
如果把姓名设为唯一，会错误限制真实业务场景。
因此系统使用 member_id 作为唯一标识，通过 ID 区分同名成员。
```

## 3. 关系模式与范式问题

### Q11：你的关系模型有哪些表？

参考回答：

```text
USERS(user_id PK, username UK, password_hash, real_name, email UK, created_at)
GENEALOGIES(genealogy_id PK, title, family_surname, revision_time, creator_user_id FK, description, created_at)
GENEALOGY_COLLABORATORS(genealogy_id PK/FK, user_id PK/FK, role, invited_at)
MEMBERS(member_id PK, genealogy_id FK, name, gender, birth_year, death_year, generation, biography, created_at)
PARENT_CHILD_RELATIONS(relation_id PK, genealogy_id FK, parent_id FK, child_id FK, relation_type)
MARRIAGES(marriage_id PK, genealogy_id FK, person1_id FK, person2_id FK, marriage_year, divorce_year, description)
```

### Q12：为什么说你的关系模型满足 1NF？

参考回答：

```text
因为所有表的字段都是原子值。
例如成员姓名、性别、出生年份、死亡年份、代数、生平简介都分别存储在独立字段中。
亲子关系和婚姻关系没有作为多值字段存储在 members 表中，而是拆成单独关系表。
因此不存在重复组和非原子属性，满足 1NF。
```

### Q13：为什么满足 2NF？

参考回答：

```text
大多数表使用单字段主键，例如 users.user_id、members.member_id，非主属性都完全依赖主键。
对于 genealogy_collaborators 这种复合主键表，主键是 (genealogy_id, user_id)，role 和 invited_at 都依赖整个复合主键，而不是只依赖 genealogy_id 或 user_id 的一部分。
因此不存在部分函数依赖，满足 2NF。
```

### Q14：为什么满足 3NF？

参考回答：

```text
各表中的非主属性不依赖其他非主属性。
例如 users 中 username、password_hash、real_name、email 都直接依赖 user_id。
members 中 name、gender、birth_year、death_year、generation、biography 都直接依赖 member_id。
marriages 中 marriage_year、divorce_year、description 都依赖 marriage_id 或候选键，而不依赖其他非主属性。
因此不存在传递依赖，满足 3NF。
```

### Q15：是否满足 BCNF？

参考回答：

```text
主要表基本满足 BCNF。
例如 users 表中 user_id 和 username 都可作为候选键，决定其他属性。
genealogy_collaborators 中候选键是 (genealogy_id, user_id)，role 和 invited_at 由该候选键决定。
parent_child_relations 和 marriages 也通过主键和唯一约束控制候选键。
因此主要函数依赖的决定因素都是候选键，基本满足 BCNF。
```

## 4. 约束与触发器问题

### Q16：你设计了哪些主键？

参考回答：

```text
users.user_id
genealogies.genealogy_id
members.member_id
parent_child_relations.relation_id
marriages.marriage_id
genealogy_collaborators 的复合主键 (genealogy_id, user_id)
```

### Q17：你设计了哪些外键？

参考回答：

```text
genealogies.creator_user_id 引用 users.user_id。
members.genealogy_id 引用 genealogies.genealogy_id。
genealogy_collaborators.genealogy_id 引用 genealogies.genealogy_id。
genealogy_collaborators.user_id 引用 users.user_id。
parent_child_relations.parent_id 和 child_id 都引用 members.member_id。
marriages.person1_id 和 person2_id 都引用 members.member_id。
```

### Q18：有哪些 CHECK 约束？

参考回答：

```text
users 表中 username 唯一，email 唯一。
genealogy_collaborators 中 role 必须是 editor 或 viewer。
members 中 gender 必须是 M 或 F。
members 中 birth_year 不能晚于 death_year。
members 中 generation 必须大于等于 1。
parent_child_relations 中 relation_type 必须是 father 或 mother，parent_id 不能等于 child_id。
marriages 中 person1_id 不能等于 person2_id，marriage_year 不能晚于 divorce_year。
```

### Q19：为什么还需要触发器，CHECK 不够吗？

参考回答：

```text
CHECK 约束只能检查当前行中的字段，不能方便地跨表查询其他成员信息。
例如“父亲必须是男性”“父母出生年份必须早于子女”“父母和子女必须属于同一个族谱”，这些都需要查询 members 表中的父母和子女信息。
因此使用触发器在插入或更新 parent_child_relations 前进行跨表检查。
```

### Q20：父母出生年份早于子女是如何实现的？

参考回答：

```text
在 sql/03_triggers.sql 中定义了 check_parent_child_relation 触发器函数。
插入或更新 parent_child_relations 前，触发器会根据 NEW.parent_id 和 NEW.child_id 查询 members 表，取出父母和子女的 birth_year。
如果 parent_birth 和 child_birth 都不为空，并且 parent_birth >= child_birth，就抛出异常，拒绝该关系。
```

### Q21：如何防止重复婚姻关系？

参考回答：

```text
首先 marriages 表上有 UNIQUE(genealogy_id, person1_id, person2_id)。
其次触发器 check_marriage_relation 会统一 person1_id 和 person2_id 的顺序。
比如输入 (8, 3) 时，会自动调整成 (3, 8)。
这样 (3, 8) 和 (8, 3) 不会被当成两条不同婚姻关系。
```

## 5. SQL 核心查询问题

### Q22：配偶和所有子女如何用一个 SQL 查询？

参考回答：

```text
我使用 UNION ALL 把两个查询合并。
第一部分从 marriages 表查询配偶，通过 CASE 判断目标成员位于 person1_id 还是 person2_id。
第二部分从 parent_child_relations 表查询 child_id，再连接 members 表获得子女详情。
每条结果增加 relation 字段，标识 spouse 或 child。
```

### Q23：祖先查询为什么使用 Recursive CTE？

参考回答：

```text
族谱层级深度不固定，不能预先写死几层 JOIN。
Recursive CTE 可以从目标成员的父母开始，不断向上查父母的父母，直到没有更上层祖先。
因此它适合解决不确定深度的祖先追溯问题。
```

### Q24：递归祖先查询的终止条件是什么？

参考回答：

```text
递归查询每一轮都会根据上一轮查到的 parent_id 继续查其父母。
当 parent_child_relations 中再也找不到新的父母记录时，递归自然结束。
PostgreSQL 的 Recursive CTE 会在递归部分不再产生新行时停止。
```

### Q25：平均寿命最长的一代怎么统计？

参考回答：

```text
先用 death_year - birth_year 计算每个成员寿命。
再按 generation 分组，使用 AVG 计算每一代平均寿命。
最后 ORDER BY avg_life DESC LIMIT 1，取平均寿命最长的一代。
为了保证结果有效，过滤掉 birth_year、death_year 或 generation 为空的记录。
```

### Q26：如何查询 50 岁以上且没有配偶的男性？

参考回答：

```text
条件包括三部分。
第一，gender = 'M' 限定男性。
第二，COALESCE(death_year, 当前年份) - birth_year > 50 计算年龄。
第三，用 NOT EXISTS 检查 marriages 表中不存在该成员作为 person1_id 或 person2_id 的记录。
```

### Q27：如何找出早于同辈平均出生年份的成员？

参考回答：

```text
先用一个 CTE generation_avg 按 generation 分组，计算每一代的平均出生年份。
然后把 members 表和 generation_avg 按 generation 连接。
最后筛选 m.birth_year < g.avg_birth_year。
这样可以找出每一代中出生年份早于同代平均值的成员。
```

## 6. 索引与性能问题

### Q28：针对姓名模糊查询设计了什么索引？

参考回答：

```text
我使用 PostgreSQL 的 pg_trgm 扩展和 GIN 索引。
索引语句是：
CREATE INDEX idx_members_name_trgm ON members USING GIN (name gin_trgm_ops);
它用于优化 name LIKE '%关键字%' 这种前后都有通配符的模糊查询。
普通 B-Tree 索引对这种查询效果不好，因此使用 trigram 索引。
```

### Q29：根据父节点 ID 查询子节点用了什么索引？

参考回答：

```text
使用 parent_child_relations(parent_id) 索引。
后代树、分支导出和四代后代查询都需要根据 parent_id 查 child_id，因此该索引可以减少扫描范围。
```

### Q30：为什么还建立 child_id 索引？

参考回答：

```text
child_id 索引用于反向查询父母。
祖先查询和递归向上追溯时，会根据 child_id 找 parent_id。
所以 parent_id 索引用于向下查后代，child_id 索引用于向上查祖先。
```

### Q31：性能对比怎么做的？

参考回答：

```text
在 sql/10_performance_explain.sql 中，先 DROP INDEX 删除 parent_id 和 child_id 索引，执行四代后代查询并使用 EXPLAIN ANALYZE 记录执行计划和耗时。
然后重新 CREATE INDEX，再执行同样的四代后代查询。
最后对比有无索引时的 Execution Time。
```

### Q32：你的性能测试结果如何？

参考回答：

```text
在我的测试中，无 parent_id/child_id 索引时，四代后代查询耗时约 1.509 ms。
创建索引后，同样查询耗时约 0.519 ms。
说明 parent_child_relations(parent_id) 和 child_id 索引能有效优化层级查询。
```

### Q33：为什么小数据下索引提升看起来不一定特别大？

参考回答：

```text
小数据量下，全表扫描成本本身很低，所以索引优势不一定明显。
但在 10 万级成员数据下，parent_child_relations 表规模变大，索引可以避免大量无关行扫描。
因此索引的价值主要体现在大数据和高频查询场景。
```

## 7. 数据生成与导入导出问题

### Q34：数据是手工插入还是工具生成？

参考回答：

```text
小数据和演示数据使用 SQL seed 脚本插入。
大规模数据使用自编 Python 工具 tools/generate_data.py 生成 CSV。
```

### Q35：数据生成工具生成了哪些文件？

参考回答：

```text
users.csv
genealogies.csv
genealogy_collaborators.csv
members.csv
parent_child_relations.csv
marriages.csv
```

### Q36：是否满足 10 万数据要求？

参考回答：

```text
满足。tools/generate_data.py 默认要求 total-members 至少 100000。
它会生成至少 10 个族谱，其中至少一个族谱有 50000 以上成员，并模拟至少 30 代传承关系。
```

### Q37：如何导入 CSV？

参考回答：

```text
使用 PostgreSQL 的 \copy 命令。
导入脚本是 sql/08_load_generated_csv.sql。
它会把 generated_data 目录下的 CSV 文件导入 users、genealogies、members、parent_child_relations、marriages 等表。
导入后还会使用 setval 修正自增序列，避免后续新增数据主键冲突。
```

### Q38：如何导出数据库？

参考回答：

```text
使用 PostgreSQL 的 pg_dump 工具。
命令是：
pg_dump "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -Fc -f generated_data/genealogy_lab.backup
其中 -Fc 表示 custom 格式备份。
```

### Q39：如何导出某个分支？

参考回答：

```text
使用 sql/09_export_branch.sql。
该脚本通过 Recursive CTE 从 root_id 指定的成员开始，向下递归查找后代，并按 max_depth 控制导出层数。
最后导出到 generated_data/branch_export.csv。
```

## 8. 系统功能问题

### Q40：登录和注册是怎么实现的？

参考回答：

```text
登录界面输入用户名和密码后，AuthService 会对密码做 SHA-256 哈希，然后查询 users 表中是否存在 username 和 password_hash 同时匹配的记录。
注册时，AuthService 先对密码做哈希，再通过 UserDao::insert 插入 users 表。
```

### Q41：为什么不保存明文密码？

参考回答：

```text
明文密码不安全。如果数据库泄露，用户密码会直接暴露。
本项目使用 SHA-256 哈希保存密码，登录时比较哈希值。
虽然真实系统还应该加盐，但对于课程设计，哈希存储已经比明文更安全。
```

### Q42：用户只能看到自己创建或受邀族谱是怎么实现的？

参考回答：

```text
通过 GenealogyDao::findAccessibleByUser 实现。
SQL 查询 genealogies 表，并 LEFT JOIN genealogy_collaborators 表。
WHERE 条件是 g.creator_user_id = 当前用户 ID 或 c.user_id = 当前用户 ID。
这样用户只能看到自己创建的族谱和自己被邀请参与的族谱。
```

### Q43：权限控制怎么做？

参考回答：

```text
系统分三种权限。
创建者 owner 可以管理族谱、邀请协作者、维护成员和关系。
编辑者 editor 可以维护成员和关系，但不能删除族谱或邀请协作者。
查看者 viewer 只能浏览和查询。
界面层会根据当前用户角色禁用按钮，同时写操作函数内部也会再次检查权限，防止绕过界面触发操作。
```

### Q44：Dashboard 显示哪些内容？

参考回答：

```text
Dashboard 显示当前用户、可访问族谱数量、当前族谱、成员总数、男性人数和比例、女性人数和比例、最大代数、血缘关系数、婚姻关系数以及最近新增成员。
统计数据由 DashboardService 查询数据库获得。
```

### Q45：成员管理支持哪些功能？

参考回答：

```text
成员管理支持新增、编辑、删除、详情查看、姓名模糊搜索和表头排序。
性别使用下拉框显示男/女。
出生年、死亡年和代数会进行基本输入校验。
```

### Q46：后代树是怎么画出来的？

参考回答：

```text
后代树使用 Qt 的 QGraphicsView 和 QGraphicsScene 绘制。
系统先递归计算每个子树需要的宽度，再把父节点放在子树中间，子节点按左右分布。
父节点和子节点之间用线连接。
节点卡片显示姓名、ID、性别和代数，点击节点可以查看成员详情。
```

### Q47：亲缘链路是怎么实现的？

参考回答：

```text
亲缘链路在业务层使用 BFS 实现。
系统先从 parent_child_relations 表取出亲子边，将 parent-child 看成无向边构建图。
然后从成员 A 出发广度优先搜索成员 B。
找到后通过 previous 记录回溯路径。
界面层用 QGraphicsView 把路径画成节点和箭头。
```

### Q48：亲缘链路是否包含婚姻关系？

参考回答：

```text
当前 TreeService 的 BFS 主要使用 parent_child_relations 中的血缘边构建图。
界面中的婚姻关系有单独维护和统计。
如果后续要把配偶也纳入亲缘链路，可以在 buildGraph 中额外读取 marriages 表，把 person1_id 和 person2_id 也作为无向边加入图。
```

## 9. C++ 代码实现问题

### Q49：为什么要分 DAO、Service、UI 三层？

参考回答：

```text
DAO 层负责 SQL 和数据库访问。
Service 层负责业务逻辑，例如登录认证、统计、树查询和 BFS。
UI 层负责 Qt 界面展示和用户交互。
这样分层后，界面不直接写复杂 SQL，代码结构更清晰，也方便测试和维护。
```

### Q50：QSqlQuery 中为什么使用 bindValue？

参考回答：

```text
bindValue 用于参数绑定，避免直接拼接 SQL 字符串。
这样可以减少 SQL 注入风险，也能让代码更清晰。
例如 WHERE username = :username，然后使用 bindValue(':username', username) 绑定实际值。
```

### Q51：为什么 DAO 函数返回 optional？

参考回答：

```text
对于按 ID 或用户名查询单条记录的函数，查询结果可能存在，也可能不存在。
使用 std::optional 可以明确表达“有值”或“无值”，比返回空对象更清晰。
例如 findById 找不到成员时返回 std::nullopt。
```

### Q52：错误信息怎么处理？

参考回答：

```text
DAO 层保存 lastError_。
如果 query.exec() 失败，就把 query.lastError().text() 保存下来。
UI 层在操作失败时通过 dao.lastError() 显示中文提示或数据库错误信息。
```

### Q53：为什么 Stage2Smoke 有必要？

参考回答：

```text
Stage2Smoke 是自动冒烟测试。
它会验证数据库连接、admin 登录、族谱加载、Dashboard 统计和成员列表。
验收前运行它可以快速确认核心链路没有损坏。
```

## 10. 演示和提交问题

### Q54：你要提交哪些文件？

参考回答：

```text
实验报告 PDF 或 Word。
ER 图图片。
SQL 执行结果截图。
EXPLAIN 性能分析截图或 performance_explain.txt。
数据库备份文件 genealogy_lab.backup。
分支导出文件 branch_export.csv。
数据生成工具源码 tools/generate_data.py。
项目源码 src、sql、tools、tests、scripts、docs。
README.md 和 CMakeLists.txt。
```

### Q55：数据库备份怎么生成？

参考回答：

```text
使用 PostgreSQL 的 pg_dump：
pg_dump "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -Fc -f generated_data/genealogy_lab.backup
```

### Q56：如果老师要求现场运行，运行命令是什么？

参考回答：

```bash
cd "/mnt/c/Users/Sherry Peng/OneDrive/桌面/shujuku"
cmake -S . -B build -G Ninja
cmake --build build
./scripts/run_wsl.sh
```

### Q57：如果中文无法输入怎么办？

参考回答：

```text
项目提供 scripts/setup_fcitx_wsl.sh 和 scripts/run_wsl.sh。
setup 脚本配置 fcitx5 拼音输入法。
run 脚本设置 QT_IM_MODULE=fcitx、XMODIFIERS=@im=fcitx、QT_QPA_PLATFORM=xcb 等环境变量。
因此推荐通过 ./scripts/run_wsl.sh 启动程序。
```

### Q58：如果脚本出现 bash\r 错误怎么办？

参考回答：

```text
这是 Windows CRLF 换行导致的。
可以执行：
dos2unix scripts/*.sh
chmod +x scripts/*.sh
然后重新运行 ./scripts/run_wsl.sh。
```

### Q59：如果数据库连接失败怎么办？

参考回答：

```text
先检查 PostgreSQL 服务是否启动：
pg_isready
再检查数据库和用户是否存在：
genealogy_lab
genealogy_user / genealogy_pass
然后重新执行 sql/01_schema.sql、sql/02_indexes.sql、sql/03_triggers.sql 和 seed 数据脚本。
```

### Q60：项目还有哪些可以继续改进？

参考回答：

```text
可以继续加入分页加载、GUI 导出 CSV、亲缘链路中加入婚姻边、更多角色管理功能，以及更完整的单元测试。
当前版本已经覆盖课程验收要求的核心功能，包括图形界面、数据库建模、递归查询、大数据生成、导入导出和索引性能分析。
```

