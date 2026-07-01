# Test arena — Combat + KOTH + full 4-team CTF in a 5000×3000 box.

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
SEG_MODE = "empty"

SPAWN_Y = 10.0


def build() -> MapDef:
    g = MapDef("testarena")

    # --- Spawns (four corners) ---
    g.add_pad(index=0, x=-4000.0, y=SPAWN_Y, z=-4000.0, room=1)
    g.add_pad(index=1, x=4000.0, y=SPAWN_Y, z=-4000.0, room=1)
    g.add_pad(index=2, x=-4000.0, y=SPAWN_Y, z=4000.0, room=1)
    g.add_pad(index=3, x=4000.0, y=SPAWN_Y, z=4000.0, room=1)

    # --- Weapon + paired rifle ammo (CMP150) ---
    g.add_pad(index=4, x=0.0, y=SPAWN_Y, z=0.0, room=1)
    g.add_pad(index=5, x=1500.0, y=SPAWN_Y, z=0.0, room=1)

    # --- KOTH hill ---
    g.add_pad(index=6, x=0.0, y=SPAWN_Y, z=1500.0, room=1)

    # --- CTF Case + CaseRespawn (teams 0–3, uff reference layout) ---
    g.add_pad(index=7, x=-4200.0, y=SPAWN_Y, z=-4200.0, room=1)
    g.add_pad(index=8, x=-4200.0, y=SPAWN_Y, z=-4000.0, room=1)
    g.add_pad(index=9, x=4200.0, y=SPAWN_Y, z=-4200.0, room=1)
    g.add_pad(index=10, x=4200.0, y=SPAWN_Y, z=-4000.0, room=1)
    g.add_pad(index=11, x=-4200.0, y=SPAWN_Y, z=4200.0, room=1)
    g.add_pad(index=12, x=-4200.0, y=SPAWN_Y, z=4000.0, room=1)
    g.add_pad(index=13, x=4200.0, y=SPAWN_Y, z=4200.0, room=1)
    g.add_pad(index=14, x=4200.0, y=SPAWN_Y, z=4000.0, room=1)

    g.add_intro(Spawn(pad=0))
    g.add_intro(Spawn(pad=1))
    g.add_intro(Spawn(pad=2))
    g.add_intro(Spawn(pad=3))

    add_floor_weapons(g, [(4, W.WEAPON_CMP150)])
    add_ammo_row(g, [5], ammotype=W.AMMOTYPE_RIFLE)

    add_mp_scenarios(
        g,
        cases=[(0, 7, 8), (1, 9, 10), (2, 11, 12), (3, 13, 14)],
        hill_pads=[6],
    )

    add_loadout_intro(g)
    return g


def build_tiles_json():
    return floor_box_tiles("testarena", half=BOX_HALF, y=0.0, room_index=1)
