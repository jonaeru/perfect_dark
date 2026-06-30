"""High-level helpers for common Combat Simulator map layouts."""

from __future__ import annotations

from .core import MapDef
from .intro import Spawn, Weapon, Ammo, Case, CaseRespawn, Hill
from .props import Weapon as WeaponProp, AmmoCrate
from .tiles import _DEFAULT_FLAGS
from . import weapons as W


def add_spawn_grid(
    g: MapDef,
    positions: list[tuple[float, float]],
    *,
    y: float = 0.0,
    room: int = 1,
    start_index: int = 0,
) -> list[int]:
    """Place spawn pads and matching intro Spawn commands. Returns pad indices."""
    indices: list[int] = []
    for offset, (x, z) in enumerate(positions):
        idx = start_index + offset
        g.add_pad(index=idx, x=x, y=y, z=z, room=room)
        g.add_intro(Spawn(pad=idx))
        indices.append(idx)
    return indices


def add_mp_scenarios(
    g: MapDef,
    *,
    cases: list[tuple[int, int, int]],
    hill_pads: list[int],
) -> None:
    """Register Capture-the-Case and King-of-the-Hill intro anchors.

    Each case entry is (team, case_pad, respawn_pad).
    """
    for team, case_pad, respawn_pad in cases:
        g.add_intro(Case(team=team, pad=case_pad))
        g.add_intro(CaseRespawn(team=team, pad=respawn_pad))
    for pad in hill_pads:
        g.add_intro(Hill(pad=pad))


def add_loadout_intro(
    g: MapDef,
    *,
    weapons: tuple[int, ...] = W.INTRO_WEAPONS,
    ammo: tuple[int, ...] = W.INTRO_AMMO,
    ammo_qty: int = 100,
) -> None:
    """Standard Combat Simulator starting weapons and ammo."""
    for weapon_id in weapons:
        g.add_intro(Weapon(weapon_id=weapon_id, dualweapon=-1))
    for ammotype in ammo:
        g.add_intro(Ammo(ammotype=ammotype, quantity=ammo_qty))


def add_floor_weapons(
    g: MapDef,
    pad_weapon_pairs: list[tuple[int, int]],
    *,
    scale: int = 0x0100,
) -> None:
    for pad, weapon_id in pad_weapon_pairs:
        g.add_prop(WeaponProp(
            weapon=weapon_id,
            scale=scale,
            model=0,
            chr_=pad,
            flags=W.OBJFLAG_FALL,
        ))


def add_ammo_row(
    g: MapDef,
    pads: list[int],
    *,
    ammotype: int = W.AMMOTYPE_PISTOL,
    scale: int = 0x0019,
    model: int = W.MODEL_MULTI_AMMO_CRATE,
) -> None:
    for pad in pads:
        g.add_prop(AmmoCrate(
            ammotype=ammotype,
            scale=scale,
            model=model,
            pad=pad,
            flags=W.OBJFLAG_FALL,
            flags2=W.OBJFLAG2_IMMUNETOANTI,
            maxdamage=1000,
        ))


def floor_box_tiles(
    name: str,
    *,
    half: float = 5000.0,
    y: float = 0.0,
    room_index: int = 1,
) -> dict:
    """Single-room floor collision matching a centred box arena."""
    room_key = f"ROOM_{name.upper()}_{room_index:04d}"
    return {
        "rooms": {
            f"ROOM_{name.upper()}_0000": [],
            room_key: [floor_tile(half=half, y=y)],
        }
    }


def floor_tile(*, half: float = 5000.0, y: float = 0.0) -> dict:
    h = int(half)
    yi = int(y)
    return {
        **_DEFAULT_FLAGS,
        "floortype": "default",
        "floorcolour": 4095,
        "vertices": [
            {"x": -h, "y": yi, "z": -h},
            {"x": -h, "y": yi, "z": h},
            {"x": h, "y": yi, "z": h},
            {"x": h, "y": yi, "z": -h},
        ],
    }


# Pads must sit slightly ABOVE the floor. The engine's ground search
# (cdFindGroundFromList) only accepts a floor whose Y is strictly less than the
# search cylinder's Y (`ground < pos.y`). If a spawn pad is exactly on the floor
# (Y == floor Y) the floor is rejected and the player falls into the void. The
# player is then placed exactly at the floor height, so this small offset does
# NOT cause fall damage. See docs/MAP_CREATION.md §11.1.
PAD_FLOOR_OFFSET = 10.0


def configure_matrix_test_room(
    half: float = 5000.0,
    pad_y: float = PAD_FLOOR_OFFSET,
    name: str = "uff",
) -> MapDef:
    """Matrix Test Room (uff) — a fully playable Combat Simulator box arena.

    All pads sit inside the [-half, +half] floor box (so players spawn on the
    visible, walled floor rather than in the void) and at ``pad_y`` just above
    the floor (so the ground search accepts the floor). The layout assumes
    ``half >= 4200`` (the outermost case pads sit at +/-4200).
    """
    g = MapDef(name)

    spawn_positions = [
        (-4000, -4000), (4000, -4000), (-4000, 4000), (4000, 4000),
        (-2000, -2000), (2000, -2000), (-2000, 2000), (2000, 2000),
        (0, -3000), (0, 3000), (-3000, 0), (3000, 0),
        (-1000, -1000), (1000, -1000), (-1000, 1000), (1000, 1000),
    ]
    add_spawn_grid(g, spawn_positions, y=pad_y)

    weapon_layout = [
        (16, W.WEAPON_FALCON2), (17, W.WEAPON_MAGSEC4), (18, W.WEAPON_MAULER),
        (19, W.WEAPON_CMP150), (20, W.WEAPON_SUPERDRAGON), (21, W.WEAPON_LAPTOPGUN),
        (22, W.WEAPON_ROCKETLAUNCHER), (23, W.WEAPON_SNIPERRIFLE),
        (24, W.WEAPON_CROSSBOW), (25, W.WEAPON_TRANQUILIZER),
    ]
    for pad, _ in weapon_layout:
        x, z = _pad_xy_for_index(pad)
        g.add_pad(index=pad, x=x, y=pad_y, z=z, room=1)
    add_floor_weapons(g, weapon_layout)

    cases = [(0, 26, 27), (1, 28, 29), (2, 30, 31), (3, 32, 33)]
    for _, case_pad, respawn_pad in cases:
        for pad in (case_pad, respawn_pad):
            x, z = _pad_xy_for_index(pad)
            g.add_pad(index=pad, x=x, y=pad_y, z=z, room=1)

    hill_pads = [34, 35, 36, 37]
    for pad in hill_pads:
        x, z = _pad_xy_for_index(pad)
        g.add_pad(index=pad, x=x, y=pad_y, z=z, room=1)

    add_mp_scenarios(g, cases=cases, hill_pads=hill_pads)

    ammo_pads = list(range(38, 58))
    for pad in ammo_pads:
        x, z = _pad_xy_for_index(pad)
        g.add_pad(index=pad, x=x, y=pad_y, z=z, room=1)
    add_ammo_row(g, ammo_pads)

    add_loadout_intro(g)
    return g


def _pad_xy_for_index(pad: int) -> tuple[float, float]:
    """Pad positions from mods/uff_pads.json (scaled game units / 6)."""
    table: dict[int, tuple[float, float]] = {
        16: (0, 0), 17: (0, -1500), 18: (0, 1500), 19: (-1500, 0), 20: (1500, 0),
        21: (-3500, -3500), 22: (3500, -3500), 23: (-3500, 3500), 24: (3500, 3500),
        25: (-4500, 0),
        26: (-4200, -4200), 27: (-4200, -4000),
        28: (4200, -4200), 29: (4200, -4000),
        30: (-4200, 4200), 31: (-4200, 4000),
        32: (4200, 4200), 33: (4200, 4000),
        34: (0, 500), 35: (0, -500), 36: (-500, 0), 37: (500, 0),
    }
    if pad in table:
        return table[pad]
    if 38 <= pad <= 57:
        row = (pad - 38) // 5
        col = (pad - 38) % 5
        return (-3000 + col * 1500, -3000 + row * 1500)
    return (0.0, 0.0)
