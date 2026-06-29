#!/usr/bin/env python3
"""Read-only inspection deliverable generator for the Matrix arena (stage `uff`).

Produces, into this directory:
  * uff_map.html      interactive three.js viewer (box faces + tiles + pads + axes)
  * uff_map.obj       OBJ export (box faces + tiles as separate groups)
  * uff_gdl_dump.txt  human-readable decode of the seg display list

This script ONLY reads the repo's Python generators (tools/pdmap/seg.py,
src/levels/uff.py via tools.pdmap.core.load_level_module) — it does NOT modify
game source, touch git, or build the game. It is a diagnosis aid for the
"phantom collidable surface near the origin" bug.
"""

import json
import os
import struct
import sys

# --- Repo import setup -----------------------------------------------------
# This file lives at <ROOT>/journal/uff_viewer/. The pdmap package is imported
# as `tools.pdmap.*`, exactly like the repo's own scripts do.
HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(os.path.dirname(HERE))
if ROOT not in sys.path:
    sys.path.insert(0, ROOT)

from tools.pdmap import seg as segmod
from tools.pdmap.core import load_level_module

ORIGIN_RADIUS = 500.0  # geometry whose verts fall within this of origin = SUSPECT


# ===========================================================================
# 1. Pull exact geometry straight from the repo generators
# ===========================================================================
def load_geometry():
    uff = load_level_module("uff")
    half = float(uff.BOX_HALF)
    height = float(uff.BOX_HEIGHT)

    # 6 box faces: each ((x,y,z)*4, colour_index). Exact builder output.
    raw_faces = segmod._faces(half, height)
    face_colours = segmod.DEFAULT_FACE_COLOURS

    face_names = [
        "F0 floor (Y=0)",
        "F1 ceiling (Y=H)",
        "F2 wall -Z",
        "F3 wall +X",
        "F4 wall +Z",
        "F5 wall -X",
    ]
    faces = []
    for i, (v0, v1, v2, v3, cidx) in enumerate(raw_faces):
        rgba = face_colours[i]
        faces.append({
            "index": i,
            "name": face_names[i],
            "verts": [list(v0), list(v1), list(v2), list(v3)],
            "rgba": rgba,
            "hex": "#%02X%02X%02X" % ((rgba >> 24) & 0xFF, (rgba >> 16) & 0xFF, (rgba >> 8) & 0xFF),
        })

    # Collision tiles from build_tiles_json()
    tiles_json = uff.build_tiles_json()
    tiles = []
    for room_key, tlist in tiles_json["rooms"].items():
        for t in tlist:
            verts = [[v["x"], v["y"], v["z"]] for v in t["vertices"]]
            tiles.append({
                "room": room_key,
                "verts": verts,
                "floortype": t.get("floortype", "?"),
                "floorcolour": t.get("floorcolour", 0),
            })

    # Pads from the MapDef build(), classified by how they are used.
    mapdef = uff.build()
    spawn_pads, weapon_pads, ammo_pads, scenario_pads = set(), set(), set(), set()
    for cmd in mapdef.intro:
        if isinstance_byname(cmd, "Spawn"):
            spawn_pads.add(cmd.pad)
        elif isinstance_byname(cmd, ("Case", "CaseRespawn", "Hill")):
            scenario_pads.add(cmd.pad)
    for p in mapdef.props:
        # WeaponProp / AmmoCrate both store the pad in .pad
        cls = type(p).__name__
        if cls == "Weapon":
            weapon_pads.add(p.pad)
        elif cls in ("AmmoCrate", "AmmoCrateMulti"):
            ammo_pads.add(p.pad)

    pads = []
    for p in mapdef.pads:
        idx = p.index
        if idx in spawn_pads:
            kind = "spawn"
        elif idx in weapon_pads:
            kind = "weapon"
        elif idx in ammo_pads:
            kind = "ammo"
        elif idx in scenario_pads:
            kind = "scenario"
        else:
            kind = "other"
        pads.append({
            "index": idx,
            "pos": [p.x, p.y, p.z],
            "room": p.room,
            "kind": kind,
        })

    return {
        "half": half,
        "height": height,
        "faces": faces,
        "tiles": tiles,
        "pads": pads,
    }


def isinstance_byname(obj, names):
    if isinstance(names, str):
        names = (names,)
    return type(obj).__name__ in names


def level_to_editor_json(name: str) -> dict:
    """Convert an existing src/levels/<name>.py module into pass-2 editor JSON."""
    mod = load_level_module(name)
    half = float(getattr(mod, "BOX_HALF", 5000.0))
    height = float(getattr(mod, "BOX_HEIGHT", 3000.0))
    mapdef = mod.build()

    spawn_pads: set[int] = set()
    weapon_pads: set[int] = set()
    ammo_pads: set[int] = set()
    weapon_by_pad: dict[int, int] = {}
    ammo_by_pad: dict[int, int] = {}
    scenario_by_pad: dict[int, tuple[str, int]] = {}

    for cmd in mapdef.intro:
        if isinstance_byname(cmd, "Spawn"):
            spawn_pads.add(cmd.pad)
        elif isinstance_byname(cmd, "Case"):
            scenario_by_pad[cmd.pad] = ("case", cmd.team)
        elif isinstance_byname(cmd, "CaseRespawn"):
            scenario_by_pad[cmd.pad] = ("case", cmd.team)
        elif isinstance_byname(cmd, "Hill"):
            scenario_by_pad[cmd.pad] = ("hill", 0)

    for prop in mapdef.props:
        cls = type(prop).__name__
        if cls == "Weapon":
            weapon_pads.add(prop.pad)
            weapon_by_pad[prop.pad] = prop.weapon
        elif cls in ("AmmoCrate", "AmmoCrateMulti"):
            ammo_pads.add(prop.pad)
            ammo_by_pad[prop.pad] = getattr(prop, "ammotype", 0x04)

    pads = []
    for p in mapdef.pads:
        idx = p.index
        if idx in spawn_pads:
            kind = "spawn"
        elif idx in weapon_pads:
            kind = "weapon"
        elif idx in ammo_pads:
            kind = "ammo"
        elif idx in scenario_by_pad:
            kind = "scenario"
        else:
            kind = "other"

        entry: dict = {
            "index": idx,
            "type": kind,
            "x": p.x,
            "y": p.y,
            "z": p.z,
            "room": p.room,
        }
        if kind == "weapon":
            entry["weapon"] = weapon_by_pad.get(idx, 0x11)
        elif kind == "ammo":
            entry["ammoType"] = ammo_by_pad.get(idx, 0x04)
            entry["quantity"] = 200
        elif kind == "scenario":
            sc, team = scenario_by_pad.get(idx, ("hill", 0))
            entry["scenario"] = sc
            entry["team"] = team
        pads.append(entry)

    return {
        "name": name,
        "box_half": half,
        "box_height": height,
        "pads": pads,
    }


def build_level_catalog() -> dict:
    """Embed every src/levels/*.py module as importable editor JSON."""
    levels_dir = os.path.join(ROOT, "src", "levels")
    catalog: dict = {}
    if not os.path.isdir(levels_dir):
        return catalog
    for fname in sorted(os.listdir(levels_dir)):
        if not fname.endswith(".py") or fname.startswith("_"):
            continue
        lvl = fname[:-3]
        try:
            catalog[lvl] = level_to_editor_json(lvl)
        except Exception as exc:  # noqa: BLE001
            print(f"  catalog skip {lvl}: {exc}")
    return catalog


# ===========================================================================
# 2. Decode the seg display list exactly as seg.py generates it
# ===========================================================================
OPCODE_NAMES = {
    0x01: "G_VTX(F3DEX-alt)",
    0x04: "G_VTX",
    0x05: "G_MODIFYVTX",
    0x06: "G_CULLDL",
    0x07: "G_COL(vtxcolours)",
    0xB6: "G_CLEARGEOMETRYMODE",
    0xB7: "G_SETGEOMETRYMODE",
    0xB8: "G_ENDDL",
    0xB9: "G_SETOTHERMODE_L",
    0xBA: "G_SETOTHERMODE_H",
    0xBB: "G_TEXTURE",
    0xBC: "G_MOVEWORD",
    0xBD: "G_POPMTX",
    0xBE: "G_GEOMETRYMODE",
    0xBF: "G_TRI1",
    0xDA: "G_MTX",
    0xDE: "G_DL",
    0xDF: "G_ENDDL(alt)",
    0xE2: "G_SETOTHERMODE_L",
    0xE3: "G_SETOTHERMODE_H",
    0xE6: "G_RDPLOADSYNC",
    0xE7: "G_RDPPIPESYNC",
    0xE8: "G_RDPTILESYNC",
    0xE9: "G_RDPFULLSYNC",
    0xF8: "G_SETFOGCOLOR",
    0xFB: "G_SETENVCOLOR",
    0xFC: "G_SETCOMBINE",
    0xFD: "G_SETTIMG",
}


def decode_gdl(gdl_bytes):
    """Yield decoded command dicts for an 8-byte-per-cmd F3DEX2 display list."""
    cmds = []
    for off in range(0, len(gdl_bytes), 8):
        w0, w1 = struct.unpack(">II", gdl_bytes[off:off + 8])
        op = (w0 >> 24) & 0xFF
        name = OPCODE_NAMES.get(op, "UNKNOWN")
        c = {"offset": off, "w0": w0, "w1": w1, "op": op, "name": name, "extra": ""}

        if op == 0x04:  # G_VTX as emitted by seg._g_vtx_cmd
            numbytes = w0 & 0xFFFF
            p = (w0 >> 16) & 0xFF
            nverts_nibble = ((p >> 4) & 0xF) + 1
            v0 = p & 0xF
            nverts_from_bytes = numbytes // 12
            c["g_vtx"] = {
                "nverts_nibble": nverts_nibble,
                "v0": v0,
                "numbytes": numbytes,
                "nverts_from_bytes": nverts_from_bytes,
                "addr": w1,
                "overflow": nverts_from_bytes > 16,
                "nibble_mismatch": nverts_nibble != nverts_from_bytes,
            }
            c["extra"] = (
                f"loads {nverts_from_bytes} verts (nibble says {nverts_nibble}), "
                f"v0={v0}, {numbytes} bytes, seg-addr 0x{w1:08X}"
            )
        elif op == 0xBF:  # G_TRI1 as emitted by seg._g_tri1_cmd (index*10)
            i = ((w1 >> 16) & 0xFF) // 10
            j = ((w1 >> 8) & 0xFF) // 10
            k = (w1 & 0xFF) // 10
            c["g_tri1"] = {"i": i, "j": j, "k": k}
            c["extra"] = f"tri verts ({i}, {j}, {k})"
        elif op == 0x07:
            ncols = ((w0 >> 16) & 0xFF) // 4 + 1  # inverse of seg patch (approx)
            c["extra"] = f"vertex colour list, seg-addr 0x{w1:08X}"
        return_cmd = c
        cmds.append(return_cmd)
    return cmds


def build_gdl_for_uff():
    """Reconstruct the exact room display list seg.py produces for uff."""
    ncols = len(segmod.DEFAULT_FACE_COLOURS) + 1
    setup_gdl = segmod._extract_setup_gdl(segmod.DEFAULT_TEMPLATE_SEG, ncols)
    gdl = segmod._build_gdl(setup_gdl)
    nverts_total = len(segmod._faces(5000, 3000)) * 4  # full box = 24
    return gdl, nverts_total


def decode_onesk_seg(path):
    """Decode the room display list of an ON-DISK bg_uff.seg (the SHIPPED seg).

    Lets us verify whether the binary the game actually loads contains the
    suspected single-G_VTX(24) overflow, independent of what seg.py would
    generate today. Returns (loads, lines) or (None, [error]).
    """
    try:
        data = open(path, "rb").read()
        w0, sec1_cmp, prim_cmp = struct.unpack(">III", data[0:12])
        room = segmod.unzip1172(data[12 + prim_cmp:12 + sec1_cmp])
        gdl_ptr = struct.unpack(">I", room[32:36])[0]
        room_base = segmod.SEG_BASE + w0
        off = gdl_ptr - room_base
        loads, lines, count = [], [], 0
        while 0 <= off < len(room) - 7:
            a, b = struct.unpack(">II", room[off:off + 8])
            op = (a >> 24) & 0xFF
            if op == 0x04:
                nb = a & 0xFFFF
                vb = nb // 12
                nib = (((a >> 16) & 0xFF) >> 4 & 0xF) + 1
                loads.append(vb)
                lines.append(f"    G_VTX bytes={nb} verts={vb} nibble={nib} addr=0x{b:08X}")
            elif op == 0xBF:
                lines.append(f"    G_TRI1 ({((b>>16)&0xFF)//10},{((b>>8)&0xFF)//10},{(b&0xFF)//10})")
            elif op == 0xB8:
                lines.append("    G_ENDDL")
                break
            off += 8
            count += 1
            if count > 400:
                break
        return loads, lines
    except Exception as e:  # noqa: BLE001
        return None, [f"    decode error: {e!r}"]


# ===========================================================================
# 3. Writers
# ===========================================================================
def near_origin(verts, radius=ORIGIN_RADIUS):
    for (x, y, z) in verts:
        if (x * x + y * y + z * z) ** 0.5 <= radius:
            return True
    return False


def write_obj(geo, path):
    lines = ["# uff (Matrix arena) export — box faces + collision tiles",
             "# Coordinates are world units; +Y is up.", ""]
    vcount = 0

    lines.append("o uff_box_faces")
    for f in geo["faces"]:
        lines.append(f"g {f['name'].replace(' ', '_')}")
        base = vcount
        for (x, y, z) in f["verts"]:
            lines.append(f"v {x} {y} {z}")
            vcount += 1
        lines.append(f"f {base+1} {base+2} {base+3}")
        lines.append(f"f {base+1} {base+3} {base+4}")

    lines.append("")
    lines.append("o uff_collision_tiles")
    for ti, t in enumerate(geo["tiles"]):
        lines.append(f"g tile_{ti}_{t['room']}")
        base = vcount
        for (x, y, z) in t["verts"]:
            lines.append(f"v {x} {y} {z}")
            vcount += 1
        n = len(t["verts"])
        if n >= 3:
            face = "f " + " ".join(str(base + 1 + k) for k in range(n))
            lines.append(face)

    with open(path, "w") as fp:
        fp.write("\n".join(lines) + "\n")


def write_gdl_dump(geo, cmds, nverts_total, path):
    out = []
    out.append("=" * 72)
    out.append("UFF (Matrix arena) — seg display-list decode")
    out.append("Source: tools/pdmap/seg.py  (_extract_setup_gdl + _build_gdl, mode=full)")
    out.append("=" * 72)
    out.append("")
    out.append(f"Box half-extent : {geo['half']}  (X,Z span [-{geo['half']}, +{geo['half']}])")
    out.append(f"Box height      : {geo['height']}  (Y span [0, {geo['height']}])")
    out.append(f"Total box verts : {nverts_total}  (6 faces x 4 verts)")
    out.append("")
    out.append("-" * 72)
    out.append("DISPLAY LIST (in order)")
    out.append("-" * 72)

    vtx_loads = []
    covered = 0
    for c in cmds:
        line = f"  +0x{c['offset']:03X}  {c['w0']:08X} {c['w1']:08X}  {c['name']}"
        if c["extra"]:
            line += f"\n            -> {c['extra']}"
        out.append(line)
        if "g_vtx" in c:
            vtx_loads.append(c["g_vtx"])
            covered += c["g_vtx"]["nverts_from_bytes"]

    out.append("")
    out.append("-" * 72)
    out.append("G_VTX LOAD SUMMARY  (F3DEX2 count nibble is 4-bit -> max 16/load)")
    out.append("-" * 72)
    any_overflow = False
    for n, v in enumerate(vtx_loads):
        flag = ""
        if v["overflow"]:
            flag = "  <<< OVERFLOW! loads >16 verts (PHANTOM-SURFACE SUSPECT)"
            any_overflow = True
        elif v["nibble_mismatch"]:
            flag = "  <<< nibble/byte-count mismatch"
        out.append(
            f"  load #{n}: bytes={v['numbytes']:>4}  verts={v['nverts_from_bytes']:>2}  "
            f"nibble={v['nverts_nibble']:>2}  v0={v['v0']}  addr=0x{v['addr']:08X}{flag}"
        )
    out.append("")
    out.append(f"  G_VTX loads          : {len(vtx_loads)}")
    out.append(f"  verts covered by loads: {covered}")
    out.append(f"  total verts in buffer : {nverts_total}")
    out.append("")
    if any_overflow:
        out.append("  VERDICT: A G_VTX load exceeds 16 vertices. F3DEX2's 4-bit count")
        out.append("           nibble wraps, so bgPopulateVtxBatchType under-loads the")
        out.append("           collision batch while still walking every triangle ->")
        out.append("           PHANTOM collision triangles near the origin. THIS is the bug.")
    else:
        out.append("  VERDICT: No G_VTX load exceeds 16 vertices; every load is self-")
        out.append("           contained (one 4-vert load + its 2 triangles per face).")
        out.append("           The seg geometry as generated does NOT overflow the F3DEX2")
        out.append("           count nibble, so a single-G_VTX(24) overflow is NOT present")
        out.append("           in the current generator output. If a phantom surface still")
        out.append("           appears in-game, the shipped seg may predate this per-face")
        out.append("           fix (rebuild the seg), or the phantom comes from a stale")
        out.append("           collision batch elsewhere — see seg.py _build_gdl comment.")
    out.append("")

    # Origin proximity report
    out.append("-" * 72)
    out.append(f"ORIGIN PROXIMITY (anything with a vertex within {ORIGIN_RADIUS:g} units)")
    out.append("-" * 72)
    hits = []
    for f in geo["faces"]:
        if near_origin(f["verts"]):
            hits.append(f"  FACE {f['name']}: {f['verts']}")
    for ti, t in enumerate(geo["tiles"]):
        if near_origin(t["verts"]):
            hits.append(f"  TILE {ti} ({t['room']}): {t['verts']}")
    if hits:
        out.extend(hits)
    else:
        out.append("  (no box face or collision tile has any vertex near the origin)")
    out.append("")

    # --- Shipped/built seg verification ------------------------------------
    out.append("-" * 72)
    out.append("SHIPPED / BUILT bg_uff.seg VERIFICATION (decoded from disk)")
    out.append("-" * 72)
    seg_paths = [
        os.path.join(ROOT, "mods", "mod_allinone", "files", "bgdata", "bg_uff.seg"),
        os.path.join(ROOT, "build", "ntsc-final", "assets", "files", "bgdata", "bg_uff.seg"),
    ]
    found_any = False
    for sp in seg_paths:
        if not os.path.exists(sp):
            out.append(f"  (absent) {os.path.relpath(sp, ROOT)}")
            continue
        found_any = True
        loads, lines = decode_onesk_seg(sp)
        out.append(f"  {os.path.relpath(sp, ROOT)}")
        out.extend(lines)
        if loads is not None:
            mx = max(loads) if loads else 0
            ov = any(v > 16 for v in loads)
            out.append(f"    -> G_VTX load sizes={loads} max={mx} OVERFLOW(>16)={ov}")
        out.append("")
    if found_any:
        out.append("  NOTE: If the on-disk seg already uses small per-face loads (<=16),")
        out.append("        the G_VTX(24) overflow is NOT the cause of any phantom surface")
        out.append("        observed in a currently-running build -- look elsewhere (stale")
        out.append("        cached seg in the running process, collision tiles, or a")
        out.append("        different bg batch).")
    out.append("")

    with open(path, "w") as fp:
        fp.write("\n".join(out) + "\n")


def write_html(geo, cmds, nverts_total, path, level_catalog=None):
    # near-origin flags for HUD
    near_faces = [f["name"] for f in geo["faces"] if near_origin(f["verts"])]
    near_tiles = [f"tile{ti}({t['room']})" for ti, t in enumerate(geo["tiles"]) if near_origin(t["verts"])]

    vtx_loads = [c["g_vtx"] for c in cmds if "g_vtx" in c]
    max_load = max((v["nverts_from_bytes"] for v in vtx_loads), default=0)
    any_overflow = any(v["overflow"] for v in vtx_loads)

    data = {
        "name": "uff",
        "half": geo["half"],
        "height": geo["height"],
        "faces": geo["faces"],
        "tiles": geo["tiles"],
        "pads": geo["pads"],
        "originRadius": ORIGIN_RADIUS,
        "nearFaces": near_faces,
        "nearTiles": near_tiles,
        "gdl": {
            "loads": len(vtx_loads),
            "maxLoad": max_load,
            "anyOverflow": any_overflow,
            "totalVerts": nverts_total,
        },
        # pass-3: embedded level snapshots for Import-from-level (no server needed).
        "levelCatalog": level_catalog or {},
    }
    data_json = json.dumps(data)

    html = HTML_TEMPLATE.replace("/*__DATA__*/", data_json)
    with open(path, "w") as fp:
        fp.write(html)


HTML_TEMPLATE = r"""<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="utf-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>uff (Matrix arena) — 3D inspector</title>
<style>
  :root { --panel: rgba(16,20,28,0.88); --border:#2a3242; --muted:#8a93a6; --accent:#2c7be5; }
  html, body { margin:0; height:100%; overflow:hidden; background:#0b0e14; color:#e6e6e6;
    font:13px/1.5 -apple-system, BlinkMacSystemFont, "SF Pro Text", Helvetica, Arial, sans-serif; }
  #c { position:fixed; inset:0; display:block; width:100vw; height:100vh; }
  .panel { position:fixed; background:var(--panel); border:1px solid var(--border); border-radius:10px;
    padding:12px 14px; backdrop-filter:blur(6px); box-shadow:0 6px 24px rgba(0,0,0,0.45); }
  #hud { top:12px; left:12px; width:262px; max-height:calc(100vh - 24px); overflow-y:auto; }
  #panelRight { top:12px; right:12px; width:210px; }
  #hud h1 { font-size:14px; margin:0 0 4px; letter-spacing:.2px; }
  #hud .sub { color:var(--muted); margin:0 0 10px; font-size:12px; }
  h2 { font-size:11px; text-transform:uppercase; letter-spacing:.6px; color:var(--muted); margin:0 0 6px; }
  .row { display:flex; align-items:center; gap:8px; margin:3px 0; justify-content:space-between; }
  .row > span:first-child { display:flex; align-items:center; gap:8px; }
  .sw { width:12px; height:12px; border-radius:3px; flex:0 0 auto; border:1px solid rgba(255,255,255,.25); }
  .muted { color:var(--muted); }
  .danger { color:#ff5d5d; font-weight:600; }
  .ok { color:#57d977; font-weight:600; }
  hr { border:0; border-top:1px solid #232a37; margin:10px 0; }
  .toggle { cursor:pointer; user-select:none; display:flex; align-items:center; gap:7px; margin:4px 0; }
  .toggle input { margin:0; }
  .sub-toggles { margin-left:18px; padding-left:8px; border-left:1px solid #232a37; }
  .btns { display:grid; grid-template-columns:1fr 1fr; gap:6px; margin-bottom:8px; }
  button { font:inherit; color:#e6e6e6; background:#1b2330; border:1px solid var(--border);
    border-radius:7px; padding:6px 8px; cursor:pointer; transition:background .12s, border-color .12s; }
  button:hover { background:#243049; border-color:#3a465c; }
  button.wide { grid-column:1 / -1; }
  button.active { background:var(--accent); border-color:var(--accent); color:#fff; }
  #help { left:12px; bottom:12px; max-width:420px; font-size:12px; color:#c7cedb; }
  #help kbd { background:#1b2330; border:1px solid var(--border); border-bottom-width:2px;
    border-radius:5px; padding:1px 6px; font-family:ui-monospace,Menlo,monospace; font-size:11px; color:#e6e6e6; }
  #help .hint { color:var(--muted); margin-top:4px; }
  #info { right:12px; bottom:12px; width:240px; display:none; }
  #info .close { float:right; cursor:pointer; color:var(--muted); }
  #info h2 { margin:0 0 6px; color:#e6e6e6; text-transform:none; font-size:13px; letter-spacing:0; }
  #info code { color:#cdd6e6; }
  .small { font-size:11px; color:var(--muted); }
  code { color:#cdd6e6; }

  /* ---------- pass-2: edit-mode UI ---------- */
  /* Edit panel sits under the view/mode panel on the right; hidden until edit mode. */
  #editPanel { top:12px; right:12px; width:248px; max-height:calc(100vh - 24px);
    overflow-y:auto; display:none; }
  body.editing #editPanel { display:block; }
  /* When editing, slide the view/mode panel to the left of the edit panel so they don't stack. */
  body.editing #panelRight { right:272px; }
  #editPanel h1 { font-size:14px; margin:0 0 2px; }
  #editPanel .sub { color:var(--muted); margin:0 0 10px; font-size:12px; }
  /* Tool palette: one button per placeable pad type + a plain select tool. */
  .tools { display:grid; grid-template-columns:1fr 1fr; gap:6px; margin-bottom:6px; }
  .tools button { display:flex; align-items:center; gap:7px; justify-content:flex-start; }
  .tools .dot { width:11px; height:11px; border-radius:50%; flex:0 0 auto;
    border:1px solid rgba(255,255,255,.3); }
  /* Numeric field rows in the properties / geometry panels. */
  .field { display:flex; align-items:center; gap:8px; margin:5px 0; }
  .field label { flex:0 0 64px; color:var(--muted); }
  .field input[type=number], .field input[type=text], .field select {
    flex:1 1 auto; min-width:0; font:inherit; color:#e6e6e6; background:#10141c;
    border:1px solid var(--border); border-radius:6px; padding:4px 6px; }
  .field.xyz input { flex:1 1 0; width:0; }
  .field input[type=range] { flex:1 1 auto; }
  .field .val { flex:0 0 56px; text-align:right; color:#cdd6e6; font-variant-numeric:tabular-nums; }
  /* Properties panel (bottom-right) for the selected pad. */
  #props { right:12px; bottom:12px; width:268px; display:none; max-height:60vh; overflow-y:auto; }
  body.editing #props.shown { display:block; }
  #props h2 { color:#e6e6e6; text-transform:none; font-size:13px; letter-spacing:0; margin:0 0 6px; }
  #props .close { float:right; cursor:pointer; color:var(--muted); }
  /* Validation status line: green ok / yellow warn / red error chips. */
  #validate { font-size:12px; }
  #validate .chip { display:inline-block; padding:1px 7px; border-radius:10px; margin:2px 4px 2px 0;
    background:#1b2330; border:1px solid var(--border); }
  #validate .warn { color:#ffd166; }
  #validate .err  { color:#ff5d5d; }
  /* Export/import textarea. */
  #ioWrap, #pyWrap { display:none; margin-top:6px; }
  #ioWrap.shown, #pyWrap.shown { display:block; }
  #ioText, #pyText { width:100%; box-sizing:border-box; height:150px; resize:vertical; font:11px/1.45 ui-monospace,Menlo,monospace;
    color:#cdd6e6; background:#0c0f16; border:1px solid var(--border); border-radius:7px; padding:6px; }
  #buildCmds { margin:0; padding:8px; background:#0c0f16; border:1px solid var(--border); border-radius:7px;
    font:11px/1.5 ui-monospace,Menlo,monospace; color:#cdd6e6; white-space:pre-wrap; word-break:break-all; }
  .export-note { font-size:11px; color:var(--muted); margin:4px 0 0; line-height:1.45; }
  .editbadge { position:fixed; top:12px; left:50%; transform:translateX(-50%); z-index:5;
    background:#2c7be5; color:#fff; font-weight:700; letter-spacing:.4px; padding:5px 14px;
    border-radius:20px; box-shadow:0 4px 16px rgba(0,0,0,.4); display:none; }
  body.editing .editbadge { display:block; }
</style>
</head>
<body>
<canvas id="c"></canvas>

<div id="hud" class="panel">
  <h1>uff — Matrix combat-sim arena</h1>
  <p class="sub">±5000 box · 1 unit = 1 in-game world unit</p>
  <div id="counts"></div>
  <hr/>
  <h2>Layers</h2>
  <div id="toggles"></div>
  <hr/>
  <h2>Legend</h2>
  <div id="legend"></div>
  <hr/>
  <div id="origin"></div>
  <div id="gdl"></div>
</div>

<div id="panelRight" class="panel">
  <h2>View presets</h2>
  <div class="btns">
    <button data-view="iso">Isometric</button>
    <button data-view="top">Top</button>
    <button data-view="front">Front</button>
    <button data-view="side">Side</button>
    <button data-view="eye" class="wide">Player eye (floor)</button>
    <button id="resetBtn" class="wide">Reset view</button>
  </div>
  <h2>Mode</h2>
  <button id="flyBtn" class="wide">Enter fly mode (F)</button>
  <button id="editBtn" class="wide" style="margin-top:6px">Enter edit mode (E)</button>
  <button id="helpBtn" class="wide" style="margin-top:6px">Hide help</button>
</div>

<div class="editbadge">● EDIT MODE</div>

<!-- pass-2: map-editing panel (visible only in edit mode) -->
<div id="editPanel" class="panel">
  <h1>Map editor</h1>
  <p class="sub">Click the floor to place · click a pad to select · drag to move</p>
  <h2>Place tool</h2>
  <div class="tools" id="tools"></div>
  <label class="toggle"><input type="checkbox" id="snapChk" /> Snap to 250-unit grid</label>
  <hr/>
  <h2>Box geometry</h2>
  <div class="field"><label>Half (XZ)</label><input type="range" id="halfRange" min="500" max="12000" step="100"><span class="val" id="halfVal"></span></div>
  <div class="field"><label>Height (Y)</label><input type="range" id="heightRange" min="500" max="8000" step="100"><span class="val" id="heightVal"></span></div>
  <button id="reframeBtn" class="wide">Reframe camera to box</button>
  <hr/>
  <h2>Validation</h2>
  <div id="validate"></div>
  <hr/>
  <h2>Map I/O</h2>
  <div class="btns">
    <button id="exportBtn">Export JSON</button>
    <button id="importBtn">Import JSON</button>
    <button id="copyBtn">Copy</button>
    <button id="downloadBtn">Download</button>
  </div>
  <div id="ioWrap">
    <textarea id="ioText" spellcheck="false" placeholder="Map JSON appears here on Export. Paste JSON and press Import to load it."></textarea>
  </div>
  <hr/>
  <h2>Export to pdmap</h2>
  <div class="field"><label>Level name</label><input type="text" id="levelName" spellcheck="false" placeholder="csim" /></div>
  <div class="btns">
    <button id="exportPyBtn">Export Python</button>
    <button id="copyPyBtn">Copy Python</button>
    <button id="downloadPyBtn">Download .py</button>
  </div>
  <div id="pyWrap">
    <textarea id="pyText" spellcheck="false" readonly placeholder="Generated src/levels/&lt;name&gt;.py appears here."></textarea>
  </div>
  <p class="export-note">Save as <code>src/levels/&lt;name&gt;.py</code>, then run the build commands below. CLI: <code>python3 journal/uff_viewer/json_to_level.py map.json</code></p>
  <hr/>
  <h2>Import from level</h2>
  <div class="field"><label>Catalog</label><select id="levelSelect"></select></div>
  <button id="loadLevelBtn" class="wide">Load selected level</button>
  <p class="export-note">Embedded snapshots of repo levels, or <code>?level=csim</code> in the URL. You can also Export JSON above and re-import later.</p>
  <hr/>
  <h2>Build commands</h2>
  <pre id="buildCmds"></pre>
  <p class="export-note"><strong>Note:</strong> <code>--test-map</code> still hardcodes the <code>uff</code> stage in C unless you wire your stage into <code>title.c</code> / stagetable. Use <code>pdmap build &lt;name&gt; --deploy --seg</code> to compile assets regardless.</p>
</div>

<!-- pass-2: selected-pad properties (bottom-right, edit mode only) -->
<div id="props" class="panel">
  <span class="close" id="propsClose">✕</span>
  <div id="propsBody"></div>
</div>

<div id="help" class="panel">
  <div id="helpOrbit">
    <strong>Orbit mode</strong> &nbsp;
    <kbd>drag</kbd> rotate · <kbd>right-drag</kbd> pan · <kbd>scroll</kbd> zoom ·
    <kbd>click</kbd> a pad / face for info · <kbd>F</kbd> fly mode
  </div>
  <div id="helpFly" style="display:none">
    <strong>Fly mode</strong> &nbsp;
    <kbd>mouse</kbd> look · <kbd>W A S D</kbd> move · <kbd>Space</kbd> up · <kbd>Ctrl</kbd>/<kbd>Q</kbd> down ·
    <kbd>Shift</kbd> faster · <kbd>Esc</kbd> release · <kbd>F</kbd> exit
    <div class="hint">Click the scene to capture the mouse.</div>
  </div>
  <div id="helpEdit" style="display:none">
    <strong>Edit mode</strong> &nbsp;
    pick a <kbd>tool</kbd> then <kbd>click floor</kbd> to add · <kbd>click</kbd> a pad to select ·
    <kbd>drag</kbd> a pad to move · <kbd>Del</kbd>/<kbd>Backspace</kbd> delete · <kbd>drag</kbd> empty space to orbit · <kbd>E</kbd> exit
  </div>
</div>

<div id="info" class="panel">
  <span class="close" id="infoClose">✕</span>
  <div id="infoBody"></div>
</div>

<script type="importmap">
{ "imports": {
  "three": "https://unpkg.com/three@0.160.0/build/three.module.js",
  "three/addons/": "https://unpkg.com/three@0.160.0/examples/jsm/"
}}
</script>
<script type="module">
import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { PointerLockControls } from 'three/addons/controls/PointerLockControls.js';

const DATA = /*__DATA__*/;

const KIND_COLORS = {
  spawn:    0x57d977,  // green
  weapon:   0xffa43d,  // orange
  ammo:     0x3dc5ff,  // cyan
  scenario: 0xc77dff,  // purple
  other:    0x999999,
};
const KIND_LABEL = {
  spawn: 'Spawn', weapon: 'Weapon', ammo: 'Ammo',
  scenario: 'Scenario (case/hill)', other: 'Other',
};
const TILE_COLOR = 0x00e0c0;     // teal wireframe
const SUSPECT_COLOR = 0xff3b3b;  // RED = within originRadius

// Box dimensions are mutable in pass-2 (the geometry sliders live-resize the box).
let HALF = DATA.half;
let HEIGHT = DATA.height;
const CENTER = new THREE.Vector3(0, HEIGHT / 2, 0);
const EYE_H = 170;  // approx in-game player eye height (world units)

// ===========================================================================
// pass-2 editor: data model + catalogs
// ---------------------------------------------------------------------------
// The map editor keeps a single mutable source-of-truth array `mapState.pads`.
// A pad's INDEX is implicitly its array position, so indices are *always*
// contiguous (0..N-1) — this mirrors pdmap, where props/intro reference pads by
// array position and any gap silently breaks pickups/spawns. Every add/delete
// calls rebuildPads() which re-derives indices + 3D meshes from this array.
// ===========================================================================
const KIND_ORDER = ['spawn', 'weapon', 'ammo', 'scenario', 'other'];

// Curated weapon catalog (weaponnum enum from src/include/constants.h, the same
// IDs tools/pdmap/weapons.py uses for WeaponProp). Values are the integer IDs.
const WEAPON_CATALOG = [
  [0x02, 'Falcon 2'], [0x05, 'MagSec 4'], [0x06, 'Mauler'], [0x07, 'Phoenix'],
  [0x08, 'DY357 Magnum'], [0x09, 'DY357-LX'], [0x0a, 'CMP150'], [0x0b, 'Cyclone'],
  [0x0c, 'Callisto NTG'], [0x0d, 'RC-P120'], [0x0e, 'Laptop Gun'], [0x0f, 'Dragon'],
  [0x10, 'K7 Avenger'], [0x11, 'AR34'], [0x12, 'SuperDragon'], [0x13, 'Shotgun'],
  [0x14, 'Reaper'], [0x15, 'Sniper Rifle'], [0x16, 'FarSight XR-20'],
  [0x17, 'Devastator'], [0x18, 'Rocket Launcher'], [0x19, 'Slayer'],
  [0x1b, 'Crossbow'], [0x1c, 'Tranquilizer'],
];
// Ammo type catalog (AMMOTYPE_* from constants.h).
const AMMO_CATALOG = [
  [0x01, 'Pistol'], [0x02, 'SMG'], [0x03, 'Crossbow'], [0x04, 'Rifle'],
  [0x05, 'Shotgun'], [0x07, 'Grenade'], [0x08, 'Rocket'],
];
const SCENARIO_CATALOG = [['case', 'Capture the Case'], ['hill', 'King of the Hill']];

function weaponName(id) { const e = WEAPON_CATALOG.find(w => w[0] === id); return e ? e[1] : ('0x' + id.toString(16)); }
function ammoName(id)   { const e = AMMO_CATALOG.find(a => a[0] === id);   return e ? e[1] : ('0x' + id.toString(16)); }

// Default type-specific fields applied when a pad is created / changes type.
function defaultsForType(t) {
  if (t === 'weapon')   return { weapon: 0x11 };           // AR34
  if (t === 'ammo')     return { ammoType: 0x04, quantity: 200 }; // Rifle x200
  if (t === 'scenario') return { scenario: 'case', team: 0 };
  return {};
}

// Build the initial editor model from the read-only DATA.pads scene.
const mapState = {
  name: DATA.name || 'uff',
  pads: DATA.pads.map(p => {
    const t = (KIND_ORDER.includes(p.kind) ? p.kind : 'other');
    return Object.assign({
      type: t, x: p.pos[0], y: p.pos[1], z: p.pos[2], room: p.room,
    }, defaultsForType(t));
  }),
};

// ---------- renderer / scene / camera ----------
const canvas = document.getElementById('c');
const renderer = new THREE.WebGLRenderer({ canvas, antialias: true });
renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
renderer.autoClear = false;  // we clear manually so the corner gizmo can overlay

const scene = new THREE.Scene();
scene.background = new THREE.Color(0x0b0e14);

// Perspective camera: near/far chosen to frame a +-5000 (10000-wide) box with
// 3000 height without clipping. far is generous; near small enough to fly inside.
const camera = new THREE.PerspectiveCamera(45, 1, 1, 400000);

// Orbit controls: damped, targeted at the box CENTER, with zoom limits that fit
// the room (you can pull back to see the whole box, push in to fly through it).
const controls = new OrbitControls(camera, renderer.domElement);
controls.enableDamping = true;
controls.dampingFactor = 0.08;
controls.rotateSpeed = 0.8;
controls.panSpeed = 0.9;
controls.zoomSpeed = 0.9;
controls.target.copy(CENTER);
controls.minDistance = 60;
controls.maxDistance = HALF * 8;

// ---------- lights ----------
scene.add(new THREE.AmbientLight(0xffffff, 0.9));
const dirLight = new THREE.DirectionalLight(0xffffff, 0.65);
dirLight.position.set(1, 2, 1);
scene.add(dirLight);

// ---------- bounding sphere + framing helper ----------
const box3 = new THREE.Box3(
  new THREE.Vector3(-HALF, 0, -HALF),
  new THREE.Vector3(HALF, HEIGHT, HALF)
);
const bsphere = box3.getBoundingSphere(new THREE.Sphere());

// Distance at which the whole box fits, honoring the *narrower* of the vertical
// and horizontal FOV so it frames correctly on any window aspect ratio.
function fitDistance(margin = 1.6) {
  const vFov = THREE.MathUtils.degToRad(camera.fov);
  const hFov = 2 * Math.atan(Math.tan(vFov / 2) * camera.aspect);
  return (bsphere.radius / Math.sin(Math.min(vFov, hFov) / 2)) * margin;
}

// ---------- grid / axes / origin marker ----------
// grid + axes are rebuilt when the box is resized (pass-2), so they are `let`.
let grid = new THREE.GridHelper(HALF * 2, 20, 0x39435a, 0x222a38);
scene.add(grid);

let axes = new THREE.AxesHelper(HALF * 0.6);
scene.add(axes);

const originRing = new THREE.Mesh(
  new THREE.SphereGeometry(DATA.originRadius, 24, 16),
  new THREE.MeshBasicMaterial({ color: SUSPECT_COLOR, wireframe: true, transparent: true, opacity: 0.35 })
);
scene.add(originRing);

function nearOrigin(verts) {
  for (const v of verts) {
    if (Math.hypot(v[0], v[1], v[2]) <= DATA.originRadius) return true;
  }
  return false;
}
function centroid(v) {
  const n = v.length; let x = 0, y = 0, z = 0;
  for (const p of v) { x += p[0]; y += p[1]; z += p[2]; }
  return new THREE.Vector3(x / n, y / n, z / n);
}

// ---------- billboard labels (off by default, distance-scaled in the loop) ----------
const labelGroup = new THREE.Group();    labelGroup.visible = false;    scene.add(labelGroup);
const padLabelGroup = new THREE.Group(); padLabelGroup.visible = false; scene.add(padLabelGroup);

function roundRect(ctx, x, y, w, h, r) {
  ctx.beginPath();
  ctx.moveTo(x + r, y);
  ctx.arcTo(x + w, y, x + w, y + h, r);
  ctx.arcTo(x + w, y + h, x, y + h, r);
  ctx.arcTo(x, y + h, x, y, r);
  ctx.arcTo(x, y, x + w, y, r);
  ctx.closePath();
}
function makeLabel(text, color) {
  const cv = document.createElement('canvas');
  const ctx = cv.getContext('2d');
  ctx.font = 'bold 40px monospace';
  const w = Math.ceil(ctx.measureText(text).width) + 28;
  const h = 72;
  cv.width = w; cv.height = h;
  ctx.font = 'bold 40px monospace';
  ctx.fillStyle = 'rgba(8,10,16,0.82)';
  roundRect(ctx, 1, 1, w - 2, h - 2, 14); ctx.fill();
  ctx.fillStyle = color; ctx.textBaseline = 'middle';
  ctx.fillText(text, 14, h / 2 + 2);
  const tex = new THREE.CanvasTexture(cv);
  tex.minFilter = THREE.LinearFilter;
  const spr = new THREE.Sprite(new THREE.SpriteMaterial({ map: tex, depthTest: false, transparent: true }));
  spr.userData.aspect = w / h;
  return spr;
}

// ---------- box faces (semi-transparent, double-sided, pickable) ----------
// The arena box is the exact axis-aligned ±half / 0..height volume that the
// pdmap builder (seg._faces) produces, so we can regenerate it procedurally in
// JS for live resize without losing fidelity. Face colours/names come from the
// builder output (DATA.faces) in canonical order: floor, ceiling, -Z, +X, +Z, -X.
const faceGroup = new THREE.Group(); scene.add(faceGroup);
const faceLabelGroup = new THREE.Group(); faceLabelGroup.visible = false; scene.add(faceLabelGroup);
let pickFaces = [];

// Canonical face vertex sets for a box of the given half-extent / height.
function boxFaceVerts(h, H) {
  return [
    [[-h, 0, -h], [h, 0, -h], [h, 0, h], [-h, 0, h]],       // F0 floor   Y=0
    [[-h, H, -h], [h, H, -h], [h, H, h], [-h, H, h]],       // F1 ceiling Y=H
    [[-h, 0, -h], [h, 0, -h], [h, H, -h], [-h, H, -h]],     // F2 wall -Z
    [[h, 0, -h], [h, 0, h], [h, H, h], [h, H, -h]],         // F3 wall +X
    [[-h, 0, h], [h, 0, h], [h, H, h], [-h, H, h]],         // F4 wall +Z
    [[-h, 0, -h], [-h, 0, h], [-h, H, h], [-h, H, -h]],     // F5 wall -X
  ];
}

let boxEdges = null;
// (re)build the 6 translucent box faces + bright edge outline + face labels.
function buildBox() {
  for (const m of faceGroup.children) { m.geometry?.dispose?.(); m.material?.dispose?.(); }
  faceGroup.clear();
  for (const s of faceLabelGroup.children) { s.material?.map?.dispose?.(); s.material?.dispose?.(); }
  faceLabelGroup.clear();
  pickFaces = [];

  const vsets = boxFaceVerts(HALF, HEIGHT);
  for (let i = 0; i < vsets.length; i++) {
    const v = vsets[i];
    const meta = DATA.faces[i] || { name: 'face ' + i, hex: '#8899aa' };
    const geom = new THREE.BufferGeometry();
    geom.setAttribute('position', new THREE.BufferAttribute(new Float32Array([
      ...v[0], ...v[1], ...v[2],
      ...v[0], ...v[2], ...v[3],
    ]), 3));
    geom.computeVertexNormals();
    const suspect = nearOrigin(v);
    const col = suspect ? SUSPECT_COLOR : parseInt(meta.hex.slice(1), 16);
    // Flat (unlit) so every face reads its true colour evenly.
    const mat = new THREE.MeshBasicMaterial({
      color: col, transparent: true, opacity: suspect ? 0.5 : 0.22,
      side: THREE.DoubleSide, depthWrite: false,
    });
    const mesh = new THREE.Mesh(geom, mat);
    mesh.userData = { type: 'face', name: meta.name, hex: meta.hex, verts: v, suspect };
    faceGroup.add(mesh);
    pickFaces.push(mesh);

    const c = centroid(v);
    const lab = makeLabel(meta.name, suspect ? '#ff5d5d' : meta.hex);
    lab.position.set(c.x, c.y + 120, c.z);
    faceLabelGroup.add(lab);
  }

  // crisp, bright cube outline so the room reads clearly even with faint faces.
  boxEdges = new THREE.LineSegments(
    new THREE.EdgesGeometry(new THREE.BoxGeometry(HALF * 2, HEIGHT, HALF * 2)),
    new THREE.LineBasicMaterial({ color: 0xdbe4f5, transparent: true, opacity: 0.9 })
  );
  boxEdges.position.copy(CENTER);
  faceGroup.add(boxEdges);
}
buildBox();

// ---------- collision tiles (wireframe + faint fill) ----------
const tileGroup = new THREE.Group();
scene.add(tileGroup);
for (let ti = 0; ti < DATA.tiles.length; ti++) {
  const t = DATA.tiles[ti];
  const v = t.verts;
  const suspect = nearOrigin(v);
  const pts = v.map(p => new THREE.Vector3(p[0], p[1], p[2]));
  pts.push(pts[0].clone());
  tileGroup.add(new THREE.Line(
    new THREE.BufferGeometry().setFromPoints(pts),
    new THREE.LineBasicMaterial({ color: suspect ? SUSPECT_COLOR : TILE_COLOR })
  ));
  if (v.length >= 3) {
    const arr = [];
    for (let k = 1; k < v.length - 1; k++) { arr.push(...v[0], ...v[k], ...v[k + 1]); }
    const fg = new THREE.BufferGeometry();
    fg.setAttribute('position', new THREE.BufferAttribute(new Float32Array(arr), 3));
    fg.computeVertexNormals();
    tileGroup.add(new THREE.Mesh(fg, new THREE.MeshBasicMaterial({
      color: suspect ? SUSPECT_COLOR : TILE_COLOR, transparent: true,
      opacity: suspect ? 0.4 : 0.10, side: THREE.DoubleSide, depthWrite: false,
    })));
  }
  const c = centroid(v);
  const lab = makeLabel('tile ' + ti + ' · ' + t.room, suspect ? '#ff5d5d' : '#00e0c0');
  lab.position.set(c.x, 80, c.z);
  labelGroup.add(lab);
}

// ---------- pads (rebuilt from mapState; grouped by kind so each kind can toggle) ----------
const padRoot = new THREE.Group();
scene.add(padRoot);
const padGroups = {};
for (const k of KIND_ORDER) { padGroups[k] = new THREE.Group(); padRoot.add(padGroups[k]); }
const padKindVisible = {}; for (const k of KIND_ORDER) padKindVisible[k] = true;
const padGeom = new THREE.SphereGeometry(90, 16, 12);
let pickPads = [];            // pad meshes for raycasting (rebuilt each time)
let selectedIndex = -1;       // index into mapState.pads, or -1 = none

// Selection marker: a bright wireframe sphere that follows the selected pad.
const selectionMarker = new THREE.Mesh(
  new THREE.SphereGeometry(150, 18, 12),
  new THREE.MeshBasicMaterial({ color: 0xffffff, wireframe: true, transparent: true, opacity: 0.85, depthTest: false })
);
selectionMarker.visible = false;
scene.add(selectionMarker);

// Regenerate every pad mesh + label from mapState.pads. This is the ONE place
// indices are assigned (index = array position) so they are always contiguous.
function rebuildPads() {
  for (const k of KIND_ORDER) {
    for (const m of padGroups[k].children) { m.material?.dispose?.(); }
    padGroups[k].clear();
  }
  for (const s of padLabelGroup.children) { s.material?.map?.dispose?.(); s.material?.dispose?.(); }
  padLabelGroup.clear();
  pickPads = [];

  mapState.pads.forEach((p, i) => {
    p.index = i;  // enforce contiguous index == array position
    const kind = KIND_ORDER.includes(p.type) ? p.type : 'other';
    const suspect = Math.hypot(p.x, p.y, p.z) <= DATA.originRadius;
    const col = suspect ? SUSPECT_COLOR : (KIND_COLORS[kind] || 0xffffff);
    const m = new THREE.Mesh(padGeom, new THREE.MeshBasicMaterial({ color: col }));
    m.position.set(p.x, p.y, p.z);
    m.userData = { type: 'pad', index: i, kind, ref: p, suspect };
    padGroups[kind].add(m);
    pickPads.push(m);

    const lab = makeLabel('p' + i, '#cdd6e6');
    lab.position.set(p.x, p.y + 110, p.z);
    padLabelGroup.add(lab);
  });

  for (const k of KIND_ORDER) padGroups[k].visible = padKindVisible[k];
  // keep selection valid + marker positioned
  if (selectedIndex >= mapState.pads.length) selectedIndex = -1;
  updateSelectionMarker();
  if (typeof updateValidation === 'function') updateValidation();
  if (typeof updateCounts === 'function') updateCounts();
}

function updateSelectionMarker() {
  if (selectedIndex < 0 || selectedIndex >= mapState.pads.length) { selectionMarker.visible = false; return; }
  const p = mapState.pads[selectedIndex];
  selectionMarker.position.set(p.x, p.y, p.z);
  selectionMarker.visible = padRoot.visible;
}
rebuildPads();

// ---------- HUD: counts + legend ----------
// counts reflect the live editor model + current box size (pass-2).
function updateCounts() {
  document.getElementById('counts').innerHTML =
    `<div class="row"><span>Box faces</span><span class="muted">${DATA.faces.length}</span></div>` +
    `<div class="row"><span>Collision tiles</span><span class="muted">${DATA.tiles.length}</span></div>` +
    `<div class="row"><span>Pads</span><span class="muted">${mapState.pads.length}</span></div>` +
    `<div class="row"><span>Box extent</span><span class="muted">±${HALF} XZ · 0..${HEIGHT} Y</span></div>`;
}
updateCounts();

function legendRow(color, label) {
  const hex = '#' + color.toString(16).padStart(6, '0');
  return `<div class="row"><span><span class="sw" style="background:${hex}"></span>${label}</span></div>`;
}
document.getElementById('legend').innerHTML =
  legendRow(KIND_COLORS.spawn, 'Spawn pad') +
  legendRow(KIND_COLORS.weapon, 'Weapon pad') +
  legendRow(KIND_COLORS.ammo, 'Ammo pad') +
  legendRow(KIND_COLORS.scenario, 'Scenario pad (case/hill)') +
  legendRow(TILE_COLOR, 'Collision tile') +
  legendRow(SUSPECT_COLOR, `Within ${DATA.originRadius} of origin`);

// ---------- HUD: layer toggles (with pad sub-toggles by kind) ----------
const togglesEl = document.getElementById('toggles');
function addToggle(parent, label, checked, onChange) {
  const lab = document.createElement('label'); lab.className = 'toggle';
  const cb = document.createElement('input'); cb.type = 'checkbox'; cb.checked = checked;
  cb.addEventListener('change', () => onChange(cb.checked));
  lab.appendChild(cb); lab.appendChild(document.createTextNode(label));
  parent.appendChild(lab);
  onChange(checked);  // apply initial state
  return cb;
}
addToggle(togglesEl, 'Box faces', true, v => { faceGroup.visible = v; });
addToggle(togglesEl, 'Collision tiles', true, v => { tileGroup.visible = v; });
addToggle(togglesEl, 'Pads', true, v => { padRoot.visible = v; updateSelectionMarker(); });
const subWrap = document.createElement('div'); subWrap.className = 'sub-toggles'; togglesEl.appendChild(subWrap);
// Sub-toggles for every pad kind (not just present ones) so visibility persists
// across edits that add/remove a kind.
for (const k of KIND_ORDER) addToggle(subWrap, KIND_LABEL[k] || k, true, v => { padKindVisible[k] = v; padGroups[k].visible = v; });
addToggle(togglesEl, 'Labels (faces / tiles)', false, v => { faceLabelGroup.visible = v; labelGroup.visible = v; });
addToggle(togglesEl, 'Pad IDs', false, v => { padLabelGroup.visible = v; });
addToggle(togglesEl, 'Grid', true, v => { grid.visible = v; });
addToggle(togglesEl, 'Axes', true, v => { axes.visible = v; });
addToggle(togglesEl, 'Origin ' + DATA.originRadius + ' sphere', true, v => { originRing.visible = v; });

// ---------- HUD: origin proximity + GDL diagnostics ----------
const nearTxt = (DATA.nearFaces.length || DATA.nearTiles.length)
  ? `<span class="danger">FACES: ${DATA.nearFaces.join(', ') || 'none'} · TILES: ${DATA.nearTiles.join(', ') || 'none'}</span>`
  : `<span class="ok">none near origin</span>`;
document.getElementById('origin').innerHTML =
  `<div class="row"><span>Near origin</span></div><div>${nearTxt}</div>`;

const g = DATA.gdl;
const gdlTxt = g.anyOverflow
  ? `<span class="danger">G_VTX OVERFLOW: a load exceeds 16 verts → phantom-surface suspect</span>`
  : `<span class="ok">max G_VTX load = ${g.maxLoad} verts (≤16, no nibble overflow)</span>`;
document.getElementById('gdl').innerHTML =
  `<hr/><div class="row"><span>GDL G_VTX loads</span><span class="muted">${g.loads} · ${g.totalVerts} verts</span></div><div>${gdlTxt}</div>`;

// ---------- camera tween + view presets ----------
let tween = null;
function moveCamera(pos, look, dur = 650) {
  tween = {
    fp: camera.position.clone(), tp: pos.clone(),
    ft: controls.target.clone(), tt: look.clone(),
    t0: performance.now(), dur,
  };
}
function easeInOut(x) { return x < 0.5 ? 2 * x * x : 1 - Math.pow(-2 * x + 2, 2) / 2; }

const PRESET_DIRS = {
  iso:   new THREE.Vector3(1, 0.42, 1),
  top:   new THREE.Vector3(0, 1, 0.0001),
  front: new THREE.Vector3(0, 0.12, 1),
  side:  new THREE.Vector3(1, 0.12, 0),
};
function presetView(name) {
  if (mode !== 'orbit') setMode('orbit');
  if (name === 'eye') {
    // camera near the floor at room center, looking outward toward the +Z wall
    moveCamera(new THREE.Vector3(0, EYE_H, 0), new THREE.Vector3(0, EYE_H, HALF), 700);
    setActiveView('eye');
    return;
  }
  const dir = (PRESET_DIRS[name] || PRESET_DIRS.iso).clone().normalize();
  moveCamera(CENTER.clone().add(dir.multiplyScalar(fitDistance())), CENTER.clone());
  setActiveView(name);
}
function setActiveView(name) {
  document.querySelectorAll('[data-view]').forEach(b =>
    b.classList.toggle('active', b.dataset.view === name && mode === 'orbit'));
}

// ---------- fly / first-person mode (pointer-lock + WASD) ----------
const fly = new PointerLockControls(camera, renderer.domElement);
let mode = 'orbit';
const keys = {};
const clock = new THREE.Clock();
const flyBtn = document.getElementById('flyBtn');

function setMode(m) {
  if (m === mode) return;
  mode = m;
  if (m === 'fly') {
    setEditing(false);  // fly and edit are mutually exclusive (function is hoisted)
    tween = null;
    controls.enabled = false;
    document.getElementById('helpOrbit').style.display = 'none';
    document.getElementById('helpFly').style.display = '';
    flyBtn.textContent = 'Exit fly mode (F)';
    flyBtn.classList.add('active');
    setActiveView(null);
    try { fly.lock(); } catch (e) { /* pointer lock unavailable (e.g. headless) */ }
  } else {
    controls.enabled = true;
    if (fly.isLocked) fly.unlock();
    // re-anchor the orbit target a bit ahead of where the camera is now
    const fwd = new THREE.Vector3();
    camera.getWorldDirection(fwd);
    controls.target.copy(camera.position).add(fwd.multiplyScalar(900));
    document.getElementById('helpOrbit').style.display = '';
    document.getElementById('helpFly').style.display = 'none';
    flyBtn.textContent = 'Enter fly mode (F)';
    flyBtn.classList.remove('active');
  }
}
function updateFly(dt) {
  const fast = keys['ShiftLeft'] || keys['ShiftRight'];
  const step = (fast ? 9000 : 3200) * dt;
  if (keys['KeyW']) fly.moveForward(step);
  if (keys['KeyS']) fly.moveForward(-step);
  if (keys['KeyA']) fly.moveRight(-step);
  if (keys['KeyD']) fly.moveRight(step);
  if (keys['Space']) camera.position.y += step;  // E is reserved for edit mode (pass-2)
  if (keys['ControlLeft'] || keys['ControlRight'] || keys['KeyQ']) camera.position.y -= step;
}

addEventListener('keydown', e => {
  // Ignore shortcuts while typing in an editor input/textarea/select.
  const tag = (e.target && e.target.tagName) || '';
  const typing = tag === 'INPUT' || tag === 'TEXTAREA' || tag === 'SELECT';
  if (!typing) {
    if (e.code === 'KeyF' && !e.repeat) { setMode(mode === 'fly' ? 'orbit' : 'fly'); }
    if (e.code === 'KeyE' && !e.repeat) { setEditing(!editing); }
    if ((e.code === 'Delete' || e.code === 'Backspace') && editing && selectedIndex >= 0) {
      e.preventDefault(); deletePad();
    }
  }
  if (mode === 'fly' && (e.code === 'Space' || e.code.startsWith('Control'))) e.preventDefault();
  keys[e.code] = true;
});
addEventListener('keyup', e => { keys[e.code] = false; });

// re-lock the pointer if the user pressed Esc but is still in fly mode
canvas.addEventListener('click', () => { if (mode === 'fly' && !fly.isLocked) { try { fly.lock(); } catch (e) {} } });

// ---------- click-to-inspect (orbit mode) ----------
const raycaster = new THREE.Raycaster();
const ndc = new THREE.Vector2();
let downXY = null;
canvas.addEventListener('pointerdown', e => { downXY = { x: e.clientX, y: e.clientY }; });
canvas.addEventListener('pointerup', e => {
  if (editing) return;  // edit-mode pointer handling lives in the editor block
  if (mode !== 'orbit' || !downXY) { downXY = null; return; }
  const moved = Math.hypot(e.clientX - downXY.x, e.clientY - downXY.y);
  downXY = null;
  if (moved > 5) return;  // it was a drag, not a click
  ndc.x = (e.clientX / window.innerWidth) * 2 - 1;
  ndc.y = -(e.clientY / window.innerHeight) * 2 + 1;
  raycaster.setFromCamera(ndc, camera);
  let hits = raycaster.intersectObjects(pickPads, false);  // pads take priority
  if (!hits.length) hits = raycaster.intersectObjects(pickFaces, false);
  if (hits.length) showInfo(hits[0].object.userData);
});
function showInfo(u) {
  const el = document.getElementById('infoBody');
  if (u.type === 'pad') {
    const p = u.ref;
    el.innerHTML =
      `<h2>Pad ${u.index}</h2>` +
      `<div class="row"><span>Type</span><span class="muted">${KIND_LABEL[u.kind] || u.kind}</span></div>` +
      `<div class="row"><span>Room</span><span class="muted">${p.room}</span></div>` +
      `<div class="row"><span>Position</span><span class="muted"><code>${Math.round(p.x)}, ${Math.round(p.y)}, ${Math.round(p.z)}</code></span></div>` +
      (u.suspect ? `<div class="danger small">within ${DATA.originRadius} of origin</div>` : '');
  } else {
    el.innerHTML =
      `<h2>${u.name}</h2>` +
      `<div class="row"><span>Colour</span><span><span class="sw" style="background:${u.hex}"></span><span class="muted">${u.hex}</span></span></div>` +
      `<div class="small" style="margin-top:4px">vertices:</div>` +
      u.verts.map(v => `<div class="small"><code>[${v[0]}, ${v[1]}, ${v[2]}]</code></div>`).join('') +
      (u.suspect ? `<div class="danger small">within ${DATA.originRadius} of origin</div>` : '');
  }
  document.getElementById('info').style.display = 'block';
}
document.getElementById('infoClose').onclick = () => { document.getElementById('info').style.display = 'none'; };

// ---------- button wiring ----------
flyBtn.onclick = () => setMode(mode === 'fly' ? 'orbit' : 'fly');
document.getElementById('resetBtn').onclick = () => presetView('iso');
document.querySelectorAll('[data-view]').forEach(b => b.onclick = () => presetView(b.dataset.view));
const helpBtn = document.getElementById('helpBtn');
const helpPanel = document.getElementById('help');
helpBtn.onclick = () => {
  const hidden = helpPanel.style.display === 'none';
  helpPanel.style.display = hidden ? '' : 'none';
  helpBtn.textContent = hidden ? 'Hide help' : 'Show help';
};

// ===========================================================================
// pass-2 EDITOR: edit mode, place/select/drag/delete, properties, geometry,
// validation, JSON import/export.
// ===========================================================================
let editing = false;
let activeTool = null;        // null = select/move; else 'spawn'|'weapon'|'ammo'|'scenario'
let dragging = false;         // currently dragging the selected pad on the floor
let editDownXY = null;        // pointer-down screen pos (to distinguish click vs orbit-drag)
const FLOOR_PLANE = new THREE.Plane(new THREE.Vector3(0, 1, 0), 0);  // Y=0 floor
const editBtn = document.getElementById('editBtn');
const snapChk = document.getElementById('snapChk');

const rnd = v => Math.round(v);

// ---- mode toggle ----------------------------------------------------------
function setEditing(on) {
  if (on === editing) return;
  editing = on;
  document.body.classList.toggle('editing', on);
  editBtn.textContent = on ? 'Exit edit mode (E)' : 'Enter edit mode (E)';
  editBtn.classList.toggle('active', on);
  if (on && mode === 'fly') setMode('orbit');
  document.getElementById('helpEdit').style.display = on ? '' : 'none';
  document.getElementById('helpOrbit').style.display = (!on && mode === 'orbit') ? '' : 'none';
  if (on) document.getElementById('info').style.display = 'none';  // hide inspect popup
  else { selectPad(-1); canvas.style.cursor = 'default'; }
}
editBtn.onclick = () => setEditing(!editing);

// ---- tool palette ---------------------------------------------------------
const toolsEl = document.getElementById('tools');
const toolBtns = {};
const TOOL_DEFS = [['spawn', 'Spawn'], ['weapon', 'Weapon'], ['ammo', 'Ammo'], ['scenario', 'Scenario']];
(function buildTools() {
  const selBtn = document.createElement('button');
  selBtn.textContent = 'Select / Move'; selBtn.style.gridColumn = '1 / -1';
  selBtn.onclick = () => setTool(null);
  toolsEl.appendChild(selBtn);
  toolBtns['_select'] = selBtn;
  for (const [t, label] of TOOL_DEFS) {
    const b = document.createElement('button');
    const dot = document.createElement('span'); dot.className = 'dot';
    dot.style.background = '#' + (KIND_COLORS[t] || 0xffffff).toString(16).padStart(6, '0');
    b.appendChild(dot); b.appendChild(document.createTextNode(label));
    b.onclick = () => setTool(t);
    toolBtns[t] = b; toolsEl.appendChild(b);
  }
  setTool(null);
})();
function setTool(t) {
  activeTool = (t === activeTool) ? null : t;  // clicking the active tool returns to select
  for (const k of Object.keys(toolBtns)) toolBtns[k].classList.remove('active');
  toolBtns[activeTool || '_select'].classList.add('active');
  canvas.style.cursor = (editing && activeTool) ? 'crosshair' : 'default';
}

// ---- raycast helpers ------------------------------------------------------
function setNdc(e) {
  ndc.x = (e.clientX / window.innerWidth) * 2 - 1;
  ndc.y = -(e.clientY / window.innerHeight) * 2 + 1;
  raycaster.setFromCamera(ndc, camera);
}
function padMeshAt(e) {
  setNdc(e);
  const hits = raycaster.intersectObjects(pickPads, false);
  return hits.length ? hits[0].object : null;
}
function floorPointAt(e) {
  setNdc(e);
  const pt = new THREE.Vector3();
  return raycaster.ray.intersectPlane(FLOOR_PLANE, pt) ? pt : null;
}
function snapClamp(pt) {
  let x = THREE.MathUtils.clamp(pt.x, -HALF, HALF);
  let z = THREE.MathUtils.clamp(pt.z, -HALF, HALF);
  if (snapChk.checked) { x = Math.round(x / 250) * 250; z = Math.round(z / 250) * 250; }
  return { x, z };
}

// ---- model ops (add / delete / select) ------------------------------------
function defaultRoom() {
  if (selectedIndex >= 0) return mapState.pads[selectedIndex].room;
  if (mapState.pads.length) return mapState.pads[0].room;
  return 1;
}
function addPad(type, pt) {
  const s = snapClamp(pt);
  const pad = Object.assign({ type, x: rnd(s.x), y: 0, z: rnd(s.z), room: defaultRoom() }, defaultsForType(type));
  mapState.pads.push(pad);
  rebuildPads();               // reassigns contiguous indices + meshes
  selectPad(mapState.pads.length - 1);
}
function deletePad() {
  if (selectedIndex < 0) return;
  mapState.pads.splice(selectedIndex, 1);  // splice keeps the array dense → indices stay 0..N-1
  selectedIndex = -1;
  rebuildPads();
  renderProps();
}
function selectPad(i) {
  selectedIndex = (i >= 0 && i < mapState.pads.length) ? i : -1;
  updateSelectionMarker();
  renderProps();
}

// Live-sync only the selected pad's mesh/label/marker during a drag (cheap).
function syncSelectedMesh() {
  const i = selectedIndex; if (i < 0 || i >= mapState.pads.length) return;
  const p = mapState.pads[i];
  const mesh = pickPads[i];
  if (mesh) {
    mesh.position.set(p.x, p.y, p.z);
    const kind = KIND_ORDER.includes(p.type) ? p.type : 'other';
    const suspect = Math.hypot(p.x, p.y, p.z) <= DATA.originRadius;
    mesh.material.color.set(suspect ? SUSPECT_COLOR : (KIND_COLORS[kind] || 0xffffff));
  }
  const lab = padLabelGroup.children[i]; if (lab) lab.position.set(p.x, p.y + 110, p.z);
  selectionMarker.position.set(p.x, p.y, p.z);
}

// ---- pointer interactions (capture phase so we win over OrbitControls) -----
canvas.addEventListener('pointerdown', e => {
  if (!editing || mode !== 'orbit' || e.button !== 0) return;
  editDownXY = { x: e.clientX, y: e.clientY };
  const hit = padMeshAt(e);
  if (hit) {
    selectPad(hit.userData.index);
    dragging = true;
    controls.enabled = false;  // OrbitControls.onPointerDown bails when disabled
  }
}, true);

canvas.addEventListener('pointermove', e => {
  if (!editing || !dragging) return;
  const pt = floorPointAt(e); if (!pt) return;
  const s = snapClamp(pt);
  const p = mapState.pads[selectedIndex];
  p.x = rnd(s.x); p.z = rnd(s.z);  // keep Y (floor height) while dragging on the plane
  syncSelectedMesh();
  refreshPropsLive();
  updateValidation();
}, true);

canvas.addEventListener('pointerup', e => {
  if (!editing || mode !== 'orbit') return;
  if (dragging) { dragging = false; controls.enabled = true; renderProps(); return; }
  if (!editDownXY) return;
  const moved = Math.hypot(e.clientX - editDownXY.x, e.clientY - editDownXY.y);
  editDownXY = null;
  if (moved > 5) return;            // was an orbit drag on empty space
  const hit = padMeshAt(e);
  if (hit) { selectPad(hit.userData.index); return; }
  if (activeTool) { const pt = floorPointAt(e); if (pt) addPad(activeTool, pt); }
  else selectPad(-1);               // click empty in select mode → deselect
}, true);

// ---- properties panel -----------------------------------------------------
const propsPanel = document.getElementById('props');
document.getElementById('propsClose').onclick = () => selectPad(-1);

function optionList(pairs, sel) {
  return pairs.map(([v, label]) =>
    `<option value="${v}"${String(v) === String(sel) ? ' selected' : ''}>${label}</option>`).join('');
}
function fieldSelect(label, id, pairs, sel) {
  return `<div class="field"><label>${label}</label><select id="${id}">${optionList(pairs, sel)}</select></div>`;
}
function renderProps() {
  if (selectedIndex < 0) { propsPanel.classList.remove('shown'); return; }
  const p = mapState.pads[selectedIndex];
  const body = document.getElementById('propsBody');
  let html = `<span class="close" id="propsClose2">✕</span>` +
    `<h2>Pad ${selectedIndex} &nbsp;<span class="muted">${KIND_LABEL[p.type] || p.type}</span></h2>`;
  html += fieldSelect('Type', 'pType', KIND_ORDER.map(k => [k, KIND_LABEL[k] || k]), p.type);
  html += `<div class="field xyz"><label>X / Y / Z</label>` +
    `<input type="number" id="pX" step="10" value="${rnd(p.x)}">` +
    `<input type="number" id="pY" step="10" value="${rnd(p.y)}">` +
    `<input type="number" id="pZ" step="10" value="${rnd(p.z)}"></div>`;
  html += `<div class="field"><label>Room</label><input type="number" id="pRoom" step="1" value="${p.room}"></div>`;
  if (p.type === 'weapon') {
    html += fieldSelect('Weapon', 'pWeapon', WEAPON_CATALOG.map(([id, n]) => [id, n + '  (0x' + id.toString(16) + ')']), p.weapon);
  } else if (p.type === 'ammo') {
    html += fieldSelect('Ammo', 'pAmmo', AMMO_CATALOG.map(([id, n]) => [id, n + '  (0x' + id.toString(16) + ')']), p.ammoType);
    html += `<div class="field"><label>Quantity</label><input type="number" id="pQty" step="10" value="${p.quantity}"></div>`;
  } else if (p.type === 'scenario') {
    html += fieldSelect('Mode', 'pScenario', SCENARIO_CATALOG, p.scenario);
    html += `<div class="field"><label>Team</label><input type="number" id="pTeam" step="1" value="${p.team}"></div>`;
  }
  html += `<button id="delPadBtn" class="wide" style="margin-top:8px">Delete pad (Del)</button>`;
  const warns = [];
  if (Math.hypot(p.x, p.y, p.z) <= DATA.originRadius) warns.push(`within ${DATA.originRadius} of origin`);
  if (p.y < 0) warns.push('Y &lt; 0 — pad is below the floor');
  if (Math.abs(p.x) > HALF || Math.abs(p.z) > HALF) warns.push('outside the box footprint');
  if (warns.length) html += `<div class="danger small" style="margin-top:6px">⚠ ${warns.join(' · ')}</div>`;
  body.innerHTML = html;
  propsPanel.classList.add('shown');

  document.getElementById('propsClose2').onclick = () => selectPad(-1);
  // live-binding helpers
  const onPos = () => {
    p.x = parseFloat(document.getElementById('pX').value) || 0;
    p.y = parseFloat(document.getElementById('pY').value) || 0;
    p.z = parseFloat(document.getElementById('pZ').value) || 0;
    syncSelectedMesh(); updateValidation();
  };
  document.getElementById('pX').addEventListener('input', onPos);
  document.getElementById('pY').addEventListener('input', onPos);
  document.getElementById('pZ').addEventListener('input', onPos);
  document.getElementById('pRoom').addEventListener('input', () => {
    p.room = parseInt(document.getElementById('pRoom').value, 10) || 0; updateValidation();
  });
  document.getElementById('pType').addEventListener('change', () => {
    p.type = document.getElementById('pType').value;
    // merge in any missing type-specific defaults without clobbering shared fields
    const d = defaultsForType(p.type);
    for (const k in d) if (!(k in p)) p[k] = d[k];
    rebuildPads(); renderProps();
  });
  const wpn = document.getElementById('pWeapon');
  if (wpn) wpn.addEventListener('change', () => { p.weapon = parseInt(wpn.value, 10); });
  const amm = document.getElementById('pAmmo');
  if (amm) amm.addEventListener('change', () => { p.ammoType = parseInt(amm.value, 10); });
  const qty = document.getElementById('pQty');
  if (qty) qty.addEventListener('input', () => { p.quantity = parseInt(qty.value, 10) || 0; });
  const scn = document.getElementById('pScenario');
  if (scn) scn.addEventListener('change', () => { p.scenario = scn.value; });
  const team = document.getElementById('pTeam');
  if (team) team.addEventListener('input', () => { p.team = parseInt(team.value, 10) || 0; });
  document.getElementById('delPadBtn').onclick = () => deletePad();
}
// During a drag, just push the new X/Z into the (already open) position inputs.
function refreshPropsLive() {
  if (selectedIndex < 0) return;
  const p = mapState.pads[selectedIndex];
  const x = document.getElementById('pX'), z = document.getElementById('pZ');
  if (x) x.value = rnd(p.x); if (z) z.value = rnd(p.z);
}

// ---- box geometry controls ------------------------------------------------
const halfRange = document.getElementById('halfRange');
const heightRange = document.getElementById('heightRange');
const halfVal = document.getElementById('halfVal');
const heightVal = document.getElementById('heightVal');
halfRange.value = HALF; heightRange.value = HEIGHT;
halfVal.textContent = HALF; heightVal.textContent = HEIGHT;

// Live-resize the box: rebuild faces/edges/grid/axes + reframe references.
function applyBoxSize(half, height) {
  HALF = half; HEIGHT = height;
  CENTER.set(0, HEIGHT / 2, 0);
  buildBox();
  const gv = grid.visible, av = axes.visible;
  scene.remove(grid); grid.geometry.dispose(); grid.material.dispose();
  grid = new THREE.GridHelper(HALF * 2, 20, 0x39435a, 0x222a38); grid.visible = gv; scene.add(grid);
  scene.remove(axes); axes.geometry.dispose(); axes.material.dispose();
  axes = new THREE.AxesHelper(HALF * 0.6); axes.visible = av; scene.add(axes);
  box3.min.set(-HALF, 0, -HALF); box3.max.set(HALF, HEIGHT, HALF);
  box3.getBoundingSphere(bsphere);
  controls.maxDistance = HALF * 8;
  halfVal.textContent = HALF; heightVal.textContent = HEIGHT;
  updateCounts(); updateValidation();
}
halfRange.addEventListener('input', () => applyBoxSize(parseFloat(halfRange.value), HEIGHT));
heightRange.addEventListener('input', () => applyBoxSize(HALF, parseFloat(heightRange.value)));
document.getElementById('reframeBtn').onclick = () => presetView('iso');

// ---- validation status ----------------------------------------------------
function updateValidation() {
  const el = document.getElementById('validate');
  if (!el) return;
  const pads = mapState.pads;
  const c = { spawn: 0, weapon: 0, ammo: 0, scenario: 0, other: 0 };
  let belowFloor = 0, outside = 0, badWeapon = 0, onFloor = 0;
  for (const p of pads) {
    c[KIND_ORDER.includes(p.type) ? p.type : 'other']++;
    if (p.y < 0) belowFloor++;
    else if (p.y === 0 && (p.type === 'spawn' || p.type === 'weapon' || p.type === 'ammo')) onFloor++;
    if (Math.abs(p.x) > HALF || Math.abs(p.z) > HALF) outside++;
    if (p.type === 'weapon' && p.weapon == null) badWeapon++;
  }
  const chips = [
    `<span class="chip">${pads.length} pads</span>`,
    `<span class="chip">${c.spawn} spawn</span>`,
    `<span class="chip">${c.weapon} wpn</span>`,
    `<span class="chip">${c.ammo} ammo</span>`,
    `<span class="chip">${c.scenario} scen</span>`,
    `<span class="chip" style="color:#57d977">idx 0..${Math.max(0, pads.length - 1)} contiguous</span>`,
  ];
  const warns = [];
  if (c.spawn === 0) warns.push(`<span class="chip err">⚠ 0 spawns — players cannot spawn</span>`);
  else if (c.spawn < 4) warns.push(`<span class="chip warn">only ${c.spawn} spawn(s); want ≥ player count</span>`);
  if (belowFloor) warns.push(`<span class="chip err">${belowFloor} pad(s) below floor (Y&lt;0)</span>`);
  if (onFloor) warns.push(`<span class="chip warn">${onFloor} pad(s) at Y=0 — use Y≥10 (SPAWN_Y) or players fall through</span>`);
  if (outside) warns.push(`<span class="chip warn">${outside} pad(s) outside box XZ</span>`);
  if (badWeapon) warns.push(`<span class="chip err">${badWeapon} weapon pad(s) missing weapon id</span>`);
  el.innerHTML = chips.join('') + (warns.length ? '<div style="margin-top:4px">' + warns.join(' ') + '</div>' : '');
}

// ---- JSON import / export -------------------------------------------------
// SCHEMA (pass-3 target): {
//   name: string, box_half: number, box_height: number,
//   pads: [ { index, type, x, y, z, room,
//             // weapon:   weapon (int id), weaponName (hint)
//             // ammo:     ammoType (int id), ammoName (hint), quantity
//             // scenario: scenario ('case'|'hill'), team } ]
// }
// `index` always equals the array position (contiguous 0..N-1).
function serializeMap() {
  return {
    name: mapState.name,
    box_half: HALF,
    box_height: HEIGHT,
    pads: mapState.pads.map((p, i) => {
      const o = { index: i, type: p.type, x: rnd(p.x), y: rnd(p.y), z: rnd(p.z), room: p.room };
      if (p.type === 'weapon') { o.weapon = p.weapon; o.weaponName = weaponName(p.weapon); }
      else if (p.type === 'ammo') { o.ammoType = p.ammoType; o.ammoName = ammoName(p.ammoType); o.quantity = p.quantity; }
      else if (p.type === 'scenario') { o.scenario = p.scenario; o.team = p.team; }
      return o;
    }),
  };
}
const ioWrap = document.getElementById('ioWrap');
const ioText = document.getElementById('ioText');
function exportJSON() {
  const txt = JSON.stringify(serializeMap(), null, 2);
  ioText.value = txt;
  ioWrap.classList.add('shown');
  return txt;
}
function loadMap(obj) {
  if (!obj || !Array.isArray(obj.pads)) { alert('Map JSON must have a pads[] array.'); return false; }
  mapState.name = obj.name || mapState.name;
  if (Number.isFinite(+obj.box_half) && Number.isFinite(+obj.box_height)) {
    halfRange.value = +obj.box_half; heightRange.value = +obj.box_height;
    applyBoxSize(+obj.box_half, +obj.box_height);
  }
  mapState.pads = obj.pads.map(p => {
    const t = KIND_ORDER.includes(p.type) ? p.type : 'other';
    const pad = Object.assign({ type: t, x: +p.x || 0, y: +p.y || 0, z: +p.z || 0, room: (p.room | 0) }, defaultsForType(t));
    if (t === 'weapon' && p.weapon != null) pad.weapon = +p.weapon;
    if (t === 'ammo') { if (p.ammoType != null) pad.ammoType = +p.ammoType; if (p.quantity != null) pad.quantity = +p.quantity; }
    if (t === 'scenario') { if (p.scenario) pad.scenario = p.scenario; if (p.team != null) pad.team = +p.team; }
    return pad;
  });
  selectedIndex = -1;
  rebuildPads();
  renderProps();
  return true;
}
document.getElementById('exportBtn').onclick = exportJSON;
document.getElementById('importBtn').onclick = () => {
  if (!ioWrap.classList.contains('shown')) { ioWrap.classList.add('shown'); ioText.focus(); return; }
  let obj; try { obj = JSON.parse(ioText.value); } catch (err) { alert('Invalid JSON: ' + err.message); return; }
  loadMap(obj);
};
document.getElementById('copyBtn').onclick = () => {
  const txt = exportJSON();
  if (navigator.clipboard) navigator.clipboard.writeText(txt).catch(() => {});
  else { ioText.select(); document.execCommand('copy'); }
};
document.getElementById('downloadBtn').onclick = () => {
  const txt = exportJSON();
  const blob = new Blob([txt], { type: 'application/json' });
  const a = document.createElement('a');
  a.href = URL.createObjectURL(blob);
  a.download = (mapState.name || 'map') + '_map.json';
  document.body.appendChild(a); a.click(); a.remove();
  setTimeout(() => URL.revokeObjectURL(a.href), 1000);
};

// ===========================================================================
// pass-3: Export to pdmap (Python level module), import from level catalog,
// build-command panel.
// ===========================================================================
const LEVEL_CATALOG = DATA.levelCatalog || {};
const levelNameInput = document.getElementById('levelName');
const pyWrap = document.getElementById('pyWrap');
const pyText = document.getElementById('pyText');
const buildCmdsEl = document.getElementById('buildCmds');
const levelSelect = document.getElementById('levelSelect');

// Weapon / ammo ID → W.* constant (mirrors journal/uff_viewer/json_to_level.py).
const WEAPON_CONST = {
  0x02: 'W.WEAPON_FALCON2', 0x05: 'W.WEAPON_MAGSEC4', 0x06: 'W.WEAPON_MAULER',
  0x0a: 'W.WEAPON_CMP150', 0x0e: 'W.WEAPON_LAPTOPGUN', 0x12: 'W.WEAPON_SUPERDRAGON',
  0x15: 'W.WEAPON_SNIPERRIFLE', 0x18: 'W.WEAPON_ROCKETLAUNCHER',
  0x1b: 'W.WEAPON_CROSSBOW', 0x1c: 'W.WEAPON_TRANQUILIZER',
};
const AMMO_CONST = {
  0x01: 'W.AMMOTYPE_PISTOL', 0x03: 'W.AMMOTYPE_RIFLE',
  0x04: 'W.AMMOTYPE_SHOTGUN', 0x05: 'W.AMMOTYPE_ROCKET',
};
const VALID_NAME = /^[a-z][a-z0-9_]{0,31}$/;

function fmtNum(v) {
  v = +v;
  if (Number.isInteger(v)) return (Math.abs(v) >= 1000 || v === 0) ? v + '.0' : String(v);
  return String(parseFloat(v.toFixed(1)));
}
function weaponExpr(id) { return WEAPON_CONST[id] || ('0x' + id.toString(16)); }
function ammoExpr(id)   { return AMMO_CONST[id]   || ('0x' + id.toString(16)); }
function sanitizeLevelName(raw) {
  const n = (raw || mapState.name || 'map').trim().toLowerCase();
  if (!VALID_NAME.test(n)) throw new Error('Invalid level name — use lowercase letters, digits, underscore.');
  return n;
}

/** Render editor state as a complete src/levels/<name>.py module (pdmap-ready). */
function jsonToLevelPy(obj) {
  const name = sanitizeLevelName(obj.name);
  const half = +obj.box_half || HALF;
  const height = +obj.box_height || HEIGHT;
  const pads = obj.pads || [];
  const spawn = [], weapon = [], ammo = [], scenario = [];
  pads.forEach((p, i) => {
    const idx = (p.index != null) ? +p.index : i;
    if (idx !== i) throw new Error('Pad index ' + idx + ' at position ' + i + ' — indices must be contiguous 0..N-1.');
    const row = [i, p];
    if (p.type === 'spawn') spawn.push(row);
    else if (p.type === 'weapon') weapon.push(row);
    else if (p.type === 'ammo') ammo.push(row);
    else if (p.type === 'scenario') scenario.push(row);
  });

  const L = [];
  const w = s => L.push(s);
  w('# Generated by journal/uff_viewer (pass-3 export).');
  w('');
  w('from tools.pdmap.builders import (');
  w('    add_loadout_intro,');
  if (weapon.length) w('    add_floor_weapons,');
  if (ammo.length) w('    add_ammo_row,');
  w('    floor_box_tiles,');
  w(')');
  w('from tools.pdmap.core import MapDef');
  const intro = new Set();
  if (spawn.length) intro.add('Spawn');
  scenario.forEach(([, p]) => intro.add(p.scenario === 'case' ? 'Case' : 'Hill'));
  if (intro.size) w('from tools.pdmap.intro import ' + [...intro].sort().join(', '));
  w('from tools.pdmap import weapons as W');
  w('');
  w('# Arena dimensions — shared by floor tiles and generic box seg (--seg).');
  w('BOX_HALF = ' + fmtNum(half));
  w('BOX_HEIGHT = ' + fmtNum(height));
  w('');
  w('# Pads slightly above Y=0 so ground search accepts the floor (see MAP_CREATION.md).');
  w('SPAWN_Y = 10.0');
  w('');
  w('');
  w('def build() -> MapDef:');
  w('    g = MapDef("' + name + '")');
  w('');
  w('    # --- Pads (indices MUST match array order 0..N-1) ---');
  pads.forEach((p, i) => {
    w('    g.add_pad(index=' + i + ', x=' + fmtNum(p.x) + ', y=' + fmtNum(p.y) +
      ', z=' + fmtNum(p.z) + ', room=' + (p.room | 0) + ')  # ' + (p.type || 'other'));
  });
  if (spawn.length) {
    w('');
    w('    # --- Spawn intro commands ---');
    spawn.forEach(([i]) => w('    g.add_intro(Spawn(pad=' + i + '))'));
  }
  if (weapon.length) {
    w('');
    w('    # --- Weapon pickups (floor props) ---');
    const pairs = weapon.filter(([, p]) => p.weapon != null)
      .map(([i, p]) => '(' + i + ', ' + weaponExpr(+p.weapon) + ')').join(', ');
    if (pairs) w('    add_floor_weapons(g, [' + pairs + '])');
    const miss = weapon.filter(([, p]) => p.weapon == null).map(([i]) => i);
    if (miss.length) w('    # WARNING: weapon pad(s) ' + miss.join(', ') + ' missing weapon id');
  }
  if (ammo.length) {
    w('');
    w('    # --- Ammo crates (floor props) ---');
    const byType = {};
    ammo.forEach(([i, p]) => {
      const t = +(p.ammoType != null ? p.ammoType : 0x04);
      (byType[t] = byType[t] || []).push(i);
    });
    Object.keys(byType).sort((a, b) => +a - +b).forEach(t => {
      w('    add_ammo_row(g, [' + byType[t].join(', ') + '], ammotype=' + ammoExpr(+t) + ')');
    });
  }
  if (scenario.length) {
    w('');
    w('    # --- Scenario anchors (Capture the Case / King of the Hill) ---');
    scenario.forEach(([i, p]) => {
      if (p.scenario === 'case') {
        w('    g.add_intro(Case(team=' + (p.team | 0) + ', pad=' + i + '))');
        w('    # NOTE: CaseRespawn pad not in editor export — add a second scenario pad if needed');
      } else {
        w('    g.add_intro(Hill(pad=' + i + '))');
      }
    });
  }
  w('');
  w('    add_loadout_intro(g)');
  w('    return g');
  w('');
  w('');
  w('def build_tiles_json():');
  w('    return floor_box_tiles("' + name + '", half=BOX_HALF, y=0.0, room_index=1)');
  w('');
  return L.join('\n');
}

function exportPython() {
  const obj = serializeMap();
  obj.name = sanitizeLevelName(levelNameInput.value || mapState.name);
  const txt = jsonToLevelPy(obj);
  pyText.value = txt;
  pyWrap.classList.add('shown');
  updateBuildCmds(obj.name);
  return txt;
}

function updateBuildCmds(name) {
  let lvl;
  try { lvl = sanitizeLevelName(name); } catch (e) { lvl = 'myarena'; }
  buildCmdsEl.textContent =
    '# 1. Save exported Python to src/levels/' + lvl + '.py\n' +
    '# 2. Build pads, tiles, setup, and box seg:\n' +
    'python3 tools/pdmap.py build ' + lvl + ' --deploy --seg\n\n' +
    '# 3. Run the game (still loads uff unless title.c is wired for ' + lvl + '):\n' +
    './build/pd.arm64 --test-map --scenario-0 --moddir mods/mod_allinone';
}

function populateLevelSelect() {
  levelSelect.innerHTML = '';
  const names = Object.keys(LEVEL_CATALOG).sort();
  if (!names.length) {
    const o = document.createElement('option');
    o.value = ''; o.textContent = '(no embedded levels)';
    levelSelect.appendChild(o);
    return;
  }
  names.forEach(n => {
    const o = document.createElement('option');
    o.value = n;
    o.textContent = n + ' (' + (LEVEL_CATALOG[n].pads || []).length + ' pads)';
    levelSelect.appendChild(o);
  });
}

function loadFromCatalog(name) {
  const snap = LEVEL_CATALOG[name];
  if (!snap) { alert('Level "' + name + '" not in embedded catalog.'); return false; }
  if (!loadMap(snap)) return false;
  levelNameInput.value = name;
  updateBuildCmds(name);
  return true;
}

document.getElementById('exportPyBtn').onclick = exportPython;
document.getElementById('copyPyBtn').onclick = () => {
  const txt = exportPython();
  if (navigator.clipboard) navigator.clipboard.writeText(txt).catch(() => {});
  else { pyText.select(); document.execCommand('copy'); }
};
document.getElementById('downloadPyBtn').onclick = () => {
  const obj = serializeMap();
  obj.name = sanitizeLevelName(levelNameInput.value || mapState.name);
  const txt = jsonToLevelPy(obj);
  const blob = new Blob([txt], { type: 'text/x-python' });
  const a = document.createElement('a');
  a.href = URL.createObjectURL(blob);
  a.download = obj.name + '.py';
  document.body.appendChild(a); a.click(); a.remove();
  setTimeout(() => URL.revokeObjectURL(a.href), 1000);
  pyText.value = txt;
  pyWrap.classList.add('shown');
  updateBuildCmds(obj.name);
};
document.getElementById('loadLevelBtn').onclick = () => {
  const n = levelSelect.value;
  if (n) loadFromCatalog(n);
};
levelNameInput.addEventListener('input', () => updateBuildCmds(levelNameInput.value || mapState.name));

populateLevelSelect();
levelNameInput.value = mapState.name || 'uff';
updateBuildCmds(mapState.name);

// ?level=csim URL param switches the initial embedded snapshot.
(function applyLevelQueryParam() {
  const q = new URLSearchParams(location.search).get('level');
  if (q && LEVEL_CATALOG[q]) loadFromCatalog(q);
})();

// expose hooks for headless scripted sanity checks (pass-2/3 VERIFY)
window.__editor = { setEditing, addPad, selectPad, deletePad, exportJSON, loadMap, serializeMap, mapState, setTool, exportPython, jsonToLevelPy, loadFromCatalog };

updateValidation();  // initial pass once everything is defined

// ---------- corner orientation gizmo (mini axis indicator) ----------
const gizmoScene = new THREE.Scene();
gizmoScene.add(new THREE.AxesHelper(1));
const gizmoCam = new THREE.PerspectiveCamera(50, 1, 0.1, 10);
function makeGizmoLabel(text, color) {
  const cv = document.createElement('canvas'); cv.width = cv.height = 64;
  const ctx = cv.getContext('2d');
  ctx.font = 'bold 44px monospace'; ctx.fillStyle = color;
  ctx.textAlign = 'center'; ctx.textBaseline = 'middle';
  ctx.fillText(text, 32, 34);
  const tex = new THREE.CanvasTexture(cv);
  const s = new THREE.Sprite(new THREE.SpriteMaterial({ map: tex, depthTest: false }));
  s.scale.set(0.45, 0.45, 0.45);
  return s;
}
[['X', '#ff6b6b', new THREE.Vector3(1.25, 0, 0)],
 ['Y', '#6bff6b', new THREE.Vector3(0, 1.25, 0)],
 ['Z', '#6ba8ff', new THREE.Vector3(0, 0, 1.25)]].forEach(([t, c, p]) => {
  const s = makeGizmoLabel(t, c); s.position.copy(p); gizmoScene.add(s);
});

// ---------- resize + initial framing ----------
let W = 1, H = 1;
function resize() {
  W = window.innerWidth; H = window.innerHeight;
  renderer.setSize(W, H);  // updateStyle=true: also sets canvas CSS size so the
                           // 2x-DPI drawing buffer is displayed at viewport size
                           // (without this the canvas overflows on retina displays)
  camera.aspect = W / H;
  camera.updateProjectionMatrix();
}
window.addEventListener('resize', resize);
resize();

// snap to the isometric framing on load (no tween) so the box fills the view
(function setInitialView() {
  const dir = PRESET_DIRS.iso.clone().normalize();
  camera.position.copy(CENTER.clone().add(dir.multiplyScalar(fitDistance())));
  controls.target.copy(CENTER);
  controls.update();
  setActiveView('iso');
})();

// keep billboard labels a readable size regardless of camera distance
function updateLabels() {
  for (const grp of [labelGroup, faceLabelGroup, padLabelGroup]) {
    if (!grp.visible) continue;
    for (const s of grp.children) {
      const d = camera.position.distanceTo(s.position);
      const h = Math.max(120, d * 0.045);
      s.scale.set(h * (s.userData.aspect || 4), h, 1);
    }
  }
}

const GIZ = 110;
function renderGizmo() {
  renderer.clearDepth();
  renderer.setScissorTest(true);
  renderer.setViewport(W - GIZ - 8, 8, GIZ, GIZ);
  renderer.setScissor(W - GIZ - 8, 8, GIZ, GIZ);
  gizmoCam.position.set(0, 0, 0);
  gizmoCam.quaternion.copy(camera.quaternion);
  gizmoCam.translateZ(3.2);          // back off along the view axis
  gizmoCam.lookAt(0, 0, 0);
  renderer.render(gizmoScene, gizmoCam);
  renderer.setScissorTest(false);
  renderer.setViewport(0, 0, W, H);
}

(function loop() {
  requestAnimationFrame(loop);
  const dt = Math.min(clock.getDelta(), 0.05);
  if (mode === 'orbit') {
    if (tween) {
      const k = easeInOut(Math.min(1, (performance.now() - tween.t0) / tween.dur));
      camera.position.lerpVectors(tween.fp, tween.tp, k);
      controls.target.lerpVectors(tween.ft, tween.tt, k);
      camera.lookAt(controls.target);
      if (k >= 1) tween = null;
    } else {
      controls.update();
    }
  } else {
    updateFly(dt);
  }
  updateLabels();
  renderer.clear();
  renderer.render(scene, camera);
  renderGizmo();
})();
</script>
</body>
</html>
"""


def main():
    geo = load_geometry()
    gdl_bytes, nverts_total = build_gdl_for_uff()
    cmds = decode_gdl(gdl_bytes)

    out_html = os.path.join(HERE, "uff_map.html")
    out_obj = os.path.join(HERE, "uff_map.obj")
    out_dump = os.path.join(HERE, "uff_gdl_dump.txt")

    write_html(geo, cmds, nverts_total, out_html, build_level_catalog())
    write_obj(geo, out_obj)
    write_gdl_dump(geo, cmds, nverts_total, out_dump)

    # console summary
    print("Wrote:")
    print(" ", out_html)
    print(" ", out_obj)
    print(" ", out_dump)
    print()
    print(f"box half={geo['half']} height={geo['height']} "
          f"faces={len(geo['faces'])} tiles={len(geo['tiles'])} pads={len(geo['pads'])}")
    vtx_loads = [c["g_vtx"] for c in cmds if "g_vtx" in c]
    print(f"G_VTX loads={len(vtx_loads)} "
          f"sizes={[v['nverts_from_bytes'] for v in vtx_loads]} "
          f"max={max((v['nverts_from_bytes'] for v in vtx_loads), default=0)} "
          f"overflow={any(v['overflow'] for v in vtx_loads)}")
    near_f = [f["name"] for f in geo["faces"] if near_origin(f["verts"])]
    near_t = [i for i, t in enumerate(geo["tiles"]) if near_origin(t["verts"])]
    print(f"near-origin faces={near_f} tiles={near_t}")


if __name__ == "__main__":
    main()
