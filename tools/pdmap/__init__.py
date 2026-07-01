import argparse
import sys
import os
import subprocess

from .core import (
    load_level_module, ROOT, MOD_DIRS
)
from .validate import validate_all
from .info import info
from .from_json import EditorMapSpec, load_editor_json
from .pipeline import build_from_module, build_from_spec
from .level_codegen import render_level_module
from .play import MOD_CHOICES, play_command, detect_pd_binary, TEST_MAP_SLOT


def _mod_bgdata(mod_key: str) -> str:
    if mod_key not in MOD_CHOICES:
        raise ValueError(f"Unknown mod {mod_key!r}; choose from {', '.join(MOD_CHOICES)}")
    return os.path.join(ROOT, MOD_CHOICES[mod_key], "files", "bgdata")


def cmd_build(args):
    name = args.name
    print(f"Building level: {name}")

    mod_dirs = None
    if args.deploy:
        mod_dirs = MOD_DIRS

    seg_script = None
    try:
        mod = load_level_module(name)
        seg_script = getattr(mod, "SEG_SCRIPT", None) or (
            mod.seg_script() if hasattr(mod, "seg_script") else None
        )
    except Exception:
        mod = None

    has_box_dims = mod is not None and hasattr(mod, "BOX_HALF") and hasattr(mod, "BOX_HEIGHT")
    want_seg = args.seg is not None or bool(seg_script) or has_box_dims
    seg_script_path = None
    if args.seg and args.seg != "":
        seg_script_path = args.seg
    elif seg_script:
        seg_script_path = seg_script

    try:
        errors, warnings = build_from_module(
            name,
            deploy=args.deploy,
            want_seg=want_seg,
            seg_script_path=seg_script_path,
            mod_dirs=mod_dirs,
            skip_validate=args.no_validate,
            verbose=True,
        )
    except Exception as exc:
        print(f"ERROR: build failed: {exc}", file=sys.stderr)
        sys.exit(1)

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


def cmd_register(args):
    from .register import plan_registration, write_plan

    name = args.name.strip().lower()
    try:
        plan = plan_registration(name)
    except ValueError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(1)

    if plan.already_registered:
        print(f"WARNING: {name} appears already registered in files.h", file=sys.stderr)

    path = write_plan(name)
    print(plan.render_markdown() if args.print else f"Wrote registration plan -> {os.path.relpath(path, ROOT)}")

    if args.apply:
        from .register import apply_registration

        try:
            changed = apply_registration(name)
        except ValueError as exc:
            print(f"ERROR: {exc}", file=sys.stderr)
            sys.exit(1)
        if changed:
            print(f"\nApplied registration patches ({len(changed)} files):")
            for p in changed:
                print(f"  - {os.path.relpath(p, ROOT)}")
            print(f"\nNext: make -j8 && python3 tools/pdmap.py build {name} --deploy")
        else:
            print(f"\n{name}: registration snippets already present (no file changes)")
    elif not args.print:
        print(f"\nOpen {path} and apply the four snippets, or run: pdmap register {name} --apply")


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


def cmd_from_json(args):
    """Build (and optionally play) directly from Map Editor JSON — no level module required."""
    try:
        data = load_editor_json(args.json_file if args.json_file != "-" else None)
    except Exception as exc:
        print(f"ERROR: failed to load JSON: {exc}", file=sys.stderr)
        sys.exit(1)

    deploy_name = (args.deploy_as or args.name or data.get("name") or "map").strip().lower()
    try:
        spec = EditorMapSpec.from_json(data, deploy_name=deploy_name)
    except ValueError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(1)

    print(f"Building from JSON → {deploy_name}")
    print(f"  Pads: {len(spec.mapdef.pads)}  Props: {len(spec.mapdef.props)}  "
          f"Box: {spec.box_half:.0f} × {spec.box_height:.0f}")

    if args.write_level:
        level_path = os.path.join(ROOT, "src", "levels", f"{deploy_name}.py")
        if args.backup and os.path.exists(level_path):
            bak = level_path + ".bak"
            import shutil
            shutil.copy2(level_path, bak)
            print(f"  Backed up existing level -> {os.path.relpath(bak, ROOT)}")
        os.makedirs(os.path.dirname(level_path), exist_ok=True)
        with open(level_path, "w", encoding="utf-8") as fp:
            fp.write(render_level_module(spec))
        print(f"  Wrote level module -> {os.path.relpath(level_path, ROOT)}")

    mod_dirs = [_mod_bgdata(args.mod)] if args.deploy else None
    seg_mode = args.seg_mode or "empty"

    try:
        errors, warnings = build_from_spec(
            spec,
            deploy=args.deploy,
            want_seg=not args.no_seg,
            seg_mode=seg_mode if not args.no_seg else None,
            mod_dirs=mod_dirs,
            skip_validate=args.no_validate,
            verbose=True,
        )
    except Exception as exc:
        print(f"ERROR: build failed: {exc}", file=sys.stderr)
        sys.exit(1)

    for w in warnings:
        print(f"  [WARN] {w}")
    for e in errors:
        print(f"  [ERROR] {e}", file=sys.stderr)
    if errors:
        print(f"Build finished with {len(errors)} validation error(s)", file=sys.stderr)
        sys.exit(1)

    if args.play:
        pd_binary = args.binary or detect_pd_binary()
        if not os.path.isfile(pd_binary):
            print(f"ERROR: pd binary not found at {pd_binary}", file=sys.stderr)
            print("Build the game first: cmake --build build --target pd", file=sys.stderr)
            sys.exit(1)
        cmd = play_command(
            mod_key=args.mod,
            scenario=args.scenario,
            pd_binary=pd_binary,
            deploy_name=deploy_name,
        )
        print("\nLaunching:", " ".join(cmd))
        subprocess.run(cmd, cwd=ROOT, check=False)

    print("Done.")


def cmd_register(args):
    from .register import plan_registration, write_plan

    name = args.name.strip().lower()
    try:
        plan = plan_registration(name)
    except ValueError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(1)

    if plan.already_registered:
        print(f"WARNING: {name} appears already registered in files.h", file=sys.stderr)

    path = write_plan(name)
    print(plan.render_markdown() if args.print else f"Wrote registration plan -> {os.path.relpath(path, ROOT)}")

    if args.apply:
        from .register import apply_registration

        try:
            changed = apply_registration(name)
        except ValueError as exc:
            print(f"ERROR: {exc}", file=sys.stderr)
            sys.exit(1)
        if changed:
            print(f"\nApplied registration patches ({len(changed)} files):")
            for p in changed:
                print(f"  - {os.path.relpath(p, ROOT)}")
            print(f"\nNext: make -j8 && python3 tools/pdmap.py build {name} --deploy")
        else:
            print(f"\n{name}: registration snippets already present (no file changes)")
    elif not args.print:
        print(f"\nOpen {path} and apply the four snippets, or run: pdmap register {name} --apply")


def cmd_learn(args):
    from .learn.engine import LearnEngine, LEARN_DIR
    import json

    engine = LearnEngine()
    state_path = os.path.join(LEARN_DIR, "state.json")
    iteration = 1
    if os.path.exists(state_path):
        with open(state_path, encoding="utf-8") as fp:
            iteration = int(json.load(fp).get("iteration", 0)) + 1

    if args.learn_cmd == "run":
        report = engine.run(iteration=iteration)
        os.makedirs(LEARN_DIR, exist_ok=True)
        with open(state_path, "w", encoding="utf-8") as fp:
            json.dump({"iteration": iteration, "last_run": report.run_id}, fp, indent=2)
        print(report.summary())
        if report.gaps:
            print(f"\nTop gaps written to journal/map_learn/gaps.md")
        if report.probe_errors:
            print("\nProbe errors:", file=sys.stderr)
            for e in report.probe_errors:
                print(f"  {e}", file=sys.stderr)
            sys.exit(1)
        print(f"\nSpec: docs/MAP_DETERMINISTIC_SPEC.md")
        return

    if args.learn_cmd == "report":
        print(engine.report_text())
        return

    if args.learn_cmd == "emit-spec":
        path = engine.emit_spec()
        print(f"Wrote {path}")
        return

    if args.learn_cmd == "gaps":
        gaps_path = os.path.join(LEARN_DIR, "gaps.md")
        if os.path.exists(gaps_path):
            print(open(gaps_path, encoding="utf-8").read())
        else:
            print("No gaps file yet — run: pdmap learn run")
        return


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

    p_json = sub.add_parser(
        "from-json",
        help="Build directly from Map Editor JSON (deterministic, no level module required)",
    )
    p_json.add_argument(
        "json_file",
        nargs="?",
        default="-",
        help="Editor JSON file (default: stdin)",
    )
    p_json.add_argument(
        "--name",
        help="Level name in JSON (default: JSON name field)",
    )
    p_json.add_argument(
        "--deploy-as",
        help=f"Asset name for build/deploy (default: name). Use '{TEST_MAP_SLOT}' for --test-map.",
    )
    p_json.add_argument(
        "--mod",
        choices=sorted(MOD_CHOICES),
        default="mod_allinone",
        help="Mod directory to deploy into (default: mod_allinone)",
    )
    p_json.add_argument(
        "--scenario",
        type=int,
        choices=sorted({0, 1, 2, 3, 4, 5}),
        default=0,
        help="MP scenario for --test-map (default: 0 Combat)",
    )
    p_json.add_argument(
        "--seg-mode",
        default="empty",
        help="PDMAP_SEG_MODE for box seg (default: empty — collision from tiles only)",
    )
    p_json.add_argument("--no-seg", action="store_true", help="Skip box seg build")
    p_json.add_argument(
        "--deploy", "-d",
        action="store_true",
        default=True,
        help="Deploy to mod after successful validation (default: on)",
    )
    p_json.add_argument("--no-deploy", dest="deploy", action="store_false")
    p_json.add_argument("--no-validate", action="store_true", help="Skip post-build validation")
    p_json.add_argument(
        "--write-level",
        action="store_true",
        help="Also write src/levels/<name>.py (optional; not required to play)",
    )
    p_json.add_argument(
        "--backup",
        action="store_true",
        default=True,
        help="Backup existing level module when --write-level (default: on)",
    )
    p_json.add_argument("--no-backup", dest="backup", action="store_false")
    p_json.add_argument("--play", action="store_true", help="Launch pd after build")
    p_json.add_argument("--binary", help="Path to pd binary (default: auto-detect)")
    p_json.add_argument("-v", "--verbose", action="store_true", help="Verbose build logging")
    p_json.set_defaults(func=cmd_from_json)

    p_learn = sub.add_parser(
        "learn",
        help="Deterministic learning engine — probe, verify, document map creation",
    )
    learn_sub = p_learn.add_subparsers(dest="learn_cmd")
    p_learn_run = learn_sub.add_parser("run", help="Run all probes and update knowledge")
    p_learn_run.set_defaults(func=cmd_learn, learn_cmd="run")
    p_learn_report = learn_sub.add_parser("report", help="Show knowledge status")
    p_learn_report.set_defaults(func=cmd_learn, learn_cmd="report")
    p_learn_emit = learn_sub.add_parser("emit-spec", help="Regenerate MAP_DETERMINISTIC_SPEC.md")
    p_learn_emit.set_defaults(func=cmd_learn, learn_cmd="emit-spec")
    p_learn_gaps = learn_sub.add_parser("gaps", help="Print open documentation gaps")
    p_learn_gaps.set_defaults(func=cmd_learn, learn_cmd="gaps")

    p_reg = sub.add_parser("register", help="Generate stage registration plan (four C wiring points)")
    p_reg.add_argument("name", help="Level / asset name")
    p_reg.add_argument("--print", action="store_true", help="Print plan to stdout instead of writing file")
    p_reg.add_argument(
        "--apply",
        action="store_true",
        help="Patch files.h, list.c, stagetable.c, setup.c, and constants.h (idempotent)",
    )
    p_reg.set_defaults(func=cmd_register)

    args = parser.parse_args()
    if args.command is None:
        parser.print_help()
        sys.exit(1)

    args.func(args)


if __name__ == "__main__":
    main()
