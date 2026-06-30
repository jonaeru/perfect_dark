"""Procedural box-arena .seg generator.

Builds a single-room box level (geometry + colours + F3DEX2 display list)
from scratch and assembles a valid Perfect Dark bg seg, reusing a stock seg
as the Section-2/Section-3 template.

This is the reusable engine behind ``scripts/build_custom_seg.py``. A level
module only has to declare the box half-extent and height; the seg, floor
tiles, and spawn layout all derive from the same dimensions, so the visible
walls always match the collision floor and contain the spawn pads.
"""

import os
import struct
import zlib

from .core import ROOT

# Stock seg reused for the Section-2 (texture list) and Section-3 (bbox/light)
# blocks. Its room block is also the source of the working "setup" display-list
# prologue (geometry mode, combine, etc).
DEFAULT_TEMPLATE_SEG = os.path.join(
    ROOT, "mods", "mod_gex", "files", "bgdata", "bg_mp17.seg"
)

SEG_BASE = 0x0F000000

# Primary section decompressed size — must be large enough that the
# host-converted room table (20 * 22 = 440 bytes) plus header, portals and
# bgcmds fits. 512 bytes (0x200) works.
PRIMARY_SIZE = 512
ROOM_BASE = SEG_BASE + PRIMARY_SIZE

# Per-face vertex colours (RGBA8): floor, ceiling, -Z, +X, +Z, -X.
DEFAULT_FACE_COLOURS = [
    0x4040FFFF,  # floor:   blue
    0x40FF40FF,  # ceiling: green
    0xFF4040FF,  # wall -Z: red
    0xFF40FFFF,  # wall +X: magenta
    0xFFFF40FF,  # wall +Z: yellow
    0x40FFFFFF,  # wall -X: cyan
]


def zip1172(data):
    co = zlib.compressobj(9, zlib.DEFLATED, -15)
    raw = co.compress(data) + co.flush()
    return b"\x11\x73" + len(data).to_bytes(3, "big") + raw


def unzip1172(blob):
    assert blob[0:2] == b"\x11\x73"
    declen = int.from_bytes(blob[2:5], "big")
    return zlib.decompress(blob[5:], -15)[:declen]


def _be16(v):
    return struct.pack(">h", v)


def _vertex(x, y, z, colour_index=0):
    return _be16(x) + _be16(y) + _be16(z) + bytes([0, colour_index * 4]) + struct.pack(">HH", 0, 0)


def _faces(half, height):
    h = int(half)
    t = int(height)
    return [
        ((-h, 0, -h), (h, 0, -h), (h, 0, h), (-h, 0, h), 0),
        ((-h, t, -h), (h, t, -h), (h, t, h), (-h, t, h), 1),
        ((-h, 0, -h), (h, 0, -h), (h, t, -h), (-h, t, -h), 2),
        ((h, 0, -h), (h, 0, h), (h, t, h), (h, t, -h), 3),
        ((h, 0, h), (-h, 0, h), (-h, t, h), (h, t, h), 4),
        ((-h, 0, h), (-h, 0, -h), (-h, t, -h), (-h, t, h), 5),
    ]


def _build_vertices(faces):
    verts = b""
    for (v0, v1, v2, v3, colour_index) in faces:
        for v in (v0, v1, v2, v3):
            verts += _vertex(v[0], v[1], v[2], colour_index + 1)
    return verts


def _build_colours(face_colours):
    out = struct.pack(">I", 0xFFFFFFFF)
    for c in face_colours:
        out += struct.pack(">I", c)
    return out


def _gfx(w0, w1):
    return struct.pack(">II", w0 & 0xFFFFFFFF, w1 & 0xFFFFFFFF)


def _g_vtx_cmd(nverts, v0, addr):
    p = ((nverts - 1) & 0xF) << 4 | (v0 & 0xF)
    return _gfx((0x04 << 24) | (p << 16) | (12 * nverts), addr)


# F3DEX2 packs the vertex count in a 4-bit nibble (max 16/load). Loads above 16
# wrap the nibble; bgPopulateVtxBatchType reads the nibble while the renderer
# uses the byte length — phantom collision triangles near the origin result.
MAX_G_VTX_LOAD = 16


def validate_gdl_g_vtx(gdl: bytes) -> list[str]:
    """Return human-readable errors for invalid G_VTX loads in a display list."""
    errors: list[str] = []
    load_index = 0
    for off in range(0, len(gdl), 8):
        if off + 8 > len(gdl):
            break
        w0, _w1 = struct.unpack(">II", gdl[off:off + 8])
        op = (w0 >> 24) & 0xFF
        if op == 0xB8:
            break
        if op != 0x04:
            continue
        numbytes = w0 & 0xFFFF
        nverts_from_bytes = numbytes // 12
        p = (w0 >> 16) & 0xFF
        nverts_nibble = ((p >> 4) & 0xF) + 1
        load_index += 1
        if nverts_from_bytes > MAX_G_VTX_LOAD:
            errors.append(
                f"G_VTX load #{load_index} requests {nverts_from_bytes} verts "
                f"(max {MAX_G_VTX_LOAD} per F3DEX2 nibble) — phantom collision "
                f"near origin/camera"
            )
        elif nverts_nibble != nverts_from_bytes:
            errors.append(
                f"G_VTX load #{load_index}: byte length implies "
                f"{nverts_from_bytes} verts but nibble says {nverts_nibble} "
                f"(bgPopulateVtxBatchType under-loads; phantom collision)"
            )
    return errors


def validate_seg_phantom_viewport(seg_data: bytes) -> list[str]:
    """Fail builds whose room GDL still draws in-box wall faces (viewport phantom).

    With the camera inside the box arena, huge wall quads that extend behind the
    eye used to be partially near-plane clipped into screen-filling junk. Play
    and Test/Play should ship ``PDMAP_SEG_MODE=empty`` (tiles-only collision).
    """
    errors: list[str] = []
    try:
        prim_size, sec1_cmp, prim_cmp = struct.unpack(">III", seg_data[0:12])
        room_blob = seg_data[12 + prim_cmp:12 + sec1_cmp]
        room = unzip1172(room_blob)
        gdl_ptr = struct.unpack(">I", room[32:36])[0]
        gdl_start = gdl_ptr - (SEG_BASE + prim_size)
        if gdl_start < 0 or gdl_start >= len(room):
            return errors
        gdl = room[gdl_start:]
        g_vtx_after_setup = 0
        for off in range(0, len(gdl), 8):
            if off + 8 > len(gdl):
                break
            w0, _w1 = struct.unpack(">II", gdl[off:off + 8])
            op = (w0 >> 24) & 0xFF
            if op == 0xB8:
                break
            if op == 0x04:
                g_vtx_after_setup += 1
        if g_vtx_after_setup > 0:
            errors.append(
                f"seg room GDL has {g_vtx_after_setup} G_VTX face load(s) — "
                f"PDMAP_SEG_MODE=empty is required for in-box FPS (wall quads "
                f"behind the camera become a viewport-attached phantom sheet; "
                f"floor collision comes from tiles)"
            )
    except Exception as exc:
        errors.append(f"seg viewport validation failed: {exc!r}")
    return errors


def validate_seg_g_vtx(seg_data: bytes) -> list[str]:
    """Decode a bg *.seg room GDL and validate every G_VTX load."""
    try:
        prim_size, sec1_cmp, prim_cmp = struct.unpack(">III", seg_data[0:12])
        room_blob = seg_data[12 + prim_cmp:12 + sec1_cmp]
        room = unzip1172(room_blob)
        gdl_ptr = struct.unpack(">I", room[32:36])[0]
        gdl_start = gdl_ptr - (SEG_BASE + prim_size)
        if gdl_start < 0 or gdl_start >= len(room):
            return [f"invalid GDL offset {gdl_start} in room block"]
        return validate_gdl_g_vtx(room[gdl_start:])
    except Exception as exc:
        return [f"seg G_VTX validation failed: {exc!r}"]


def _g_tri1_cmd(i, j, k):
    w1 = ((i * 10) << 16) | ((j * 10) << 8) | (k * 10)
    return _gfx(0xBF000000, w1)


def _enddl():
    return _gfx(0xB8000000, 0x00000000)


def _face_gdl(face_index):
    """Display list for a single face: load its 4 verts, draw two triangles.

    Single winding only. Drawing both windings (coincident back faces) reliably
    corrupts the host GL driver's heap, so we never do that.
    """
    gdl = _g_vtx_cmd(4, 0, 0x0E000000 + face_index * 4 * 12)
    gdl += _g_tri1_cmd(0, 1, 2)
    gdl += _g_tri1_cmd(0, 2, 3)
    return gdl


def _build_gdl(setup_gdl):
    """Build the room display list.

    Default ("full") draws the complete six-face coloured box. Getting this
    stable required several fixes:
      - Section-3 must list exactly roomcount-1 bbox/gfxdatalen entries. The
        room table has 1 real room + 1 end-marker (roomcount 2), so Section-3
        carries exactly 1 entry. With a mismatched count the engine read
        gfxdatalen at the wrong offset (0), under-allocated the room, and
        corrupted the heap intermittently.
      - Near-plane clipping was added to fast3d (gfx_pc.cpp
        gfx_clip_triangle_near) so box walls extending behind the in-box camera
        no longer feed behind-eye vertices to the GL driver.
      - Geometry is single-winding; drawing coincident back faces corrupts the
        GL heap.

    Modes (PDMAP_SEG_MODE):
      full   (default) ceiling + four walls (no floor face — avoids near-plane clip)
      box    ceiling + two walls (no floor)
      floor  floor quad only (debug; clips viewport when camera inside box)
      empty  no geometry (collision-only; recommended for Test/Play in-box FPS)

    All modes are playable; floor/wall collision comes from the tiles file,
    not the seg geometry.
    """
    import os as _os
    # empty = collision-only (tiles floor); recommended for in-box FPS Test/Play.
    # full = coloured walls for editor preview; safe only with near-plane cull fix.
    mode = _os.environ.get("PDMAP_SEG_MODE", "empty")

    if mode == "empty":
        return setup_gdl + _enddl()

    if mode == "floor":
        return setup_gdl + _face_gdl(0) + _enddl()

    # Face sets for in-box cameras (Y=SPAWN_Y). Face 0 (floor at Y=0) is
    # deliberately omitted from playable modes: the huge floor quad sits behind
    # the near plane when the player stands inside the box, and fast3d clips it
    # into a dark/blurred sheet glued to the viewport. Floor collision still
    # comes from bg_*_tilesZ — the seg floor is visual-only.
    #   full = ceiling + four walls (Matrix room colours)
    #   box  = ceiling + two walls (minimal shell)
    if mode == "full":
        face_list = [1, 2, 3, 4, 5]
    elif mode == "box":
        face_list = [1, 2, 3]
    else:
        face_list = [1, 2, 3, 4, 5]

    # Per-face vertex loads (one G_VTX of 4 verts per face, then its 2
    # single-winding triangles). This is REQUIRED for correctness, not just an
    # optimisation:
    #
    #   The F3DEX2 G_VTX count lives in a 4-bit nibble (max 16 verts/load). A
    #   single G_VTX(24) wraps that field to (24-1)&0xF + 1 == 8. The fast3d
    #   renderer derives the count from the byte length (12*24) so it still
    #   draws all 24 verts correctly, but the engine's collision/hit code
    #   (bgPopulateVtxBatchType / bgTestHitInVtxBatch in src/game/bg.c) reads
    #   the nibble and only loads 8 vertices into its local batch buffer. It
    #   then walks ALL the triangles, so the four wall faces (verts 8..23)
    #   index past the loaded 8 into stale buffer memory -> PHANTOM collision
    #   triangles near the origin that take bullet holes and block movement.
    #
    #   Loading 4 verts per face keeps every batch's count valid (==4) and
    #   self-contained: each face's two triangles index 0..3 relative to that
    #   load's base, so both the renderer and the collision walk read exactly
    #   the verts they reference. Near-plane clipping in fast3d still keeps
    #   behind-eye wall vertices from crashing the GL driver.
    gdl = setup_gdl
    for fi in face_list:
        gdl += _face_gdl(fi)
    return gdl + _enddl()


def _align8(v):
    return (v + 7) & ~7


def _extract_setup_gdl(template_seg, ncols):
    seg = open(template_seg, "rb").read()
    _, sec1_cmp, prim_cmp = struct.unpack(">III", seg[0:12])
    room_blob = seg[12 + prim_cmp:12 + sec1_cmp]
    template_room = unzip1172(room_blob)

    # mp17 template uses PRIMARY_SIZE = 0x7c (124).
    template_base = SEG_BASE + 0x7C
    gdl_start = (struct.unpack(">I", template_room[0x20:0x24])[0]) - template_base
    setup = template_room[gdl_start:gdl_start + 12 * 8]
    assert len(setup) == 96, f"setup extraction failed: {len(setup)} bytes"

    assert struct.unpack(">I", setup[88:92])[0] & 0xFF000000 == 0x07000000, \
        "expected G_COL as last command"
    patched = bytearray(setup)
    w0 = (0x07 << 24) | (((ncols - 1) << 2) << 16) | (4 * ncols)
    struct.pack_into(">I", patched, 11 * 8, w0)
    # Use segment 0x0d (colours) offset 0 — the PC port room renderer only sets
    # segments 0x0d (colours) and 0x0e (vertices) during bgRenderRoomPass.
    struct.pack_into(">I", patched, 11 * 8 + 4, 0x0D000000)

    assert struct.unpack(">I", setup[72:76])[0] & 0xFF000000 == 0xB7000000, \
        "expected G_SETGEOMETRYMODE at offset 72"
    struct.pack_into(">II", patched, 9 * 8, 0xB6000000, 0x00023000)
    struct.pack_into(">II", patched, 10 * 8, 0xB7000000, 0x00000205)

    return bytes(patched)


def _build_room_block(template_seg, faces, face_colours):
    nverts = len(faces) * 4
    ncols = len(face_colours) + 1
    verts = _build_vertices(faces)
    cols = _build_colours(face_colours)

    setup_gdl = _extract_setup_gdl(template_seg, ncols)
    gdl = _build_gdl(setup_gdl)

    off_blocks = 24
    off_verts = _align8(off_blocks + 20)
    off_cols = _align8(off_verts + len(verts))
    off_gdl = _align8(off_cols + len(cols))
    total = off_gdl + len(gdl)

    buf = bytearray(total)

    struct.pack_into(">IIII", buf, 0,
                     ROOM_BASE + off_verts, ROOM_BASE + off_cols,
                     ROOM_BASE + off_blocks, 0)
    struct.pack_into(">hhhh", buf, 16, -1, 0, nverts, ncols)

    buf[off_blocks] = 0
    struct.pack_into(">IIII", buf, off_blocks + 4,
                     0, ROOM_BASE + off_gdl,
                     ROOM_BASE + off_verts, ROOM_BASE + off_cols)

    buf[off_verts:off_verts + len(verts)] = verts
    buf[off_cols:off_cols + len(cols)] = cols
    buf[off_gdl:off_gdl + len(gdl)] = gdl

    return bytes(buf)


def _build_primary(prim_inf, room_cmp):
    buf = bytearray(prim_inf)

    room_tbl_ofs = 24
    portal_ofs = room_tbl_ofs + 20 * 20
    bgcmd_ofs = portal_ofs + 10

    struct.pack_into(">IIIIII", buf, 0,
                     0,
                     SEG_BASE + room_tbl_ofs,
                     SEG_BASE + portal_ofs,
                     SEG_BASE + bgcmd_ofs,
                     0,
                     0)

    # Room table. Index 0 is the dummy room; index 1 is the single real room;
    # index 2 is the end-marker (points just past room 1's data so the engine
    # can compute room 1's compressed size); index 3+ terminate with zero.
    #
    # The engine sets g_Vars.roomcount = number of NONZERO entries (here 2:
    # indices 1 and 2), and reads the Section-3 bbox/gfxdatalen lists for
    # roomcount-1 (= 1) rooms. _patch_section3_gfxdatalen MUST emit exactly that
    # many entries (SEG_NUM_ROOMS - 1) or the gfxdatalen list is read at the
    # wrong offset (yielding 0 -> a too-small room allocation and corruption).
    for i in range(20):
        pos = room_tbl_ofs + i * 20
        if i == 1:
            pg = ROOM_BASE
        elif i == 2:
            pg = ROOM_BASE + room_cmp
        else:
            pg = 0
        struct.pack_into(">IfffBB", buf, pos, pg, 0.0, 0.0, 0.0, 0, 0)

    struct.pack_into(">HhbB", buf, portal_ofs, 0, 0, 0, 0)
    struct.pack_into(">I", buf, portal_ofs + 6, 0)
    struct.pack_into(">BbI", buf, bgcmd_ofs, 0, 0, 0)

    return bytes(buf)


def _patch_section3_gfxdatalen(rest, new_room_len):
    _, s2_cmp = struct.unpack(">HH", rest[0:4])
    s2_end = 4 + s2_cmp

    # Must equal g_Vars.roomcount - 1. The room table (build_primary) has 2
    # nonzero entries (the real room + the end-marker), so roomcount is 2 and
    # the engine reads exactly one bbox + one gfxdatalen + one numlights entry.
    num_entries = 1
    bbox = bytearray()
    for i in range(num_entries):
        if i == 0:
            bbox += struct.pack(">hhhhhh", -32768, -32768, -32768, 32767, 32767, 32767)
        else:
            bbox += struct.pack(">hhhhhh", 0, 0, 0, 0, 0, 0)

    gfxdatalen = bytearray(num_entries * 2)
    # Room 0 (index 0 here) gfxdatalen. The engine reads this as a count of
    # 16-byte units: g_Rooms[r].gfxdatalen = ALIGN16(value * 0x10 + 0x100), and
    # uses it to size the room allocation. Writing the raw decompressed room
    # length yields a comfortably large allocation (value*16 >> room size),
    # which is safe headroom for the host-format expansion. NOTE: num_entries
    # MUST equal the number of real+phantom rooms in the room table (see
    # build_primary), or the engine reads gfxdatalen from the wrong offset.
    struct.pack_into(">H", gfxdatalen, 0, min(0xFFFF, new_room_len))
    numlights = bytearray(num_entries)

    s3_decompressed = bytes(bbox + gfxdatalen + numlights)
    s3_blob = zip1172(s3_decompressed)
    s3_header = struct.pack(">HH", len(s3_decompressed), len(s3_blob))
    return rest[:s2_end] + s3_header + s3_blob


def build_box_seg(*, half=5000, height=3000, face_colours=None, template_seg=None) -> bytes:
    """Return the bytes of a single-room box arena seg.

    The box spans X,Z in [-half, +half] and Y in [0, height], floor at Y=0.
    These are world units — pass the same ``half`` used for the floor tiles so
    the visible walls match the collision floor.
    """
    if face_colours is None:
        face_colours = DEFAULT_FACE_COLOURS
    if template_seg is None:
        template_seg = DEFAULT_TEMPLATE_SEG

    seg = open(template_seg, "rb").read()
    _, sec1_cmp, _ = struct.unpack(">III", seg[0:12])
    rest = seg[12 + sec1_cmp:]

    faces = _faces(half, height)
    new_room = _build_room_block(template_seg, faces, face_colours)
    new_room_blob = zip1172(new_room)

    primary_n64 = _build_primary(PRIMARY_SIZE, len(new_room_blob))
    primary_blob = zip1172(primary_n64)

    rest = _patch_section3_gfxdatalen(rest, len(new_room))

    result = (struct.pack(">III", PRIMARY_SIZE,
                          len(primary_blob) + len(new_room_blob),
                          len(primary_blob))
              + primary_blob + new_room_blob + rest)

    g_vtx_errors = validate_seg_g_vtx(result)
    if g_vtx_errors:
        raise ValueError(
            "Refusing to emit box seg with invalid G_VTX loads:\n  "
            + "\n  ".join(g_vtx_errors)
        )

    import os as _os
    mode = _os.environ.get("PDMAP_SEG_MODE", "empty")
    if mode in ("empty",):
        viewport_errors = validate_seg_phantom_viewport(result)
        if viewport_errors:
            raise ValueError(
                "Refusing to emit box seg that would cause viewport phantom:\n  "
                + "\n  ".join(viewport_errors)
            )

    return result


def write_box_seg(out_path, *, half=5000, height=3000, face_colours=None,
                  template_seg=None) -> str:
    data = build_box_seg(half=half, height=height,
                         face_colours=face_colours, template_seg=template_seg)
    os.makedirs(os.path.dirname(out_path), exist_ok=True)
    with open(out_path, "wb") as f:
        f.write(data)
    return out_path
