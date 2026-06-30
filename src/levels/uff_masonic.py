"""Masonic checkerboard reference floor — Matrix Test Room at ±5000.

Visual scale/bounds debug map: grey checkerboard floor seg (500-unit cells,
no walls) plus full ±BOX_HALF tile collision. Same spawn grid as stock uff
(``configure_matrix_test_room``).

Play (--test-map always loads STAGE_TEST_UFF / bg_uff.* assets):

  python3 tools/pdmap.py build uff_masonic --deploy --deploy-as uff
  ./build/pd.arm64 --test-map --moddir mods/mod_allinone

Editor Test/Play: level ``uff_masonic``, Deploy → ``uff`` (test-map slot).

Seg: ``PDMAP_SEG_MODE=masonic`` (set via ``SEG_MODE`` below). Floor collision
still comes from tiles; seg is visual reference only.
"""

from tools.pdmap.builders import configure_matrix_test_room, floor_box_tiles

BOX_HALF = 5000.0
BOX_HEIGHT = 3000.0
SPAWN_Y = 10.0

# Procedural seg mode read by tools/pdmap.py build (checkerboard floor, no walls).
SEG_MODE = "masonic"
MASONIC_CELL = 500.0


def build():
    return configure_matrix_test_room(half=BOX_HALF, pad_y=SPAWN_Y, name="uff_masonic")


def build_tiles_json():
    return floor_box_tiles("uff_masonic", half=BOX_HALF, y=0.0, room_index=1)
