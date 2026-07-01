#!/usr/bin/env python3
"""Ensure Matrix Test Room (uff) uses a collision-only seg before LLM Play.

In-box FPS with wall quads in bg_uff.seg causes a viewport-attached phantom sheet
that moves with the camera and blocks movement. Play must ship PDMAP_SEG_MODE=empty
(floor collision from tiles only). See docs/MAP_CREATION.md §11.11–11.12.
"""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MOD_SEG = ROOT / "mods" / "mod_allinone" / "files" / "bgdata" / "bg_uff.seg"
CANONICAL_SEG = ROOT / "scripts" / "bg_uff.seg"


def _validate_mod_seg() -> list[str]:
    sys.path.insert(0, str(ROOT))
    from tools.pdmap.seg import validate_seg_g_vtx

    data = MOD_SEG.read_bytes()
    return validate_seg_g_vtx(data)


def main() -> int:
    env = os.environ.copy()
    env["PDMAP_SEG_MODE"] = "empty"

    cmd = [sys.executable, str(ROOT / "tools" / "pdmap.py"), "build", "uff", "--deploy"]
    result = subprocess.run(cmd, cwd=ROOT, env=env, capture_output=True, text=True)
    if result.returncode != 0:
        if result.stdout:
            print(result.stdout, end="", file=sys.stderr)
        if result.stderr:
            print(result.stderr, end="", file=sys.stderr)
        return result.returncode

    if not MOD_SEG.is_file():
        print(f"FAIL: missing {MOD_SEG}", file=sys.stderr)
        return 1

    errors = _validate_mod_seg()
    if errors:
        print("FAIL: mod_allinone bg_uff.seg still invalid:", file=sys.stderr)
        for err in errors:
            print(f"  {err}", file=sys.stderr)
        return 1

    # Keep scripts/bg_uff.seg in sync so Node fallback copy paths stay empty-mode too.
    shutil.copy2(MOD_SEG, CANONICAL_SEG)

    size = MOD_SEG.stat().st_size
    print(
        f"ok: mod_allinone bg_uff.seg ({size} bytes, tiles-only, G_VTX=0); "
        f"synced scripts/bg_uff.seg"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
