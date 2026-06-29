#!/usr/bin/env python3
"""build_custom_seg.py -- generate the Matrix Test Room box seg (bg_uff.seg).

Thin wrapper around ``tools.pdmap.seg.build_box_seg``. The box dimensions are
read from the level module (src/levels/uff.py) so the seg geometry, floor
tiles, and spawn layout all share one source of truth.

Output: scripts/bg_uff.seg
Run:    python3 scripts/build_custom_seg.py
"""

import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)

from tools.pdmap.core import load_level_module
from tools.pdmap.seg import write_box_seg

NAME = "uff"
OUT = os.path.join(ROOT, "scripts", f"bg_{NAME}.seg")


def main():
    level = load_level_module(NAME)
    half = getattr(level, "BOX_HALF", 5000.0)
    height = getattr(level, "BOX_HEIGHT", 3000.0)

    write_box_seg(OUT, half=half, height=height)

    size = os.path.getsize(OUT)
    print(f"wrote {OUT} ({size} bytes; box half={half:.0f} height={height:.0f})")


if __name__ == "__main__":
    main()
