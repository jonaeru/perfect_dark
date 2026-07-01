import os
import json
from typing import Optional

from .core import MapDef, load_level_module, ROOT, ROMID, BUILD_DIR
from .seg import validate_seg_g_vtx
from .intro import Spawn, Case, CaseRespawn, Hill


def validate_mapdef(
    mapdef: MapDef,
    *,
    tiles_room_count: int | None = None,
) -> list[str]:
    """Logical checks on a MapDef before compiling assets (no binary I/O)."""
    errors: list[str] = []
    pad_indices = {p.index for p in mapdef.pads}

    for position, p in enumerate(mapdef.pads):
        if p.index != position:
            errors.append(
                f"Pad at position {position} has index {p.index}; pad indices "
                f"must be contiguous from 0 (the asset compiler numbers pads by "
                f"array position and props/intro reference them by position)"
            )

    for p in mapdef.pads:
        if p.room < 0:
            errors.append(f"Pad {p.index}: negative room {p.room}")
        if p.y <= 0.0:
            errors.append(
                f"Pad {p.index}: Y={p.y} is at/below the floor; the ground "
                f"search will reject the floor and the player will fall through "
                f"(use a small positive Y such as 10)"
            )
        if tiles_room_count is not None and p.room >= tiles_room_count:
            errors.append(
                f"Pad {p.index} spawns in room {p.room} but tiles have "
                f"{tiles_room_count} rooms"
            )

    has_spawn = any(isinstance(cmd, Spawn) for cmd in mapdef.intro)
    if not has_spawn:
        errors.append("Intro has no Spawn commands — Combat Simulator cannot place players")

    for i, cmd in enumerate(mapdef.intro):
        from .intro import INTRO_CODE_MAP
        code = getattr(cmd, "code", None)
        if code is not None and code not in INTRO_CODE_MAP and code != 0xFF:
            errors.append(f"Intro[{i}]: unknown command code {code:#x}")
        if isinstance(cmd, Spawn) and cmd.pad not in pad_indices:
            errors.append(f"Intro Spawn references missing pad {cmd.pad}")
        if isinstance(cmd, (Case, CaseRespawn, Hill)) and cmd.pad not in pad_indices:
            errors.append(
                f"Intro {type(cmd).__name__} references missing pad {cmd.pad}"
            )

    # CTF: each team with a Case pad needs at least one CaseRespawn for that team.
    case_teams = {cmd.team for cmd in mapdef.intro if isinstance(cmd, Case)}
    respawn_teams = {cmd.team for cmd in mapdef.intro if isinstance(cmd, CaseRespawn)}
    for team in sorted(case_teams):
        if team not in respawn_teams:
            errors.append(
                f"CTF team {team} has Case pad(s) but no case_respawn pad "
                f"(add scenario pads with scenario='case_respawn' and matching team)"
            )

    for i, prop in enumerate(mapdef.props):
        try:
            data = prop.pack()
            if len(data) == 0:
                errors.append(f"Prop[{i}] packed to zero bytes")
        except Exception as e:
            errors.append(f"Prop[{i}] pack failed: {e}")

    return errors


def validate_all(name: str, mapdef: Optional[MapDef] = None) -> tuple[list[str], list[str]]:
    errors: list[str] = []
    warnings: list[str] = []

    if mapdef is None:
        try:
            mod = load_level_module(name)
            mapdef = mod.build()
        except Exception as e:
            errors.append(f"Cannot load level module: {e}")
            return errors, warnings

    errors.extend(validate_mapdef(mapdef))

    for p in mapdef.pads:
        if p.y > 200.0:
            warnings.append(
                f"Pad {p.index}: Y={p.y} is far above the floor; spawns may take fall damage"
            )

    has_spawn = any(isinstance(cmd, Spawn) for cmd in mapdef.intro)
    if has_spawn and not any(isinstance(cmd, Case) for cmd in mapdef.intro):
        warnings.append("Intro has no Case commands — Capture the Case scenario will not work")
    if has_spawn and not any(isinstance(cmd, Hill) for cmd in mapdef.intro):
        warnings.append("Intro has no Hill commands — King of the Hill scenario will not work")

    if mapdef.intro:
        first = mapdef.intro[0]
        if not isinstance(first, Spawn):
            warnings.append(
                f"First intro command should be Spawn, got {type(first).__name__}"
            )

    tiles_json_path = os.path.join(ROOT, "src", "assets", ROMID, "tiles", f"{name}.json")
    if os.path.exists(tiles_json_path):
        with open(tiles_json_path) as f:
            tiles_data = json.load(f)
        room_names = list(tiles_data.get("rooms", {}).keys())
        for p in mapdef.pads:
            if p.room >= len(room_names):
                errors.append(
                    f"Pad {p.index} spawns in room {p.room} but tiles have "
                    f"{len(room_names)} rooms"
                )

    seg_path = os.path.join(BUILD_DIR, f"bg_{name}.seg")
    if not os.path.exists(seg_path):
        warnings.append(f"Seg not built at {seg_path} — run build with --seg if needed")
    else:
        with open(seg_path, "rb") as seg_fp:
            for msg in validate_seg_g_vtx(seg_fp.read()):
                errors.append(f"Seg G_VTX: {msg}")

    setup_path = os.path.join(ROOT, "build", ROMID, f"Ump_setup{name}Z")
    if not os.path.exists(setup_path):
        warnings.append(f"Setup binary missing at {setup_path}")

    return errors, warnings
