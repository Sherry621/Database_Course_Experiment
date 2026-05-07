CREATE OR REPLACE FUNCTION check_parent_child_relation()
RETURNS TRIGGER AS $$
DECLARE
    parent_genealogy INT;
    child_genealogy INT;
    parent_birth INT;
    child_birth INT;
BEGIN
    SELECT genealogy_id, birth_year
    INTO parent_genealogy, parent_birth
    FROM members
    WHERE member_id = NEW.parent_id;

    SELECT genealogy_id, birth_year
    INTO child_genealogy, child_birth
    FROM members
    WHERE member_id = NEW.child_id;

    IF parent_genealogy <> child_genealogy OR NEW.genealogy_id <> parent_genealogy THEN
        RAISE EXCEPTION '亲子关系双方必须属于同一个族谱';
    END IF;

    IF parent_birth IS NOT NULL AND child_birth IS NOT NULL AND parent_birth >= child_birth THEN
        RAISE EXCEPTION '父母出生年份必须早于子女出生年份';
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS trg_check_parent_child_relation ON parent_child_relations;
CREATE TRIGGER trg_check_parent_child_relation
BEFORE INSERT OR UPDATE ON parent_child_relations
FOR EACH ROW
EXECUTE FUNCTION check_parent_child_relation();

CREATE OR REPLACE FUNCTION check_marriage_relation()
RETURNS TRIGGER AS $$
DECLARE
    p1_genealogy INT;
    p2_genealogy INT;
BEGIN
    SELECT genealogy_id INTO p1_genealogy FROM members WHERE member_id = NEW.person1_id;
    SELECT genealogy_id INTO p2_genealogy FROM members WHERE member_id = NEW.person2_id;

    IF p1_genealogy <> p2_genealogy OR NEW.genealogy_id <> p1_genealogy THEN
        RAISE EXCEPTION '婚姻关系双方必须属于同一个族谱';
    END IF;

    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

DROP TRIGGER IF EXISTS trg_check_marriage_relation ON marriages;
CREATE TRIGGER trg_check_marriage_relation
BEFORE INSERT OR UPDATE ON marriages
FOR EACH ROW
EXECUTE FUNCTION check_marriage_relation();
