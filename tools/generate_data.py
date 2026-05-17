#!/usr/bin/env python3
"""Generate CSV data for the genealogy database course project."""

from __future__ import annotations

import argparse
import csv
import hashlib
import random
from datetime import UTC, datetime
from pathlib import Path


SURNAMES = ["赵", "钱", "孙", "李", "周", "吴", "郑", "王", "冯", "陈"]
GIVEN_NAMES = [
    "明",
    "华",
    "强",
    "芳",
    "伟",
    "娜",
    "敏",
    "杰",
    "磊",
    "静",
    "勇",
    "丽",
    "涛",
    "慧",
    "超",
    "琳",
]


def sha256_password(password: str) -> str:
    return hashlib.sha256(password.encode("utf-8")).hexdigest()


def write_csv(path: Path, fieldnames: list[str], rows: list[dict[str, object]]) -> None:
    with path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def distribute_members(total_members: int) -> list[int]:
    first_size = max(55_000, total_members // 2 + 5_000)
    remaining = total_members - first_size
    base = remaining // 9
    sizes = [first_size] + [base] * 9
    for index in range(remaining - base * 9):
        sizes[index + 1] += 1
    return sizes


def member_name(surname: str, member_id: int, generation: int) -> str:
    return f"{surname}{GIVEN_NAMES[member_id % len(GIVEN_NAMES)]}{generation % 10}"


def generation_counts(target_size: int, generation_count: int) -> list[int]:
    counts = [target_size // generation_count] * generation_count
    for index in range(target_size % generation_count):
        counts[index] += 1
    counts[0] = max(counts[0], 20)
    return counts


def death_year_for(birth_year: int) -> int | str:
    if birth_year >= 1965:
        return ""
    return birth_year + random.randint(58, 88)


def generate_genealogy(
    genealogy_id: int,
    surname: str,
    target_size: int,
    first_member_id: int,
    first_relation_id: int,
    first_marriage_id: int,
) -> tuple[
    list[dict[str, object]],
    list[dict[str, object]],
    list[dict[str, object]],
    int,
    int,
    int,
]:
    members: list[dict[str, object]] = []
    relations: list[dict[str, object]] = []
    marriages: list[dict[str, object]] = []

    member_id = first_member_id
    relation_id = first_relation_id
    marriage_id = first_marriage_id
    now = datetime.now(UTC).replace(tzinfo=None).isoformat(timespec="seconds")

    generation_count = 35 if target_size >= 50_000 else 12
    counts = generation_counts(target_size, generation_count)

    previous_males: list[int] = []
    previous_females: list[int] = []

    for generation, count in enumerate(counts, start=1):
        current_males: list[int] = []
        current_females: list[int] = []
        birth_base = 1320 + (generation - 1) * 20 if generation_count >= 30 else 1780 + (generation - 1) * 20

        for offset in range(count):
            if offset == 0:
                gender = "M"
            elif offset == 1:
                gender = "F"
            else:
                gender = "M" if member_id % 2 == 0 else "F"

            birth_year = birth_base + random.randint(-2, 2)
            members.append(
                {
                    "member_id": member_id,
                    "genealogy_id": genealogy_id,
                    "name": member_name(surname, member_id, generation),
                    "gender": gender,
                    "birth_year": birth_year,
                    "death_year": death_year_for(birth_year),
                    "generation": generation,
                    "biography": f"{surname}氏第{generation}代成员，模拟数据编号{member_id}",
                    "created_at": now,
                }
            )

            if gender == "M":
                current_males.append(member_id)
            else:
                current_females.append(member_id)

            if previous_males and previous_females:
                father_id = (
                    previous_males[offset % len(previous_males)]
                    if offset < len(previous_males)
                    else random.choice(previous_males)
                )
                mother_id = (
                    previous_females[offset % len(previous_females)]
                    if offset < len(previous_females)
                    else random.choice(previous_females)
                )
                relations.append(
                    {
                        "relation_id": relation_id,
                        "genealogy_id": genealogy_id,
                        "parent_id": father_id,
                        "child_id": member_id,
                        "relation_type": "father",
                    }
                )
                relation_id += 1
                relations.append(
                    {
                        "relation_id": relation_id,
                        "genealogy_id": genealogy_id,
                        "parent_id": mother_id,
                        "child_id": member_id,
                        "relation_type": "mother",
                    }
                )
                relation_id += 1

            member_id += 1

        pair_count = min(len(current_males), len(current_females), max(1, count // 8))
        for index in range(pair_count):
            person1_id, person2_id = sorted((current_males[index], current_females[index]))
            marriages.append(
                {
                    "marriage_id": marriage_id,
                    "genealogy_id": genealogy_id,
                    "person1_id": person1_id,
                    "person2_id": person2_id,
                    "marriage_year": birth_base + 22,
                    "divorce_year": "",
                    "description": f"{surname}氏第{generation}代婚姻关系",
                }
            )
            marriage_id += 1

        previous_males = current_males or previous_males
        previous_females = current_females or previous_females

    return members, relations, marriages, member_id, relation_id, marriage_id


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate genealogy CSV data.")
    parser.add_argument("--out", default="generated_data", help="Output directory.")
    parser.add_argument("--total-members", type=int, default=100_000, help="Total member count, at least 100000.")
    parser.add_argument("--seed", type=int, default=20260510)
    args = parser.parse_args()

    if args.total_members < 100_000:
        raise SystemExit("--total-members must be at least 100000 for the lab requirement")

    random.seed(args.seed)
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    now = datetime.now(UTC).replace(tzinfo=None).isoformat(timespec="seconds")
    password_hash = sha256_password("123456")

    users = [
        {
            "user_id": user_id,
            "username": f"user{user_id}",
            "password_hash": password_hash,
            "real_name": f"用户{user_id}",
            "email": f"user{user_id}@example.com",
            "created_at": now,
        }
        for user_id in range(1, 6)
    ]
    users[0]["username"] = "admin"
    users[0]["real_name"] = "管理员"
    users[0]["email"] = "admin@example.com"

    genealogies: list[dict[str, object]] = []
    collaborators: list[dict[str, object]] = []
    all_members: list[dict[str, object]] = []
    all_relations: list[dict[str, object]] = []
    all_marriages: list[dict[str, object]] = []

    member_id = 1
    relation_id = 1
    marriage_id = 1

    for genealogy_id, target_size in enumerate(distribute_members(args.total_members), start=1):
        surname = SURNAMES[genealogy_id - 1]
        creator_user_id = ((genealogy_id - 1) % len(users)) + 1
        genealogies.append(
            {
                "genealogy_id": genealogy_id,
                "title": f"{surname}氏族谱",
                "family_surname": surname,
                "revision_time": "2026-05-10",
                "creator_user_id": creator_user_id,
                "description": f"{surname}氏族谱，模拟数据规模 {target_size}",
                "created_at": now,
            }
        )
        collaborators.append(
            {
                "genealogy_id": genealogy_id,
                "user_id": (creator_user_id % len(users)) + 1,
                "role": "editor",
                "invited_at": now,
            }
        )

        members, relations, marriages, member_id, relation_id, marriage_id = generate_genealogy(
            genealogy_id,
            surname,
            target_size,
            member_id,
            relation_id,
            marriage_id,
        )
        all_members.extend(members)
        all_relations.extend(relations)
        all_marriages.extend(marriages)

    write_csv(out / "users.csv", ["user_id", "username", "password_hash", "real_name", "email", "created_at"], users)
    write_csv(
        out / "genealogies.csv",
        ["genealogy_id", "title", "family_surname", "revision_time", "creator_user_id", "description", "created_at"],
        genealogies,
    )
    write_csv(out / "genealogy_collaborators.csv", ["genealogy_id", "user_id", "role", "invited_at"], collaborators)
    write_csv(
        out / "members.csv",
        [
            "member_id",
            "genealogy_id",
            "name",
            "gender",
            "birth_year",
            "death_year",
            "generation",
            "biography",
            "created_at",
        ],
        all_members,
    )
    write_csv(
        out / "parent_child_relations.csv",
        ["relation_id", "genealogy_id", "parent_id", "child_id", "relation_type"],
        all_relations,
    )
    write_csv(
        out / "marriages.csv",
        ["marriage_id", "genealogy_id", "person1_id", "person2_id", "marriage_year", "divorce_year", "description"],
        all_marriages,
    )

    print(f"Generated users={len(users)} genealogies={len(genealogies)}")
    print(f"Generated members={len(all_members)} relations={len(all_relations)} marriages={len(all_marriages)}")
    print(f"Output directory: {out}")


if __name__ == "__main__":
    main()
