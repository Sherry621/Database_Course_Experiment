CREATE EXTENSION IF NOT EXISTS pg_trgm;

CREATE INDEX IF NOT EXISTS idx_genealogies_creator
ON genealogies(creator_user_id);
-- postgreSQL不会自动为外键创建索引，因此我们需要手动创建索引来优化查询性能

CREATE INDEX IF NOT EXISTS idx_members_genealogy
ON members(genealogy_id);

CREATE INDEX IF NOT EXISTS idx_members_genealogy_generation
ON members(genealogy_id, generation);

CREATE INDEX IF NOT EXISTS idx_members_name_trgm
ON members
USING gin (name gin_trgm_ops);

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
