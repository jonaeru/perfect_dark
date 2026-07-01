#!/usr/bin/env python3
"""Build uff_masonic and mirror into the uff test slot for LLM Play Masonic mode.

``--test-map`` always loads STAGE_TEST_UFF → ``bg_uff.*`` in mod_allinone. Masonic
mode ships distinct tiles/pads/setup from ``src/levels/uff_masonic.py`` but must
still land in that slot. Empty seg (PDMAP_SEG_MODE=empty) is mandatory for in-box
FPS — see docs/MAP_CREATION.md §11.11–11.12 and llm-play/docs/DETERMINISTIC_MAP_CREATION.md.
"""

from __future__ import annotations

import os
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MOD_BGDATA = ROOT / "mods" / "mod_allinone" / "files" / "bgdata"
MOD_FILES = ROOT / "mods" / "mod_allinone" / "files"
CANONICAL_SEG = ROOT / "scripts" / "bg_uff.seg"
SOURCE = "uff_masonic"
TARGET = "uff"


def _validate_mod_seg(seg_path: Path) -> list[str]:
    sys.path.insert(0, str(ROOT))
    from tools.pdmap.seg import validate_seg_g_vtx

    return validate_seg_g_vtx(seg_path.read_bytes())


def _mirror_to_uff_slot() -> None:
    """Copy bg_uff_masonic.* → bg_uff.* and setup binary for --test-map."""
    pairs = [
        (f"bg_{SOURCE}.seg", f"bg_{TARGET}.seg"),
        (f"bg_{SOURCE}_tilesZ", f"bg_{TARGET}_tilesZ"),
        (f"bg_{SOURCE}_padsZ", f"bg_{TARGET}_padsZ"),
    ]
    for src_name, dst_name in pairs:
        src = MOD_BGDATA / src_name
        dst = MOD_BGDATA / dst_name
        if not src.is_file():
            raise FileNotFoundError(f"missing built asset {src}")
        shutil.copy2(src, dst)

    setup_src = MOD_FILES / f"Ump_setup{SOURCE}Z"
    setup_dst = MOD_FILES / f"Ump_setup{TARGET}Z"
    if not setup_src.is_file():
        raise FileNotFoundError(f"missing setup {setup_src}")
    shutil.copy2(setup_src, setup_dst)


def main() -> int:
    env = os.environ.copy()
    env["PDMAP_SEG_MODE"] = "empty"

    cmd = [sys.executable, str(ROOT / "tools" / "pdmap.py"), "build", SOURCE, "--deploy"]
    result = subprocess.run(cmd, cwd=ROOT, env=env, capture_output=True, text=True)
    if result.returncode != 0:
        if result.stdout:
            print(result.stdout, end="", file=sys.stderr)
        if result.stderr:
            print(result.stderr, end="", file=sys.stderr)
        return result.returncode

    try:
        _mirror_to_uff_slot()
    except OSError as exc:
        print(f"FAIL: mirror to uff test slot: {exc}", file=sys.stderr)
        return 1

    mod_seg = MOD_BGDATA / f"bg_{TARGET}.seg"
    errors = _validate_mod_seg(mod_seg)
    if errors:
        print("FAIL: mirrored bg_uff.seg still invalid:", file=sys.stderr)
        for err in errors:
            print(f"  {err}", file=sys.stderr)
        return 1

    shutil.copy2(mod_seg, CANONICAL_SEG)
    size = mod_seg.stat().st_size
    tiles_size = (MOD_BGDATA / f"bg_{TARGET}_tilesZ").stat().st_size
    print(
        f"ok: Masonic → uff test slot (seg {size} B, tiles {tiles_size} B, G_VTX=0); "
        f"synced scripts/bg_uff.seg"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
