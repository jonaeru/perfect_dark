import os
import struct

from .core import BUILD_DIR, load_level_module, MapDef, ROOT, ROMID


def get_file_size(path: str) -> str:
    if not os.path.exists(path):
        return "N/A"
    size = os.path.getsize(path)
    if size < 1024:
        return f"{size} B"
    elif size < 1024 * 1024:
        return f"{size / 1024:.1f} KB"
    else:
        return f"{size / (1024 * 1024):.1f} MB"


def info(name: str, mapdef: MapDef | None = None):
    print(f"=== Level: {name} ===")
    print()

    if mapdef is None:
        try:
            mod = load_level_module(name)
            mapdef = mod.build()
        except Exception as e:
            print(f"  Cannot load level module: {e}")
    else:
        print(f"  Pads:      {len(mapdef.pads)}")
        print(f"  Covers:    {len(mapdef.covers)}")
        print(f"  Props:     {len(mapdef.props)}")
        print(f"  Intro cmds:{len(mapdef.intro)}")
        print()

    built_assets = [
        ("TilesZ", os.path.join(BUILD_DIR, f"bg_{name}_tilesZ")),
        ("PadsZ", os.path.join(BUILD_DIR, f"bg_{name}_padsZ")),
        ("Seg", os.path.join(BUILD_DIR, f"bg_{name}.seg")),
        ("SetupZ", os.path.join(ROOT, "build", ROMID, f"Ump_setup{name}Z")),
    ]
    print("  Built assets:")
    for label, path in built_assets:
        print(f"    {label:8s}: {get_file_size(path):>8s}  {path}")

    print()
    print("  Source files:")
    tiles_json = os.path.join(ROOT, "src", "assets", ROMID, "tiles", f"{name}.json")
    pads_json = os.path.join(ROOT, "src", "assets", ROMID, "pads", f"{name}.json")
    print(f"    Tiles JSON: {get_file_size(tiles_json):>8s}  {tiles_json}")
    print(f"    Pads JSON:  {get_file_size(pads_json):>8s}  {pads_json}")
