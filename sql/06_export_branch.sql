-- Export one descendant branch as CSV backup files.
--
-- Optional psql variables:
--   root_id   branch root member id, default 1
--   max_depth max descendant depth to export, default 6
--
-- Example:
--   mkdir -p generated_data
--   psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
--     -v root_id=1 \
--     -v max_depth=6 \
--     -f sql/06_export_branch.sql

\if :{?root_id}
\else
\set root_id 1
\endif

\if :{?max_depth}
\else
\set max_depth 6
\endif

\echo 'exporting descendant branch rooted at member_id=' :root_id 'max_depth=' :max_depth 'to generated_data/branch_*.csv'

DROP TABLE IF EXISTS tmp_branch_members;

CREATE TEMP TABLE tmp_branch_members AS
WITH RECURSIVE descendant_ids(member_id, depth) AS (
    SELECT
        m.member_id,
        0 AS depth
    FROM members m
    WHERE m.member_id = :root_id

    UNION

    SELECT
        r.child_id,
        d.depth + 1
    FROM descendant_ids d
    JOIN parent_child_relations r ON r.parent_id = d.member_id
    WHERE d.depth < :max_depth
)
SELECT
    m.member_id,
    m.genealogy_id,
    min(d.depth) AS depth,
    m.name,
    m.gender,
    m.birth_year,
    m.death_year,
    m.generation,
    m.biography
FROM descendant_ids d
JOIN members m ON m.member_id = d.member_id
GROUP BY
    m.member_id,
    m.genealogy_id,
    m.name,
    m.gender,
    m.birth_year,
    m.death_year,
    m.generation,
    m.biography
ORDER BY min(d.depth), m.generation, m.member_id
;

\copy tmp_branch_members TO 'generated_data/branch_members.csv' WITH (FORMAT csv, HEADER true)

CREATE TEMP TABLE tmp_branch_parent_child_relations AS
    SELECT
        r.relation_id,
        r.genealogy_id,
        r.parent_id,
        r.child_id,
        r.relation_type
    FROM parent_child_relations r
    JOIN tmp_branch_members parent_member ON parent_member.member_id = r.parent_id
    JOIN tmp_branch_members child_member ON child_member.member_id = r.child_id
    ORDER BY r.relation_id
;

\copy tmp_branch_parent_child_relations TO 'generated_data/branch_parent_child_relations.csv' WITH (FORMAT csv, HEADER true)

CREATE TEMP TABLE tmp_branch_marriages AS
    SELECT
        mr.marriage_id,
        mr.genealogy_id,
        mr.person1_id,
        mr.person2_id,
        mr.marriage_year,
        mr.divorce_year,
        mr.description
    FROM marriages mr
    JOIN tmp_branch_members p1 ON p1.member_id = mr.person1_id
    JOIN tmp_branch_members p2 ON p2.member_id = mr.person2_id
    ORDER BY mr.marriage_id
;

\copy tmp_branch_marriages TO 'generated_data/branch_marriages.csv' WITH (FORMAT csv, HEADER true)

DROP TABLE tmp_branch_marriages;
DROP TABLE tmp_branch_parent_child_relations;
DROP TABLE tmp_branch_members;

\echo 'branch export completed: branch_members.csv, branch_parent_child_relations.csv, branch_marriages.csv'
