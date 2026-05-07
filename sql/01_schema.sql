CREATE TABLE IF NOT EXISTS users (
    user_id SERIAL PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    real_name VARCHAR(50),
    email VARCHAR(100) UNIQUE,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS genealogies (
    genealogy_id SERIAL PRIMARY KEY,
    title VARCHAR(100) NOT NULL,
    family_surname VARCHAR(20) NOT NULL,
    revision_time DATE,
    creator_user_id INT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    description TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS genealogy_collaborators (
    genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
    user_id INT NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    role VARCHAR(20) NOT NULL DEFAULT 'editor',
    invited_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (genealogy_id, user_id),
    CHECK (role IN ('editor', 'viewer'))
);

CREATE TABLE IF NOT EXISTS members (
    member_id SERIAL PRIMARY KEY,
    genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
    name VARCHAR(100) NOT NULL,
    gender CHAR(1) NOT NULL CHECK (gender IN ('M', 'F')),
    birth_year INT,
    death_year INT,
    generation INT,
    biography TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    CHECK (birth_year IS NULL OR death_year IS NULL OR birth_year <= death_year),
    CHECK (generation IS NULL OR generation >= 1)
);

CREATE TABLE IF NOT EXISTS parent_child_relations (
    relation_id SERIAL PRIMARY KEY,
    genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
    parent_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
    child_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
    relation_type VARCHAR(10) NOT NULL CHECK (relation_type IN ('father', 'mother')),
    CHECK (parent_id <> child_id),
    UNIQUE (child_id, relation_type)
);

CREATE TABLE IF NOT EXISTS marriages (
    marriage_id SERIAL PRIMARY KEY,
    genealogy_id INT NOT NULL REFERENCES genealogies(genealogy_id) ON DELETE CASCADE,
    person1_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
    person2_id INT NOT NULL REFERENCES members(member_id) ON DELETE CASCADE,
    marriage_year INT,
    divorce_year INT,
    description TEXT,
    CHECK (person1_id <> person2_id),
    CHECK (marriage_year IS NULL OR divorce_year IS NULL OR marriage_year <= divorce_year),
    UNIQUE (genealogy_id, person1_id, person2_id)
);
