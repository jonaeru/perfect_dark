import json
import os

from .core import ROOT, ROMID


_DEFAULT_FLAGS = {
    "flag0001": True, "flag0002": True, "flag0004": False,
    "flag0008": True, "flag0010": True, "flag0020": False,
    "ladder": False, "flag0080": False, "flag0100": False,
    "underwater": False, "flag0400": False, "aibotcrouch": False,
    "aibotduck": False, "flag2000": False, "die": False,
    "climbableledge": False,
}


def copy_tiles_from_template(name: str, template_name: str = "mp14") -> str:
    in_path = os.path.join(ROOT, "src", "assets", ROMID, "tiles", f"{template_name}.json")
    out_path = os.path.join(ROOT, "src", "assets", ROMID, "tiles", f"{name}.json")

    with open(in_path) as f:
        data = json.load(f)

    rooms = {}
    for key, tiles in data["rooms"].items():
        rooms[key.replace(template_name.upper(), name.upper())] = tiles
    data["rooms"] = rooms

    with open(out_path, "w") as f:
        json.dump(data, f, indent=4)

    return out_path


def gen_room_tiles(name: str, tiles_by_room: dict) -> str:
    rooms = {}
    for room_key, tile_list in tiles_by_room.items():
        rooms[room_key] = tile_list

    data = {"rooms": rooms}

    out_path = os.path.join(ROOT, "src", "assets", ROMID, "tiles", f"{name}.json")
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w") as f:
        json.dump(data, f, indent=4)
    return out_path
