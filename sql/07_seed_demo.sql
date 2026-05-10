-- Demo dataset for Qt UI testing.
-- Safe to run repeatedly. It inserts or updates deterministic sample data.
--
-- Login accounts:
--   admin   / 123456
--   editor1 / 123456
--   viewer1 / 123456

INSERT INTO users(user_id, username, password_hash, real_name, email)
VALUES
    (1, 'admin', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '管理员', 'admin@example.com'),
    (2, 'editor1', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '协作者一', 'editor1@example.com'),
    (3, 'viewer1', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '查看者一', 'viewer1@example.com')
ON CONFLICT (user_id) DO UPDATE
SET username = EXCLUDED.username,
    password_hash = EXCLUDED.password_hash,
    real_name = EXCLUDED.real_name,
    email = EXCLUDED.email;

INSERT INTO genealogies(genealogy_id, title, family_surname, revision_time, creator_user_id, description)
VALUES
    (1, '张氏族谱', '张', DATE '2026-05-08', 1, '用于课程演示的张氏族谱'),
    (2, '李氏族谱', '李', DATE '2026-05-09', 1, '用于测试多族谱切换'),
    (3, '王氏族谱', '王', DATE '2026-05-10', 2, '用于测试协作者访问')
ON CONFLICT (genealogy_id) DO UPDATE
SET title = EXCLUDED.title,
    family_surname = EXCLUDED.family_surname,
    revision_time = EXCLUDED.revision_time,
    creator_user_id = EXCLUDED.creator_user_id,
    description = EXCLUDED.description;

INSERT INTO genealogy_collaborators(genealogy_id, user_id, role)
VALUES
    (1, 2, 'editor'),
    (1, 3, 'viewer'),
    (3, 1, 'editor')
ON CONFLICT (genealogy_id, user_id) DO UPDATE
SET role = EXCLUDED.role;

INSERT INTO members(member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography)
VALUES
    -- Zhang genealogy, 5 generations
    (1, 1, '张一世', 'M', 1920, 1988, 1, '张氏第一代成员'),
    (2, 1, '李一娘', 'F', 1924, 1991, 1, '张一世配偶'),
    (3, 1, '张二世', 'M', 1948, 2010, 2, '张氏第二代长子'),
    (4, 1, '王二娘', 'F', 1951, NULL, 2, '张二世配偶'),
    (5, 1, '张二叔', 'M', 1953, NULL, 2, '张氏第二代次子'),
    (6, 1, '赵二娘', 'F', 1955, NULL, 2, '张二叔配偶'),
    (7, 1, '张三世', 'M', 1975, NULL, 3, '张氏第三代成员'),
    (8, 1, '张三妹', 'F', 1978, NULL, 3, '张氏第三代成员'),
    (9, 1, '张三堂', 'M', 1980, NULL, 3, '张二叔之子'),
    (10, 1, '陈三娘', 'F', 1977, NULL, 3, '张三世配偶'),
    (11, 1, '张四世', 'M', 2002, NULL, 4, '张氏第四代成员'),
    (12, 1, '张四妹', 'F', 2005, NULL, 4, '张氏第四代成员'),
    (13, 1, '张五世', 'M', 2026, NULL, 5, '张氏第五代成员'),

    -- Li genealogy
    (101, 2, '李一世', 'M', 1930, 1998, 1, '李氏第一代成员'),
    (102, 2, '周一娘', 'F', 1934, 2001, 1, '李一世配偶'),
    (103, 2, '李二世', 'M', 1960, NULL, 2, '李氏第二代成员'),
    (104, 2, '刘二娘', 'F', 1962, NULL, 2, '李二世配偶'),
    (105, 2, '李三世', 'F', 1988, NULL, 3, '李氏第三代成员'),
    (106, 2, '李三弟', 'M', 1991, NULL, 3, '李氏第三代成员'),

    -- Wang genealogy
    (201, 3, '王一世', 'M', 1940, 2012, 1, '王氏第一代成员'),
    (202, 3, '孙一娘', 'F', 1943, NULL, 1, '王一世配偶'),
    (203, 3, '王二世', 'M', 1970, NULL, 2, '王氏第二代成员'),
    (204, 3, '王二妹', 'F', 1973, NULL, 2, '王氏第二代成员'),
    (205, 3, '王三世', 'M', 1998, NULL, 3, '王氏第三代成员')
ON CONFLICT (member_id) DO UPDATE
SET genealogy_id = EXCLUDED.genealogy_id,
    name = EXCLUDED.name,
    gender = EXCLUDED.gender,
    birth_year = EXCLUDED.birth_year,
    death_year = EXCLUDED.death_year,
    generation = EXCLUDED.generation,
    biography = EXCLUDED.biography;

INSERT INTO parent_child_relations(relation_id, genealogy_id, parent_id, child_id, relation_type)
VALUES
    -- Zhang genealogy
    (1, 1, 1, 3, 'father'),
    (2, 1, 2, 3, 'mother'),
    (3, 1, 1, 5, 'father'),
    (4, 1, 2, 5, 'mother'),
    (5, 1, 3, 7, 'father'),
    (6, 1, 4, 7, 'mother'),
    (7, 1, 3, 8, 'father'),
    (8, 1, 4, 8, 'mother'),
    (9, 1, 5, 9, 'father'),
    (10, 1, 6, 9, 'mother'),
    (11, 1, 7, 11, 'father'),
    (12, 1, 10, 11, 'mother'),
    (13, 1, 7, 12, 'father'),
    (14, 1, 10, 12, 'mother'),
    (15, 1, 11, 13, 'father'),

    -- Li genealogy
    (101, 2, 101, 103, 'father'),
    (102, 2, 102, 103, 'mother'),
    (103, 2, 103, 105, 'father'),
    (104, 2, 104, 105, 'mother'),
    (105, 2, 103, 106, 'father'),
    (106, 2, 104, 106, 'mother'),

    -- Wang genealogy
    (201, 3, 201, 203, 'father'),
    (202, 3, 202, 203, 'mother'),
    (203, 3, 201, 204, 'father'),
    (204, 3, 202, 204, 'mother'),
    (205, 3, 203, 205, 'father')
ON CONFLICT (relation_id) DO UPDATE
SET genealogy_id = EXCLUDED.genealogy_id,
    parent_id = EXCLUDED.parent_id,
    child_id = EXCLUDED.child_id,
    relation_type = EXCLUDED.relation_type;

INSERT INTO marriages(marriage_id, genealogy_id, person1_id, person2_id, marriage_year, divorce_year, description)
VALUES
    (1, 1, 1, 2, 1945, NULL, '张氏第一代婚姻'),
    (2, 1, 3, 4, 1970, NULL, '张氏第二代婚姻'),
    (3, 1, 5, 6, 1976, NULL, '张氏第二代支系婚姻'),
    (4, 1, 7, 10, 2000, NULL, '张氏第三代婚姻'),
    (101, 2, 101, 102, 1958, NULL, '李氏第一代婚姻'),
    (102, 2, 103, 104, 1985, NULL, '李氏第二代婚姻'),
    (201, 3, 201, 202, 1965, NULL, '王氏第一代婚姻')
ON CONFLICT (marriage_id) DO UPDATE
SET genealogy_id = EXCLUDED.genealogy_id,
    person1_id = EXCLUDED.person1_id,
    person2_id = EXCLUDED.person2_id,
    marriage_year = EXCLUDED.marriage_year,
    divorce_year = EXCLUDED.divorce_year,
    description = EXCLUDED.description;

SELECT setval(pg_get_serial_sequence('users', 'user_id'), (SELECT max(user_id) FROM users));
SELECT setval(pg_get_serial_sequence('genealogies', 'genealogy_id'), (SELECT max(genealogy_id) FROM genealogies));
SELECT setval(pg_get_serial_sequence('members', 'member_id'), (SELECT max(member_id) FROM members));
SELECT setval(pg_get_serial_sequence('parent_child_relations', 'relation_id'), (SELECT max(relation_id) FROM parent_child_relations));
SELECT setval(pg_get_serial_sequence('marriages', 'marriage_id'), (SELECT max(marriage_id) FROM marriages));
