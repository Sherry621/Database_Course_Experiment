下面给出一个**基于 PPT 要求**、并且采用 **C++ 语言实现**的《“寻根溯源”族谱管理系统设计与实现》完整方案。PPT 中要求系统具备用户登录、族谱管理、成员管理、树形预览、祖先查询、亲缘链路查询、10 万级模拟数据、递归 SQL、索引优化和实验报告材料等内容，因此下面方案会围绕这些验收点展开。

---

# 一、系统总体目标

本系统用于管理多个家族族谱。每个族谱对应一个家族，用户可以注册、登录、创建族谱，也可以邀请其他用户共同编辑族谱。系统需要记录家族成员基本信息、血缘关系、婚姻关系，并支持大规模数据下的快速查询。

系统核心能力包括：

1. 多用户注册、登录与权限控制；
2. 多族谱管理；
3. 成员增删改查；
4. 族谱协作邀请；
5. 成员姓名模糊查询；
6. 家族统计 Dashboard；
7. 树形族谱预览；
8. 人物祖先追溯；
9. 两个人之间亲缘关系链路查询；
10. 10 万级模拟数据导入导出；
11. 索引优化和性能对比分析。

---

# 二、技术选型

## 1. 编程语言

使用 **C++17 / C++20**。

推荐使用：

```text
C++17 或 C++20
```

原因：

* 标准库支持较完善；
* 适合实现桌面端应用；
* 查询结果处理、树结构构造、图搜索算法实现方便；
* 可以直接连接 PostgreSQL / MySQL / SQLite 等数据库。

---

## 2. 图形化界面方案

推荐使用 **Qt 6 + C++**。

```text
Qt Widgets / Qt 6
```

理由：

* Qt 是 C++ 生态中最成熟的 GUI 框架之一；
* 支持表格、树形控件、登录窗口、弹窗、文件选择器；
* 自带数据库模块 `QtSql`；
* 可以方便展示族谱树，例如使用 `QTreeWidget`；
* 适合课程实验演示。

主要 Qt 模块：

```text
Qt Widgets
Qt SQL
Qt Core
Qt GUI
```

---

## 3. 数据库选择

推荐使用 **PostgreSQL**。

理由：

* 支持 `WITH RECURSIVE` 递归查询；
* 支持 `COPY` 批量导入导出；
* 支持复杂索引；
* 适合实现祖先查询、后代查询、亲缘链路查询；
* 和 PPT 中提到的数据库应用实践内容契合。

也可以使用 MySQL 8.0，但 PostgreSQL 对递归查询和数据导入导出更方便。

---

## 4. 总体技术架构

```text
C++ / Qt 图形界面
        |
        | QtSql / libpqxx
        |
PostgreSQL 数据库
        |
SQL 表结构、约束、索引、递归查询、数据导入导出
```

推荐项目结构：

```text
GenealogySystem/
├── CMakeLists.txt
├── main.cpp
├── ui/
│   ├── LoginWindow.h
│   ├── LoginWindow.cpp
│   ├── MainWindow.h
│   ├── MainWindow.cpp
│   ├── FamilyTreeWindow.h
│   ├── FamilyTreeWindow.cpp
│   ├── MemberDialog.h
│   └── MemberDialog.cpp
├── db/
│   ├── DatabaseManager.h
│   ├── DatabaseManager.cpp
│   ├── UserDao.h
│   ├── UserDao.cpp
│   ├── GenealogyDao.h
│   ├── GenealogyDao.cpp
│   ├── MemberDao.h
│   ├── MemberDao.cpp
│   └── RelationDao.h
│       RelationDao.cpp
├── model/
│   ├── User.h
│   ├── Genealogy.h
│   ├── Member.h
│   └── Relation.h
├── service/
│   ├── AuthService.h
│   ├── AuthService.cpp
│   ├── GenealogyService.h
│   ├── GenealogyService.cpp
│   ├── MemberService.h
│   ├── MemberService.cpp
│   ├── TreeService.h
│   └── TreeService.cpp
├── sql/
│   ├── 01_create_tables.sql
│   ├── 02_constraints.sql
│   ├── 03_indexes.sql
│   ├── 04_core_queries.sql
│   ├── 05_triggers.sql
│   └── 06_import_export.sql
└── tools/
    ├── DataGenerator.cpp
    └── generated_data/
```

---

# 三、系统功能设计

## 1. 用户注册与登录

用户采用注册制。每个用户登录后，只能看到：

1. 自己创建的族谱；
2. 被别人邀请协作的族谱。

功能包括：

```text
用户注册
用户登录
密码加密存储
查看可访问族谱
退出登录
```

界面设计：

```text
LoginWindow
├── 用户名输入框
├── 密码输入框
├── 登录按钮
└── 注册按钮
```

---

## 2. Dashboard 首页

登录后进入主界面，展示当前选中族谱的统计信息。

Dashboard 应显示：

```text
家族总人数
男性人数
女性人数
男女比例
最早出生年份
最晚出生年份
当前最大代数
婚姻关系数量
血缘关系数量
```

SQL 示例：

```sql
SELECT COUNT(*) AS total_members
FROM member
WHERE genealogy_id = :genealogy_id;

SELECT gender, COUNT(*) 
FROM member
WHERE genealogy_id = :genealogy_id
GROUP BY gender;
```

---

## 3. 族谱管理

每个族谱包含基本信息：

```text
谱名
姓氏
修谱时间
创建用户
创建时间
备注
```

功能：

```text
新建族谱
修改族谱信息
删除族谱
查看我的族谱
邀请其他用户协作
移除协作者
```

---

## 4. 成员管理

成员基本信息包括：

```text
姓名
性别
出生年份
死亡年份
生平简介
所属族谱
代数
```

成员管理功能：

```text
新增成员
修改成员
删除成员
查看成员详情
按姓名模糊查询
按性别筛选
按代数筛选
```

注意：由于存在同名同姓但不同辈分的情况，系统内部所有关系都必须使用 `member_id`，不能只靠姓名判断成员身份。

---

## 5. 血缘关系管理

血缘关系包括：

```text
父子 / 父女
母子 / 母女
```

设计时建议统一抽象为：

```text
parent_child_relation
```

字段包括：

```text
relation_id
genealogy_id
parent_id
child_id
relation_type
```

其中 `relation_type` 可取：

```text
father
mother
```

这样可以明确父亲或母亲关系。

---

## 6. 婚姻关系管理

婚姻关系包括：

```text
丈夫
妻子
结婚年份
离婚年份
备注
```

设计成独立表：

```text
marriage_relation
```

注意：

1. 同一个人可能有多个婚姻关系；
2. 婚姻关系是无向关系；
3. 查询配偶时需要同时检查 `person1_id` 和 `person2_id`。

---

## 7. 树形预览功能

PPT 要求以层级结构或缩进列表展示某分支的人物关系树状图。可以使用 Qt 的 `QTreeWidget` 实现。

界面结构：

```text
FamilyTreeWindow
├── 输入根成员 ID
├── 查询按钮
└── QTreeWidget 展示后代树
```

展示形式示例：

```text
张一世 男 1850-1920
├── 张二世 男 1875-1940
│   ├── 张三世 男 1900-1970
│   └── 张三妹 女 1905-1980
└── 张二女 女 1878-1950
```

核心实现方式：

1. 从数据库查出某成员所有后代；
2. 在 C++ 中建立 `parent_id -> children` 映射；
3. 递归构造 `QTreeWidgetItem`。

---

## 8. 祖先查询功能

输入一个成员 ID，查询其所有父辈以上祖先。

数据库层使用 PostgreSQL 的递归 CTE：

```sql
WITH RECURSIVE ancestors AS (
    SELECT 
        p.parent_id,
        p.child_id,
        1 AS depth
    FROM parent_child_relation p
    WHERE p.child_id = :member_id

    UNION ALL

    SELECT
        p.parent_id,
        p.child_id,
        a.depth + 1
    FROM parent_child_relation p
    JOIN ancestors a ON p.child_id = a.parent_id
)
SELECT 
    a.depth,
    m.member_id,
    m.name,
    m.gender,
    m.birth_year,
    m.death_year
FROM ancestors a
JOIN member m ON a.parent_id = m.member_id
ORDER BY a.depth;
```

显示形式：

```text
第 1 代祖先：父亲 / 母亲
第 2 代祖先：祖父 / 祖母 / 外祖父 / 外祖母
第 3 代祖先：曾祖父 / 曾祖母 ...
```

---

## 9. 亲缘链路查询

PPT 要求输入两个人的 ID，查询两人之间是否存在亲缘关系连成的通路，并展示链路。

这个问题本质上是图搜索问题。

### 方案一：数据库递归 CTE 查询

将血缘关系看作无向图：

```text
父母 <-> 子女
```

递归查找从 A 到 B 的路径。

PostgreSQL 示例：

```sql
WITH RECURSIVE relation_graph AS (
    SELECT parent_id AS from_id, child_id AS to_id
    FROM parent_child_relation
    WHERE genealogy_id = :genealogy_id

    UNION

    SELECT child_id AS from_id, parent_id AS to_id
    FROM parent_child_relation
    WHERE genealogy_id = :genealogy_id
),
search_path AS (
    SELECT 
        from_id,
        to_id,
        ARRAY[from_id, to_id] AS path,
        1 AS depth
    FROM relation_graph
    WHERE from_id = :member_a_id

    UNION ALL

    SELECT 
        rg.from_id,
        rg.to_id,
        sp.path || rg.to_id,
        sp.depth + 1
    FROM relation_graph rg
    JOIN search_path sp ON rg.from_id = sp.to_id
    WHERE NOT rg.to_id = ANY(sp.path)
      AND sp.depth < 20
)
SELECT path
FROM search_path
WHERE to_id = :member_b_id
ORDER BY depth
LIMIT 1;
```

### 方案二：C++ 中使用 BFS

对于课程项目演示，C++ BFS 更直观。

步骤：

1. 从数据库中读取该族谱所有 `parent-child` 边；
2. 建立邻接表；
3. 从成员 A 开始 BFS；
4. 找到成员 B 后回溯路径；
5. 展示链路。

C++ 伪代码：

```cpp
std::vector<int> findRelationPath(
    int start,
    int target,
    const std::unordered_map<int, std::vector<int>>& graph
) {
    std::queue<int> q;
    std::unordered_map<int, int> prev;
    std::unordered_set<int> visited;

    q.push(start);
    visited.insert(start);
    prev[start] = -1;

    while (!q.empty()) {
        int cur = q.front();
        q.pop();

        if (cur == target) {
            break;
        }

        auto it = graph.find(cur);
        if (it == graph.end()) continue;

        for (int next : it->second) {
            if (!visited.count(next)) {
                visited.insert(next);
                prev[next] = cur;
                q.push(next);
            }
        }
    }

    std::vector<int> path;
    if (!visited.count(target)) {
        return path;
    }

    for (int at = target; at != -1; at = prev[at]) {
        path.push_back(at);
    }

    std::reverse(path.begin(), path.end());
    return path;
}
```

展示示例：

```text
张三 -> 张三父亲 -> 张三祖父 -> 张四祖父 -> 张四父亲 -> 张四
```

---

# 四、数据库概念结构设计

## 1. 实体识别

根据 PPT 要求，主要实体包括：

```text
用户 User
族谱 Genealogy
成员 Member
族谱协作者 GenealogyCollaborator
血缘关系 ParentChildRelation
婚姻关系 MarriageRelation
```

---

## 2. ER 关系说明

### User 与 Genealogy

```text
一个用户可以创建多个族谱
一个族谱只能有一个创建者
User 1 : N Genealogy
```

### User 与 Genealogy 的协作关系

```text
一个用户可以参与多个族谱
一个族谱可以邀请多个用户协作
User M : N Genealogy
```

通过中间表：

```text
genealogy_collaborator
```

实现。

### Genealogy 与 Member

```text
一个族谱包含多个成员
一个成员属于一个族谱
Genealogy 1 : N Member
```

### Member 与 Member 的血缘关系

```text
一个成员可以有多个子女
一个成员最多有一个父亲、一个母亲
Member 1 : N Member
```

通过关系表：

```text
parent_child_relation
```

实现。

### Member 与 Member 的婚姻关系

```text
一个成员可以有多个婚姻关系
婚姻关系是成员之间的 M : N 联系
```

通过关系表：

```text
marriage_relation
```

实现。

---

# 五、关系模式设计

## 1. 用户表 user_account

```sql
CREATE TABLE user_account (
    user_id SERIAL PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    real_name VARCHAR(50),
    email VARCHAR(100) UNIQUE,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

---

## 2. 族谱表 genealogy

```sql
CREATE TABLE genealogy (
    genealogy_id SERIAL PRIMARY KEY,
    title VARCHAR(100) NOT NULL,
    family_surname VARCHAR(20) NOT NULL,
    revision_time DATE,
    creator_user_id INT NOT NULL,
    description TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT fk_genealogy_creator
        FOREIGN KEY (creator_user_id)
        REFERENCES user_account(user_id)
        ON DELETE CASCADE
);
```

---

## 3. 协作者表 genealogy_collaborator

```sql
CREATE TABLE genealogy_collaborator (
    genealogy_id INT NOT NULL,
    user_id INT NOT NULL,
    role VARCHAR(20) NOT NULL DEFAULT 'editor',
    invited_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    PRIMARY KEY (genealogy_id, user_id),

    CONSTRAINT fk_collab_genealogy
        FOREIGN KEY (genealogy_id)
        REFERENCES genealogy(genealogy_id)
        ON DELETE CASCADE,

    CONSTRAINT fk_collab_user
        FOREIGN KEY (user_id)
        REFERENCES user_account(user_id)
        ON DELETE CASCADE,

    CONSTRAINT ck_collab_role
        CHECK (role IN ('editor', 'viewer'))
);
```

---

## 4. 成员表 member

```sql
CREATE TABLE member (
    member_id SERIAL PRIMARY KEY,
    genealogy_id INT NOT NULL,
    name VARCHAR(100) NOT NULL,
    gender CHAR(1) NOT NULL,
    birth_year INT,
    death_year INT,
    generation INT,
    biography TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT fk_member_genealogy
        FOREIGN KEY (genealogy_id)
        REFERENCES genealogy(genealogy_id)
        ON DELETE CASCADE,

    CONSTRAINT ck_member_gender
        CHECK (gender IN ('M', 'F')),

    CONSTRAINT ck_member_year
        CHECK (
            birth_year IS NULL 
            OR death_year IS NULL 
            OR birth_year <= death_year
        ),

    CONSTRAINT ck_member_generation
        CHECK (generation IS NULL OR generation >= 1)
);
```

---

## 5. 血缘关系表 parent_child_relation

```sql
CREATE TABLE parent_child_relation (
    relation_id SERIAL PRIMARY KEY,
    genealogy_id INT NOT NULL,
    parent_id INT NOT NULL,
    child_id INT NOT NULL,
    relation_type VARCHAR(10) NOT NULL,

    CONSTRAINT fk_pc_genealogy
        FOREIGN KEY (genealogy_id)
        REFERENCES genealogy(genealogy_id)
        ON DELETE CASCADE,

    CONSTRAINT fk_pc_parent
        FOREIGN KEY (parent_id)
        REFERENCES member(member_id)
        ON DELETE CASCADE,

    CONSTRAINT fk_pc_child
        FOREIGN KEY (child_id)
        REFERENCES member(member_id)
        ON DELETE CASCADE,

    CONSTRAINT ck_pc_relation_type
        CHECK (relation_type IN ('father', 'mother')),

    CONSTRAINT ck_pc_not_self
        CHECK (parent_id <> child_id),

    CONSTRAINT uq_parent_child_type
        UNIQUE (child_id, relation_type)
);
```

说明：

```text
UNIQUE(child_id, relation_type)
```

表示一个孩子最多只有一个父亲和一个母亲。

---

## 6. 婚姻关系表 marriage_relation

```sql
CREATE TABLE marriage_relation (
    marriage_id SERIAL PRIMARY KEY,
    genealogy_id INT NOT NULL,
    person1_id INT NOT NULL,
    person2_id INT NOT NULL,
    marriage_year INT,
    divorce_year INT,
    description TEXT,

    CONSTRAINT fk_marriage_genealogy
        FOREIGN KEY (genealogy_id)
        REFERENCES genealogy(genealogy_id)
        ON DELETE CASCADE,

    CONSTRAINT fk_marriage_p1
        FOREIGN KEY (person1_id)
        REFERENCES member(member_id)
        ON DELETE CASCADE,

    CONSTRAINT fk_marriage_p2
        FOREIGN KEY (person2_id)
        REFERENCES member(member_id)
        ON DELETE CASCADE,

    CONSTRAINT ck_marriage_not_self
        CHECK (person1_id <> person2_id),

    CONSTRAINT ck_marriage_year
        CHECK (
            marriage_year IS NULL 
            OR divorce_year IS NULL 
            OR marriage_year <= divorce_year
        ),

    CONSTRAINT uq_marriage_pair
        UNIQUE (genealogy_id, person1_id, person2_id)
);
```

---

# 六、范式分析

该关系模式整体可以说明达到 **第三范式 3NF**，大部分表也可认为接近或满足 **BCNF**。

## 1. user_account

主键：

```text
user_id
```

函数依赖：

```text
user_id -> username, password_hash, real_name, email, created_at
username -> user_id, password_hash, real_name, email, created_at
email -> user_id, username, password_hash, real_name, created_at
```

非主属性完全依赖候选键，不存在传递依赖，满足 3NF。

---

## 2. genealogy

主键：

```text
genealogy_id
```

函数依赖：

```text
genealogy_id -> title, family_surname, revision_time, creator_user_id, description
```

族谱表中的属性均直接描述族谱，不依赖其他非主属性，满足 3NF。

---

## 3. member

主键：

```text
member_id
```

函数依赖：

```text
member_id -> genealogy_id, name, gender, birth_year, death_year, generation, biography
```

成员信息均直接依赖成员编号，不存在：

```text
member_id -> 非主属性 -> 其他非主属性
```

因此满足 3NF。

---

## 4. parent_child_relation

主键：

```text
relation_id
```

候选键：

```text
child_id, relation_type
```

主要属性直接描述一条亲子关系，不存在部分依赖和传递依赖，满足 3NF。

---

## 5. marriage_relation

主键：

```text
marriage_id
```

婚姻关系属性直接依赖婚姻关系编号，不存在传递依赖，满足 3NF。

---

# 七、约束设计

PPT 要求设计主键、外键、CHECK 约束，例如父亲出生年份必须早于子女。基础 CHECK 约束前面已经给出。

但是“父亲出生年份必须早于子女”涉及两张表之间的数据比较，普通 CHECK 难以跨表完成，因此推荐使用触发器实现。

---

## 1. 父母出生年份早于子女的触发器

```sql
CREATE OR REPLACE FUNCTION check_parent_birth_year()
RETURNS TRIGGER AS $$
DECLARE
    parent_birth INT;
    child_birth INT;
BEGIN
    SELECT birth_year INTO parent_birth
    FROM member
    WHERE member_id = NEW.parent_id;

    SELECT birth_year INTO child_birth
    FROM member
    WHERE member_id = NEW.child_id;

    IF parent_birth IS NOT NULL 
       AND child_birth IS NOT NULL 
       AND parent_birth >= child_birth THEN
        RAISE EXCEPTION '父母出生年份必须早于子女出生年份';
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_check_parent_birth_year
BEFORE INSERT OR UPDATE ON parent_child_relation
FOR EACH ROW
EXECUTE FUNCTION check_parent_birth_year();
```

---

## 2. 防止跨族谱建立关系

```sql
CREATE OR REPLACE FUNCTION check_same_genealogy()
RETURNS TRIGGER AS $$
DECLARE
    parent_gid INT;
    child_gid INT;
BEGIN
    SELECT genealogy_id INTO parent_gid
    FROM member
    WHERE member_id = NEW.parent_id;

    SELECT genealogy_id INTO child_gid
    FROM member
    WHERE member_id = NEW.child_id;

    IF parent_gid <> child_gid OR parent_gid <> NEW.genealogy_id THEN
        RAISE EXCEPTION '亲子关系中的成员必须属于同一个族谱';
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_check_same_genealogy
BEFORE INSERT OR UPDATE ON parent_child_relation
FOR EACH ROW
EXECUTE FUNCTION check_same_genealogy();
```

---

## 3. 婚姻关系成员必须属于同一族谱

```sql
CREATE OR REPLACE FUNCTION check_marriage_same_genealogy()
RETURNS TRIGGER AS $$
DECLARE
    gid1 INT;
    gid2 INT;
BEGIN
    SELECT genealogy_id INTO gid1
    FROM member
    WHERE member_id = NEW.person1_id;

    SELECT genealogy_id INTO gid2
    FROM member
    WHERE member_id = NEW.person2_id;

    IF gid1 <> gid2 OR gid1 <> NEW.genealogy_id THEN
        RAISE EXCEPTION '婚姻关系中的成员必须属于同一个族谱';
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER trg_check_marriage_same_genealogy
BEFORE INSERT OR UPDATE ON marriage_relation
FOR EACH ROW
EXECUTE FUNCTION check_marriage_same_genealogy();
```

---

# 八、核心 SQL 功能设计

## 1. 给定成员 ID，查询其配偶及所有子女

```sql
SELECT 
    'spouse' AS relation,
    m.member_id,
    m.name,
    m.gender,
    m.birth_year,
    m.death_year
FROM marriage_relation mr
JOIN member m 
    ON m.member_id = CASE 
        WHEN mr.person1_id = :member_id THEN mr.person2_id
        ELSE mr.person1_id
    END
WHERE mr.person1_id = :member_id 
   OR mr.person2_id = :member_id

UNION ALL

SELECT
    'child' AS relation,
    m.member_id,
    m.name,
    m.gender,
    m.birth_year,
    m.death_year
FROM parent_child_relation pcr
JOIN member m ON m.member_id = pcr.child_id
WHERE pcr.parent_id = :member_id;
```

---

## 2. 输入成员 A 的 ID，输出所有历代祖先

```sql
WITH RECURSIVE ancestors AS (
    SELECT 
        parent_id,
        child_id,
        1 AS depth
    FROM parent_child_relation
    WHERE child_id = :member_id

    UNION ALL

    SELECT
        p.parent_id,
        p.child_id,
        a.depth + 1
    FROM parent_child_relation p
    JOIN ancestors a ON p.child_id = a.parent_id
)
SELECT 
    depth,
    m.member_id,
    m.name,
    m.gender,
    m.birth_year,
    m.death_year
FROM ancestors a
JOIN member m ON a.parent_id = m.member_id
ORDER BY depth, m.birth_year;
```

---

## 3. 统计某个家族中平均寿命最长的一代人

```sql
SELECT 
    generation,
    AVG(death_year - birth_year) AS avg_life
FROM member
WHERE genealogy_id = :genealogy_id
  AND birth_year IS NOT NULL
  AND death_year IS NOT NULL
  AND generation IS NOT NULL
GROUP BY generation
ORDER BY avg_life DESC
LIMIT 1;
```

---

## 4. 查询所有年龄超过 50 岁且没有配偶的男性成员

```sql
SELECT 
    m.member_id,
    m.name,
    m.birth_year,
    m.death_year
FROM member m
WHERE m.genealogy_id = :genealogy_id
  AND m.gender = 'M'
  AND (
      COALESCE(m.death_year, EXTRACT(YEAR FROM CURRENT_DATE)::INT) 
      - m.birth_year
  ) > 50
  AND NOT EXISTS (
      SELECT 1
      FROM marriage_relation mr
      WHERE mr.person1_id = m.member_id
         OR mr.person2_id = m.member_id
  );
```

---

## 5. 找出出生年份早于该辈分平均出生年份的成员

```sql
WITH generation_avg AS (
    SELECT 
        generation,
        AVG(birth_year) AS avg_birth_year
    FROM member
    WHERE genealogy_id = :genealogy_id
      AND birth_year IS NOT NULL
      AND generation IS NOT NULL
    GROUP BY generation
)
SELECT 
    m.member_id,
    m.name,
    m.generation,
    m.birth_year,
    g.avg_birth_year
FROM member m
JOIN generation_avg g 
    ON m.generation = g.generation
WHERE m.genealogy_id = :genealogy_id
  AND m.birth_year < g.avg_birth_year
ORDER BY m.generation, m.birth_year;
```

---

# 九、索引设计

PPT 要求针对“姓名模糊查询”和“根据父节点 ID 查询子节点”设计索引，并比较性能。

## 1. 父节点 ID 查询子节点索引

```sql
CREATE INDEX idx_parent_child_parent_id
ON parent_child_relation(parent_id);
```

用于：

```sql
SELECT *
FROM parent_child_relation
WHERE parent_id = :parent_id;
```

---

## 2. 子节点追溯父节点索引

祖先查询需要根据 `child_id` 找父母，因此也应建立：

```sql
CREATE INDEX idx_parent_child_child_id
ON parent_child_relation(child_id);
```

---

## 3. 成员姓名模糊查询索引

如果 PostgreSQL 使用普通 `LIKE '%张%'`，B-tree 索引不一定有效。推荐使用 `pg_trgm` 三元组索引。

```sql
CREATE EXTENSION IF NOT EXISTS pg_trgm;

CREATE INDEX idx_member_name_trgm
ON member
USING gin (name gin_trgm_ops);
```

用于：

```sql
SELECT *
FROM member
WHERE name LIKE '%张三%';
```

---

## 4. 按族谱筛选成员索引

```sql
CREATE INDEX idx_member_genealogy
ON member(genealogy_id);
```

---

## 5. 按族谱和代数统计索引

```sql
CREATE INDEX idx_member_genealogy_generation
ON member(genealogy_id, generation);
```

---

## 6. 婚姻查询索引

```sql
CREATE INDEX idx_marriage_person1
ON marriage_relation(person1_id);

CREATE INDEX idx_marriage_person2
ON marriage_relation(person2_id);
```

---

# 十、性能测试设计

PPT 要求比较有无索引时，查询某曾祖父所有曾孙，也就是四代查询的执行时间差异，并提交 `EXPLAIN` 分析。

## 1. 四代查询 SQL

```sql
SELECT 
    g4.member_id,
    g4.name,
    g4.gender,
    g4.birth_year
FROM parent_child_relation r1
JOIN parent_child_relation r2 ON r2.parent_id = r1.child_id
JOIN parent_child_relation r3 ON r3.parent_id = r2.child_id
JOIN member g4 ON g4.member_id = r3.child_id
WHERE r1.parent_id = :great_grandfather_id;
```

---

## 2. 无索引测试

先删除索引：

```sql
DROP INDEX IF EXISTS idx_parent_child_parent_id;
```

执行：

```sql
EXPLAIN ANALYZE
SELECT 
    g4.member_id,
    g4.name,
    g4.gender,
    g4.birth_year
FROM parent_child_relation r1
JOIN parent_child_relation r2 ON r2.parent_id = r1.child_id
JOIN parent_child_relation r3 ON r3.parent_id = r2.child_id
JOIN member g4 ON g4.member_id = r3.child_id
WHERE r1.parent_id = :great_grandfather_id;
```

预期可能出现：

```text
Seq Scan
Hash Join
执行时间较长
```

---

## 3. 有索引测试

重新创建索引：

```sql
CREATE INDEX idx_parent_child_parent_id
ON parent_child_relation(parent_id);
```

再次执行：

```sql
EXPLAIN ANALYZE
SELECT 
    g4.member_id,
    g4.name,
    g4.gender,
    g4.birth_year
FROM parent_child_relation r1
JOIN parent_child_relation r2 ON r2.parent_id = r1.child_id
JOIN parent_child_relation r3 ON r3.parent_id = r2.child_id
JOIN member g4 ON g4.member_id = r3.child_id
WHERE r1.parent_id = :great_grandfather_id;
```

预期可能出现：

```text
Index Scan
Bitmap Index Scan
执行时间明显降低
```

实验报告中应截图：

```text
无索引 EXPLAIN ANALYZE 结果
有索引 EXPLAIN ANALYZE 结果
执行时间对比表
```

---

# 十一、数据生成方案

PPT 要求：

```text
至少 10 个族谱
至少 1 个族谱拥有 50,000 以上成员
系统总成员不少于 100,000
每个族谱内成员至少与另一个成员有亲缘关系
单个族谱内至少 30 代传承关系
```

推荐编写 C++ 数据生成工具，生成 CSV 文件，然后使用 PostgreSQL `COPY` 导入。

---

## 1. 数据生成文件

生成以下 CSV：

```text
users.csv
genealogy.csv
genealogy_collaborator.csv
member.csv
parent_child_relation.csv
marriage_relation.csv
```

---

## 2. 大族谱生成思路

为了保证至少 30 代、超过 50,000 人，可以采用分层生成。

示例：

```text
第 1 代：1 人
第 2 代：3 人
第 3 代：9 人
第 4 代：27 人
……
```

但指数增长过快，因此可以控制每代人数：

```text
前 30 代每代至少 2-5 人，保证传承深度
后续扩大每代人数，直到总人数超过 50,000
```

更稳定的生成方式：

```text
1. 先生成一条 30 代主干链，保证深度；
2. 每一代随机添加若干兄弟姐妹；
3. 给部分成员生成配偶；
4. 给每个家庭生成 1-4 个子女；
5. 持续扩展到目标人数。
```

---

## 3. C++ 数据生成核心伪代码

```cpp
struct GeneratedMember {
    int memberId;
    int genealogyId;
    std::string name;
    char gender;
    int birthYear;
    int deathYear;
    int generation;
};

void generateLargeGenealogy(int genealogyId, int targetCount) {
    int currentId = 1;

    std::vector<int> previousGeneration;
    std::vector<int> currentGeneration;

    // 生成第一代
    int rootId = currentId++;
    createMember(rootId, genealogyId, "张始祖", 'M', 1800, 1870, 1);
    previousGeneration.push_back(rootId);

    for (int gen = 2; gen <= 30 || currentId <= targetCount; gen++) {
        currentGeneration.clear();

        for (int parentId : previousGeneration) {
            int childCount = randomInt(1, 4);

            for (int i = 0; i < childCount && currentId <= targetCount; i++) {
                char gender = randomGender();
                std::string name = generateName(gender);
                int birthYear = 1800 + gen * 25 + randomInt(-3, 3);
                int deathYear = birthYear + randomInt(50, 90);

                int childId = currentId++;
                createMember(childId, genealogyId, name, gender, birthYear, deathYear, gen);
                createParentChildRelation(genealogyId, parentId, childId, "father");

                currentGeneration.push_back(childId);
            }
        }

        if (currentGeneration.empty()) {
            break;
        }

        previousGeneration = currentGeneration;
    }
}
```

---

# 十二、数据导入导出方案

## 1. CSV 导入

PostgreSQL 使用 `COPY`：

```sql
COPY user_account(user_id, username, password_hash, real_name, email, created_at)
FROM '/path/users.csv'
WITH CSV HEADER;

COPY genealogy(genealogy_id, title, family_surname, revision_time, creator_user_id, description, created_at)
FROM '/path/genealogy.csv'
WITH CSV HEADER;

COPY member(member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography, created_at)
FROM '/path/member.csv'
WITH CSV HEADER;

COPY parent_child_relation(relation_id, genealogy_id, parent_id, child_id, relation_type)
FROM '/path/parent_child_relation.csv'
WITH CSV HEADER;

COPY marriage_relation(marriage_id, genealogy_id, person1_id, person2_id, marriage_year, divorce_year, description)
FROM '/path/marriage_relation.csv'
WITH CSV HEADER;
```

导入完成后，需要修正自增序列：

```sql
SELECT setval('user_account_user_id_seq', 
              (SELECT MAX(user_id) FROM user_account));

SELECT setval('genealogy_genealogy_id_seq', 
              (SELECT MAX(genealogy_id) FROM genealogy));

SELECT setval('member_member_id_seq', 
              (SELECT MAX(member_id) FROM member));

SELECT setval('parent_child_relation_relation_id_seq', 
              (SELECT MAX(relation_id) FROM parent_child_relation));

SELECT setval('marriage_relation_marriage_id_seq', 
              (SELECT MAX(marriage_id) FROM marriage_relation));
```

---

## 2. 导出某个分支备份

例如导出某个成员及其所有后代。

```sql
COPY (
    WITH RECURSIVE descendants AS (
        SELECT 
            child_id,
            parent_id,
            1 AS depth
        FROM parent_child_relation
        WHERE parent_id = :root_member_id

        UNION ALL

        SELECT
            p.child_id,
            p.parent_id,
            d.depth + 1
        FROM parent_child_relation p
        JOIN descendants d ON p.parent_id = d.child_id
    )
    SELECT m.*
    FROM descendants d
    JOIN member m ON m.member_id = d.child_id
)
TO '/path/branch_backup.csv'
WITH CSV HEADER;
```

---

# 十三、C++ 核心类设计

## 1. DatabaseManager

负责数据库连接。

```cpp
class DatabaseManager {
public:
    static DatabaseManager& instance();

    bool connect(
        const QString& host,
        int port,
        const QString& dbName,
        const QString& user,
        const QString& password
    );

    QSqlDatabase database();

private:
    DatabaseManager() = default;
    QSqlDatabase db;
};
```

---

## 2. User 模型

```cpp
struct User {
    int userId;
    QString username;
    QString realName;
    QString email;
};
```

---

## 3. Genealogy 模型

```cpp
struct Genealogy {
    int genealogyId;
    QString title;
    QString familySurname;
    QDate revisionTime;
    int creatorUserId;
    QString description;
};
```

---

## 4. Member 模型

```cpp
struct Member {
    int memberId;
    int genealogyId;
    QString name;
    QChar gender;
    int birthYear;
    int deathYear;
    int generation;
    QString biography;
};
```

---

## 5. AuthService

负责注册、登录、密码校验。

```cpp
class AuthService {
public:
    bool registerUser(
        const QString& username,
        const QString& password,
        const QString& realName,
        const QString& email
    );

    std::optional<User> login(
        const QString& username,
        const QString& password
    );

private:
    QString hashPassword(const QString& password);
};
```

密码可以使用：

```text
SHA-256 + salt
```

课程实验中也可以简化为：

```text
SHA-256
```

但报告中应说明真实系统需要更安全的哈希算法。

---

## 6. MemberDao

负责成员增删改查。

```cpp
class MemberDao {
public:
    bool addMember(const Member& member);
    bool updateMember(const Member& member);
    bool deleteMember(int memberId);
    std::optional<Member> findById(int memberId);
    std::vector<Member> fuzzySearch(int genealogyId, const QString& keyword);
    std::vector<Member> findChildren(int memberId);
    std::vector<Member> findAncestors(int memberId);
};
```

---

## 7. TreeService

负责树形结构和亲缘链路。

```cpp
class TreeService {
public:
    std::vector<Member> getDescendants(int rootMemberId);
    std::vector<Member> getAncestors(int memberId);
    std::vector<int> findRelationPath(int genealogyId, int memberA, int memberB);

private:
    std::unordered_map<int, std::vector<int>> buildGraph(int genealogyId);
};
```

---

# 十四、Qt 界面设计

## 1. 登录窗口 LoginWindow

```text
用户名
密码
登录按钮
注册按钮
```

登录成功后打开 `MainWindow`。

---

## 2. 主窗口 MainWindow

主窗口可以分为左侧菜单和右侧内容区。

```text
MainWindow
├── 左侧菜单
│   ├── Dashboard
│   ├── 族谱管理
│   ├── 成员管理
│   ├── 树形预览
│   ├── 祖先查询
│   └── 亲缘关系查询
└── 右侧功能页面
```

---

## 3. 成员管理界面

使用 `QTableWidget` 或 `QTableView` 展示成员。

功能按钮：

```text
新增成员
编辑成员
删除成员
搜索成员
刷新
```

搜索框：

```text
输入姓名关键词
```

---

## 4. 树形预览界面

使用 `QTreeWidget`：

```text
输入根成员 ID
点击生成树
树形展示后代关系
```

---

## 5. 祖先查询界面

```text
输入成员 ID
点击查询祖先
以树形或列表展示父辈以上祖先
```

---

## 6. 亲缘关系查询界面

```text
输入成员 A ID
输入成员 B ID
点击查询
显示亲缘链路
```

示例输出：

```text
存在亲缘关系：

张三(ID=1001)
  ↓ 父子关系
张二(ID=901)
  ↓ 父子关系
张一(ID=801)
  ↓ 父子关系
张四祖(ID=700)
```

---

# 十五、C++ 数据库连接示例

使用 QtSql 连接 PostgreSQL：

```cpp
#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

bool DatabaseManager::connect(
    const QString& host,
    int port,
    const QString& dbName,
    const QString& user,
    const QString& password
) {
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName(host);
    db.setPort(port);
    db.setDatabaseName(dbName);
    db.setUserName(user);
    db.setPassword(password);

    if (!db.open()) {
        qDebug() << "Database connection failed:" << db.lastError().text();
        return false;
    }

    qDebug() << "Database connected successfully.";
    return true;
}
```

---

# 十六、C++ 模糊查询成员示例

```cpp
std::vector<Member> MemberDao::fuzzySearch(int genealogyId, const QString& keyword) {
    std::vector<Member> result;

    QSqlQuery query;
    query.prepare(
        "SELECT member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography "
        "FROM member "
        "WHERE genealogy_id = :genealogy_id "
        "AND name LIKE :keyword "
        "ORDER BY generation, birth_year"
    );

    query.bindValue(":genealogy_id", genealogyId);
    query.bindValue(":keyword", "%" + keyword + "%");

    if (!query.exec()) {
        qDebug() << query.lastError().text();
        return result;
    }

    while (query.next()) {
        Member m;
        m.memberId = query.value("member_id").toInt();
        m.genealogyId = query.value("genealogy_id").toInt();
        m.name = query.value("name").toString();
        m.gender = query.value("gender").toChar();
        m.birthYear = query.value("birth_year").toInt();
        m.deathYear = query.value("death_year").toInt();
        m.generation = query.value("generation").toInt();
        m.biography = query.value("biography").toString();

        result.push_back(m);
    }

    return result;
}
```

---

# 十七、C++ 树形展示逻辑

```cpp
void FamilyTreeWindow::buildTree(
    QTreeWidgetItem* parentItem,
    int parentMemberId
) {
    std::vector<Member> children = memberDao.findChildren(parentMemberId);

    for (const Member& child : children) {
        QString text = QString("%1  ID:%2  第%3代  %4-%5")
            .arg(child.name)
            .arg(child.memberId)
            .arg(child.generation)
            .arg(child.birthYear)
            .arg(child.deathYear);

        QTreeWidgetItem* childItem = new QTreeWidgetItem();
        childItem->setText(0, text);

        parentItem->addChild(childItem);

        buildTree(childItem, child.memberId);
    }
}
```

---

# 十八、实验报告建议结构

按照 PPT 的验收要求，实验报告建议写成以下结构：

```text
1. 实验题目
2. 实验目的
3. 开发环境
   3.1 操作系统
   3.2 C++ 编译器
   3.3 Qt 版本
   3.4 PostgreSQL 版本
4. 需求分析
5. 系统总体设计
6. 数据库概念结构设计
   6.1 实体识别
   6.2 E-R 图
   6.3 联系类型分析
7. 关系模型设计
8. 范式分析
9. 数据库表结构设计
10. 约束与触发器设计
11. 索引设计
12. 核心 SQL 实现
13. C++ 系统实现
14. 数据生成与导入导出
15. 性能测试与 EXPLAIN 分析
16. 系统运行截图
17. 遇到的问题及解决方法
18. 总结
```

---

# 十九、最终提交材料清单

根据 PPT 的要求，建议最终提交：

```text
1. 实验报告 PDF / Word
2. C++ / Qt 项目源码
3. SQL 建表脚本
4. SQL 查询脚本
5. 索引和触发器脚本
6. C++ 数据生成工具源码
7. 生成的 CSV 数据样例
8. PostgreSQL 数据库导出文件
9. 系统运行截图
10. EXPLAIN ANALYZE 性能对比截图
11. 演示视频或现场演示准备
```

数据库导出可以使用：

```bash
pg_dump -U postgres -d genealogy_system -f genealogy_system_backup.sql
```

---

# 二十、推荐实现顺序

建议按照下面顺序完成，避免一开始就陷入复杂界面：

```text
第 1 步：完成数据库表结构设计
第 2 步：完成 SQL 建表、约束、索引、触发器
第 3 步：手工插入少量测试数据
第 4 步：验证核心 SQL 查询
第 5 步：实现 C++ 数据库连接
第 6 步：实现用户注册登录
第 7 步：实现族谱管理
第 8 步：实现成员增删改查
第 9 步：实现树形预览
第 10 步：实现祖先查询
第 11 步：实现亲缘链路 BFS 查询
第 12 步：编写 C++ 数据生成工具
第 13 步：导入 10 万级数据
第 14 步：完成索引优化与 EXPLAIN 对比
第 15 步：整理实验报告和截图
```

---

# 二十一、方案总结

本方案采用：

```text
C++ + Qt + PostgreSQL
```

实现“寻根溯源”族谱管理系统。数据库层负责数据存储、约束、索引、递归查询和批量导入导出；C++ 层负责图形化界面、业务逻辑、树形展示和亲缘链路 BFS 查询。

这个方案能够完整覆盖 PPT 中提出的核心要求，包括：

```text
用户登录
用户注册
族谱协作
Dashboard 统计
成员增删改查
模糊查询
树形预览
祖先查询
亲缘链路查询
ER 图与关系模型
3NF 分析
约束与触发器
10 万级模拟数据
COPY 导入导出
索引设计
EXPLAIN 性能对比
```

如果你后续要真正开始写代码，建议先从 **数据库 SQL 脚本 + Qt 登录窗口 + 成员管理界面** 三部分开始，因为这三部分最容易形成可演示成果。


cd "Database_Course_Experiment"

cmake -S . -B build -G Ninja
cmake --build build
./build/GenealogySystem
./scripts/run_wsl.sh