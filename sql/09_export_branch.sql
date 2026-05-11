-- Export one descendant branch as a CSV backup file.
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
--     -f sql/09_export_branch.sql

\if :{?root_id}
\else
\set root_id 1
\endif

\if :{?max_depth}
\else
\set max_depth 6
\endif

\echo 'exporting descendant branch rooted at member_id=' :root_id 'max_depth=' :max_depth 'to generated_data/branch_export.csv'

DROP TABLE IF EXISTS tmp_branch_export;

CREATE TEMP TABLE tmp_branch_export AS
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
    p.parent_id,
    min(d.depth) AS depth,
    m.name,
    m.gender,
    m.birth_year,
    m.death_year,
    m.generation,
    m.biography
FROM descendant_ids d
JOIN members m ON m.member_id = d.member_id
LEFT JOIN LATERAL (
    SELECT r.parent_id
    FROM parent_child_relations r
    JOIN descendant_ids known_parent ON known_parent.member_id = r.parent_id
    WHERE r.child_id = d.member_id
    ORDER BY known_parent.depth, r.parent_id
    LIMIT 1
) p ON true
GROUP BY
    m.member_id,
    m.genealogy_id,
    p.parent_id,
    m.name,
    m.gender,
    m.birth_year,
    m.death_year,
    m.generation,
    m.biography
ORDER BY min(d.depth), m.generation, m.member_id
;

\copy tmp_branch_export TO 'generated_data/branch_export.csv' WITH (FORMAT csv, HEADER true)

DROP TABLE tmp_branch_export;

\echo 'branch export completed'
