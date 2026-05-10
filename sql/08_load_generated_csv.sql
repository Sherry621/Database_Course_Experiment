-- Load generated CSV data into PostgreSQL.
--
-- Run from Database_Course_Experiment/ after generating CSV files:
--   python3 tools/generate_data.py --out generated_data --total-members 100000
--   psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" -v ON_ERROR_STOP=1 -f sql/08_load_generated_csv.sql

TRUNCATE TABLE
    marriages,
    parent_child_relations,
    members,
    genealogy_collaborators,
    genealogies,
    users
RESTART IDENTITY CASCADE;

\copy users(user_id, username, password_hash, real_name, email, created_at) FROM 'generated_data/users.csv' WITH (FORMAT csv, HEADER true, NULL '')
\copy genealogies(genealogy_id, title, family_surname, revision_time, creator_user_id, description, created_at) FROM 'generated_data/genealogies.csv' WITH (FORMAT csv, HEADER true, NULL '')
\copy genealogy_collaborators(genealogy_id, user_id, role, invited_at) FROM 'generated_data/genealogy_collaborators.csv' WITH (FORMAT csv, HEADER true, NULL '')
\copy members(member_id, genealogy_id, name, gender, birth_year, death_year, generation, biography, created_at) FROM 'generated_data/members.csv' WITH (FORMAT csv, HEADER true, NULL '')
\copy parent_child_relations(relation_id, genealogy_id, parent_id, child_id, relation_type) FROM 'generated_data/parent_child_relations.csv' WITH (FORMAT csv, HEADER true, NULL '')
\copy marriages(marriage_id, genealogy_id, person1_id, person2_id, marriage_year, divorce_year, description) FROM 'generated_data/marriages.csv' WITH (FORMAT csv, HEADER true, NULL '')

SELECT setval(pg_get_serial_sequence('users', 'user_id'), (SELECT max(user_id) FROM users));
SELECT setval(pg_get_serial_sequence('genealogies', 'genealogy_id'), (SELECT max(genealogy_id) FROM genealogies));
SELECT setval(pg_get_serial_sequence('members', 'member_id'), (SELECT max(member_id) FROM members));
SELECT setval(pg_get_serial_sequence('parent_child_relations', 'relation_id'), (SELECT max(relation_id) FROM parent_child_relations));
SELECT setval(pg_get_serial_sequence('marriages', 'marriage_id'), (SELECT max(marriage_id) FROM marriages));

ANALYZE users;
ANALYZE genealogies;
ANALYZE genealogy_collaborators;
ANALYZE members;
ANALYZE parent_child_relations;
ANALYZE marriages;

\echo 'generated data counts'
SELECT 'genealogies' AS table_name, count(*) AS row_count FROM genealogies
UNION ALL
SELECT 'members', count(*) FROM members
UNION ALL
SELECT 'parent_child_relations', count(*) FROM parent_child_relations
UNION ALL
SELECT 'marriages', count(*) FROM marriages
ORDER BY table_name;

\echo 'largest genealogy'
SELECT genealogy_id, count(*) AS member_count, max(generation) AS max_generation
FROM members
GROUP BY genealogy_id
ORDER BY member_count DESC
LIMIT 3;
