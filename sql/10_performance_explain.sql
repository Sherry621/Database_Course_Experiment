-- Compare four-generation descendant query with and without parent_id index.
--
-- Optional psql variables:
--   root_id default 1
--
-- Example:
--   psql "postgresql://genealogy_user:genealogy_pass@localhost:5432/genealogy_lab" \
--     -v root_id=1 \
--     -f sql/10_performance_explain.sql

\if :{?root_id}
\else
\set root_id 1
\endif

\timing on

\echo 'candidate roots with many descendants'
SELECT r.parent_id AS root_id, m.name, count(*) AS child_count
FROM parent_child_relations r
JOIN members m ON m.member_id = r.parent_id
GROUP BY r.parent_id, m.name
ORDER BY child_count DESC
LIMIT 10;

\echo 'four-generation query WITHOUT parent_id index'
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
    WHERE d.depth < 4
)
SELECT *
FROM descendants
WHERE depth = 4;

\echo 'four-generation query WITH parent_id index'
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
    WHERE d.depth < 4
)
SELECT *
FROM descendants
WHERE depth = 4;
