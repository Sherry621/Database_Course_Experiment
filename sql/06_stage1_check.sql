\echo '1. table counts'
SELECT 'users' AS table_name, count(*) AS row_count FROM users
UNION ALL
SELECT 'genealogies', count(*) FROM genealogies
UNION ALL
SELECT 'genealogy_collaborators', count(*) FROM genealogy_collaborators
UNION ALL
SELECT 'members', count(*) FROM members
UNION ALL
SELECT 'parent_child_relations', count(*) FROM parent_child_relations
UNION ALL
SELECT 'marriages', count(*) FROM marriages
ORDER BY table_name;

\echo '2. accessible genealogies for admin user_id=1'
SELECT DISTINCT g.genealogy_id, g.title, g.family_surname
FROM genealogies g
LEFT JOIN genealogy_collaborators c ON c.genealogy_id = g.genealogy_id
WHERE g.creator_user_id = 1 OR c.user_id = 1
ORDER BY g.genealogy_id;

\echo '3. spouse and children for member_id=3'
SELECT 'spouse' AS relation, m.member_id, m.name, m.generation
FROM marriages mr
JOIN members m ON m.member_id = CASE
    WHEN mr.person1_id = 3 THEN mr.person2_id
    ELSE mr.person1_id
END
WHERE mr.person1_id = 3 OR mr.person2_id = 3
UNION ALL
SELECT 'child' AS relation, c.member_id, c.name, c.generation
FROM parent_child_relations pcr
JOIN members c ON c.member_id = pcr.child_id
WHERE pcr.parent_id = 3
ORDER BY relation DESC, member_id;

\echo '4. recursive ancestors for member_id=7'
WITH RECURSIVE ancestors AS (
    SELECT parent_id, child_id, 1 AS depth
    FROM parent_child_relations
    WHERE child_id = 7
    UNION ALL
    SELECT p.parent_id, p.child_id, a.depth + 1
    FROM parent_child_relations p
    JOIN ancestors a ON p.child_id = a.parent_id
)
SELECT depth, m.member_id, m.name, m.generation
FROM ancestors a
JOIN members m ON m.member_id = a.parent_id
ORDER BY depth, m.birth_year;

\echo '5. trigger should reject invalid father gender'
DO $$
DECLARE
    inserted_invalid_relation BOOLEAN := false;
BEGIN
    BEGIN
        INSERT INTO parent_child_relations(genealogy_id, parent_id, child_id, relation_type)
        VALUES (1, 4, 7, 'father');
        inserted_invalid_relation := true;
    EXCEPTION WHEN OTHERS THEN
        RAISE NOTICE 'expected trigger rejection: %', SQLERRM;
    END;

    IF inserted_invalid_relation THEN
        DELETE FROM parent_child_relations
        WHERE genealogy_id = 1
          AND parent_id = 4
          AND child_id = 7
          AND relation_type = 'father';
        RAISE EXCEPTION 'invalid father relation was inserted; trigger check failed';
    END IF;
END;
$$;
