"""Shared build pipeline: MapDef → binaries → validate → deploy."""

from __future__ import annotations

import json
import os
from typing import Any

from .core import (
    BUILD_DIR,
    MapDef,
    ROMID,
    ROOT,
    compile_pads,
    compile_tiles,
    load_level_module,
)
from .deploy import deploy_all
from .from_json import EditorMapSpec
from .pads_builder import write_pads_json
from .setup_packer import write_setup_binary
from .tiles import copy_tiles_from_template
from .validate import validate_all, validate_mapdef


def _resolve_seg_script(mod, name: str) -> str | None:
    if hasattr(mod, "SEG_SCRIPT"):
        return mod.SEG_SCRIPT
    if hasattr(mod, "seg_script"):
        return mod.seg_script()
    return None


def _build_box_seg_to_build_dir(name: str, *, half: float, height: float) -> str:
    from .seg import validate_seg_g_vtx, write_box_seg

    os.makedirs(BUILD_DIR, exist_ok=True)
    build_dst = os.path.join(BUILD_DIR, f"bg_{name}.seg")
    write_box_seg(build_dst, half=half, height=height)
    with open(build_dst, "rb") as seg_fp:
        errors = validate_seg_g_vtx(seg_fp.read())
    if errors:
        raise ValueError(
            "Box seg failed G_VTX validation:\n  " + "\n  ".join(errors)
        )
    return build_dst


def _build_seg_script_to_build_dir(name: str, script_path: str) -> str:
    import subprocess

    from .seg import validate_seg_g_vtx

    abs_script = script_path if os.path.isabs(script_path) else os.path.join(ROOT, script_path)
    if not os.path.exists(abs_script):
        raise FileNotFoundError(f"Seg script not found: {abs_script}")

    subprocess.run([__import__("sys").executable, abs_script], cwd=ROOT, check=True)

    script_dir = os.path.dirname(abs_script)
    generated = os.path.join(script_dir, f"bg_{name}.seg")
    if not os.path.exists(generated):
        raise FileNotFoundError(
            f"Seg script did not produce {generated}. "
            "Ensure the script writes bg_<name>.seg next to itself."
        )

    with open(generated, "rb") as seg_fp:
        errors = validate_seg_g_vtx(seg_fp.read())
    if errors:
        raise ValueError(
            "Seg script output failed G_VTX validation:\n  " + "\n  ".join(errors)
        )

    os.makedirs(BUILD_DIR, exist_ok=True)
    build_dst = os.path.join(BUILD_DIR, f"bg_{name}.seg")
    import shutil

    shutil.copy2(generated, build_dst)
    return build_dst


def build_from_spec(
    spec: EditorMapSpec,
    *,
    deploy: bool = False,
    want_seg: bool = True,
    seg_mode: str | None = "empty",
    mod_dirs: list[str] | None = None,
    skip_validate: bool = False,
    verbose: bool = False,
) -> tuple[list[str], list[str]]:
    """Build all assets from an ``EditorMapSpec``; validate before deploy."""
    name = spec.name
    mapdef = spec.mapdef
    warnings: list[str] = []

    if spec.y_corrected_pads:
        warnings.append(
            f"Auto-corrected pad Y to {spec.spawn_y} for pad(s) "
            f"{list(spec.y_corrected_pads)} (must be above floor Y=0)"
        )

    pre_errors = validate_mapdef(mapdef, tiles_room_count=2)
    if pre_errors:
        return pre_errors, warnings

    if want_seg:
        if seg_mode:
            os.environ.setdefault("PDMAP_SEG_MODE", seg_mode)
        if verbose:
            print(
                f"  Building box seg (half={spec.box_half:.0f} "
                f"height={spec.box_height:.0f}, mode={os.environ.get('PDMAP_SEG_MODE', 'empty')})"
            )
        _build_box_seg_to_build_dir(
            name, half=spec.box_half, height=spec.box_height
        )

    pads_json_path = write_pads_json(mapdef)
    if verbose:
        print(f"  Generated pads JSON: {pads_json_path}")
    compile_pads(name, pads_json_path)
    if verbose:
        print("  Compiled pads binary")

    tiles_json_path = os.path.join(ROOT, "src", "assets", ROMID, "tiles", f"{name}.json")
    tiles_data = spec.tiles_json()
    os.makedirs(os.path.dirname(tiles_json_path), exist_ok=True)
    with open(tiles_json_path, "w", encoding="utf-8") as fp:
        json.dump(tiles_data, fp, indent=4)
    if verbose:
        print("  Generated tiles JSON from EditorMapSpec")

    compile_tiles(name, tiles_json_path)
    if verbose:
        print("  Compiled tiles binary")

    setup_path = write_setup_binary(mapdef, name)
    if verbose:
        print(f"  Wrote setup binary: {setup_path}")

    if skip_validate:
        if deploy:
            if verbose:
                print("  Deploying...")
            deploy_all(name, mod_dirs)
            if verbose:
                print("  Deploy complete")
        return [], warnings

    errors, post_warnings = validate_all(name, mapdef)
    warnings.extend(post_warnings)
    if errors:
        return errors, warnings

    if deploy:
        if verbose:
            print("  Deploying...")
        deploy_all(name, mod_dirs)
        if verbose:
            print("  Deploy complete")

    return [], warnings


def build_from_module(
    name: str,
    *,
    deploy: bool = False,
    want_seg: bool = True,
    seg_script_path: str | None = None,
    mod_dirs: list[str] | None = None,
    skip_validate: bool = False,
    verbose: bool = False,
) -> tuple[list[str], list[str]]:
    """Build from ``src/levels/<name>.py`` (legacy / hand-authored levels)."""
    mod = load_level_module(name)
    mapdef = mod.build()
    seg_script = seg_script_path or _resolve_seg_script(mod, name)
    has_box_dims = hasattr(mod, "BOX_HALF") and hasattr(mod, "BOX_HEIGHT")
    do_seg = want_seg or bool(seg_script) or has_box_dims

    if do_seg:
        seg_mode = getattr(mod, "SEG_MODE", None)
        if seg_mode:
            os.environ.setdefault("PDMAP_SEG_MODE", str(seg_mode))
        elif has_box_dims:
            os.environ.setdefault("PDMAP_SEG_MODE", "empty")
        if seg_script:
            if verbose:
                print(f"  Building seg via {seg_script}")
            _build_seg_script_to_build_dir(name, seg_script)
        else:
            half = float(getattr(mod, "BOX_HALF", 5000.0))
            height = float(getattr(mod, "BOX_HEIGHT", 3000.0))
            if verbose:
                print(f"  Building box seg (half={half:.0f} height={height:.0f})")
            _build_box_seg_to_build_dir(name, half=half, height=height)
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

    tiles_json_path = os.path.join(ROOT, "src", "assets", ROMID, "tiles", f"{name}.json")
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

    if skip_validate:
        if deploy:
            if verbose:
                print("  Deploying...")
            deploy_all(name, mod_dirs)
        return [], []

    errors, warnings = validate_all(name, mapdef)
    if errors:
        return errors, warnings

    if deploy:
        if verbose:
            print("  Deploying...")
        deploy_all(name, mod_dirs)
        if verbose:
            print("  Deploy complete")

    return [], warnings
