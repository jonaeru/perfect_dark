"""Structured facts emitted by deterministic probes."""

from __future__ import annotations

import hashlib
import json
from dataclasses import asdict, dataclass, field
from typing import Any


@dataclass
class Fact:
    """One verified claim about map creation."""

    id: str
    category: str  # invariant | pipeline | binary | engine | editor | doc
    claim: str
    source: str  # file path, probe id, or code reference
    verified_by: str  # probe that verified this
    confidence: float = 1.0  # 1.0 = executed / code-derived
    tags: list[str] = field(default_factory=list)
    evidence: dict[str, Any] = field(default_factory=dict)

    @staticmethod
    def make_id(category: str, claim: str) -> str:
        digest = hashlib.sha256(f"{category}:{claim}".encode()).hexdigest()[:16]
        return f"{category}_{digest}"

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)

    @classmethod
    def from_dict(cls, data: dict[str, Any]) -> Fact:
        return cls(**data)


def merge_facts(existing: dict[str, Fact], new_facts: list[Fact]) -> dict[str, Fact]:
    """Merge by id; newer facts overwrite when verified_by differs or confidence higher."""
    out = dict(existing)
    for fact in new_facts:
        prev = out.get(fact.id)
        if prev is None or fact.confidence >= prev.confidence:
            out[fact.id] = fact
    return out


def load_knowledge(path: str) -> dict[str, Fact]:
    import os

    if not os.path.exists(path):
        return {}
    with open(path, encoding="utf-8") as fp:
        raw = json.load(fp)
    return {k: Fact.from_dict(v) for k, v in raw.get("facts", {}).items()}


def save_knowledge(path: str, facts: dict[str, Fact], *, meta: dict[str, Any]) -> None:
    import os

    os.makedirs(os.path.dirname(path), exist_ok=True)
    payload = {
        "version": 1,
        "meta": meta,
        "facts": {k: v.to_dict() for k, v in sorted(facts.items())},
    }
    with open(path, "w", encoding="utf-8") as fp:
        json.dump(payload, fp, indent=2, sort_keys=True)
        fp.write("\n")
