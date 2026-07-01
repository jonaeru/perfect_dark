import os
import shutil
import subprocess
import sys

from .core import BUILD_DIR, MOD_DIRS, ROOT, SETUP_DIR
from .seg import validate_seg_g_vtx


def _validate_seg_before_deploy(seg_path: str) -> None:
    """Reject stale/corrupt segs that would reintroduce phantom collision walls."""
    with open(seg_path, "rb") as seg_fp:
        errors = validate_seg_g_vtx(seg_fp.read())
    if errors:
        raise ValueError(
            f"Refusing to deploy corrupt seg {seg_path}:\n  "
            + "\n  ".join(errors)
            + "\n  Rebuild with: python3 tools/pdmap.py build <name> --seg --deploy"
        )


def deploy(name: str, mod_dirs: list[str] | None = None):
    """Deploy built assets (tiles, pads, seg) to mod bgdata directories."""
    if mod_dirs is None:
        mod_dirs = MOD_DIRS

    bgdata_assets = [
        f"bg_{name}_tilesZ",
        f"bg_{name}_padsZ",
        f"bg_{name}.seg",
    ]

    for mod_dir in mod_dirs:
        os.makedirs(mod_dir, exist_ok=True)
        for asset in bgdata_assets:
            src = os.path.join(BUILD_DIR, asset)
            dst = os.path.join(mod_dir, asset)
            if os.path.exists(src):
                if asset.endswith(".seg"):
                    _validate_seg_before_deploy(src)
                shutil.copy2(src, dst)
                print(f"  Deployed {asset} -> {dst}")
            else:
                print(f"  WARNING: {src} not found, skipping")


def deploy_setup(name: str, mod_dirs: list[str] | None = None):
    """Deploy setup binary to mod files directories (parent of bgdata)."""
    if mod_dirs is None:
        mod_dirs = MOD_DIRS

    setup_path = os.path.join(SETUP_DIR, f"Ump_setup{name}Z")
    if not os.path.exists(setup_path):
        print(f"  WARNING: Setup not found at {setup_path}")
        return

    for mod_dir in mod_dirs:
        files_dir = os.path.dirname(mod_dir)
        os.makedirs(files_dir, exist_ok=True)
        dst = os.path.join(files_dir, f"Ump_setup{name}Z")
        shutil.copy2(setup_path, dst)
        print(f"  Deployed setup -> {dst}")


def deploy_all(name: str, mod_dirs: list[str] | None = None):
    deploy(name, mod_dirs)
    deploy_setup(name, mod_dirs)


def build_box_seg_asset(name: str, *, half: float = 5000.0, height: float = 3000.0,
                        mod_dirs: list[str] | None = None) -> str:
    """Generate a generic single-room box seg directly (no external script).

    This is the turnkey path for scaffolded maps that do not ship a bespoke
    ``SEG_SCRIPT``. It uses the procedural box generator in
    ``tools.pdmap.seg`` (the same engine ``build_custom_seg.py`` wraps for uff)
    to write ``bg_<name>.seg`` into the build tree and every mod directory.

    ``half``/``height`` should match the floor tiles so the visible walls line
    up with the collision floor; the seg's Section-3 bbox spans the full s16
    range regardless, so spawns always resolve to a room.
    """
    from .seg import write_box_seg

    if mod_dirs is None:
        mod_dirs = MOD_DIRS

    os.makedirs(BUILD_DIR, exist_ok=True)
    build_dst = os.path.join(BUILD_DIR, f"bg_{name}.seg")
    write_box_seg(build_dst, half=half, height=height)
    _validate_seg_before_deploy(build_dst)
    print(f"  Built box seg -> {build_dst} (half={half:.0f} height={height:.0f})")

    for mod_dir in mod_dirs:
        os.makedirs(mod_dir, exist_ok=True)
        mod_dst = os.path.join(mod_dir, f"bg_{name}.seg")
        shutil.copy2(build_dst, mod_dst)
        _validate_seg_before_deploy(mod_dst)
        print(f"  Deployed seg -> {mod_dst}")

    return build_dst


def build_seg(name: str, script_path: str, mod_dirs: list[str] | None = None) -> str:
    """Run a seg generator script and install bg_<name>.seg to build + mods."""
    if mod_dirs is None:
        mod_dirs = MOD_DIRS

    abs_script = script_path if os.path.isabs(script_path) else os.path.join(ROOT, script_path)
    if not os.path.exists(abs_script):
        raise FileNotFoundError(f"Seg script not found: {abs_script}")

    subprocess.run([sys.executable, abs_script], cwd=ROOT, check=True)

    script_dir = os.path.dirname(abs_script)
    generated = os.path.join(script_dir, f"bg_{name}.seg")
    if not os.path.exists(generated):
        raise FileNotFoundError(
            f"Seg script did not produce {generated}. "
            f"Ensure the script writes bg_{name}.seg next to itself."
        )

    _validate_seg_before_deploy(generated)

    os.makedirs(BUILD_DIR, exist_ok=True)
    build_dst = os.path.join(BUILD_DIR, f"bg_{name}.seg")
    shutil.copy2(generated, build_dst)
    _validate_seg_before_deploy(build_dst)
    print(f"  Installed seg -> {build_dst}")

    for mod_dir in mod_dirs:
        os.makedirs(mod_dir, exist_ok=True)
        mod_dst = os.path.join(mod_dir, f"bg_{name}.seg")
        shutil.copy2(generated, mod_dst)
        _validate_seg_before_deploy(mod_dst)
        print(f"  Deployed seg -> {mod_dst}")

    return build_dst
