from tools.pdmap.builders import configure_matrix_test_room, floor_box_tiles

# Arena dimensions — single source of truth shared by the floor tiles, the
# spawn layout, and the box seg geometry. Keeping these in one place means the
# visible walls always match the collision floor and contain every spawn pad.
BOX_HALF = 5000.0
BOX_HEIGHT = 3000.0

# Default seg draw mode for box rebuilds (pdmap reads this before calling
# tools.pdmap.seg.write_box_seg). "empty" = setup GDL only, no face geometry —
# required for --test-map in-box camera (Y≈10 inside ±5000 box). Wall/ceiling
# faces (walls/full/debug) clip through the near plane → viewport sheet
# (MAP_CREATION.md §11.12). Floor collision comes from tiles, not the seg.
# Editor wall preview only: PDMAP_SEG_MODE=walls|debug|rainbow (will clip in-box).
SEG_MODE = "empty"


def build():
    return configure_matrix_test_room(half=BOX_HALF)


def build_tiles_json():
    return floor_box_tiles("uff", half=BOX_HALF, y=0.0, room_index=1)
