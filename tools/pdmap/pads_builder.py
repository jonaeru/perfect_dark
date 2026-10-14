import json
import os

from .core import MapDef, ROOT, ROMID


def mapdef_to_pads_json(mapdef: MapDef) -> dict:
    return mapdef.pack_pads_json()


def write_pads_json(mapdef: MapDef):
    name = mapdef.name
    data = mapdef.pack_pads_json()
    out_path = os.path.join(ROOT, "src", "assets", ROMID, "pads", f"{name}.json")
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "w") as f:
        json.dump(data, f, indent="\t")
    return out_path
