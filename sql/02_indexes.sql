CREATE EXTENSION IF NOT EXISTS pg_trgm;

-- postgreSQL不会自动为外键创建索引，因此我们需要手动创建索引来优化查询性能
CREATE INDEX IF NOT EXISTS idx_genealogies_creator
ON genealogies(creator_user_id);

CREATE INDEX IF NOT EXISTS idx_members_genealogy
ON members(genealogy_id);

CREATE INDEX IF NOT EXISTS idx_members_genealogy_generation
ON members(genealogy_id, generation);

CREATE INDEX IF NOT EXISTS idx_members_name_trgm
ON members
USING gin (name gin_trgm_ops);

-- B树索引适用于精确匹配和范围查询，而GIN索引适用于模糊匹配和全文搜索。这里我们用B树索引来优化基于ID的查询，用GIN索引来优化基于名字的模糊查询。
CREATE INDEX IF NOT EXISTS idx_parent_child_parent_id
ON parent_child_relations(parent_id);

CREATE INDEX IF NOT EXISTS idx_parent_child_child_id
ON parent_child_relations(child_id);

CREATE INDEX IF NOT EXISTS idx_parent_child_genealogy
ON parent_child_relations(genealogy_id);

CREATE INDEX IF NOT EXISTS idx_marriages_person1
ON marriages(person1_id);

CREATE INDEX IF NOT EXISTS idx_marriages_person2
ON marriages(person2_id);
