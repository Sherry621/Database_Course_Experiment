-- Compare great-grandchildren query with and without parent_id index.
-- In this script, depth 0 is the root member, depth 1 is children,
-- depth 2 is grandchildren, and depth 3 is great-grandchildren.
--
-- Optional psql variables:
--   root_id     default 1
--   output_file default generated_data/performance_explain.txt
--
-- Example:
--   psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
--     -v root_id=1 \
--     -v output_file=generated_data/performance_explain.txt \
--     -f sql/07_performance_explain.sql

\if :{?root_id}
\else
\set root_id 1
\endif

\if :{?output_file}
\else
\set output_file generated_data/performance_explain.txt
\endif

\echo 'writing performance explain output to' :output_file
\o :output_file

\timing on

\qecho 'candidate roots with many descendants'
SELECT r.parent_id AS root_id, m.name, count(*) AS child_count
FROM parent_child_relations r
JOIN members m ON m.member_id = r.parent_id
GROUP BY r.parent_id, m.name
ORDER BY child_count DESC
LIMIT 10;

\qecho 'great-grandchildren query WITHOUT parent_id index'
DROP INDEX IF EXISTS idx_parent_child_parent_id;
DROP INDEX IF EXISTS idx_parent_child_child_id;
ANALYZE parent_child_relations;
ANALYZE members;

EXPLAIN (ANALYZE, BUFFERS)
WITH RECURSIVE descendants AS (
    SELECT
        m.member_id,
        m.name,
        m.generation,
        0 AS depth
    FROM members m
    WHERE m.member_id = :root_id

    UNION

    SELECT
        c.member_id,
        c.name,
        c.generation,
        d.depth + 1
    FROM descendants d
    JOIN parent_child_relations r ON r.parent_id = d.member_id
    JOIN members c ON c.member_id = r.child_id
    WHERE d.depth < 3
)
SELECT *
FROM descendants
WHERE depth = 3;

\qecho 'great-grandchildren query WITH parent_id index'
CREATE INDEX IF NOT EXISTS idx_parent_child_parent_id
ON parent_child_relations(parent_id);

CREATE INDEX IF NOT EXISTS idx_parent_child_child_id
ON parent_child_relations(child_id);

ANALYZE parent_child_relations;
ANALYZE members;

EXPLAIN (ANALYZE, BUFFERS)
WITH RECURSIVE descendants AS (
    SELECT
        m.member_id,
        m.name,
        m.generation,
        0 AS depth
    FROM members m
    WHERE m.member_id = :root_id

    UNION

    SELECT
        c.member_id,
        c.name,
        c.generation,
        d.depth + 1
    FROM descendants d
    JOIN parent_child_relations r ON r.parent_id = d.member_id
    JOIN members c ON c.member_id = r.child_id
    WHERE d.depth < 3
)
SELECT *
FROM descendants
WHERE depth = 3;

\o
\echo 'performance explain output saved to' :output_file
