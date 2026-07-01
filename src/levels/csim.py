from tools.pdmap.builders import (
    add_spawn_grid,
    add_loadout_intro,
    add_mp_scenarios,
    add_floor_weapons,
    add_ammo_row,
    floor_box_tiles,
)
from tools.pdmap.core import MapDef
from tools.pdmap import weapons as W

# Arena dimensions — single source of truth shared by the floor tiles and the
# box seg geometry, so the visible walls always match the collision floor and
# contain every spawn pad. `pdmap build csim --seg` reads these to generate
# bg_csim.seg automatically (no bespoke SEG_SCRIPT required).
BOX_HALF = 5000.0
BOX_HEIGHT = 3000.0

# Spawn pads must sit slightly ABOVE the floor (Y>0). The engine ground search
# rejects a floor whose Y is not strictly below the pad, so a pad exactly on the
# floor (Y=0) makes the player fall through. The player is then dropped to the
# floor height, so this small offset does not cause fall damage.
SPAWN_Y = 10.0


def build() -> MapDef:
    g = MapDef("csim")

    # IMPORTANT: pad indices must be CONTIGUOUS from 0. The asset compiler
    # (mkpads) numbers pads by their ARRAY POSITION, and every prop / intro
    # command references a pad by that position — not by the numeric label you
    # pass to add_pad(). So the Nth pad you add MUST use index=N, or references
    # silently point at the wrong (or a non-existent) pad. `pdmap validate`
    # enforces this.

    # Pads 0-3: four corner spawns inside the [-BOX_HALF, +BOX_HALF] floor.
    add_spawn_grid(
        g,
        [(-2000.0, -2000.0), (2000.0, -2000.0),
         (-2000.0, 2000.0), (2000.0, 2000.0)],
        y=SPAWN_Y,
    )

    # Pads 4-6: weapon + ammo PICKUPS on the floor (props). These are
    # independent of the simulant pipeline and render as soon as their room is
    # onscreen.
    g.add_pad(index=4, x=0.0, y=SPAWN_Y, z=0.0, room=1)
    g.add_pad(index=5, x=-1500.0, y=SPAWN_Y, z=0.0, room=1)
    g.add_pad(index=6, x=1500.0, y=SPAWN_Y, z=0.0, room=1)
    add_ammo_row(g, [4])
    add_floor_weapons(g, [(5, W.WEAPON_CMP150), (6, W.WEAPON_MAGSEC4)])

    # Pads 7-9: scenario anchors (Capture-the-Case + King-of-the-Hill). Every
    # referenced pad must exist, so create them before wiring the intro commands.
    g.add_pad(index=7, x=-3000.0, y=SPAWN_Y, z=-3000.0, room=1)
    g.add_pad(index=8, x=3000.0, y=SPAWN_Y, z=3000.0, room=1)
    g.add_pad(index=9, x=0.0, y=SPAWN_Y, z=1500.0, room=1)
    add_mp_scenarios(g, cases=[(0, 7, 8)], hill_pads=[9])

    add_loadout_intro(g)

    return g


def build_tiles_json():
    return floor_box_tiles("csim", half=BOX_HALF, y=0.0, room_index=1)
