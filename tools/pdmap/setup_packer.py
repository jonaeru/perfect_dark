import struct
import os

from .core import MapDef, BUILD_DIR, SETUP_DIR, ROOT


def write_setup_binary(mapdef: MapDef, name: str) -> str:
    data = mapdef.pack_setup()

    sys_path = os.path.join(ROOT, "tools", "assetmgr")
    if sys_path not in __import__("sys").path:
        __import__("sys").path.insert(0, sys_path)
    import assetmgr

    zipped = assetmgr.zip(data)
    out_path = os.path.join(SETUP_DIR, f"Ump_setup{name}Z")
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "wb") as f:
        f.write(zipped)
    return out_path
