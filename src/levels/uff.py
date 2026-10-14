from tools.pdmap.builders import configure_matrix_test_room, floor_box_tiles

# Arena dimensions — single source of truth shared by the floor tiles, the
# spawn layout, and the box seg geometry. Keeping these in one place means the
# visible walls always match the collision floor and contain every spawn pad.
BOX_HALF = 5000.0
BOX_HEIGHT = 3000.0


def build():
    return configure_matrix_test_room(half=BOX_HALF)


def build_tiles_json():
    return floor_box_tiles("uff", half=BOX_HALF, y=0.0, room_index=1)


# Used by: pdmap build uff --seg
SEG_SCRIPT = "scripts/build_custom_seg.py"
