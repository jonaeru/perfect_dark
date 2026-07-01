"""Canonical editor JSON → MapDef conversion.

Single source of truth for turning Map Editor export JSON into a ``MapDef``
plus box-arena dimensions. Used by ``pdmap from-json``, ``test_map.py``, and
optional level-module export (``--write-level``).
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from typing import Any

from .builders import (
    PAD_FLOOR_OFFSET,
    add_ammo_row,
    add_floor_weapons,
    add_loadout_intro,
    floor_box_tiles,
)
from .core import MapDef
from .intro import Case, CaseRespawn, Hill, Spawn
from . import weapons as W

_VALID_NAME = re.compile(r"^[a-z][a-z0-9_]{0,31}$")

# Editor weapon IDs → weapons.py constants (fallback to hex literal).
_WEAPON_CONST: dict[int, str] = {
    W.WEAPON_FALCON2: "W.WEAPON_FALCON2",
    W.WEAPON_MAGSEC4: "W.WEAPON_MAGSEC4",
    W.WEAPON_MAULER: "W.WEAPON_MAULER",
    W.WEAPON_CMP150: "W.WEAPON_CMP150",
    W.WEAPON_LAPTOPGUN: "W.WEAPON_LAPTOPGUN",
    W.WEAPON_SUPERDRAGON: "W.WEAPON_SUPERDRAGON",
    W.WEAPON_SNIPERRIFLE: "W.WEAPON_SNIPERRIFLE",
    W.WEAPON_ROCKETLAUNCHER: "W.WEAPON_ROCKETLAUNCHER",
    W.WEAPON_CROSSBOW: "W.WEAPON_CROSSBOW",
    W.WEAPON_TRANQUILIZER: "W.WEAPON_TRANQUILIZER",
}

_AMMO_CONST: dict[int, str] = {
    W.AMMOTYPE_PISTOL: "W.AMMOTYPE_PISTOL",
    W.AMMOTYPE_RIFLE: "W.AMMOTYPE_RIFLE",
    W.AMMOTYPE_SHOTGUN: "W.AMMOTYPE_SHOTGUN",
    W.AMMOTYPE_ROCKET: "W.AMMOTYPE_ROCKET",
}


def validate_level_name(name: str) -> str:
    """Normalize and validate a level / asset name."""
    name = (name or "map").strip().lower()
    if not _VALID_NAME.match(name):
        raise ValueError(
            f"Invalid level name {name!r}; use lowercase letters, digits, underscore "
            "(must start with a letter)."
        )
    return name


def _weapon_id(raw: int) -> int:
    return int(raw)


def _ammo_id(raw: int) -> int:
    return int(raw)


@dataclass(frozen=True)
class EditorMapSpec:
    """Validated editor export: MapDef + box arena parameters."""

    name: str
    box_half: float
    box_height: float
    spawn_y: float
    mapdef: MapDef
    # Pads whose Y was auto-corrected from at/below floor to spawn_y.
    y_corrected_pads: tuple[int, ...] = ()

    @classmethod
    def from_json(
        cls,
        data: dict[str, Any],
        *,
        deploy_name: str | None = None,
        spawn_y: float | None = None,
    ) -> EditorMapSpec:
        """Parse editor JSON into a fully wired ``MapDef`` with all MP invariants."""
        name = validate_level_name(deploy_name or str(data.get("name", "map")))
        half = float(data.get("box_half", 5000))
        height = float(data.get("box_height", 3000))
        floor_y = spawn_y if spawn_y is not None else float(
            data.get("spawn_y", PAD_FLOOR_OFFSET)
        )

        pads: list[dict[str, Any]] = list(data.get("pads") or [])
        if not pads:
            raise ValueError("Refusing to build an empty map — add at least one pad.")

        g = MapDef(name)
        y_corrected: list[int] = []

        spawn_indices: list[int] = []
        weapon_rows: list[tuple[int, int]] = []
        ammo_by_type: dict[int, list[int]] = {}
        scenario_cmds: list[tuple[str, int, int]] = []  # kind, pad, team

        for i, p in enumerate(pads):
            idx = int(p.get("index", i))
            if idx != i:
                raise ValueError(
                    f"Pad at array position {i} has index={idx}; "
                    "indices must be contiguous 0..N-1."
                )

            x = float(p.get("x", 0))
            z = float(p.get("z", 0))
            y = float(p.get("y", floor_y))
            if y <= 0.0:
                y_corrected.append(i)
                y = floor_y
            room = int(p.get("room", 1))
            kind = p.get("type", "other")

            g.add_pad(index=i, x=x, y=y, z=z, room=room)

            if kind == "spawn":
                spawn_indices.append(i)
            elif kind == "weapon":
                if "weapon" not in p:
                    raise ValueError(f"Weapon pad {i} missing 'weapon' id.")
                weapon_rows.append((i, _weapon_id(int(p["weapon"]))))
            elif kind == "ammo":
                aid = _ammo_id(int(p.get("ammoType", W.AMMOTYPE_SHOTGUN)))
                ammo_by_type.setdefault(aid, []).append(i)
            elif kind == "scenario":
                sc = str(p.get("scenario", "hill"))
                team = int(p.get("team", 0))
                scenario_cmds.append((sc, i, team))

        if not spawn_indices:
            raise ValueError("Map has no spawn pads — add at least one pad with type 'spawn'.")

        for i in spawn_indices:
            g.add_intro(Spawn(pad=i))

        if weapon_rows:
            add_floor_weapons(g, weapon_rows)

        for aid, indices in sorted(ammo_by_type.items()):
            add_ammo_row(g, indices, ammotype=aid)

        for sc, pad, team in scenario_cmds:
            if sc == "case":
                g.add_intro(Case(team=team, pad=pad))
            elif sc == "case_respawn":
                g.add_intro(CaseRespawn(team=team, pad=pad))
            else:
                g.add_intro(Hill(pad=pad))

        add_loadout_intro(g)

        return cls(
            name=name,
            box_half=half,
            box_height=height,
            spawn_y=floor_y,
            mapdef=g,
            y_corrected_pads=tuple(y_corrected),
        )

    def tiles_json(self) -> dict:
        """Collision floor matching the box arena."""
        return floor_box_tiles(self.name, half=self.box_half, y=0.0, room_index=1)


def load_editor_json(path: str | None) -> dict[str, Any]:
    """Load editor JSON from a file path or stdin (``path`` is ``None`` or ``'-'``)."""
    import json
    import sys

    if path in (None, "-"):
        return json.load(sys.stdin)
    with open(path, encoding="utf-8") as fp:
        return json.load(fp)
