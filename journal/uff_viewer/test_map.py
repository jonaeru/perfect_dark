#!/usr/bin/env python3
"""One-shot build + play pipeline for maps exported from the uff viewer editor.

Reads editor JSON (file or stdin), writes ``src/levels/<name>.py``, runs
``pdmap build``, optionally deploys to a chosen mod directory, and can launch
``--test-map`` when the asset name matches the uff test slot.

Examples:
  python3 journal/uff_viewer/test_map.py map.json --play
  python3 journal/uff_viewer/test_map.py - --mod mod_dark_noon --scenario 4 --dry-run <<'EOF'
  {...}
  EOF
"""

from __future__ import annotations

import argparse
import json
import os
import platform
import shutil
import subprocess
import sys
from typing import Any

HERE = os.path.dirname(os.path.abspath(__file__))
_env_root = os.environ.get("PD_REPO_ROOT", "").strip()
ROOT = os.path.abspath(_env_root) if _env_root else os.path.dirname(os.path.dirname(HERE))
if ROOT not in sys.path:
    sys.path.insert(0, ROOT)
if HERE not in sys.path:
    sys.path.insert(0, HERE)

from json_to_level import json_to_level_py  # noqa: E402
from tools.pdmap.core import ROOT as PDMAP_ROOT  # noqa: E402
from tools.pdmap.deploy import deploy_all, build_box_seg_asset, build_seg  # noqa: E402
from tools.pdmap.core import (  # noqa: E402
    load_level_module,
    compile_tiles,
    compile_pads,
    BUILD_DIR,
)
from tools.pdmap.pads_builder import write_pads_json  # noqa: E402
from tools.pdmap.setup_packer import write_setup_binary  # noqa: E402
from tools.pdmap.tiles import copy_tiles_from_template  # noqa: E402
from tools.pdmap.validate import validate_all  # noqa: E402

assert ROOT == PDMAP_ROOT

# Mod directories discoverable from mods/*/files/bgdata
MOD_CHOICES: dict[str, str] = {
    "mod_allinone": "mods/mod_allinone",
    "mod_dark_noon": "mods/mod_dark_noon",
    "mod_gex": "mods/mod_gex",
    "mod_kakariko": "mods/mod_kakariko",
    "mod_goldfinger_64": "mods/mod_goldfinger_64",
}

# ``--test-map`` in title.c always boots STAGE_TEST_UFF (bg_uff.* assets).
TEST_MAP_SLOT = "uff"
STAGE_TEST_UFF = 0x4D

SCENARIO_CHOICES = {
    0: "Combat",
    1: "Hold the Briefcase",
    2: "Hacker Central",
    3: "Pop a Cap",
    4: "King of the Hill",
    5: "Capture the Case",
}

# MPWEAPON_* ids passed to title.c --test-map (see src/include/constants.h).
DEFAULT_LOADOUT = [0x01, 0x09, 0x10, 0x04, 0x00, 0x25]  # Falcon2, CMP150, AR34, MagSec4, None, Shield


def parse_loadout(raw: str | None) -> list[int]:
    """Parse six comma-separated MPWEAPON ids for --loadout."""
    if not raw:
        return list(DEFAULT_LOADOUT)
    parts = [p.strip() for p in raw.split(",") if p.strip()]
    if len(parts) != 6:
        raise ValueError(f"--loadout needs exactly 6 weapon ids, got {len(parts)}: {raw!r}")
    return [int(p, 0) for p in parts]


def _mod_bgdata(mod_key: str) -> str:
    if mod_key not in MOD_CHOICES:
        raise ValueError(f"Unknown mod {mod_key!r}; choose from {', '.join(MOD_CHOICES)}")
    return os.path.join(ROOT, MOD_CHOICES[mod_key], "files", "bgdata")


def _all_mod_bgdata_dirs() -> list[str]:
    """Every mod bgdata folder — box segs must stay in sync across mods."""
    return [
        os.path.join(ROOT, MOD_CHOICES[k], "files", "bgdata")
        for k in MOD_CHOICES
        if os.path.isdir(os.path.join(ROOT, MOD_CHOICES[k], "files", "bgdata"))
    ]


def _detect_pd_binary() -> str:
    machine = platform.machine().lower()
    if machine in ("arm64", "aarch64"):
        name = "pd.arm64"
    elif sys.platform == "darwin":
        name = "pd.arm64" if machine == "arm64" else "pd.x86_64"
    else:
        name = "pd.x86_64"
    path = os.path.join(ROOT, "build", name)
    if os.path.isfile(path):
        return path
    alt = os.path.join(ROOT, "build", "pd.x86_64")
    if os.path.isfile(alt):
        return alt
    return path


def _load_json(path: str | None) -> dict[str, Any]:
    if path in (None, "-"):
        return json.load(sys.stdin)
    with open(path, encoding="utf-8") as fp:
        return json.load(fp)


def _resolve_seg_script(mod, name: str) -> str | None:
    if hasattr(mod, "SEG_SCRIPT"):
        return mod.SEG_SCRIPT
    if hasattr(mod, "seg_script"):
        return mod.seg_script()
    return None


def level_is_box_arena(name: str) -> bool:
    """True when the level module declares BOX_HALF/BOX_HEIGHT (procedural box seg)."""
    mod = load_level_module(name)
    return hasattr(mod, "BOX_HALF") and hasattr(mod, "BOX_HEIGHT")


def build_level(
    name: str,
    *,
    deploy: bool,
    seg: bool,
    mod_key: str,
    skip_validate: bool,
    verbose: bool,
) -> tuple[list[str], list[str]]:
    """Build pads/tiles/setup/(seg) for ``name``; return (errors, warnings)."""
    mod_dirs = [_mod_bgdata(mod_key)]
    seg_mod_dirs = _all_mod_bgdata_dirs()

    mod = load_level_module(name)
    mapdef = mod.build()
    seg_script = _resolve_seg_script(mod, name)
    # Box-arena levels must always regenerate seg before deploy; skipping seg
    # and copying BUILD_DIR redeployed the pre-fix G_VTX(24) blob (phantom wall).
    has_box_dims = hasattr(mod, "BOX_HALF") and hasattr(mod, "BOX_HEIGHT")
    want_seg = seg or seg_script or has_box_dims
    if has_box_dims and not seg and verbose:
        print(
            "  NOTE: box arena — seg rebuild forced (ignoring --no-seg; stale G_VTX "
            "seg causes phantom collision wall)"
        )

    if want_seg:
        if seg_script:
            if verbose:
                print(f"  Building seg via {seg_script}")
            build_seg(name, seg_script, seg_mod_dirs)
        else:
            half = float(getattr(mod, "BOX_HALF", 5000.0))
            height = float(getattr(mod, "BOX_HEIGHT", 3000.0))
            if verbose:
                print(f"  Building generic box seg (half={half:.0f} height={height:.0f})")
            # Inside-box test-map cameras clip visible seg faces through the near
            # plane (dark sheet glued to the viewport). Tiles carry collision; an
            # empty seg keeps play clean. Override with PDMAP_SEG_MODE=full for
            # coloured wall previews from the shell when needed.
            os.environ.setdefault("PDMAP_SEG_MODE", "empty")
            build_box_seg_asset(name, half=half, height=height, mod_dirs=seg_mod_dirs)
    elif deploy and verbose:
        print(
            "  WARNING: seg build skipped; deploy will copy existing BUILD_DIR seg "
            "(must pass G_VTX validation)"
        )

    pads_json_path = write_pads_json(mapdef)
    if verbose:
        print(f"  Generated pads JSON: {pads_json_path}")
    compile_pads(name, pads_json_path)
    if verbose:
        print("  Compiled pads binary")

    tiles_json_path = os.path.join(ROOT, "src", "assets", "ntsc-final", "tiles", f"{name}.json")
    if hasattr(mod, "build_tiles_json"):
        tiles_data = mod.build_tiles_json()
        os.makedirs(os.path.dirname(tiles_json_path), exist_ok=True)
        with open(tiles_json_path, "w", encoding="utf-8") as fp:
            json.dump(tiles_data, fp, indent=4)
        if verbose:
            print("  Generated tiles JSON from build_tiles_json()")
    elif os.path.exists(tiles_json_path):
        if verbose:
            print(f"  Using existing tiles JSON: {tiles_json_path}")
    else:
        template = getattr(mapdef, "tiles_template", "mp14")
        tiles_json_path = copy_tiles_from_template(name, template)
        if verbose:
            print(f"  Copied tiles JSON from template ({template})")

    compile_tiles(name, tiles_json_path)
    if verbose:
        print("  Compiled tiles binary")

    setup_path = write_setup_binary(mapdef, name)
    if verbose:
        print(f"  Wrote setup binary: {setup_path}")

    if deploy:
        if verbose:
            print(f"  Deploying to {mod_key}...")
        deploy_all(name, mod_dirs)
        if verbose:
            print("  Deploy complete")

    if skip_validate:
        return [], []

    return validate_all(name, mapdef)


def write_level_module(data: dict[str, Any], name: str, *, backup: bool) -> str:
    """Write ``src/levels/<name>.py`` from editor JSON; return path."""
    level_path = os.path.join(ROOT, "src", "levels", f"{name}.py")
    data = dict(data)
    data["name"] = name
    py_src = json_to_level_py(data)

    if backup and os.path.exists(level_path):
        bak = level_path + ".bak"
        shutil.copy2(level_path, bak)
        print(f"  Backed up existing level -> {os.path.relpath(bak, ROOT)}")

    os.makedirs(os.path.dirname(level_path), exist_ok=True)
    with open(level_path, "w", encoding="utf-8") as fp:
        fp.write(py_src)
    print(f"  Wrote level module -> {os.path.relpath(level_path, ROOT)}")
    return level_path


def play_command(
    *,
    mod_key: str,
    scenario: int,
    pd_binary: str,
    use_test_map: bool,
    num_sims: int = 8,
    sim_difficulty: int = 2,
    loadout: list[int] | None = None,
    mp_options: int = 0,
) -> list[str]:
    mod_path = os.path.join(ROOT, MOD_CHOICES[mod_key])
    cmd = [pd_binary]
    if use_test_map:
        cmd.append("--test-map")
        cmd.append(f"--scenario-{scenario}")
        cmd.extend(["--num-sims", str(num_sims)])
        cmd.extend(["--sim-difficulty", str(sim_difficulty)])
        weapons = loadout if loadout is not None else DEFAULT_LOADOUT
        cmd.extend(["--loadout", ",".join(str(w) for w in weapons)])
        if mp_options:
            cmd.extend(["--mp-options", str(mp_options)])
    else:
        cmd.extend(["--boot-stage", str(STAGE_TEST_UFF)])
        cmd.append("--skip-intro")
    cmd.extend(["--moddir", mod_path])
    return cmd


def emit_last_artifacts(
    data: dict[str, Any],
    shell_script: str,
    *,
    json_path: str | None = None,
    sh_path: str | None = None,
) -> None:
    """Persist JSON + shell script beside the viewer for copy/paste workflows."""
    json_path = json_path or os.path.join(HERE, ".last_test.json")
    sh_path = sh_path or os.path.join(HERE, ".last_test.sh")
    with open(json_path, "w", encoding="utf-8") as fp:
        json.dump(data, fp, indent=2)
    with open(sh_path, "w", encoding="utf-8") as fp:
        fp.write(shell_script)
    os.chmod(sh_path, 0o755)
    print(f"  Wrote {os.path.relpath(json_path, ROOT)}")
    print(f"  Wrote {os.path.relpath(sh_path, ROOT)}")


def build_shell_script(args: argparse.Namespace, data: dict[str, Any]) -> str:
    """Self-contained script: JSON heredoc + test_map.py invocation."""
    json_blob = json.dumps(data, indent=2)
    flags = [
        f"--level {args.level}",
        f"--mod {args.mod}",
        f"--scenario {args.scenario}",
        f"--num-sims {args.num_sims}",
        f"--sim-difficulty {args.sim_difficulty}",
        f"--loadout {','.join(str(w) for w in args.loadout)}",
    ]
    if args.mp_options:
        flags.append(f"--mp-options {args.mp_options}")
    if args.seg:
        flags.append("--seg")
    if args.deploy:
        flags.append("--deploy")
    if args.skip_validate:
        flags.append("--skip-validate")
    if args.rebuild_game:
        flags.append("--rebuild-game")
    if args.play:
        flags.append("--play")
    if args.verbose:
        flags.append("--verbose")
    if args.backup:
        flags.append("--backup")
    if args.deploy_as and args.deploy_as != args.level:
        flags.append(f"--deploy-as {args.deploy_as}")

    flag_line = " \\\n  ".join(flags)
    return (
        "#!/bin/bash\n"
        "# Generated by journal/uff_viewer — run from repo root.\n"
        "set -euo pipefail\n"
        f'cd "{ROOT}"\n'
        "python3 journal/uff_viewer/test_map.py - \\\n"
        f"  {flag_line} <<'__PDMAP_EDITOR_JSON__'\n"
        f"{json_blob}\n"
        "__PDMAP_EDITOR_JSON__\n"
    )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(
        description="Build and optionally play a map from uff viewer editor JSON.",
    )
    parser.add_argument("json_file", nargs="?", default="-", help="Editor JSON file (default: stdin)")
    parser.add_argument("--level", help="Level module / asset name (default: JSON name field)")
    parser.add_argument(
        "--deploy-as",
        help="Asset name for pdmap build/deploy (default: --level). Use 'uff' for --test-map.",
    )
    parser.add_argument(
        "--mod",
        choices=sorted(MOD_CHOICES),
        default="mod_allinone",
        help="Mod directory to deploy bgdata into (default: mod_allinone)",
    )
    parser.add_argument(
        "--scenario",
        type=int,
        choices=sorted(SCENARIO_CHOICES),
        default=0,
        help="MP scenario for --test-map (default: 0 Combat)",
    )
    parser.add_argument(
        "--num-sims",
        type=int,
        default=8,
        help="Simulant count for --test-map quick-team (default: 8, stock cap 4)",
    )
    parser.add_argument(
        "--sim-difficulty",
        type=int,
        default=2,
        choices=range(0, 6),
        help="Bot difficulty 0=Meat … 5=Dark (default: 2 Normal)",
    )
    parser.add_argument(
        "--loadout",
        type=parse_loadout,
        default=parse_loadout(None),
        help="Six MPWEAPON ids comma-separated for match loadout",
    )
    parser.add_argument(
        "--mp-options",
        type=lambda x: int(x, 0),
        default=0,
        help="MP options bitmask for --test-map (decimal or 0x hex)",
    )
    parser.add_argument("--seg", action="store_true", default=True, help="Build box seg (default: on)")
    parser.add_argument("--no-seg", dest="seg", action="store_false", help="Skip seg build")
    parser.add_argument("--deploy", action="store_true", default=True, help="Deploy to mod (default: on)")
    parser.add_argument("--no-deploy", dest="deploy", action="store_false", help="Skip mod deploy")
    parser.add_argument("--skip-validate", action="store_true", help="Skip pdmap validate after build")
    parser.add_argument("--rebuild-game", action="store_true", help="Run cmake --build build --target pd")
    parser.add_argument("--play", action="store_true", help="Launch pd binary after build")
    parser.add_argument("--dry-run", action="store_true", help="Print planned commands only")
    parser.add_argument("--verbose", "-v", action="store_true", help="Verbose build logging")
    parser.add_argument(
        "--backup",
        action="store_true",
        default=True,
        help="Backup existing src/levels/<name>.py before overwrite (default: on)",
    )
    parser.add_argument("--no-backup", dest="backup", action="store_false")
    parser.add_argument("--binary", help="Path to pd binary (default: auto-detect pd.arm64 / pd.x86_64)")
    parser.add_argument(
        "--write-artifacts",
        action="store_true",
        help="Also write journal/uff_viewer/.last_test.json and .last_test.sh",
    )
    args = parser.parse_args(argv)

    data = _load_json(args.json_file if args.json_file != "-" else None)
    level_name = (args.level or data.get("name") or "map").strip().lower()
    deploy_name = (args.deploy_as or level_name).strip().lower()
    data["name"] = level_name

    pd_binary = args.binary or _detect_pd_binary()
    use_test_map = deploy_name == TEST_MAP_SLOT

    print(f"Level module : {level_name}")
    print(f"Deploy/build : {deploy_name}")
    print(f"Mod target   : {args.mod} ({MOD_CHOICES[args.mod]})")
    print(f"Scenario     : {args.scenario} ({SCENARIO_CHOICES[args.scenario]})")
    print(f"Simulants    : {args.num_sims} · difficulty {args.sim_difficulty}")
    print(f"Loadout      : {','.join(str(w) for w in args.loadout)}")
    if args.mp_options:
        print(f"MP options   : {args.mp_options} (0x{args.mp_options:x})")
    if args.play and not use_test_map:
        print(
            f"WARNING: --test-map loads the '{TEST_MAP_SLOT}' asset slot (STAGE_TEST_UFF). "
            f"Deploy name is '{deploy_name}' — game may not load your map unless stage is registered.",
            file=sys.stderr,
        )

    shell_script = build_shell_script(
        argparse.Namespace(**{**vars(args), "level": level_name, "deploy_as": deploy_name}),
        data,
    )

    if args.dry_run:
        print("\n--- dry run ---")
        print(shell_script)
        play = play_command(
            mod_key=args.mod,
            scenario=args.scenario,
            pd_binary=pd_binary,
            use_test_map=use_test_map and args.play,
            num_sims=args.num_sims,
            sim_difficulty=args.sim_difficulty,
            loadout=args.loadout,
            mp_options=args.mp_options,
        )
        print("\n# play command:")
        print(" ".join(play))
        return 0

    if args.write_artifacts:
        emit_last_artifacts(data, shell_script)

    # Write level module (editor name) but build/deploy under deploy_name when different.
    build_data = dict(data)
    build_data["name"] = deploy_name
    write_level_module(build_data, deploy_name, backup=args.backup)
    if level_name != deploy_name:
        write_level_module(data, level_name, backup=args.backup)

    print(f"\nBuilding {deploy_name}...")
    errors, warnings = build_level(
        deploy_name,
        deploy=args.deploy,
        seg=args.seg,
        mod_key=args.mod,
        skip_validate=args.skip_validate,
        verbose=True,
    )
    for w in warnings:
        print(f"  [WARN] {w}")
    for e in errors:
        print(f"  [ERROR] {e}", file=sys.stderr)
    if errors:
        print(f"Build finished with {len(errors)} validation error(s)", file=sys.stderr)
        return 1

    if args.rebuild_game:
        print("\nRebuilding game binary...")
        subprocess.run(
            ["cmake", "--build", "build", "--target", "pd", "-j4"],
            cwd=ROOT,
            check=True,
        )

    if args.play:
        if not os.path.isfile(pd_binary):
            print(f"ERROR: pd binary not found at {pd_binary}", file=sys.stderr)
            print("Build the game first: cmake --build build --target pd", file=sys.stderr)
            return 1
        cmd = play_command(
            mod_key=args.mod,
            scenario=args.scenario,
            pd_binary=pd_binary,
            use_test_map=use_test_map,
            num_sims=args.num_sims,
            sim_difficulty=args.sim_difficulty,
            loadout=args.loadout,
            mp_options=args.mp_options,
        )
        print("\nLaunching:", " ".join(cmd))
        subprocess.run(cmd, cwd=ROOT, check=False)

    print("\nDone.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
