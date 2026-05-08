# 阶段一验收结果：数据库初始化与小数据验证

执行时间：2026-05-08

## 1. 数据库连接

当前数据库连接正常：

```text
database: genealogy_lab
user: genealogy_user
RDBMS: PostgreSQL 16.13
```

## 2. 已执行脚本

执行目录：

```text
Database_Course_Experiment/
```

按顺序执行：

```bash
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/01_schema.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/02_indexes.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/03_triggers.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/05_seed_small.sql
psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/06_stage1_check.sql
```

## 3. 小数据导入结果

```text
users: 2
genealogies: 1
genealogy_collaborators: 1
members: 7
parent_child_relations: 7
marriages: 2
```

测试账号：

```text
username: admin
password: 123456
```

## 4. 权限验证

用户 `admin`，即 `user_id = 1`，可以访问：

```text
genealogy_id: 1
title: 张氏族谱
family_surname: 张
```

这说明 `genealogies.creator_user_id` 与后续协作者权限查询路径可用。

## 5. 核心查询验证

### 配偶及子女查询

对 `member_id = 3` 查询，结果为：

```text
spouse: 王二娘(member_id=4)
child: 张三世(member_id=5)
child: 张三妹(member_id=6)
```

说明婚姻关系表和亲子关系表都可以正确关联到成员表。

### 递归祖先查询

对 `member_id = 7` 查询，递归结果为：

```text
depth=1: 张三世(member_id=5)
depth=2: 张二世(member_id=3), 王二娘(member_id=4)
depth=3: 张一世(member_id=1), 李一娘(member_id=2)
```

说明 `WITH RECURSIVE` 祖先追溯查询可用，符合族谱深度不确定的实验要求。

## 6. 触发器验证

执行错误数据：

```sql
INSERT INTO parent_child_relations(genealogy_id, parent_id, child_id, relation_type)
VALUES (1, 4, 7, 'father');
```

其中 `member_id = 4` 是女性，却被插入为 `father`。触发器返回：

```text
父亲关系要求父节点性别为男性
```

说明数据库层能够阻止明显违反业务规则的亲子关系。

## 7. 阶段一结论

阶段一通过：

```text
表结构已创建。
索引已创建。
触发器已创建并生效。
小规模测试数据已导入。
核心查询路径已验证。
Qt 程序后续可以基于同一套表结构继续开发。
```
