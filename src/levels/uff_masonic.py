# Masonic test arena — LLM Play agent sessions at Matrix ±5000.
#
# Compact full-scenario layout (Combat + KOTH + 4-team CTF) distinct from the
# dense uff reference room (configure_matrix_test_room / Quick test). Deployed
# into the uff test slot via scripts/ensure-llm-masonic-map-assets.py because
# --test-map always loads STAGE_TEST_UFF / bg_uff.*.

from tools.pdmap.builders import (
    add_loadout_intro,
    add_floor_weapons,
    add_ammo_row,
    add_mp_scenarios,
    floor_box_tiles,
)
from tools.pdmap.core import MapDef
from tools.pdmap.intro import Spawn
from tools.pdmap import weapons as W

BOX_HALF = 5000.0
BOX_HEIGHT = 3000.0
# Tiles-only seg — in-box FPS has no phantom wall / near-plane sheet (MAP_CREATION §11.11–11.12).
SEG_MODE = "empty"
SPAWN_Y = 10.0


def build() -> MapDef:
    g = MapDef("uff_masonic")

    # Four corner spawns (room 1, inside ±5000 floor).
    g.add_pad(index=0, x=-4000.0, y=SPAWN_Y, z=-4000.0, room=1)
    g.add_pad(index=1, x=4000.0, y=SPAWN_Y, z=-4000.0, room=1)
    g.add_pad(index=2, x=-4000.0, y=SPAWN_Y, z=4000.0, room=1)
    g.add_pad(index=3, x=4000.0, y=SPAWN_Y, z=4000.0, room=1)

    # Weapon pickups + ammo at cardinal offsets.
    g.add_pad(index=4, x=0.0, y=SPAWN_Y, z=-1500.0, room=1)
    g.add_pad(index=5, x=0.0, y=SPAWN_Y, z=1500.0, room=1)
    g.add_pad(index=6, x=-1500.0, y=SPAWN_Y, z=0.0, room=1)
    g.add_pad(index=7, x=1500.0, y=SPAWN_Y, z=0.0, room=1)

    # CTF case + respawn per team (uff reference corners).
    g.add_pad(index=8, x=-4200.0, y=SPAWN_Y, z=-4200.0, room=1)
    g.add_pad(index=9, x=-4200.0, y=SPAWN_Y, z=-4000.0, room=1)
    g.add_pad(index=10, x=4200.0, y=SPAWN_Y, z=-4200.0, room=1)
    g.add_pad(index=11, x=4200.0, y=SPAWN_Y, z=-4000.0, room=1)
    g.add_pad(index=12, x=-4200.0, y=SPAWN_Y, z=4200.0, room=1)
    g.add_pad(index=13, x=-4200.0, y=SPAWN_Y, z=4000.0, room=1)
    g.add_pad(index=14, x=4200.0, y=SPAWN_Y, z=4200.0, room=1)
    g.add_pad(index=15, x=4200.0, y=SPAWN_Y, z=4000.0, room=1)

    # KOTH hill anchor (centre-east).
    g.add_pad(index=16, x=500.0, y=SPAWN_Y, z=0.0, room=1)

    g.add_intro(Spawn(pad=0))
    g.add_intro(Spawn(pad=1))
    g.add_intro(Spawn(pad=2))
    g.add_intro(Spawn(pad=3))

    add_floor_weapons(g, [(4, W.WEAPON_AR34), (5, W.WEAPON_SHOTGUN)])
    add_ammo_row(g, [6], ammotype=W.AMMOTYPE_RIFLE)
    add_ammo_row(g, [7], ammotype=W.AMMOTYPE_SHOTGUN)

    add_mp_scenarios(
        g,
        cases=[(0, 8, 9), (1, 10, 11), (2, 12, 13), (3, 14, 15)],
        hill_pads=[16],
    )

    add_loadout_intro(g)
    return g


def build_tiles_json():
    return floor_box_tiles("uff_masonic", half=BOX_HALF, y=0.0, room_index=1)
