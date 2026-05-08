-- Small deterministic dataset for stage-one database verification.
-- Login account used by the Qt app:
--   username: admin
--   password: 123456

INSERT INTO users(user_id, username, password_hash, real_name, email)
VALUES
    (1, 'admin', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '管理员', 'admin@example.com'),
    (2, 'editor1', '8d969eef6ecad3c29a3a629280e686cf0c3f5d5a86aff3ca12020c923adc6c92', '协作者一', 'editor1@example.com')
ON CONFLICT (user_id) DO UPDATE
SET username = EXCLUDED.username,
    password_hash = EXCLUDED.password_hash,
    real_name = EXCLUDED.real_name,
    email = EXCLUDED.email;

INSERT INTO genealogies(genealogy_id, title, family_surname, revision_time, creator_user_id, description)
VALUES
    (1, '张氏族谱', '张', DATE '2026-05-08', 1, '阶段一小规模验证数据')
ON CONFLICT (genealogy_id) DO UPDATE
SET title = EXCLUDED.title,
    family_surname = EXCLUDED.family_surname,
    revision_time = EXCLUDED.revision_time,
    creator_user_id = EXCLUDED.creator_user_id,
    description = EXCLUDED.description;

INSERT INTO genealogy_collaborators(genealogy_id, user_id, role)
VALUES
    (1, 2, 'editor')
ON CONFLICT (genealogy_id, user_id) DO UPDATE
SET role = EXCLUDED.role;

INSERT INTO members(member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography)
VALUES
    (1, 1, '张一世', 'M', 1920, 1988, 1, '张氏第一代成员'),
    (2, 1, '李一娘', 'F', 1924, 1991, 1, '张一世配偶'),
    (3, 1, '张二世', 'M', 1948, 2010, 2, '张氏第二代成员'),
    (4, 1, '王二娘', 'F', 1951, NULL, 2, '张二世配偶'),
    (5, 1, '张三世', 'M', 1975, NULL, 3, '张氏第三代成员'),
    (6, 1, '张三妹', 'F', 1978, NULL, 3, '张氏第三代成员'),
    (7, 1, '张四世', 'M', 2002, NULL, 4, '张氏第四代成员')
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
    (1, 1, 1, 3, 'father'),
    (2, 1, 2, 3, 'mother'),
    (3, 1, 3, 5, 'father'),
    (4, 1, 4, 5, 'mother'),
    (5, 1, 3, 6, 'father'),
    (6, 1, 4, 6, 'mother'),
    (7, 1, 5, 7, 'father')
ON CONFLICT (relation_id) DO UPDATE
SET genealogy_id = EXCLUDED.genealogy_id,
    parent_id = EXCLUDED.parent_id,
    child_id = EXCLUDED.child_id,
    relation_type = EXCLUDED.relation_type;

INSERT INTO marriages(marriage_id, genealogy_id, person1_id, person2_id, marriage_year, divorce_year, description)
VALUES
    (1, 1, 1, 2, 1945, NULL, '第一代婚姻'),
    (2, 1, 3, 4, 1970, NULL, '第二代婚姻')
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
