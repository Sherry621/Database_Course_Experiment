# SQL 语句代码详细解析

本文档专门说明项目中所有涉及 SQL 的代码，包括数据库脚本 `sql/*.sql`，以及 C++ 代码中通过 `QSqlQuery::prepare()` 执行的 SQL。验收时可以用它回答“SQL 写在哪里、每条 SQL 做什么、为什么这样设计”的问题。

## 1. SQL 在项目中的位置

项目中的 SQL 分为三类：

| 位置 | 作用 |
|---|---|
| `sql/*.sql` | 建表、索引、触发器、核心查询、导入导出、性能测试 |
| `src/db/*.cpp` | DAO 数据访问层，执行增删改查 |
| `src/service/*.cpp` | 业务服务层，执行登录认证和 Dashboard 统计 |

C++ 中统一使用 Qt 的 `QSqlQuery` 执行 SQL：

```cpp
QSqlQuery query(DatabaseManager::instance().database());
query.prepare("SELECT ... WHERE id = :id");
query.bindValue(":id", id);
query.exec();
```

这样做的好处：

- 使用参数绑定，避免直接拼接字符串。
- 减少 SQL 注入风险。
- 便于复用数据库连接。
- 出错时可以通过 `query.lastError().text()` 获取数据库错误。

## 2. 数据库结构脚本

### 2.1 `sql/01_schema.sql`

该文件负责创建系统的六张核心表。

#### `users`

```sql
CREATE TABLE IF NOT EXISTS users (
    user_id SERIAL PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    real_name VARCHAR(50),
    email VARCHAR(100) UNIQUE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);
```

作用：

```text
保存系统用户，支持登录和注册。
```

关键设计：

- `user_id SERIAL PRIMARY KEY`：用户主键，自增。
- `username UNIQUE`：用户名不能重复。
- `password_hash`：保存密码哈希，不直接保存明文密码。
- `email UNIQUE`：邮箱唯一。



逐行解析：

SQL：

```sql
CREATE TABLE IF NOT EXISTS users (
    user_id SERIAL PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    real_name VARCHAR(50),
    email VARCHAR(100) UNIQUE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);
```

逐行说明：

```text
CREATE TABLE IF NOT EXISTS users (
```

创建 `users` 表。如果表已经存在，不重复创建，避免重复执行初始化脚本时报错。

```text
user_id SERIAL PRIMARY KEY,
```

定义用户主键。`SERIAL` 表示 PostgreSQL 自动生成递增整数，`PRIMARY KEY` 保证每个用户唯一。

```text
username VARCHAR(50) NOT NULL UNIQUE,
```

定义用户名，最长 50 个字符。`NOT NULL` 表示不能为空，`UNIQUE` 表示用户名不能重复。

```text
password_hash VARCHAR(255) NOT NULL,
```

保存密码哈希值。系统不保存明文密码，登录时把输入密码哈希后再比较。

```text
real_name VARCHAR(50),
```

保存用户真实姓名，可以为空。

```text
email VARCHAR(100) UNIQUE,
```

保存邮箱，允许为空，但如果填写则不能重复。

```text
created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
```

保存创建时间，默认值为当前时间。

```text
);
```

结束建表语句。
#### `genealogies`

```sql
CREATE TABLE IF NOT EXISTS genealogies (
    genealogy_id SERIAL PRIMARY KEY,
    title VARCHAR(100) NOT NULL,
    family_surname VARCHAR(20) NOT NULL,
    revision_time DATE,
    creator_user_id INT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    description TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);
```

作用：

```text
保存族谱基本信息。
```

关键设计：

- `creator_user_id` 外键引用 `users(user_id)`。
- `ON DELETE CASCADE` 表示创建用户被删除时，其创建的族谱也会被删除。
- 一个用户可以创建多个族谱，所以 `users` 到 `genealogies` 是一对多关系。



逐行解析：

SQL：

```sql
CREATE TABLE IF NOT EXISTS genealogies (
    genealogy_id SERIAL PRIMARY KEY,
    title VARCHAR(100) NOT NULL,
    family_surname VARCHAR(20) NOT NULL,
    revision_time DATE,
    creator_user_id INT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    description TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);
```

逐行说明：

```text
CREATE TABLE IF NOT EXISTS genealogies (
```

创建族谱表。

```text
genealogy_id SERIAL PRIMARY KEY,
```

族谱主键，自增。

```text
title VARCHAR(100) NOT NULL,
```

谱名，不能为空。

```text
family_surname VARCHAR(20) NOT NULL,
```

姓氏，不能为空。

```text
revision_time DATE,
```

修谱时间，使用日期类型。

```text
creator_user_id INT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
```

创建用户 ID，外键引用 `users(user_id)`。`ON DELETE CASCADE` 表示如果创建用户被删除，其创建的族谱也随之删除。

```text
description TEXT,
```

族谱备注，使用 `TEXT` 支持较长说明。

```text
created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
```

族谱创建时间。
#### `genealogy_collaborators`

```sql
CREATE TABLE IF NOT EXISTS genealogy_collaborators (
    genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
    user_id INT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    role VARCHAR(20) NOT NULL DEFAULT 'editor',
    invited_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (genealogy_id, user_id),
    CHECK (role IN ('editor', 'viewer'))
);
```

作用：

```text
保存用户和族谱之间的协作关系。
```

关键设计：

- `(genealogy_id, user_id)` 是复合主键，避免同一个用户被重复加入同一个族谱。
- `role` 只能是 `editor` 或 `viewer`。
- 族谱创建者不需要写入协作者表，创建者身份由 `genealogies.creator_user_id` 判断。



逐行解析：

SQL：

```sql
CREATE TABLE IF NOT EXISTS genealogy_collaborators (
    genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
    user_id INT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    role VARCHAR(20) NOT NULL DEFAULT 'editor',
    invited_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (genealogy_id, user_id),
    CHECK (role IN ('editor', 'viewer'))
);
```

逐行说明：

```text
genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
```

协作的族谱 ID，引用族谱表。族谱删除后，对应协作者记录自动删除。

```text
user_id INT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
```

协作者用户 ID，引用用户表。用户删除后，对应协作者记录自动删除。

```text
role VARCHAR(20) NOT NULL DEFAULT 'editor',
```

协作者角色，默认是 `editor`。

```text
invited_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
```

记录邀请时间。

```text
PRIMARY KEY (genealogy_id, user_id),
```

复合主键。一个用户在同一个族谱中只能有一条协作记录。

```text
CHECK (role IN ('editor', 'viewer'))
```

限制角色只能是 `editor` 或 `viewer`。
#### `members`

```sql
CREATE TABLE IF NOT EXISTS members (
    member_id SERIAL PRIMARY KEY,
    genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
    name VARCHAR(100) NOT NULL,
    gender CHAR(1) NOT NULL CHECK (gender IN ('M', 'F')),
    birth_year INT,
    death_year INT,
    generation INT,
    biography TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CHECK (birth_year IS NULL OR death_year IS NULL OR birth_year <= death_year),
    CHECK (generation IS NULL OR generation >= 1)
);
```

作用：

```text
保存族谱中的人物信息。
```

关键设计：

- `member_id` 是唯一标识，用于区分同名成员。
- `genealogy_id` 表示成员属于哪个族谱。
- `gender` 用 `M/F` 存储，界面显示为 `男/女`。
- `birth_year <= death_year` 保证生卒年基本合法。
- `generation >= 1` 保证代数为正数。



逐行解析：

SQL：

```sql
CREATE TABLE IF NOT EXISTS members (
    member_id SERIAL PRIMARY KEY,
    genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
    name VARCHAR(100) NOT NULL,
    gender CHAR(1) NOT NULL CHECK (gender IN ('M', 'F')),
    birth_year INT,
    death_year INT,
    generation INT,
    biography TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CHECK (birth_year IS NULL OR death_year IS NULL OR birth_year <= death_year),
    CHECK (generation IS NULL OR generation >= 1)
);
```

逐行说明：

```text
member_id SERIAL PRIMARY KEY,
```

成员主键。使用 ID 区分同名同姓成员。

```text
genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
```

成员所属族谱。族谱删除时，该族谱下成员自动删除。

```text
name VARCHAR(100) NOT NULL,
```

成员姓名，不能为空。

```text
gender CHAR(1) NOT NULL CHECK (gender IN ('M', 'F')),
```

性别字段，只允许 `M` 或 `F`。界面中显示为“男”和“女”。

```text
birth_year INT,
death_year INT,
```

出生年份和死亡年份，允许为空。

```text
generation INT,
```

成员代数，允许为空。

```text
biography TEXT,
```

生平简介。

```text
created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
```

成员创建时间。

```text
CHECK (birth_year IS NULL OR death_year IS NULL OR birth_year <= death_year),
```

如果出生年和死亡年都填写，则出生年不能晚于死亡年。

```text
CHECK (generation IS NULL OR generation >= 1)
```

如果填写代数，则代数必须大于等于 1。
#### `parent_child_relations`

```sql
CREATE TABLE IF NOT EXISTS parent_child_relations (
    relation_id SERIAL PRIMARY KEY,
    genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
    parent_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
    child_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
    relation_type VARCHAR(10) NOT NULL CHECK (relation_type IN ('father', 'mother')),
    CHECK (parent_id <> child_id),
    UNIQUE (child_id, relation_type)
);
```

作用：

```text
保存父亲/母亲与子女之间的血缘关系。
```

关键设计：

- 这是 `members` 表的自关联。
- `parent_id` 和 `child_id` 都引用 `members(member_id)`。
- `relation_type` 区分父亲和母亲。
- `UNIQUE(child_id, relation_type)` 保证一个孩子最多有一个父亲记录和一个母亲记录。



逐行解析：

SQL：

```sql
CREATE TABLE IF NOT EXISTS parent_child_relations (
    relation_id SERIAL PRIMARY KEY,
    genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
    parent_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
    child_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
    relation_type VARCHAR(10) NOT NULL CHECK (relation_type IN ('father', 'mother')),
    CHECK (parent_id <> child_id),
    UNIQUE (child_id, relation_type)
);
```

逐行说明：

```text
relation_id SERIAL PRIMARY KEY,
```

亲子关系主键。

```text
genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
```

关系所属族谱。

```text
parent_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
```

父母节点 ID，引用成员表。

```text
child_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
```

子女节点 ID，引用成员表。

```text
relation_type VARCHAR(10) NOT NULL CHECK (relation_type IN ('father', 'mother')),
```

关系类型，只能是父亲或母亲。

```text
CHECK (parent_id <> child_id),
```

防止自己成为自己的父母。

```text
UNIQUE (child_id, relation_type)
```

保证一个孩子最多只有一个父亲记录和一个母亲记录。
#### `marriages`

```sql
CREATE TABLE IF NOT EXISTS marriages (
    marriage_id SERIAL PRIMARY KEY,
    genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
    person1_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
    person2_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
    marriage_year INT,
    divorce_year INT,
    description TEXT,
    CHECK (person1_id <> person2_id),
    CHECK (marriage_year IS NULL OR divorce_year IS NULL OR marriage_year <= divorce_year),
    UNIQUE (genealogy_id, person1_id, person2_id)
);
```

作用：

```text
保存成员之间的婚姻关系。
```

逐行解析：

```text
CREATE TABLE IF NOT EXISTS marriages (
```

创建婚姻关系表。如果表已经存在，则不重复创建，保证初始化脚本可以多次执行。

```text
marriage_id SERIAL PRIMARY KEY,
```

定义婚姻关系主键。`SERIAL` 表示自增整数，`PRIMARY KEY` 表示每条婚姻记录唯一。

```text
genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
```

记录该婚姻关系属于哪个族谱。它引用 `genealogies(genealogy_id)`，并且族谱删除时，相关婚姻记录自动删除。

```text
person1_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
```

婚姻关系中的第一个成员，引用 `members(member_id)`。如果该成员被删除，相关婚姻记录自动删除。

```text
person2_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
```

婚姻关系中的第二个成员，同样引用成员表。

```text
marriage_year INT,
```

结婚年份，允许为空，因为部分族谱资料可能缺失具体年份。

```text
divorce_year INT,
```

离婚年份，允许为空。没有离婚或资料未知时可以不填。

```text
description TEXT,
```

婚姻关系备注，使用 `TEXT` 以支持较长说明。

```text
CHECK (person1_id <> person2_id),
```

检查婚姻双方不能是同一个成员。

```text
CHECK (marriage_year IS NULL OR divorce_year IS NULL OR marriage_year <= divorce_year),
```

检查结婚年份和离婚年份的逻辑关系：如果两个年份都填写，则结婚年份不能晚于离婚年份。

```text
UNIQUE (genealogy_id, person1_id, person2_id)
```

保证同一个族谱中同一对成员不会重复保存婚姻关系。

```text
);
```

结束建表语句。

关键设计：

- 也是 `members` 表的自关联。
- `person1_id <> person2_id` 防止自己和自己结婚。
- `marriage_year <= divorce_year` 保证婚姻年份合法。
- `UNIQUE(genealogy_id, person1_id, person2_id)` 防止重复婚姻记录。

## 3. 索引脚本

### `sql/02_indexes.sql`

该文件负责优化常用查询。

#### 族谱创建者索引

```sql
CREATE INDEX IF NOT EXISTS idx_genealogies_creator
ON genealogies(creator_user_id);
```

用途：

```text
快速查询某个用户创建的族谱。
```

对应功能：

```text
登录后加载当前用户可访问族谱。
```



逐行解析：

示例：

```sql
CREATE INDEX IF NOT EXISTS idx_parent_child_parent_id
ON parent_child_relations(parent_id);
```

逐行说明：

```text
CREATE INDEX IF NOT EXISTS idx_parent_child_parent_id
```

创建名为 `idx_parent_child_parent_id` 的索引，如果已经存在则跳过。

```text
ON parent_child_relations(parent_id);
```

索引建立在 `parent_child_relations` 表的 `parent_id` 字段上。

用途：

```text
优化“根据父节点查询子节点”的查询，例如后代树和四代后代性能测试。
```

姓名模糊查询索引：

```sql
CREATE INDEX IF NOT EXISTS idx_members_name_trgm
ON members USING GIN (name gin_trgm_ops);
```

逐行说明：

```text
CREATE INDEX IF NOT EXISTS idx_members_name_trgm
```

创建姓名模糊查询索引。

```text
ON members USING GIN (name gin_trgm_ops);
```

在 `members.name` 上建立 GIN 索引，并使用 `pg_trgm` 的三元组操作类。

用途：

```text
优化 name LIKE '%关键字%' 查询。
```
#### 成员族谱索引

```sql
CREATE INDEX IF NOT EXISTS idx_members_genealogy
ON members(genealogy_id);
```

用途：

```text
快速查询某个族谱下的所有成员。
```

对应功能：

```text
成员管理列表、Dashboard 统计。
```

#### 成员代数索引

```sql
CREATE INDEX IF NOT EXISTS idx_members_genealogy_generation
ON members(genealogy_id, generation);
```

用途：

```text
按族谱和代数筛选或排序成员。
```

对应功能：

```text
成员列表、统计分析、同辈平均出生年份查询。
```

#### 姓名模糊查询索引

```sql
CREATE INDEX IF NOT EXISTS idx_members_name_trgm
ON members USING GIN (name gin_trgm_ops);
```

用途：

```text
优化 LIKE '%关键字%' 形式的姓名模糊查询。
```

说明：

普通 B-Tree 索引通常不能很好优化前后都有 `%` 的模糊查询。PostgreSQL 的 `pg_trgm` 扩展可以把字符串拆成三元片段，再用 GIN 索引加速模糊匹配。

对应功能：

```text
成员管理页面姓名关键词搜索。
```

#### 亲子关系索引

```sql
CREATE INDEX IF NOT EXISTS idx_parent_child_parent_id
ON parent_child_relations(parent_id);

CREATE INDEX IF NOT EXISTS idx_parent_child_child_id
ON parent_child_relations(child_id);

CREATE INDEX IF NOT EXISTS idx_parent_child_genealogy
ON parent_child_relations(genealogy_id);
```

用途：

- `parent_id`：快速查询某个成员的子女。
- `child_id`：快速向上追溯父母和祖先。
- `genealogy_id`：快速取出某个族谱内的所有亲子边。

对应功能：

```text
后代树、祖先查询、亲缘链路 BFS、性能测试。
```

#### 婚姻关系索引

```sql
CREATE INDEX IF NOT EXISTS idx_marriages_person1
ON marriages(person1_id);

CREATE INDEX IF NOT EXISTS idx_marriages_person2
ON marriages(person2_id);
```

用途：

```text
快速查询某个成员的配偶。
```

对应功能：

```text
核心 SQL 查询配偶、婚姻关系维护。
```

## 4. 触发器脚本

### `sql/03_triggers.sql`

触发器用于处理 CHECK 约束难以表达的跨表业务规则。

### 4.1 亲子关系触发器

函数：

```sql
CREATE OR REPLACE FUNCTION check_parent_child_relation()
RETURNS TRIGGER AS $$
...
$$ LANGUAGE plpgsql;
```

触发时机：

```sql
CREATE TRIGGER trg_check_parent_child_relation
BEFORE INSERT OR UPDATE ON parent_child_relations
FOR EACH ROW
EXECUTE FUNCTION check_parent_child_relation();
```

含义：

```text
每次插入或修改亲子关系前，都先执行 check_parent_child_relation。
```

核心检查：

```sql
SELECT genealogy_id, birth_year, gender, generation
INTO parent_genealogy, parent_birth, parent_gender, parent_generation
FROM members
WHERE member_id = NEW.parent_id;
```

读取父母节点信息。

```sql
SELECT genealogy_id, birth_year, generation
INTO child_genealogy, child_birth, child_generation
FROM members
WHERE member_id = NEW.child_id;
```

读取子女节点信息。

```sql
IF parent_genealogy <> child_genealogy OR NEW.genealogy_id <> parent_genealogy THEN
    RAISE EXCEPTION '亲子关系双方必须属于同一个族谱';
END IF;
```

保证父母和子女属于同一族谱。

```sql
IF NEW.relation_type = 'father' AND parent_gender <> 'M' THEN
    RAISE EXCEPTION '父亲关系要求父节点性别为男性';
END IF;
```

保证父亲节点必须是男性。

```sql
IF NEW.relation_type = 'mother' AND parent_gender <> 'F' THEN
    RAISE EXCEPTION '母亲关系要求父节点性别为女性';
END IF;
```

保证母亲节点必须是女性。

```sql
IF parent_birth IS NOT NULL AND child_birth IS NOT NULL AND parent_birth >= child_birth THEN
    RAISE EXCEPTION '父母出生年份必须早于子女出生年份';
END IF;
```

保证父母出生年份早于子女。

```sql
IF parent_generation IS NOT NULL AND child_generation IS NOT NULL
   AND parent_generation >= child_generation THEN
    RAISE EXCEPTION '父母辈分必须早于子女辈分';
END IF;
```

保证父母代数小于子女代数。



逐行解析：

SQL 片段：

```sql
CREATE OR REPLACE FUNCTION check_parent_child_relation()
RETURNS TRIGGER AS $$
DECLARE
    parent_genealogy INT;
    child_genealogy INT;
    parent_birth INT;
    child_birth INT;
    parent_gender CHAR(1);
    parent_generation INT;
    child_generation INT;
BEGIN
```

逐行说明：

```text
CREATE OR REPLACE FUNCTION check_parent_child_relation()
```

创建或替换触发器函数。

```text
RETURNS TRIGGER AS $$
```

声明该函数返回触发器类型。

```text
DECLARE
```

声明局部变量区域。

```text
parent_genealogy、child_genealogy
```

保存父母和子女所属族谱。

```text
parent_birth、child_birth
```

保存父母和子女出生年份。

```text
parent_gender
```

保存父母节点性别，用于判断父亲/母亲关系是否合法。

```text
parent_generation、child_generation
```

保存父母和子女代数。

```text
BEGIN
```

触发器函数逻辑开始。

SQL 片段：

```sql
SELECT genealogy_id, birth_year, gender, generation
INTO parent_genealogy, parent_birth, parent_gender, parent_generation
FROM members
WHERE member_id = NEW.parent_id;
```

逐行说明：

```text
SELECT genealogy_id, birth_year, gender, generation
```

查询父母节点的族谱、出生年、性别和代数。

```text
INTO parent_genealogy, parent_birth, parent_gender, parent_generation
```

把查询结果存入局部变量。

```text
FROM members
```

数据来自成员表。

```text
WHERE member_id = NEW.parent_id;
```

`NEW` 表示即将插入或更新的亲子关系记录。这里用新记录中的 `parent_id` 找父母成员。

SQL 片段：

```sql
IF parent_genealogy <> child_genealogy OR NEW.genealogy_id <> parent_genealogy THEN
    RAISE EXCEPTION '亲子关系双方必须属于同一个族谱';
END IF;
```

逐行说明：

```text
IF parent_genealogy <> child_genealogy OR NEW.genealogy_id <> parent_genealogy THEN
```

如果父母和子女所属族谱不同，或者关系记录中的族谱 ID 与成员族谱不一致，则进入异常分支。

```text
RAISE EXCEPTION '亲子关系双方必须属于同一个族谱';
```

抛出数据库异常，阻止插入或更新。

```text
END IF;
```

结束判断。

SQL 片段：

```sql
IF NEW.relation_type = 'father' AND parent_gender <> 'M' THEN
    RAISE EXCEPTION '父亲关系要求父节点性别为男性';
END IF;
```

逐行说明：

```text
IF NEW.relation_type = 'father' AND parent_gender <> 'M' THEN
```

如果关系类型是父亲，但父节点性别不是男性，则不合法。

```text
RAISE EXCEPTION ...
```

抛出异常，拒绝错误数据。

SQL 片段：

```sql
IF parent_birth IS NOT NULL AND child_birth IS NOT NULL AND parent_birth >= child_birth THEN
    RAISE EXCEPTION '父母出生年份必须早于子女出生年份';
END IF;
```

逐行说明：

```text
parent_birth IS NOT NULL AND child_birth IS NOT NULL
```

只有父母和子女出生年都存在时才比较。

```text
parent_birth >= child_birth
```

如果父母出生年份不早于子女，则不合法。
### 4.2 婚姻关系触发器

函数：

```sql
CREATE OR REPLACE FUNCTION check_marriage_relation()
RETURNS TRIGGER AS $$
...
$$ LANGUAGE plpgsql;
```

触发时机：

```sql
CREATE TRIGGER trg_check_marriage_relation
BEFORE INSERT OR UPDATE ON marriages
FOR EACH ROW
EXECUTE FUNCTION check_marriage_relation();
```

核心检查：

```sql
IF NEW.person1_id = NEW.person2_id THEN
    RAISE EXCEPTION '婚姻关系双方不能是同一个人';
END IF;
```

防止自己和自己建立婚姻关系。

```sql
IF NEW.person1_id > NEW.person2_id THEN
    tmp_person_id := NEW.person1_id;
    NEW.person1_id := NEW.person2_id;
    NEW.person2_id := tmp_person_id;
END IF;
```

统一 `person1_id` 和 `person2_id` 的顺序。这样 `(3, 8)` 和 `(8, 3)` 会统一保存成 `(3, 8)`，配合唯一约束避免重复。

```sql
SELECT genealogy_id INTO p1_genealogy FROM members WHERE member_id = NEW.person1_id;
SELECT genealogy_id INTO p2_genealogy FROM members WHERE member_id = NEW.person2_id;
```

查询婚姻双方所属族谱。

```sql
IF p1_genealogy <> p2_genealogy OR NEW.genealogy_id <> p1_genealogy THEN
    RAISE EXCEPTION '婚姻关系双方必须属于同一个族谱';
END IF;
```

保证婚姻双方属于同一族谱。

## 5. 核心查询脚本

### `sql/04_core_queries.sql`

该文件对应老师要求的五类核心 SQL。

### 5.1 查询配偶及所有子女

```sql
SELECT 'spouse' AS relation, m.*
FROM marriages mr
JOIN members m ON m.member_id = CASE
    WHEN mr.person1_id = :member_id THEN mr.person2_id
    ELSE mr.person1_id
END
WHERE mr.person1_id = :member_id OR mr.person2_id = :member_id
UNION ALL
SELECT 'child' AS relation, c.*
FROM parent_child_relations pcr
JOIN members c ON c.member_id = pcr.child_id
WHERE pcr.parent_id = :member_id;
```

参数：

```text
:member_id 目标成员 ID
```

实现原理：

- 第一段从 `marriages` 查配偶。
- `CASE` 用于判断目标成员在 `person1_id` 还是 `person2_id`。
- 第二段从 `parent_child_relations` 查子女。
- `UNION ALL` 把配偶和子女结果合并。
- `relation` 字段标明结果类型是 `spouse` 还是 `child`。



逐行解析：

SQL：

```sql
SELECT 'spouse' AS relation, m.*
FROM marriages mr
JOIN members m ON m.member_id = CASE
    WHEN mr.person1_id = :member_id THEN mr.person2_id
    ELSE mr.person1_id
END
WHERE mr.person1_id = :member_id OR mr.person2_id = :member_id
UNION ALL
SELECT 'child' AS relation, c.*
FROM parent_child_relations pcr
JOIN members c ON c.member_id = pcr.child_id
WHERE pcr.parent_id = :member_id;
```

逐行说明：

```text
SELECT 'spouse' AS relation, m.*
```

查询配偶，并增加一列 `relation`，值固定为 `spouse`。

```text
FROM marriages mr
```

从婚姻关系表查询，别名为 `mr`。

```text
JOIN members m ON m.member_id = CASE
```

连接成员表，配偶也是成员表中的一条记录。这里用 `CASE` 判断配偶 ID。

```text
WHEN mr.person1_id = :member_id THEN mr.person2_id
```

如果目标成员是 `person1_id`，则配偶是 `person2_id`。

```text
ELSE mr.person1_id
```

否则配偶是 `person1_id`。

```text
WHERE mr.person1_id = :member_id OR mr.person2_id = :member_id
```

只查询包含目标成员的婚姻记录。

```text
UNION ALL
```

把配偶查询结果和子女查询结果合并。

```text
SELECT 'child' AS relation, c.*
```

查询子女，并把关系类型标记为 `child`。

```text
FROM parent_child_relations pcr
```

从亲子关系表查询。

```text
JOIN members c ON c.member_id = pcr.child_id
```

连接成员表，获取子女详情。

```text
WHERE pcr.parent_id = :member_id;
```

目标成员作为父母，查出其所有子女。
### 5.2 递归查询所有祖先

```sql
WITH RECURSIVE ancestors AS (
    SELECT parent_id, child_id, 1 AS depth
    FROM parent_child_relations
    WHERE child_id = :member_id
    UNION ALL
    SELECT p.parent_id, p.child_id, a.depth + 1
    FROM parent_child_relations p
    JOIN ancestors a ON p.child_id = a.parent_id
)
SELECT depth, m.*
FROM ancestors a
JOIN members m ON m.member_id = a.parent_id
ORDER BY depth, m.birth_year;
```

参数：

```text
:member_id 目标成员 ID
```

实现原理：

- 初始查询找出目标成员的父母。
- 递归部分继续查父母的父母。
- `depth` 表示距离目标成员的层级。
- 最后连接 `members` 表得到祖先详情。

为什么使用 Recursive CTE：

```text
族谱层级深度不固定，无法预先写死 JOIN 层数。
```



逐行解析：

SQL：

```sql
WITH RECURSIVE ancestors AS (
    SELECT parent_id, child_id, 1 AS depth
    FROM parent_child_relations
    WHERE child_id = :member_id
    UNION ALL
    SELECT p.parent_id, p.child_id, a.depth + 1
    FROM parent_child_relations p
    JOIN ancestors a ON p.child_id = a.parent_id
)
SELECT depth, m.*
FROM ancestors a
JOIN members m ON m.member_id = a.parent_id
ORDER BY depth, m.birth_year;
```

逐行说明：

```text
WITH RECURSIVE ancestors AS (
```

定义一个递归公用表表达式，临时结果表名为 `ancestors`。

```text
SELECT parent_id, child_id, 1 AS depth
```

递归的初始查询。先查目标成员的直接父母，并把深度设为 1。

```text
FROM parent_child_relations
```

从亲子关系表中查。

```text
WHERE child_id = :member_id
```

目标成员作为子女，查出其父母。

```text
UNION ALL
```

把初始查询结果和后续递归查询结果合并。`ALL` 表示不去重，效率更高。

```text
SELECT p.parent_id, p.child_id, a.depth + 1
```

递归查询。继续查上一层祖先的父母，并把深度加 1。

```text
FROM parent_child_relations p
```

再次从亲子关系表中查。

```text
JOIN ancestors a ON p.child_id = a.parent_id
```

把当前关系中的 `child_id` 与上一轮查到的 `parent_id` 匹配。含义是：查“上一轮父母”的父母。

```text
)
```

递归 CTE 定义结束。

```text
SELECT depth, m.*
```

输出深度和成员详情。

```text
FROM ancestors a
```

从递归结果中取数据。

```text
JOIN members m ON m.member_id = a.parent_id
```

把祖先 ID 连接到成员表，获取姓名、性别、生卒年等完整信息。

```text
ORDER BY depth, m.birth_year;
```

先按距离目标成员的层级排序，再按出生年份排序。
### 5.3 统计平均寿命最长的一代

```sql
SELECT generation, AVG(death_year - birth_year) AS avg_life
FROM members
WHERE genealogy_id = :genealogy_id
  AND birth_year IS NOT NULL
  AND death_year IS NOT NULL
  AND generation IS NOT NULL
GROUP BY generation
ORDER BY avg_life DESC
LIMIT 1;
```

参数：

```text
:genealogy_id 族谱 ID
```

实现原理：

- 使用 `death_year - birth_year` 计算寿命。
- 按 `generation` 分组。
- 使用 `AVG` 计算每代平均寿命。
- 按平均寿命降序排列，取第一条。

### 5.4 查询 50 岁以上且没有配偶的男性成员

```sql
SELECT m.*
FROM members m
WHERE m.genealogy_id = :genealogy_id
  AND m.gender = 'M'
  AND COALESCE(m.death_year, EXTRACT(YEAR FROM CURRENT_DATE)::INT) - m.birth_year > 50
  AND NOT EXISTS (
      SELECT 1
      FROM marriages mr
      WHERE mr.person1_id = m.member_id OR mr.person2_id = m.member_id
  );
```

实现原理：

- `gender = 'M'` 限定男性。
- `COALESCE(death_year, 当前年份)`：如果未死亡，用当前年份估算年龄。
- `NOT EXISTS` 检查婚姻表中不存在该成员。

### 5.5 查询出生年份早于同辈平均出生年份的成员

```sql
WITH generation_avg AS (
    SELECT generation, AVG(birth_year) AS avg_birth_year
    FROM members
    WHERE genealogy_id = :genealogy_id
      AND birth_year IS NOT NULL
      AND generation IS NOT NULL
    GROUP BY generation
)
SELECT m.*, g.avg_birth_year
FROM members m
JOIN generation_avg g ON g.generation = m.generation
WHERE m.genealogy_id = :genealogy_id
  AND m.birth_year < g.avg_birth_year
ORDER BY m.generation, m.birth_year;
```

实现原理：

- `generation_avg` 先计算每一代平均出生年份。
- 主查询把成员和同代平均值连接。
- 筛选 `m.birth_year < g.avg_birth_year`。
- 输出比同辈平均出生年份更早的成员。

## 6. 数据导入导出脚本

### 6.1 `sql/05_load_generated_csv.sql`

该脚本使用 `\copy` 批量导入 CSV。

核心语句：

```sql
\copy users(...) FROM 'generated_data/users.csv' WITH (FORMAT csv, HEADER true, NULL '')
\copy genealogies(...) FROM 'generated_data/genealogies.csv' WITH (FORMAT csv, HEADER true, NULL '')
\copy members(...) FROM 'generated_data/members.csv' WITH (FORMAT csv, HEADER true, NULL '')
```

说明：

- `FORMAT csv` 表示 CSV 格式。
- `HEADER true` 表示第一行是表头。
- `NULL ''` 表示空字符串导入为 NULL。
- `\copy` 是 psql 客户端命令，适合从本机文件导入数据库。

导入后执行：

```sql
SELECT setval(pg_get_serial_sequence(...), (SELECT max(...) FROM ...));
```

作用：

```text
修正 SERIAL 自增序列，避免后续新增数据时主键冲突。
```

逐行解析：

```text
\copy users(...)
```

使用 psql 的 `\copy` 命令导入 `users.csv`。

```text
FROM 'generated_data/users.csv'
```

指定 CSV 文件路径。

```text
WITH (FORMAT csv, HEADER true, NULL '')
```

指定文件格式为 CSV，第一行为表头，空字符串按 NULL 处理。

```text
pg_get_serial_sequence('users', 'user_id')
```

获取 `users.user_id` 对应的自增序列名称。

```text
SELECT max(user_id) FROM users
```

查询当前表中最大的用户 ID。

```text
setval(...)
```

把自增序列当前值设置为表中最大 ID。这样后续界面新增用户或成员时，不会因为序列值落后于已导入数据而产生主键冲突。

### 6.2 `INSERT ... ON CONFLICT` 数据脚本通用写法

小数据脚本 `早期小数据脚本` 和演示数据脚本 `早期演示数据脚本` 中大量使用如下结构：

```sql
INSERT INTO users(user_id, username, password_hash, real_name, email)
VALUES (...)
ON CONFLICT (user_id) DO UPDATE
SET username = EXCLUDED.username,
    password_hash = EXCLUDED.password_hash,
    real_name = EXCLUDED.real_name,
    email = EXCLUDED.email;
```

作用：

```text
插入测试数据。如果主键已经存在，则更新旧数据，让 seed 脚本可以重复执行。
```

逐行解析：

```text
INSERT INTO users(user_id, username, password_hash, real_name, email)
```

指定要向 `users` 表插入哪些字段。

```text
VALUES (...)
```

给出要插入的数据值。

```text
ON CONFLICT (user_id) DO UPDATE
```

如果 `user_id` 主键冲突，不报错，而是执行更新。

```text
SET username = EXCLUDED.username
```

把已有记录的 `username` 更新为本次插入数据中的 `username`。

```text
EXCLUDED
```

PostgreSQL 中的特殊引用，表示“本次原本要插入但因为冲突被排除的那一行数据”。

为什么使用这种写法：

```text
验收前可能多次执行初始化脚本。使用 ON CONFLICT 后，脚本具备幂等性，重复执行也不会因为主键已存在而失败。
```

### 6.3 `sql/06_export_branch.sql`

该脚本导出指定成员的后代分支。

核心递归：

```sql
WITH RECURSIVE descendant_ids(member_id, depth) AS (
    SELECT m.member_id, 0
    FROM members m
    WHERE m.member_id = :root_id

    UNION

    SELECT r.child_id, d.depth + 1
    FROM descendant_ids d
    JOIN parent_child_relations r ON r.parent_id = d.member_id
    WHERE d.depth < :max_depth
)
```

参数：

```text
:root_id 根成员 ID
:max_depth 最大导出深度
```

实现原理：

- 从根成员开始。
- 递归查每一层子女。
- 使用 `depth` 控制导出层数。
- 最后用 `\copy` 导出到 `generated_data/branch_export.csv`。



逐行解析：

SQL 核心：

```sql
WITH RECURSIVE descendant_ids(member_id, depth) AS (
    SELECT m.member_id, 0
    FROM members m
    WHERE m.member_id = :root_id

    UNION

    SELECT r.child_id, d.depth + 1
    FROM descendant_ids d
    JOIN parent_child_relations r ON r.parent_id = d.member_id
    WHERE d.depth < :max_depth
)
```

逐行说明：

```text
WITH RECURSIVE descendant_ids(member_id, depth) AS (
```

定义递归结果表，包含成员 ID 和深度。

```text
SELECT m.member_id, 0
```

初始查询返回根成员，深度为 0。

```text
FROM members m
WHERE m.member_id = :root_id
```

根据输入的根成员 ID 找到起点。

```text
UNION
```

合并初始成员和递归查到的后代成员。

```text
SELECT r.child_id, d.depth + 1
```

递归查询子女，并把深度加 1。

```text
FROM descendant_ids d
```

从上一轮已经查到的成员继续往下扩展。

```text
JOIN parent_child_relations r ON r.parent_id = d.member_id
```

根据父节点 ID 找到子节点。

```text
WHERE d.depth < :max_depth
```

限制最大递归深度，防止导出过多层级。
### 6.4 `sql/07_performance_explain.sql`

该脚本用于性能对比。

无索引阶段：

```sql
DROP INDEX IF EXISTS idx_parent_child_parent_id;
DROP INDEX IF EXISTS idx_parent_child_child_id;
ANALYZE parent_child_relations;
ANALYZE members;
EXPLAIN (ANALYZE, BUFFERS) ...
```

有索引阶段：

```sql
CREATE INDEX IF NOT EXISTS idx_parent_child_parent_id
ON parent_child_relations(parent_id);

CREATE INDEX IF NOT EXISTS idx_parent_child_child_id
ON parent_child_relations(child_id);

ANALYZE parent_child_relations;
ANALYZE members;
EXPLAIN (ANALYZE, BUFFERS) ...
```

测试查询：

```sql
WITH RECURSIVE descendants AS (...)
SELECT *
FROM descendants
WHERE depth = 4;
```

用途：

```text
对比查询某曾祖父所有曾孙，即四代后代查询，在有无 parent_id 索引时的执行计划和执行时间。
```

`EXPLAIN (ANALYZE, BUFFERS)` 的含义：

- `EXPLAIN`：显示执行计划。
- `ANALYZE`：实际执行并统计耗时。
- `BUFFERS`：显示缓冲区读取情况。



逐行解析：

无索引阶段：

```sql
DROP INDEX IF EXISTS idx_parent_child_parent_id;
DROP INDEX IF EXISTS idx_parent_child_child_id;
ANALYZE parent_child_relations;
ANALYZE members;
EXPLAIN (ANALYZE, BUFFERS)
...
```

逐行说明：

```text
DROP INDEX IF EXISTS idx_parent_child_parent_id;
```

删除父节点索引，用于模拟无索引场景。

```text
DROP INDEX IF EXISTS idx_parent_child_child_id;
```

删除子节点索引。

```text
ANALYZE parent_child_relations;
ANALYZE members;
```

让 PostgreSQL 重新收集表统计信息，保证执行计划更准确。

```text
EXPLAIN (ANALYZE, BUFFERS)
```

执行 SQL 并输出真实执行时间、执行计划和缓冲区使用情况。

有索引阶段：

```sql
CREATE INDEX IF NOT EXISTS idx_parent_child_parent_id
ON parent_child_relations(parent_id);
```

逐行说明：

```text
CREATE INDEX IF NOT EXISTS idx_parent_child_parent_id
```

创建父节点索引。

```text
ON parent_child_relations(parent_id);
```

索引字段是 `parent_id`，正好对应“根据父节点查子节点”的查询条件。
## 7. C++ 中的 SQL：用户模块

### 7.1 `UserDao::findByUsername`

文件：

```text
src/db/UserDao.cpp
```

SQL：

```sql
SELECT user_id, username, real_name, email
FROM users
WHERE username = :username
```

用途：

```text
根据用户名查询用户信息。
```

参数：

```text
:username 用户名
```

实现细节：

- 使用 `query.bindValue(":username", username)` 绑定参数。
- 查询不到时返回 `std::nullopt`。

### 7.2 `UserDao::findById`

SQL：

```sql
SELECT user_id, username, real_name, email
FROM users
WHERE user_id = :user_id
```

用途：

```text
根据用户 ID 查询用户。
```

### 7.3 `UserDao::insert`

SQL：

```sql
INSERT INTO users(username, password_hash, real_name, email)
VALUES(:username, :password_hash, :real_name, :email)
```

用途：

```text
注册新用户。
```

说明：

密码在 `AuthService` 中先做 SHA-256 哈希，然后才传入 DAO 保存。

## 8. C++ 中的 SQL：登录认证

### `AuthService::login`

文件：

```text
src/service/AuthService.cpp
```

SQL：

```sql
SELECT user_id, username, real_name, email
FROM users
WHERE username = :username AND password_hash = :password_hash
```

用途：

```text
验证用户名和密码是否匹配。
```

实现原理：

- 用户输入密码。
- C++ 使用 `QCryptographicHash::Sha256` 计算哈希。
- SQL 用用户名和密码哈希同时匹配。
- 匹配到记录则登录成功，否则失败。

## 9. C++ 中的 SQL：族谱模块

### 9.1 `GenealogyDao::findAccessibleByUser`

SQL：

```sql
SELECT DISTINCT g.genealogy_id, g.title, g.family_surname, g.revision_time,
       g.creator_user_id, g.description
FROM genealogies g
LEFT JOIN genealogy_collaborators c ON c.genealogy_id = g.genealogy_id
WHERE g.creator_user_id = :user_id OR c.user_id = :user_id
ORDER BY g.genealogy_id
```

用途：

```text
查询当前用户能看到的族谱。
```

实现原理：

- 如果 `g.creator_user_id = 当前用户`，说明用户是创建者。
- 如果 `c.user_id = 当前用户`，说明用户是受邀协作者。
- `LEFT JOIN` 让创建者即使没有协作者记录，也能查到自己的族谱。
- `DISTINCT` 防止连接后出现重复族谱。



逐行解析：

SQL：

```sql
SELECT DISTINCT g.genealogy_id, g.title, g.family_surname, g.revision_time,
       g.creator_user_id, g.description
FROM genealogies g
LEFT JOIN genealogy_collaborators c ON c.genealogy_id = g.genealogy_id
WHERE g.creator_user_id = :user_id OR c.user_id = :user_id
ORDER BY g.genealogy_id
```

逐行说明：

```text
SELECT DISTINCT ...
```

查询族谱字段，并使用 `DISTINCT` 去重。

```text
FROM genealogies g
```

从族谱表查询，别名为 `g`。

```text
LEFT JOIN genealogy_collaborators c ON c.genealogy_id = g.genealogy_id
```

左连接协作者表。即使某个族谱没有协作者记录，只要它属于创建者，也能被查出来。

```text
WHERE g.creator_user_id = :user_id OR c.user_id = :user_id
```

满足任一条件即可访问：当前用户是创建者，或者当前用户是协作者。

```text
ORDER BY g.genealogy_id
```

按族谱 ID 排序。
### 9.2 `GenealogyDao::findById`

SQL：

```sql
SELECT genealogy_id, title, family_surname, revision_time, creator_user_id, description
FROM genealogies
WHERE genealogy_id = :genealogy_id
```

用途：

```text
查询单个族谱详情。
```

### 9.3 `GenealogyDao::insert`

SQL：

```sql
INSERT INTO genealogies(title, family_surname, revision_time, creator_user_id, description)
VALUES(:title, :family_surname, :revision_time, :creator_user_id, :description)
```

用途：

```text
新增族谱。
```

### 9.4 `GenealogyDao::update`

SQL：

```sql
UPDATE genealogies
SET title = :title,
    family_surname = :family_surname,
    revision_time = :revision_time,
    description = :description
WHERE genealogy_id = :genealogy_id
```

用途：

```text
编辑族谱信息。
```

说明：

C++ 中会检查 `query.numRowsAffected()`。如果影响行数为 0，说明族谱不存在或没有更新成功。

### 9.5 `GenealogyDao::remove`

SQL：

```sql
DELETE FROM genealogies
WHERE genealogy_id = :genealogy_id AND creator_user_id = :creator_user_id
```

用途：

```text
删除族谱。
```

关键点：

```text
SQL 中直接要求 creator_user_id 等于当前用户 ID。
```

这意味着即使按钮被绕过，非创建者也无法删除族谱。

### 9.6 `GenealogyDao::addCollaboratorByUsername`

SQL：

```sql
INSERT INTO genealogy_collaborators(genealogy_id, user_id, role)
SELECT :genealogy_id, user_id, :role
FROM users
WHERE username = :username
ON CONFLICT (genealogy_id, user_id)
DO UPDATE SET role = EXCLUDED.role
```

用途：

```text
根据用户名邀请协作者，或更新已有协作者角色。
```

实现原理：

- `INSERT ... SELECT`：先从 `users` 表根据用户名找到 `user_id`，再插入协作者表。
- 如果用户名不存在，`SELECT` 没有结果，则不会插入。
- `ON CONFLICT`：如果该用户已经是该族谱协作者，则更新角色。
- `EXCLUDED.role` 表示本次准备插入的新角色值。



逐行解析：

SQL：

```sql
INSERT INTO genealogy_collaborators(genealogy_id, user_id, role)
SELECT :genealogy_id, user_id, :role FROM users WHERE username = :username
ON CONFLICT (genealogy_id, user_id) DO UPDATE SET role = EXCLUDED.role
```

逐行说明：

```text
INSERT INTO genealogy_collaborators(genealogy_id, user_id, role)
```

向协作者表插入族谱 ID、用户 ID 和角色。

```text
SELECT :genealogy_id, user_id, :role FROM users WHERE username = :username
```

从用户表中根据用户名查到 `user_id`，再组成要插入的协作者记录。

```text
ON CONFLICT (genealogy_id, user_id)
```

如果复合主键冲突，说明该用户已经是这个族谱的协作者。

```text
DO UPDATE SET role = EXCLUDED.role
```

冲突时不报错，而是更新角色。`EXCLUDED.role` 表示这次准备插入的新角色。
### 9.7 `GenealogyDao::roleForUser`

SQL：

```sql
SELECT role
FROM genealogy_collaborators
WHERE genealogy_id = :genealogy_id AND user_id = :user_id
```

用途：

```text
查询当前用户在某族谱中的协作者角色。
```

说明：

- 创建者角色不在协作者表里。
- UI 层会先判断 `genealogies.creator_user_id`，如果是创建者则角色为 `owner`。
- 否则再调用该 SQL 查询 `editor` 或 `viewer`。

## 10. C++ 中的 SQL：成员模块

### 10.1 `MemberDao::findById`

SQL：

```sql
SELECT member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography
FROM members
WHERE member_id = :member_id
```

用途：

```text
根据成员 ID 查询成员详情。
```

### 10.2 `MemberDao::findByGenealogy`

SQL：

```sql
SELECT member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography
FROM members
WHERE genealogy_id = :genealogy_id
  AND (:keyword = '' OR name LIKE :like_keyword)
ORDER BY generation NULLS LAST, birth_year NULLS LAST, member_id
LIMIT 500
```

用途：

```text
查询某个族谱下的成员列表，并支持姓名模糊搜索。
```

参数：

```text
:genealogy_id 当前族谱 ID
:keyword 原始关键词
:like_keyword 加上百分号后的模糊匹配关键词，例如 %张%
```

实现原理：

- 当 `keyword` 为空时，条件 `:keyword = ''` 为真，返回全部成员。
- 当 `keyword` 不为空时，执行 `name LIKE '%关键词%'`。
- `NULLS LAST` 让空代数或空出生年排在最后。
- `LIMIT 500` 防止大数据量下界面一次加载过多记录。



逐行解析：

SQL：

```sql
SELECT member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography
FROM members
WHERE genealogy_id = :genealogy_id
  AND (:keyword = '' OR name LIKE :like_keyword)
ORDER BY generation NULLS LAST, birth_year NULLS LAST, member_id
LIMIT 500
```

逐行说明：

```text
SELECT member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography
```

查询成员列表需要显示的字段。

```text
FROM members
```

数据来自成员表。

```text
WHERE genealogy_id = :genealogy_id
```

只查询当前族谱下的成员。

```text
AND (:keyword = '' OR name LIKE :like_keyword)
```

如果关键词为空，则返回全部成员；如果关键词不为空，则按姓名模糊匹配。

```text
ORDER BY generation NULLS LAST, birth_year NULLS LAST, member_id
```

按代数、出生年、成员 ID 排序。空值排在最后。

```text
LIMIT 500
```

最多返回 500 条，避免大数据表一次性加载过多导致界面卡顿。
### 10.3 `MemberDao::findRecentByGenealogy`

SQL：

```sql
SELECT member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography
FROM members
WHERE genealogy_id = :genealogy_id
ORDER BY member_id DESC
LIMIT :limit
```

用途：

```text
Dashboard 显示最近新增成员。
```

说明：

`member_id` 是自增主键，越大通常表示越晚插入，所以按 `member_id DESC` 取最近记录。

### 10.4 `MemberDao::findChildren`

SQL：

```sql
SELECT m.member_id, m.genealogy_id, m.name, m.gender, m.birth_year, m.death_year,
       m.generation, m.biography
FROM parent_child_relations r
JOIN members m ON m.member_id = r.child_id
WHERE r.parent_id = :member_id
ORDER BY m.birth_year NULLS LAST, m.member_id
```

用途：

```text
查询某个成员的所有子女。
```

对应功能：

```text
图形化后代树。
```

### 10.5 `MemberDao::findAncestors`

SQL：

```sql
WITH RECURSIVE ancestors AS (
    SELECT parent_id, child_id, 1 AS depth
    FROM parent_child_relations
    WHERE child_id = :member_id
    UNION ALL
    SELECT p.parent_id, p.child_id, a.depth + 1
    FROM parent_child_relations p
    JOIN ancestors a ON p.child_id = a.parent_id
)
SELECT m.member_id, m.genealogy_id, m.name, m.gender, m.birth_year, m.death_year,
       m.generation, m.biography
FROM ancestors a
JOIN members m ON m.member_id = a.parent_id
ORDER BY a.depth, m.birth_year NULLS LAST
```

用途：

```text
递归查询某个成员的所有祖先。
```

实现原理：

1. 初始层：查当前成员的父母。
2. 递归层：用上一层父母继续向上找父母。
3. 直到没有新的父母记录。
4. 最终连接 `members` 表返回祖先详情。

### 10.6 `MemberDao::insert`

SQL：

```sql
INSERT INTO members(genealogy_id, name, gender, birth_year, death_year, generation, biography)
VALUES(:genealogy_id, :name, :gender, :birth_year, :death_year, :generation, :biography)
```

用途：

```text
新增成员。
```

说明：

C++ 中把年份或代数输入框为空时转换为 `NULL`：

```cpp
member.birthYear == 0 ? QVariant() : QVariant(member.birthYear)
```

### 10.7 `MemberDao::update`

SQL：

```sql
UPDATE members
SET name = :name,
    gender = :gender,
    birth_year = :birth_year,
    death_year = :death_year,
    generation = :generation,
    biography = :biography
WHERE member_id = :member_id
```

用途：

```text
编辑成员信息。
```

### 10.8 `MemberDao::remove`

SQL：

```sql
DELETE FROM members
WHERE member_id = :member_id
```

用途：

```text
删除成员。
```

说明：

由于亲子关系和婚姻关系中的成员外键使用 `ON DELETE CASCADE`，删除成员时，与该成员相关的关系记录也会自动删除。

## 11. C++ 中的 SQL：关系模块

### 11.1 `RelationDao::findParentChildEdges`

SQL：

```sql
SELECT parent_id, child_id
FROM parent_child_relations
WHERE genealogy_id = :genealogy_id
```

用途：

```text
查询某个族谱中的所有亲子边。
```

对应功能：

```text
TreeService 构建图结构，用于亲缘链路 BFS 查询。
```

说明：

代码中把每条亲子关系作为无向边加入图：

```text
parent -> child
child -> parent
```

这样从任意成员都可以沿血缘边查找另一个成员。

### 11.2 `RelationDao::addParentChild`

SQL：

```sql
INSERT INTO parent_child_relations(genealogy_id, parent_id, child_id, relation_type)
VALUES(:genealogy_id, :parent_id, :child_id, :relation_type)
```

用途：

```text
新增父子/母子关系。
```

说明：

合法性由数据库触发器进一步检查，例如性别、代数、出生年份和族谱一致性。

### 11.3 `RelationDao::addMarriage`

SQL：

```sql
INSERT INTO marriages(genealogy_id, person1_id, person2_id, marriage_year, divorce_year, description)
VALUES(:genealogy_id, :person1_id, :person2_id, :marriage_year, :divorce_year, :description)
```

用途：

```text
新增婚姻关系。
```

说明：

C++ 层先使用 `std::minmax(person1Id, person2Id)` 统一双方 ID 顺序，数据库触发器也会再次兜底检查。

## 12. C++ 中的 SQL：Dashboard 统计

### 12.1 成员统计

文件：

```text
src/service/DashboardService.cpp
```

SQL：

```sql
SELECT COUNT(*) AS total_members,
       COUNT(*) FILTER (WHERE gender = 'M') AS male_members,
       COUNT(*) FILTER (WHERE gender = 'F') AS female_members,
       COALESCE(MAX(generation), 0) AS max_generation
FROM members
WHERE genealogy_id = :genealogy_id
```

用途：

```text
统计当前族谱总人数、男性人数、女性人数和最大代数。
```

关键语法：

- `COUNT(*)`：总人数。
- `COUNT(*) FILTER (WHERE gender = 'M')`：只统计男性。
- `COUNT(*) FILTER (WHERE gender = 'F')`：只统计女性。
- `MAX(generation)`：最大代数。
- `COALESCE(..., 0)`：如果没有成员，返回 0 而不是 NULL。



逐行解析：

SQL：

```sql
SELECT COUNT(*) AS total_members,
       COUNT(*) FILTER (WHERE gender = 'M') AS male_members,
       COUNT(*) FILTER (WHERE gender = 'F') AS female_members,
       COALESCE(MAX(generation), 0) AS max_generation
FROM members WHERE genealogy_id = :genealogy_id
```

逐行说明：

```text
SELECT COUNT(*) AS total_members
```

统计成员总数，并命名为 `total_members`。

```text
COUNT(*) FILTER (WHERE gender = 'M') AS male_members
```

只统计性别为男性的成员。

```text
COUNT(*) FILTER (WHERE gender = 'F') AS female_members
```

只统计性别为女性的成员。

```text
COALESCE(MAX(generation), 0) AS max_generation
```

查询最大代数。如果没有成员，`MAX` 会返回 NULL，`COALESCE` 把它转换成 0。

```text
FROM members WHERE genealogy_id = :genealogy_id
```

统计范围限定为当前族谱。
### 12.2 血缘关系统计

SQL：

```sql
SELECT COUNT(*)
FROM parent_child_relations
WHERE genealogy_id = :genealogy_id
```

用途：

```text
统计当前族谱中的亲子关系数量。
```

### 12.3 婚姻关系统计

SQL：

```sql
SELECT COUNT(*)
FROM marriages
WHERE genealogy_id = :genealogy_id
```

用途：

```text
统计当前族谱中的婚姻关系数量。
```

## 13. 自动测试中的 SQL 调用

### `已删除的早期冒烟测试脚本`

该测试本身不直接写复杂 SQL，而是调用已有 DAO 和 Service。

测试链路：

```text
DatabaseManager::connect
AuthService::login
GenealogyDao::findAccessibleByUser
DashboardService::loadStats
MemberDao::findByGenealogy
```

因此它间接验证了以下 SQL：

- 登录 SQL。
- 查询可访问族谱 SQL。
- Dashboard 统计 SQL。
- 成员列表 SQL。

通过输出：

```text
PASS: login, genealogy loading, dashboard, and member list are working.
```

说明核心数据库链路可用。

## 14. SQL 与功能对应关系

| 功能 | 主要 SQL 位置 |
|---|---|
| 登录 | `AuthService::login` |
| 注册 | `UserDao::insert` |
| 加载可访问族谱 | `GenealogyDao::findAccessibleByUser` |
| 族谱增删改查 | `GenealogyDao` |
| 邀请协作者 | `GenealogyDao::addCollaboratorByUsername` |
| 权限角色查询 | `GenealogyDao::roleForUser` |
| 成员列表 | `MemberDao::findByGenealogy` |
| 成员模糊搜索 | `MemberDao::findByGenealogy` + `idx_members_name_trgm` |
| 最近成员 | `MemberDao::findRecentByGenealogy` |
| 成员新增编辑删除 | `MemberDao::insert/update/remove` |
| 后代树 | `MemberDao::findChildren` |
| 祖先查询 | `MemberDao::findAncestors` |
| 亲缘链路 | `RelationDao::findParentChildEdges` + BFS |
| 亲子关系维护 | `RelationDao::addParentChild` + 触发器 |
| 婚姻关系维护 | `RelationDao::addMarriage` + 触发器 |
| Dashboard | `DashboardService::loadStats` |
| 核心 SQL 作业要求 | `sql/04_core_queries.sql` |
| COPY 导入 | `sql/05_load_generated_csv.sql` |
| 分支导出 | `sql/06_export_branch.sql` |
| 索引性能对比 | `sql/07_performance_explain.sql` |

## 15. 验收讲解建议

如果老师问“SQL 部分怎么实现”，可以按以下顺序回答：

```text
1. 先说明数据库有六张核心表，建表在 sql/01_schema.sql。
2. 再说明约束分两层：简单约束用 CHECK 和外键，复杂跨表约束用触发器。
3. 说明查询分两类：界面功能 SQL 在 DAO/Service 中，课程要求 SQL 在 sql/04_core_queries.sql。
4. 重点讲 Recursive CTE：祖先查询和分支导出都用 WITH RECURSIVE。
5. 讲索引：姓名模糊查询用 pg_trgm + GIN，父节点查子节点用 parent_id 索引。
6. 讲数据工程：Python 生成 CSV，PostgreSQL \copy 批量导入，pg_dump 或分支导出备份。
7. 讲性能：sql/07_performance_explain.sql 删除索引和重建索引，对比 EXPLAIN ANALYZE。
```


