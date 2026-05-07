-- 1. 给定成员 ID 查询配偶及所有子女
SELECT 'spouse' AS relation, m.*
FROM marriages mr
JOIN members m ON m.member_id = CASE
    WHEN mr.person1_id = :member_id THEN mr.person2_id
    ELSE mr.person1_id
END
WHERE mr.person1_id = :member_id OR mr.person2_id = :member_id
UNION ALL
SELECT 'child' AS relation, c.*
FROM parent_child_relations pcr
JOIN members c ON c.member_id = pcr.child_id
WHERE pcr.parent_id = :member_id;

-- 2. 递归查询所有祖先
WITH RECURSIVE ancestors AS (
    SELECT parent_id, child_id, 1 AS depth
    FROM parent_child_relations
    WHERE child_id = :member_id
    UNION ALL
    SELECT p.parent_id, p.child_id, a.depth + 1
    FROM parent_child_relations p
    JOIN ancestors a ON p.child_id = a.parent_id
)
SELECT depth, m.*
FROM ancestors a
JOIN members m ON m.member_id = a.parent_id
ORDER BY depth, m.birth_year;

-- 3. 统计某个家族中平均寿命最长的一代人
SELECT generation, AVG(death_year - birth_year) AS avg_life
FROM members
WHERE genealogy_id = :genealogy_id
  AND birth_year IS NOT NULL
  AND death_year IS NOT NULL
  AND generation IS NOT NULL
GROUP BY generation
ORDER BY avg_life DESC
LIMIT 1;

-- 4. 查询年龄超过 50 岁且没有配偶的男性成员
SELECT m.*
FROM members m
WHERE m.genealogy_id = :genealogy_id
  AND m.gender = 'M'
  AND COALESCE(m.death_year, EXTRACT(YEAR FROM CURRENT_DATE)::INT) - m.birth_year > 50
  AND NOT EXISTS (
      SELECT 1
      FROM marriages mr
      WHERE mr.person1_id = m.member_id OR mr.person2_id = m.member_id
  );

-- 5. 找出出生年份早于该辈分平均出生年份的成员
WITH generation_avg AS (
    SELECT generation, AVG(birth_year) AS avg_birth_year
    FROM members
    WHERE genealogy_id = :genealogy_id
      AND birth_year IS NOT NULL
      AND generation IS NOT NULL
    GROUP BY generation
)
SELECT m.*, g.avg_birth_year
FROM members m
JOIN generation_avg g ON g.generation = m.generation
WHERE m.genealogy_id = :genealogy_id
  AND m.birth_year < g.avg_birth_year
ORDER BY m.generation, m.birth_year;
