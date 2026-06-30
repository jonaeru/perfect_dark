import os
import json
from typing import Optional

from .core import MapDef, load_level_module, ROOT, ROMID, BUILD_DIR
from .seg import validate_seg_g_vtx, validate_seg_phantom_viewport
from .intro import Spawn, Case, CaseRespawn, Hill


def validate_all(
    name: str,
    mapdef: Optional[MapDef] = None,
    *,
    level_module: str | None = None,
) -> tuple[list[str], list[str]]:
    errors: list[str] = []
    warnings: list[str] = []

    if mapdef is None:
        try:
            mod = load_level_module(name)
            mapdef = mod.build()
        except Exception as e:
            errors.append(f"Cannot load level module: {e}")
            return errors, warnings

    pad_indices = {p.index for p in mapdef.pads}

    # Pad references are POSITIONAL. mkpads numbers pads by their array order
    # (enumerate), and every prop / intro command resolves a pad by that
    # position, not by the numeric label passed to add_pad(). So the pad added
    # at position k MUST have index == k (contiguous from 0), or references
    # point at the wrong/non-existent pad. This was a silent footgun: spawns at
    # low indices happened to work while higher sparse indices (weapons, cases)
    # pointed past the pad array and never spawned.
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
        # Pads must sit slightly ABOVE the floor. The engine's ground search
        # (cdFindGroundFromList) only accepts a floor whose Y is strictly below
        # the pad (`ground < pos.y`); a pad exactly on the floor (Y==floorY)
        # makes the player fall through. The player is then placed at the floor
        # height, so a small offset does not cause fall damage.
        if p.y <= 0.0:
            errors.append(
                f"Pad {p.index}: Y={p.y} is at/below the floor; the ground "
                f"search will reject the floor and the player will fall through "
                f"(use a small positive Y such as 10)"
            )
        elif p.y > 200.0:
            warnings.append(
                f"Pad {p.index}: Y={p.y} is far above the floor; spawns may take fall damage"
            )

    intro_codes = {getattr(cmd, "code", None) for cmd in mapdef.intro}
    has_spawn = any(isinstance(cmd, Spawn) for cmd in mapdef.intro)
    if not has_spawn:
        errors.append("Intro has no Spawn commands — Combat Simulator cannot place players")

    if has_spawn and not any(isinstance(cmd, Case) for cmd in mapdef.intro):
        warnings.append("Intro has no Case commands — Capture the Case scenario will not work")
    if has_spawn and not any(isinstance(cmd, Hill) for cmd in mapdef.intro):
        warnings.append("Intro has no Hill commands — King of the Hill scenario will not work")

    for i, cmd in enumerate(mapdef.intro):
        from .intro import INTRO_CODE_MAP
        code = getattr(cmd, "code", None)
        if code is not None and code not in INTRO_CODE_MAP and code != 0xFF:
            errors.append(f"Intro[{i}]: unknown command code {code:#x}")
        if isinstance(cmd, Spawn) and cmd.pad not in pad_indices:
            errors.append(f"Intro Spawn references missing pad {cmd.pad}")
        # Case / CaseRespawn / Hill anchors must also point at real pads, or the
        # scenario object is placed at a bogus pad (silent before; now caught).
        if isinstance(cmd, (Case, CaseRespawn, Hill)) and cmd.pad not in pad_indices:
            errors.append(
                f"Intro {type(cmd).__name__} references missing pad {cmd.pad}"
            )

    for i, prop in enumerate(mapdef.props):
        try:
            data = prop.pack()
            if len(data) == 0:
                errors.append(f"Prop[{i}] packed to zero bytes")
        except Exception as e:
            errors.append(f"Prop[{i}] pack failed: {e}")

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

        # Empty-seg play mode has no floor collision from bg_*.seg — only tiles.
        # floor_box_tiles() must span [-BOX_HALF, +BOX_HALF] or the arena shrinks
        # to a tiny patch and players fall off immediately (see MAP_CREATION §11.13).
        try:
            mod = load_level_module(level_module or name)
            expected_half = getattr(mod, "BOX_HALF", None)
            seg_mode = getattr(mod, "SEG_MODE", None)
        except Exception:
            expected_half = None
            seg_mode = None
        if expected_half is not None:
            floor_tiles: list[dict] = []
            for room_tiles in tiles_data.get("rooms", {}).values():
                for tile in room_tiles:
                    verts = tile.get("vertices") or []
                    if len(verts) >= 4:
                        floor_tiles.append(tile)
            if not floor_tiles:
                errors.append(
                    "No floor tiles in tiles JSON; PDMAP_SEG_MODE=empty requires "
                    "tile collision for the full arena"
                )
            else:
                xs: list[float] = []
                zs: list[float] = []
                for tile in floor_tiles:
                    for v in tile["vertices"]:
                        xs.append(float(v["x"]))
                        zs.append(float(v["z"]))
                min_x, max_x = min(xs), max(xs)
                min_z, max_z = min(zs), max(zs)
                half = float(expected_half)
                tol = 1.0
                if (
                    min_x > -half + tol
                    or max_x < half - tol
                    or min_z > -half + tol
                    or max_z < half - tol
                ):
                    errors.append(
                        f"Floor tile bounds x=[{min_x:.0f},{max_x:.0f}] "
                        f"z=[{min_z:.0f},{max_z:.0f}] do not cover "
                        f"BOX_HALF={half:.0f} (±{half:.0f}); empty seg has no "
                        f"floor collision outside tiles"
                    )

    seg_path = os.path.join(BUILD_DIR, f"bg_{name}.seg")
    if not os.path.exists(seg_path):
        warnings.append(f"Seg not built at {seg_path} — run build with --seg if needed")
    else:
        with open(seg_path, "rb") as seg_fp:
            seg_bytes = seg_fp.read()
            for msg in validate_seg_g_vtx(seg_bytes):
                errors.append(f"Seg G_VTX: {msg}")
            # Masonic/checkerboard floor draws many G_VTX(4) loads by design.
            if seg_mode not in ("masonic",):
                for msg in validate_seg_phantom_viewport(seg_bytes):
                    errors.append(f"Seg viewport: {msg}")

    setup_path = os.path.join(ROOT, "build", ROMID, f"Ump_setup{name}Z")
    if not os.path.exists(setup_path):
        warnings.append(f"Setup binary missing at {setup_path}")

    return errors, warnings
