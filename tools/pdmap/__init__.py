import argparse
import sys
import os
import json
import subprocess

from .core import (
    MapDef, load_level_module, compile_tiles, compile_pads,
    BUILD_DIR, ROOT, ROMID
)
from .pads_builder import write_pads_json
from .setup_packer import write_setup_binary
from .tiles import copy_tiles_from_template
from .deploy import deploy_all, build_seg, build_box_seg_asset
from .validate import validate_all
from .info import info


def _resolve_seg_script(mod, name: str) -> str | None:
    if hasattr(mod, "SEG_SCRIPT"):
        return mod.SEG_SCRIPT
    if hasattr(mod, "seg_script"):
        return mod.seg_script()
    return None


def cmd_build(args):
    name = args.name
    print(f"Building level: {name}")

    try:
        mod = load_level_module(name)
        mapdef = mod.build()
    except Exception as exc:
        print(f"ERROR: failed to load level module: {exc}", file=sys.stderr)
        sys.exit(1)

    seg_script = _resolve_seg_script(mod, name)
    # --seg is present whenever args.seg is not None (it defaults to None and is
    # set to "" by a bare flag or to a path when one is given). A level may also
    # opt into seg generation via SEG_SCRIPT without the flag (e.g. uff).
    want_seg = args.seg is not None or bool(seg_script)
    if want_seg:
        try:
            if seg_script:
                # Bespoke generator script declared by the level module.
                print(f"  Building seg via {seg_script}")
                build_seg(name, seg_script)
            elif args.seg:
                # Explicit script path passed on the command line.
                print(f"  Building seg via {args.seg}")
                build_seg(name, args.seg)
            else:
                # Bare --seg with no SEG_SCRIPT: generate a generic box arena seg
                # directly. Box dimensions come from the level module (BOX_HALF /
                # BOX_HEIGHT) so they match the floor tiles; defaults otherwise.
                half = float(getattr(mod, "BOX_HALF", 5000.0))
                height = float(getattr(mod, "BOX_HEIGHT", 3000.0))
                print("  Building generic box seg (no SEG_SCRIPT)")
                build_box_seg_asset(name, half=half, height=height)
        except Exception as exc:
            print(f"ERROR: seg build failed: {exc}", file=sys.stderr)
            sys.exit(1)

    try:
        pads_json_path = write_pads_json(mapdef)
        print(f"  Generated pads JSON: {pads_json_path}")
        compile_pads(name, pads_json_path)
        print("  Compiled pads binary")

        tiles_json_path = os.path.join(ROOT, "src", "assets", ROMID, "tiles", f"{name}.json")
        if hasattr(mod, "build_tiles_json"):
            tiles_data = mod.build_tiles_json()
            os.makedirs(os.path.dirname(tiles_json_path), exist_ok=True)
            with open(tiles_json_path, "w") as f:
                json.dump(tiles_data, f, indent=4)
            print("  Generated tiles JSON from build_tiles_json()")
        elif os.path.exists(tiles_json_path):
            print(f"  Using existing tiles JSON: {tiles_json_path}")
        else:
            template = getattr(mapdef, "tiles_template", "mp14")
            tiles_json_path = copy_tiles_from_template(name, template)
            print(f"  Copied tiles JSON from template ({template}): {tiles_json_path}")

        compile_tiles(name, tiles_json_path)
        print("  Compiled tiles binary")

        setup_path = write_setup_binary(mapdef, name)
        print(f"  Wrote setup binary: {setup_path}")
    except subprocess.CalledProcessError as exc:
        print(f"ERROR: asset compiler failed (exit {exc.returncode})", file=sys.stderr)
        sys.exit(exc.returncode)
    except Exception as exc:
        print(f"ERROR: build failed: {exc}", file=sys.stderr)
        sys.exit(1)

    if args.deploy:
        print("  Deploying...")
        deploy_all(name)
        print("  Deploy complete")

    if not args.no_validate:
        errors, warnings = validate_all(name, mapdef)
        for w in warnings:
            print(f"  [WARN] {w}")
        for e in errors:
            print(f"  [ERROR] {e}")
        if errors:
            print(f"Build finished with {len(errors)} validation error(s)", file=sys.stderr)
            sys.exit(1)

    print(f"Build complete for {name}")


def cmd_info(args):
    info(args.name)


def cmd_validate(args):
    print(f"Validating level: {args.name}")
    errors, warnings = validate_all(args.name)
    for w in warnings:
        print(f"  [WARN] {w}")
    for e in errors:
        print(f"  [ERROR] {e}")
    if not errors and not warnings:
        print("  No issues found")
    elif errors:
        print(f"  {len(errors)} error(s), {len(warnings)} warning(s)")
        sys.exit(1)


def cmd_deploy(args):
    print(f"Deploying level: {args.name}")
    deploy_all(args.name)
    print("Deploy complete")


def cmd_list(args):
    levels_dir = os.path.join(ROOT, "src", "levels")
    if not os.path.isdir(levels_dir):
        print(f"Levels directory not found: {levels_dir}")
        return
    files = sorted(f for f in os.listdir(levels_dir) if f.endswith(".py") and not f.startswith("__"))
    print(f"Levels ({len(files)}):")
    for f in files:
        fname = f[:-3]
        try:
            mod = load_level_module(fname)
            if hasattr(mod, "build"):
                m = mod.build()
                suffix = f" ({len(m.pads)} pads, {len(m.props)} props)"
            else:
                suffix = " (no build())"
        except Exception:
            suffix = " (error loading)"
        print(f"  {fname}{suffix}")


def cmd_init(args):
    name = args.name
    level_path = os.path.join(ROOT, "src", "levels", f"{name}.py")
    if os.path.exists(level_path):
        print(f"Level {name} already exists at {level_path}")
        return

    os.makedirs(os.path.dirname(level_path), exist_ok=True)

    template = f'''from tools.pdmap.builders import (
    add_spawn_grid,
    add_loadout_intro,
    add_mp_scenarios,
    add_floor_weapons,
    add_ammo_row,
    floor_box_tiles,
)
from tools.pdmap.core import MapDef
from tools.pdmap import weapons as W

# Arena dimensions — single source of truth shared by the floor tiles and the
# box seg geometry, so the visible walls always match the collision floor and
# contain every spawn pad. `pdmap build {name} --seg` reads these to generate
# bg_{name}.seg automatically (no bespoke SEG_SCRIPT required).
BOX_HALF = 5000.0
BOX_HEIGHT = 3000.0

# Spawn pads must sit slightly ABOVE the floor (Y>0). The engine ground search
# rejects a floor whose Y is not strictly below the pad, so a pad exactly on the
# floor (Y=0) makes the player fall through. The player is then dropped to the
# floor height, so this small offset does not cause fall damage.
SPAWN_Y = 10.0


def build() -> MapDef:
    g = MapDef("{name}")

    # IMPORTANT: pad indices must be CONTIGUOUS from 0. The asset compiler
    # (mkpads) numbers pads by their ARRAY POSITION, and every prop / intro
    # command references a pad by that position — not by the numeric label you
    # pass to add_pad(). So the Nth pad you add MUST use index=N, or references
    # silently point at the wrong (or a non-existent) pad. `pdmap validate`
    # enforces this.

    # Pads 0-3: four corner spawns inside the [-BOX_HALF, +BOX_HALF] floor.
    add_spawn_grid(
        g,
        [(-2000.0, -2000.0), (2000.0, -2000.0),
         (-2000.0, 2000.0), (2000.0, 2000.0)],
        y=SPAWN_Y,
    )

    # Pads 4-6: weapon + ammo PICKUPS on the floor (props). These are
    # independent of the simulant pipeline and render as soon as their room is
    # onscreen.
    g.add_pad(index=4, x=0.0, y=SPAWN_Y, z=0.0, room=1)
    g.add_pad(index=5, x=-1500.0, y=SPAWN_Y, z=0.0, room=1)
    g.add_pad(index=6, x=1500.0, y=SPAWN_Y, z=0.0, room=1)
    add_ammo_row(g, [4])
    add_floor_weapons(g, [(5, W.WEAPON_CMP150), (6, W.WEAPON_MAGSEC4)])

    # Pads 7-9: scenario anchors (Capture-the-Case + King-of-the-Hill). Every
    # referenced pad must exist, so create them before wiring the intro commands.
    g.add_pad(index=7, x=-3000.0, y=SPAWN_Y, z=-3000.0, room=1)
    g.add_pad(index=8, x=3000.0, y=SPAWN_Y, z=3000.0, room=1)
    g.add_pad(index=9, x=0.0, y=SPAWN_Y, z=1500.0, room=1)
    add_mp_scenarios(g, cases=[(0, 7, 8)], hill_pads=[9])

    add_loadout_intro(g)

    return g


def build_tiles_json():
    return floor_box_tiles("{name}", half=BOX_HALF, y=0.0, room_index=1)
'''

    with open(level_path, "w") as f:
        f.write(template)

    print(f"Created level module: {level_path}")
    print("Next steps:")
    print(f"  1. Edit {level_path}")
    print(f"  2. Run: python3 tools/pdmap.py build {name} --deploy")
    print("  3. Wire stage registration if this is a new stage (files.h, list.c, stagetable.c, setup.c)")


def main():
    parser = argparse.ArgumentParser(
        prog="pdmap",
        description="Perfect Dark map creation pipeline (pads, tiles, setup, seg, deploy)",
    )

    sub = parser.add_subparsers(dest="command")

    p_build = sub.add_parser("build", help="Build a level end-to-end")
    p_build.add_argument("name", help="Level name (e.g. uff)")
    p_build.add_argument("--deploy", "-d", action="store_true", help="Deploy to mod directories after build")
    p_build.add_argument("--seg", nargs="?", const="", default=None,
                         help="Build seg file (uses level SEG_SCRIPT when flag given without path)")
    p_build.add_argument("--no-validate", action="store_true",
                         help="Skip post-build validation (WIP maps)")
    p_build.set_defaults(func=cmd_build)

    p_info = sub.add_parser("info", help="Show level statistics")
    p_info.add_argument("name", help="Level name")
    p_info.set_defaults(func=cmd_info)

    p_val = sub.add_parser("validate", help="Validate level for errors")
    p_val.add_argument("name", help="Level name")
    p_val.set_defaults(func=cmd_validate)

    p_deploy = sub.add_parser("deploy", help="Deploy built assets")
    p_deploy.add_argument("name", help="Level name")
    p_deploy.set_defaults(func=cmd_deploy)

    p_list = sub.add_parser("list", help="List levels")
    p_list.set_defaults(func=cmd_list)

    p_init = sub.add_parser("init", help="Scaffold a new level")
    p_init.add_argument("name", help="New level name")
    p_init.set_defaults(func=cmd_init)

    args = parser.parse_args()
    if args.command is None:
        parser.print_help()
        sys.exit(1)

    args.func(args)


if __name__ == "__main__":
    main()
