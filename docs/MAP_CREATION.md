# Perfect Dark — Map Creation Reference

Codebase-accurate guide to creating custom multiplayer maps in this fork. Every
claim is grounded in the actual source and build pipeline.

---

## 1. Architecture — what a map actually is

A playable arena is **five asset files** plus **four wiring points** in the source.

### The five asset files (per stage `<name>`)

| File (deployed path) | What it is | Build source |
|---|---|---|
| `bgdata/bg_<name>.seg` | Room geometry (F3DEX2 display lists) + room bboxes | `scripts/build_custom_seg.py` or copy from another stage |
| `bgdata/bg_<name>_tiles` | Collision floor tiles (per room) | `tools/assetmgr/mktiles/mktiles.py` compiles from JSON |
| `bgdata/bg_<name>_pads` | Pad locations (spawn/weapon/obj anchors) | `tools/assetmgr/mkpads/mkpads.py` compiles from JSON |
| `bgdata/Usetup<name>` | Solo mode setup (props/paths/AI) | C source → compiled |
| `bgdata/Ump_setup<name>` | MP setup — intro cmds (spawns/weapons) | `tools/pdmap` generates setup binary directly |

The game loads these at stage init via `stagetable.c` (see §3).

### The four wiring points

1. **`src/include/files.h`** — a `FILE_*` constant per asset file, e.g.
   `FILE_BG_UFF_SEG 0x0025`, `FILE_BG_UFF_TILES 0x01d4`, `FILE_BG_UFF_PADS 0x01d3`,
   `FILE_USETUPUFF 0x01d1`, `FILE_UMP_SETUPUFF 0x01d2`. Sequential, no gaps.

2. **`src/assets/<romid>/files/list.c`** — maps each `FILE_*` index to a filesystem
   path: `/*0x025*/ "bgdata/bg_uff.seg"`. The array index must match the `FILE_*` value.

3. **`src/game/stagetable.c`** — `g_Stages[]` row ties a `STAGE_*` id to its five files:
   `{ id, ?, lightambient, lightwall, lighthill, ?, BG_SEG, BG_TILES, BG_PADS,
   USETUP, UMP_SETUP, ... bbox-cull, alarm, extragunmem }`.

4. **`src/game/mplayer/setup.c`** — `g_MpArenas[]` lists the arena in the Combat
   Simulator menu (`{ STAGE_*, requirefeature, nameLangId }`). `groups[]` puts it
   under a section header with **absolute indices** into `g_MpArenas[]`.

> **Menu gotcha:** `groups[]` offsets are absolute `g_MpArenas` indices. Insert an
> arena and every later group header points at the wrong map. Keep group offsets
> in sync with the array.

### Relevant file paths (this project)

| Path | Purpose |
|---|---|
| `build/ntsc-final/assets/files/bgdata/` | Built tiles/pads/setup binaries |
| `build/ntsc-final/assets/files/seg/` | Built seg files |
| `mods/mod_allinone/files/bgdata/` | Deployed mod target |
| `tools/pdmap/` | Map creation CLI package |
| `tools/assetmgr/mktiles/` | Tiles JSON → binary compiler |
| `tools/assetmgr/mkpads/` | Pads JSON → binary compiler |
| `src/levels/<name>.py` | Level definition module |

---

## 2. Coordinate system

Perfect Dark uses a right-handed **Y-up** coordinate system.

| Axis | Direction |
|---|---|
| X+ | Right (east) |
| Y+ | Up |
| Z+ | South (into screen) |

**Units:** 1 game unit ≈ 1 cm. A room box of `(-5000, -714, -5000) → (5000, 714, 5000)`
is roughly 100 m × 14 m × 100 m.

**The ×6 transform:** Source assets (tiles JSON, pads JSON) store coordinates
**multiplied by 6** (`to_scaled_hex()` in `tools/pdmap/core.py`). The game
divides by 6 at runtime (`pd/arm/bg.cpp`). So a floor at `Y=0` source units
becomes `Y=0` in scaled, and a pad at `Y=20` becomes `Y=120` scaled.

Pads must be **above** the floor (typically `Y=1..20` in source units) and
**within** the room's Section-3 bbox.

---

## 3. The `.seg` file format

Reverse-engineered from `src/lib/port/src/preprocess/filebg.c`. The on-disk layout
has **three compressed sections** inside a 12-byte header.

```
Offset  Size  Field
0x00    4     uncompressed size of primary block
0x04    4     compressed size of Section 1
0x08    4     compressed size of primary block (same as offset 0x00)

Section 1 (1172-rarezipped, decompresses to `primary` + per-room gfxdata):
  0x00  primary block — 6 pointers {0, rooms, portals, bgcmds, lights, 0}
                        + room table + portals + bgcmds + lights
  ...   room 0 gfxdata
  ...   room 1 gfxdata
  ...

Section 2: [u16 infsize|0x7fff][u16 cmpsize] → 1172 → texture-id list

Section 3: [u16 infsize|0x7fff][u16 cmpsize] → 1172 → bbox table (6×s16/room)
            + gfxdatalen list (u16/room) + per-room light counts
```

The Section-3 bboxes are what the engine uses to match spawn positions to rooms.
If a pad's position falls outside all bboxes, the player doesn't get assigned to
any room → no collision → falls into the void.

### F3DEX2 encodings that matter

- `gsSPVertex(addr, n, v0)`: `w0 = (4<<24) | (((n-1)<<4|v0)<<16) | (12*n)`
  `w1 = 0x0e000000 | ofs` (vertices live in segment `0x0e`)
- `gsSP1Triangle(a, b, c, 0)`: `w0 = 0xbf000000`, `w1 = (a*10<<16)|(b*10<<8)|(c*10)`
  (indices ×10 — microcode optimisation)
- `G_SETCOMBINE = 0xfc`, `G_ENDDL = 0xb8`

Opcode byte is read via `gdl->bytes[GFX_W0_BYTE(0)]`. On the 64-bit LE port that's
physical byte 3, not byte 0.

---

## 4. The tiles format

### JSON schema

Tiles are stored per-room. Each room has an array of tile entries:

```json
{
  "rooms": {
    "ROOM_MAP_0000": [],
    "ROOM_MAP_0001": [
      {
        "flag0001": true, "flag0002": true, "flag0004": false,
        "flag0008": true, "flag0010": true, "flag0020": false,
        "ladder": false, "flag0080": false, "flag0100": false,
        "underwater": false, "flag0400": false,
        "aibotcrouch": false, "aibotduck": false,
        "flag2000": false, "die": false, "climbableledge": false,
        "floortype": "default",
        "floorcolour": 4095,
        "vertices": [
          {"x": -5000, "y": 0, "z": -5000},
          {"x": -5000, "y": 0, "z": -2500},
          {"x": -2500, "y": 0, "z": -2500},
          {"x": -2500, "y": 0, "z": -5000}
        ]
      }
    ]
  }
}
```

Room 0 must be **empty** (no tiles). The engine uses room 0 for "no room" / void.

### Tile flags (bitmask from 16 boolean fields)

| Field | Bit | Meaning |
|---|---|---|
| flag0001 | 0 | Floor passable |
| flag0002 | 1 | Wall collision |
| flag0004 | 2 | Unknown |
| flag0008 | 3 | Can stand on |
| flag0010 | 4 | Can walk through |
| flag0020 | 5 | No shadows |
| ladder | 6 | Ladder surface |
| flag0080 | 7 | Unknown |
| flag0100 | 8 | No camera collision |
| underwater | 9 | Underwater zone |
| flag0400 | 10 | Unknown |
| aibotcrouch | 11 | AI crouch here |
| aibotduck | 12 | AI duck here |
| flag2000 | 13 | Second surface |
| die | 14 | Instant kill |
| climbableledge | 15 | Ledge climb |

### Floor types

| Value | Name |
|---|---|
| default | 0 |
| wood | 1 |
| stone | 2 |
| carpet | 3 |
| metal | 4 |
| mud | 5 |
| water | 6 |
| dirt | 7 |
| snow | 8 |

| Constant | Value | Meaning |
|---|---|---|
| TILE3_FLAG_KEEPWALL | 0x01 | Keep wall collision |
| TILE3_FLAG_NODRAW | 0x02 | Invisible |
| TILE3_FLAG_DOOR | 0x04 | Door surface |
| TILE3_FLAG_NOWALLOBJECT | 0x08 | No wall collision |
| TILE3_FLAG_WATER | 0x10 | Water surface |
| TILE3_FLAG_CEIL | 0x20 | Ceiling tile |
| TILE3_FLAG_FLOOR | 0x40 | Floor tile |
| TILE3_FLAG_GLASS | 0x80 | Glass surface |
| TILE3_FLAG_BOTTOMLESS_PIT | 0x100 | Instant death |
| TILE3_FLAG_NO_FALL_CAMERA | 0x200 | No fall camera |
| TILE3_FLAG_LIGHTMAP | 0x400 | Lightmapped |
| TILE3_FLAG_LIGHTMAP_OFFSET | 0x800 | Alternative lightmap |
| TILE3_FLAG_WET_FLOOR | 0x1000 | Wet surface sound |
| TILE3_FLAG_JUNGLE_FLOOR | 0x2000 | Jungle surface sound |
| TILE3_FLAG_LAVA_FLOOR | 0x4000 | Lava damage |
| TILE3_FLAG_SNOW_FLOOR | 0x8000 | Snow surface sound |
| TILE3_FLAG_SLIPPERY_FLOOR | 0x10000 | Ice physics |

### Binary layout

The compiled binary has a simple header:
```
u32 num_vertices          (total across all rooms)
u32 num_rooms
...room data...
```

The engine loads it in `cdInitTiles()` (`src/game/cdroid.c`). Each tile's vertices
are offset by the per-room vertex base. `cdCollectGeoForCyl()` checks
`roomnum < g_TileNumRooms` — spawning in a room ≥ `num_rooms` silently skips
collision.

---

## 5. The pads format

### JSON schema

```json
{
  "pads": [
    {
      "id": "PAD_UFF_0000",
      "pos": [-4000, 20, -4000],
      "dir": [0.0, 1.0, 0.0],
      "up": [0.0, 0.0, -1.0],
      "xmin": -100.0, "xmax": 100.0,
      "ymin": -100.0, "ymax": 100.0,
      "zmin": -100.0, "zmax": 100.0,
      "aiwaitlift": false, "aionlift": false,
      "aiwalkdirect": false, "aidrop": false,
      "aicrouch": false, "aiignorey": false, "aiduck": false,
      "liftnum": 0, "room": 1
    }
  ],
  "waypoints": [ ... ],
  "waygroups": [ ... ],
  "cover": [
    {
      "id": "COVER_UFF_0000",
      "pos": [500, 10, 500],
      "dir": [0, 0, 1],
      "special": 1, "unk1a": 14269
    }
  ]
}
```

Coordinates in `pos` are raw game coordinates (not scaled). The tool auto-detects
whether to encode as float32 or int16 based on type. If all pos values are ints,
they are packed as signed 16-bit (two's complement).

The `dir` vector is the "look" direction and `up` is the "up" vector. Alignment
flags are automatically computed by the `mkpads` tool.

### Pad flags (internal, per-pad in binary)

These flags are auto-computed by `mkpads` from the JSON fields:

| Flag | Value | Trigger |
|---|---|---|
| PADFLAG_INTPOS | 0x0001 | pos values are all integers |
| PADFLAG_UPALIGNTOX | 0x0002 | up aligned to ±X axis |
| PADFLAG_UPALIGNTOY | 0x0004 | up aligned to ±Y axis |
| PADFLAG_UPALIGNTOZ | 0x0008 | up aligned to ±Z axis |
| PADFLAG_UPALIGNINVERT | 0x0010 | up sum is -1 (inverted) |
| PADFLAG_LOOKALIGNTOX | 0x0020 | dir aligned to ±X axis |
| PADFLAG_LOOKALIGNTOY | 0x0040 | dir aligned to ±Y axis |
| PADFLAG_LOOKALIGNTOZ | 0x0080 | dir aligned to ±Z axis |
| PADFLAG_LOOKALIGNINVERT | 0x0100 | dir sum is -1 (inverted) |
| PADFLAG_HASBBOXDATA | 0x0200 | bbox differs from defaults |
| PADFLAG_AIWAITLIFT | 0x0400 | aiwaitlift=true |
| PADFLAG_AIONLIFT | 0x0800 | aionlift=true |
| PADFLAG_AIWALKDIRECT | 0x1000 | aiwalkdirect=true |
| PADFLAG_AIDROP | 0x2000 | aidrop=true |
| PADFLAG_AICROUCH | 0x4000 | aicrouch=true |
| PADFLAG_AIIGNOREY | 0x8000 | aiignorey=true |
| PADFLAG_AIDUCK | 0x10000 | aiduck=true |

Waypoints and waygroups define the AI navigation graph. Each waypoint is linked
to a pad and to a waygroup. Waygroups connect rooms for inter-room pathfinding.

> **The waypoint graph MUST be symmetric (bidirectional) or bots crash the game.**
> Perfect Dark's route finder (`waypointFindRoute` / `waypointDiscoverSteps` in
> `padhalllv.c`) discovers step numbers outward from the source through each
> node's neighbour list, then reconstructs the path by walking neighbour links
> *back* from the destination. If A lists B as a neighbour but B does not list A,
> the back-walk can hit a NULL neighbour and dereference it (`EXC_BAD_ACCESS` at
> `curto->step += 10000`). A naive k-nearest graph is **not** symmetric.
>
> `pdmap` now builds a symmetric graph automatically: for every edge A→B it also
> adds B→A (`MapDef.pack_pads_json()` in `core.py`). As a belt-and-braces
> measure the engine also has a NULL-guard in `waypointFindRoute` so an imperfect
> custom graph degrades to "no route" instead of crashing.

### Binary layout (generated by `mkpads`)

```
u32 num_pads
u32 num_cover
u32 waypoints_start
u32 waygroups_start
u32 cover_start
--- pad offset table ---
u16 offset_to_pad_0
u16 offset_to_pad_1
...
--- pad records (variable length per flags) ---
u8 flags, u8 room|liftnum, u8: 3-bit flags, 10-bit room, 4-bit liftnum
s16/s32 pos_x, pos_y, pos_z  (int16 or float32 depending on PADFLAG_INTPOS)
float32 up_x, up_y, up_z    (skipped if axis-aligned)
float32 dir_x, dir_y, dir_z  (skipped if axis-aligned)
float32 xmin, xmax, ymin, ymax, zmin, zmax  (skipped if default)
--- waypoint records ---
...
--- waygroup records ---
...
--- cover records ---
float32 pos_x, pos_y, pos_z
float32 dir_x, dir_y, dir_z
u16 special_type
s16 unk1a
```

---

## 6. The setup format

The setup binary is generated by `MapDef.pack_setup()` in `tools/pdmap/core.py`
(there is **no** `setup_packer.py`). Layout:

```
8-word header (struct n64_stagesetup, all big-endian offsets from file base):
  word[0] = ptr_waypoints (always 0)
  word[1] = ptr_waygroups (always 0)
  word[2] = ptr_cover (always 0)
  word[3] = ptr_intro
  word[4] = ptr_props     = 0x20
  word[5] = ptr_paths
  word[6] = ptr_ailists
  word[7] = ptr_padfiledata (always 0)

--- props binary (at offset 0x20) ---
Each prop subclass packs its own data. All have a type word at the start.
Props end with EndProps (type 0x34 = one u32).

--- intro commands (at ptr_intro) ---
Variable length records, each with a code u32 first.
Terminator is code 12 (one u32).

--- ailist bytecode + ailist table (at ptr_ailists) ---  ← see §6.1, REQUIRED for MP
--- paths table (at ptr_paths) ---
  One null struct n64_path (8 bytes) terminator when there are no paths.
```

The host port preprocesses this N64-format file into the host struct layout in
`port/src/preprocess/filesetup.c` (`convertSetup` → `convertProps`,
`convertIntro`, `convertLists`, `convertAiLists`, `convertPaths`). The binary is
then RareZip-compressed by `assetmgr.zip()` for deployment.

### 6.1 The ailist table — REQUIRED to spawn simulants (was a silent bug)

> **This was the root cause of "no simulants ever appear".** The original
> `pack_setup()` wrote only `header + props + intro + 8 null bytes`, so
> `ptr_ailists` pointed at an *empty* table. Combat Simulator stages need an
> ailist to actually place their bots — without it, simulants are allocated at
> the origin and immediately fall out of the world.

How it works:

- The engine reads the setup's `ailists[]` table (`struct ailist { u8 *list; s32 id; }`,
  null-terminated) and registers each list (`setup.c`, `g_StageSetup.ailists`).
- For every ailist whose **id ≥ 0x1000**, `game_00b820.c` auto-creates a hidden
  "background" chr (chrnum = `id - 0x60`, so id `0x1000` → chrnum **4000**) and
  runs that ailist at match start. No `chr(...)` entry is needed in the setup.
- The standard MP init ailist (id `0x1000`) runs:
  `mp_init_simulants` (→ `botSpawnAll`), `rebuild_teams`, `rebuild_squadrons`,
  then `set_ailist(CHR_SELF, GAILIST_IDLE)`, `endlist`.

AI bytecode is **2-byte big-endian opcode + args**; total command length comes
from `g_CommandLengths[opcode]` (`chrai.c`). The list ends at `endlist`
(`cmd[1] == 0x04`). The bytecode `pack_setup()` emits for id `0x1000`:

```
01 85           mp_init_simulants    (opcode 0x0185, len 2)
01 45           rebuild_teams        (opcode 0x0145, len 2)
01 46           rebuild_squadrons    (opcode 0x0146, len 2)
00 05 fd 00 00  set_ailist(CHR_SELF=0xfd, GAILIST_IDLE=0x0000)  (opcode 0x0005, len 5)
00 04           endlist              (opcode 0x0004, len 2)
```

File layout produced: `... intro, ailist_bytecode (4-byte aligned),
ailist_table {ptr_list, 0x1000}{0,0}, paths_terminator`. The preprocessor copies
each list (length via `chraiGetAilistLength`) and relocates the table pointers.

`pdmap` now emits this automatically for every map, so any generated combat-sim
stage gets working bots with no manual editing.

### Intro command field semantics (CRITICAL — was wrong in mklevel.py)

| Command | Code | Format | param1 | param2 | Notes |
|---|---|---|---|---|---|
| `Spawn` | 0 | 3 words | pad index | — | player will spawn here |
| `Weapon` | 1 | 4 words | weapon ID | **dual-wield weapon ID or -1 for single** | NOT a pad index! `invGiveSingleWeapon()` when param2=-1, `invGiveDoubleWeapon()` otherwise |
| `Ammo` | 2 | 4 words | ammo type | **quantity** (e.g. 100) | NOT a pad index! |
| `Outfit` | 5 | 2 words | outfit ID | — | player outfit |
| `WatchTime` | 7 | 3 words | minutes | seconds | time of day |
| `Case` | 9 | 4 words | team | pad index | CTF case location |
| `CaseRespawn` | 10 | 4 words | team | pad index | CTF case respawn |
| `Hill` | 11 | 2 words | pad index | — | King of the Hill zone |
| `End` | 12 | 1 word | — | — | terminator |

### Weapon IDs (relevant ones)

| Constant | Value | Weapon |
|---|---|---|
| WEAPON_FALCON2 | 0x0027 | Falcon 2 |
| WEAPON_CMP150 | 0x0029 | CMP 150 |
| WEAPON_AR34 | 0x0033 | AR 34 |
| WEAPON_MAGSEC4 | 0x002e | MagSec 4 |
| WEAPON_SHOTGUN | 0x0030 | Shotgun |
| WEAPON_SNIPERRIFLE | 0x0032 | Sniper Rifle |
| WEAPON_DRAGON | 0x003a | Dragon |
| WEAPON_KNIFE | 0x003f | Combat Knife |
| WEAPON_GRENADE | 0x0028 | Grenade |
| WEAPON_REMOTEMINE | 0x002f | Remote Mine |
| WEAPON_PROXIMITYMINE | 0x0035 | Proximity Mine |
| WEAPON_SHIELD | 0x0040 | Shield |
| WEAPON_NIGHTVISION | 0x0041 | Night Vision |
| WEAPON_SLOWMOTION | 0x0042 | Slow Motion (watch) |

### Ammo types

| Constant | Value | For weapon |
|---|---|---|
| AMMO_FALCON | 0x01 | Falcon 2 |
| AMMO_CMP150 | 0x02 | CMP 150 |
| AMMO_AR34 | 0x03 | AR 34 |
| AMMO_MAGSEC4 | 0x04 | MagSec 4 |
| AMMO_SHOTGUN | 0x05 | Shotgun |
| AMMO_SNIPER | 0x06 | Sniper Rifle |
| AMMO_DRAGON | 0x07 | Dragon |
| AMMO_KNIFE | 0x0b | Combat Knife |
| AMMO_GRENADE | 0x0c | Grenade |
| AMMO_REMOTEMINE | 0x0e | Remote Mine |
| AMMO_PROXIMITYMINE | 0x0f | Proximity Mine |

---

## 7. The `pdmap` system

`pdmap` is a unified CLI that replaces `mklevel.py`. It handles every step of the
map creation pipeline: scaffolding, building, deployment, validation, and inspection.

### CLI commands

| Command | What it does |
|---|---|
| `pdmap init <name>` | Scaffold a new level from template |
| `pdmap build <name>` | Full pipeline (JSON → binary + assemble + deploy) |
| `pdmap info <name>` | Show statistics from compiled binaries |
| `pdmap validate <name>` | Run checks for common errors |
| `pdmap deploy <name>` | Copy built assets to mod directories |
| `pdmap list` | List all levels in `src/levels/` |

### Quick start

```bash
# Scaffold a new map
python3 tools/pdmap.py init mymap

# Edit the level module
vim src/levels/mymap.py

# Build and deploy
python3 tools/pdmap.py build mymap --deploy

# Validate
python3 tools/pdmap.py validate mymap
```

### Intelligent validation

`pdmap validate` catches the bugs we actually hit during development:

1. **Room mismatch** — each pad's room must exist in tiles binary `num_rooms`
2. **Intro format** — Weapon/ammo param2 semantics checked against known command codes
3. **Cover refs** — all pad references in covers must be valid
4. **Prop pack sanity** — verifies every prop produces non-empty binary
5. **Spawn first** — warns if first intro command isn't Spawn

---

## 8. Level module pattern (`src/levels/<name>.py`)

Each level is defined by a Python module in `src/levels/`. The module must export
a `build() -> MapDef` function.

### Minimal module

```python
from tools.pdmap.core import MapDef
from tools.pdmap.props import StdObject
from tools.pdmap.intro import Spawn, Weapon, Ammo


def build() -> MapDef:
    g = MapDef("mymap")

    g.add_pad(0, x=0.0, y=20.0, z=0.0, room=1)
    g.add_pad(1, x=256.0, y=20.0, z=256.0, room=1)

    g.add_cover(256.0, 0.0, y=10.0, dir_z=-1, special=1)

    g.add_prop(StdObject(model=2, pad=0))

    g.add_intro(Spawn(pad=0))
    g.add_intro(Spawn(pad=1))
    g.add_intro(Weapon(weapon_id=0x0027, dualweapon=-1))
    g.add_intro(Ammo(ammotype=0x01, quantity=100))

    return g
```

### Custom tiles JSON

If your level needs custom tiles (beyond the template), export a
`build_tiles_json() -> dict` function:

```python
def build_tiles_json() -> dict:
    from tools.pdmap.tiles import gen_quad_tile_data, gen_tiles_json
    verts, tiles = gen_quad_tile_data(
        -256, -256, 256, 256,
        floor_y=0.0, ceil_y=256.0,
        room=1,
    )
    return gen_tiles_json("mymap_tiles", verts, tiles)
```

---

## 9. Runtime loading — what happens at stage init

When a multiplayer match starts, the engine (`src/game/bg.c`, `src/game/mplayer/setup.c`):

1. **Loads tiles** — `cdInitTiles()` reads the tiles binary, sets up collision grid
2. **Loads seg** — `bg_load_stage()` decompresses sections, links room geometry
3. **Loads pads** — `pdLoad()` reads pad positions and cover links
4. **Loads setup** — `setupLoad()` reads props and intro commands
5. **Processes intro** — `playerreset.c` scans intro for `INTROCMD_SPAWN` → populates
   `g_SpawnPoints[]`. Weapons/ammo from intro are placed at their referenced pads.
6. **Spawns players** — `playerChooseSpawnLocation()` picks a spawn by distance scoring
   from `g_SpawnPoints[]`

### Simulant (bot) spawn pipeline — the full chain

Getting bots into a custom MP arena requires **three** independent things to be
correct. Missing any one yields "no simulants" (or a crash). In call order:

1. **Quick-team config** (`title.c` for `--test-map`, or the menu otherwise):
   ```c
   g_MpSetup.chrslots = 0x01;                        // player 1 ONLY (see gotcha)
   g_Vars.mpquickteam = MPQUICKTEAM_PLAYERSANDSIMS;  // enable bots
   g_Vars.mpquickteamnumsims = 8;                     // requested simulants
   g_Vars.mpsimdifficulty = BOTDIFF_NORMAL;
   ```
   `mpStartMatch()` → `mpConfigureQuickTeamSimulants()` loops `numsims` times,
   each `mpGetSlotForNewBot()` (first FREE bot slot) + `mpCreateBotFromProfile()`
   (fills the bot's config and sets its `chrslots` bit).

   > **chrslots gotcha:** `mpGetSlotForNewBot()` returns the first bot slot whose
   > `chrslots` bit is *unset*, capped at slot 7. If you pre-set the simulant bits
   > (e.g. `chrslots = 0x01 | 0x0ff0`) **before** quick-team runs, every slot
   > looks occupied → it always returns the capped slot 7 → all 8 creations
   > overwrite the same config while slots 0–6 stay configless → bots can't spawn.
   > Set **only the player bit** and let quick-team allocate the bot slots.

2. **Bot allocation** during stage setup (`setup.c`, `botmgrAllocateBot`): creates
   each bot chr at the **origin `(0,0,0)` in room −1**, AI = `MA_AIBOTMAINLOOP`.
   At this point the bots exist but are *not yet placed*.

   > **8-bots cap:** `maxsimulants` is **4** unless the `MPFEATURE_8BOTS` challenge
   > is unlocked. A fresh profile gets 4 bots even if you ask for 8. This is
   > stock behaviour, not a bug.

3. **Bot placement** via the MP init ailist (see §6.1): `mp_init_simulants` →
   `botSpawnAll()` → `botSpawn()` → `scenarioChooseSpawnLocation()` (uses
   `g_SpawnPoints[]` from the intro's `INTROCMD_SPAWN` commands) +
   `chrMoveToPos()` to ground each bot. **If the setup has no id-`0x1000` ailist,
   this never runs and the bots stay at `(0,0,0)`/room −1 and fall into the void.**

Diagnostic signature of each failure mode (observed via a one-shot prop census):

| Symptom | active chr | bot pos / room | Cause |
|---|---|---|---|
| no bots at all | 0 | — | quick-team disabled, or chrslots gotcha |
| bots invisible, fall forever | 4 | `(0,-99900,0)` room `[-1]` | no `0x1000` ailist (placement never ran) |
| crash a few seconds in | 4 | `(2629,101,0)` room `[1]` | asymmetric waypoints (nav back-walk NULL) |
| **working** | 4 | `(x,~100,z)` room `[1]`, moving | all three correct |

Without `g_Vars.mpquickteam != MPQUICKTEAM_NONE`, `mpConfigureQuickTeamSimulants()`
is never called and no bots are created.

### Props vs simulants — different spawn paths

Weapon/ammo **pickups** (props) are placed directly from the setup `props[]`
block during stage setup and get their room from their pad — they render
immediately and are **not** affected by the bot pipeline above. If "pickups don't
show" but the census reports `weapon`/`obj` props active and onscreen, they *are*
rendering; they may just be small/scattered (e.g. an oversized arena). A prop is
drawn only when one of its `prop->rooms[]` has `ROOMFLAG_ONSCREEN`
(`propsTick` → `propsSort` populate `g_Vars.onscreenprops`).

### The `--moddir` requirement

Without `--moddir mods/mod_allinone`, the game loads assets from embedded ROM data.
Custom tiles/pads/setup files only exist in the mod directory. Always pass
`--moddir mods/mod_allinone` when testing custom maps.

---

## 10. Stage registration — the four wiring points

To make the game actually load your new map, you must wire it at four points:

### Step 1: `src/include/files.h`

Add five `FILE_*` constants (one per asset file, sequential IDs):

```c
#define FILE_BG_MYMAP_SEG    0x00dd   // pick next available
#define FILE_BG_MYMAP_TILES  0x00de
#define FILE_BG_MYMAP_PADS   0x00df
#define FILE_USETUPMYMAP     0x00e0
#define FILE_UMP_SETUPMYMAP  0x00e1
```

### Step 2: `src/assets/<romid>/files/list.c`

Add paths at the matching indices (the array is indexed by FILE_* value):

```c
/*0x00dd*/ "bgdata/bg_mymap.seg",
/*0x00de*/ "bgdata/bg_mymap_tiles",
/*0x00df*/ "bgdata/bg_mymap_pads",
/*0x00e0*/ "bgdata/Usetupmymap",
/*0x00e1*/ "bgdata/Ump_setupmymap",
```

### Step 3: `src/game/stagetable.c`

Add a `g_Stages[]` row:

```c
{
    STAGE_TEST_MYMAP,   // stage ID from constants.h
    0,                  // unk
    0x7FFF, 0x7FFF, 0x7FFF, // ambient, wall, hill light
    0,                  // unk
    FILE_BG_MYMAP_SEG,
    FILE_BG_MYMAP_TILES,
    FILE_BG_MYMAP_PADS,
    FILE_USETUPMYMAP,
    FILE_UMP_SETUPMYMAP,
    // ...bbox-cull, alarm, extragunmem...
},
```

### Step 4: `src/game/mplayer/setup.c`

Add to `g_MpArenas[]`:

```c
{ STAGE_TEST_MYMAP, 0, 0, },  // { stage, requirefeature, nameLangId }
```

And update `groups[]` offsets if you inserted anywhere other than the end.

---

## 11. Common pitfalls (and fixes)

### 11.1 Player falls through floor

**Symptoms:** Player spawns, falls endlessly, never lands.

**Causes:**
1. Pad room > tiles `num_rooms` — `cdCollectGeoForCyl()` skips collision
2. Pad position outside seg Section-3 bbox — no room assignment
3. Tiles floor tile missing — no collision surface at that location

**Fix:** `pdmap validate <name>` checks room mismatch. Verify pad Y is above
floor Y (typically `pad_y = floor_y + 20`).

### 11.2 Intro weapon doesn't appear

**Symptoms:** Weapon intro command processes but player gets no weapon.

**Cause:** `INTROCMD_WEAPON` param2 is a dual-wield weapon ID or -1, not a pad
index. Passing a pad index causes `invGiveDoubleWeapon()` to fail (pad index
is not a valid weapon ID, and Falcon2 doesn't support dual-wield anyway).

**Fix:**
```python
# WRONG (was in mklevel.py):
Intro.Weapon(weapon_id=0x27, pad=pad_index)

# CORRECT:
Intro.Weapon(weapon_id=0x27, dualweapon=-1)
```

### 11.3 Intro ammo doesn't appear

**Symptoms:** Ammo intro command processes but zero/small ammo given.

**Cause:** `INTROCMD_AMMO` param2 is quantity, not a pad index. Passing a pad
index gave absurdly large ammo or clipped.

**Fix:**
```python
# WRONG:
Intro.Ammo(ammotype=0x01, pad=pad_index)

# CORRECT:
Intro.Ammo(ammotype=0x01, quantity=100)
```

### 11.4 "Matrix white" — room renders but all white

**Cause:** `gfxMakeRoomWhite()` rewrites `G_SETCOMBINE` to `G_CC_PRIMITIVE`
for test stages. This forces flat white rendering regardless of vertex colours.

**Fix (already applied):** Change `G_CC_PRIMITIVE` → `G_CC_SHADE` in
`gfxreplace.c` and rename to `gfxMakeRoomUseShade()`. Remove the
`gDPSetPrimColor` calls in `bg.c`. Provide per-face vertex colours in
`build_custom_seg.py` via the `FACE_COLOURS` array.

To customise the test room colours, edit `scripts/build_custom_seg.py`,
change the 6 RGBA8888 values in `FACE_COLOURS`, then:
```bash
python3 scripts/build_custom_seg.py
cp scripts/bg_uff.seg build/ntsc-final/assets/files/bgdata/bg_uff.seg
cp scripts/bg_uff.seg mods/mod_allinone/files/bgdata/bg_uff.seg
```

### 11.5 Wrong map shows in menu

**Symptoms:** Selected "custom map" in Combat Simulator, a different map loaded.

**Cause:** `groups[]` offsets in `mplayer/setup.c` are absolute indices into
`g_MpArenas[]`. Inserting a new arena shifts all later indices but the group
headers weren't updated.

**Fix:** After adding an arena, check every `groups[]` entry and adjust offsets.

### 11.6 Seg section-3 bbox mismatch

**Symptoms:** Player falls through floor even though pads and tiles are correct.

**Cause:** The seg file's Section-3 bboxes don't cover the spawn positions.
The engine uses these bboxes to assign players to rooms (for collision and
rendering). If no bbox contains the spawn, the player has no room.

**Fix:** Verify the seg's bbox covers all spawn pads. For a custom seg, the
bbox in `build_custom_seg.py` must be large enough.

### 11.7 Mod binary vs source drift

**Symptoms:** Fixed source code but game still shows old behaviour.

**Cause:** The deployed mod files (`mods/mod_allinone/files/bgdata/`) contain
pre-built binaries that don't automatically update when source changes.

**Fix:** Always rebuild and redeploy:
```bash
python3 tools/pdmap.py build uff --deploy
```

### 11.8 No simulants ever appear

**Symptoms:** Combat Sim / `--test-map` loads, player + pickups are fine, but
**zero bots** (or fewer than requested) and they're never visible.

**Causes & fixes (check in this order — see §9 for the full pipeline):**

1. **Setup has no MP init ailist.** Most common for generated maps. The bots are
   allocated but never placed (`botSpawnAll` never runs), so they fall to
   `Y≈-99900` in room −1. Fix: the setup must contain an ailist with **id 0x1000**
   (`mp_init_simulants`, `rebuild_teams`, `rebuild_squadrons`). `pdmap` emits this
   automatically now (§6.1); legacy/empty setups do not.
2. **chrslots pre-set before quick-team.** Setting the simulant bits manually
   (`0x01 | 0x0ff0`) breaks `mpGetSlotForNewBot()`. Set only the player bit.
3. **Quick-team disabled.** `g_Vars.mpquickteam` must be `!= MPQUICKTEAM_NONE`.
4. **Only 4 of 8 spawn** — expected unless `MPFEATURE_8BOTS` is unlocked.

### 11.9 Game crashes a few seconds after bots spawn

**Symptoms:** Bots appear/begin moving, then `EXC_BAD_ACCESS` in
`waypointFindRoute` (`padhalllv.c`), backtrace through `navFindRoute` →
`chrGoToRoomPos` → `chrGoToProp` → `botTickUnpaused`.

**Cause:** Asymmetric waypoint navigation graph (the route back-walk dereferences
a NULL neighbour). See §5.

**Fix:** Generate a symmetric waypoint graph (automatic in `pdmap` now). The
engine also NULL-guards `waypointFindRoute` so a bad graph yields no route
instead of crashing.

### 11.10 Bots stand at the origin / pile in one corner

**Symptoms:** Bots spawn but cluster at `(0,0,0)` or one spot.

**Cause:** No usable `INTROCMD_SPAWN` pads (so `g_SpawnPoints[]` is empty when
`botSpawn` runs), or all spawns resolve to the same pad. `botSpawn` falls back to
the origin when no spawn location can be chosen.

**Fix:** Provide several `Spawn(pad=...)` intro commands at distinct pads that sit
above the floor and inside the room bbox (same requirements as player spawns).

---

## 12. Full recipe — new arena from scratch in 6 steps

### Step 1: Scaffold the level

```bash
python3 tools/pdmap.py init mymap
```

This creates `src/levels/mymap.py` and `mods/mymap_tiles.json`.

### Step 2: Define geometry

Edit `scripts/build_custom_seg.py` to generate `bg_mymap.seg` with room geometry
and Section-3 bboxes that cover your playable area. Copy an existing stage's
seg as a starting point.

### Step 3: Edit the level module

Edit `src/levels/mymap.py`:

```python
from tools.pdmap.core import MapDef
from tools.pdmap.props import StdObject, Door
from tools.pdmap.intro import Spawn, Weapon, Ammo
from tools.pdmap.tiles import gen_quad_tile_data, gen_tiles_json

def build() -> MapDef:
    g = MapDef("mymap")
    g.add_pad(0, x=0.0, y=20.0, z=0.0, room=1)
    g.add_pad(1, x=128.0, y=20.0, z=128.0, room=1)
    g.add_intro(Spawn(pad=0))
    g.add_intro(Spawn(pad=1))
    g.add_intro(Weapon(0x0027, dualweapon=-1))
    g.add_intro(Ammo(0x01, quantity=100))
    g.add_cover(128.0, 128.0, y=10.0, dir_z=-1, special=1)
    g.add_prop(StdObject(model=1, pad=0))
    return g
```

### Step 4: Wire the stage

Add `FILE_*` IDs in `files.h`, paths in `list.c`, a `g_Stages[]` row in
`stagetable.c`, and a `g_MpArenas[]` entry in `mplayer/setup.c`.
See §10 for details.

### Step 5: Build and deploy

```bash
python3 tools/pdmap.py build mymap --deploy
```

This generates pads JSON → compiles pads binary, generates tiles JSON (or
copies from template) → compiles tiles binary, assembles + compresses setup
binary, and deploys everything to `mods/mod_allinone/files/bgdata/`.

### Step 6: Validate and test

```bash
python3 tools/pdmap.py validate mymap
python3 tools/pdmap.py info mymap
```

Launch the game with:
```bash
cd build && ./pd.arm64 --test-map mymap --moddir mods/mod_allinone
```

---

## Appendix A: Prop type reference

| Type | Class | Code | Description |
|---|---|---|---|
| `GenericObject` | base | 0x00 | Generic world object |
| `Door` | Door | 0x01 | Animated door |
| `DoorScale` | DoorScale | 0x02 | Door scale modifier |
| `StdObject` | StdObject | 0x03 | Static object |
| `Key` | Key | 0x04 | Key object |
| `Camera` | Camera | 0x06 | Security camera |
| `AmmoCrate` | AmmoCrate | 0x07 | Ammo crate |
| `Weapon` | Weapon (props) | 0x08 | Weapon on ground |
| `Chr` | Chr | 0x09 | Character spawn |
| `SingleMonitor` | SingleMonitor | 0x0a | Single monitor |
| `MultiMonitor` | MultiMonitor | 0x0b | Multi monitor |
| `Autogun` | Autogun | 0x0d | Automatic gun |
| `LinkGuns` | LinkGuns | 0x0e | Link gun ports |
| `Debris` | Debris | 0x0f | Debris object |
| `Hat` | Hat | 0x11 | Hat object |
| `LiftDoor` | LiftDoor | 0x13 | Lift door |
| `AmmoCrateMulti` | AmmoCrateMulti | 0x14 | Multi ammo crate |
| `Shield` | Shield | 0x15 | Shield object |
| `Tag` | Tag | 0x16 | Stage tag (objectives) |
| `BeginObjective` | BeginObjective | 0x17 | Begin objective block |
| `EndObjective` | EndObjective | 0x18 | End objective block |
| `RequireObjectDestroyed` | RequireObjectDestroyed | 0x19 | Objective: destroy |
| `CompleteFlags` | CompleteFlags | 0x1a | Set complete flag |
| `FailFlags` | FailFlags | 0x1b | Set fail flag |
| `RequireObjectCollected` | RequireObjectCollected | 0x1c | Objective: collect |
| `RequireObjectThrown` | RequireObjectThrown | 0x1d | Objective: throw |
| `RequireObjectHolographed` | RequireObjectHolographed | 0x1e | Objective: holograph |
| `RequireRoomEntered` | RequireRoomEntered | 0x20 | Objective: enter room |
| `RequireObjectThrownInRoom` | RequireObjectThrownInRoom | 0x21 | Objective: throw in room |
| `Briefing` | Briefing | 0x23 | Briefing text |
| `GasBottle` | GasBottle | 0x24 | Gas bottle |
| `RenameObject` | RenameObject | 0x25 | Rename object |
| `PadlockedDoor` | PadlockedDoor | 0x26 | Padlocked door |
| `Glass` | Glass | 0x2a | Breakable glass |
| `Safe` | Safe | 0x2b | Safe object |
| `SafeItem` | SafeItem | 0x2c | Safe contents |
| `CameraPos` | CameraPos | 0x2e | Camera position |
| `TintedGlass` | TintedGlass | 0x2f | Tinted glass |
| `Lift` | Lift | 0x30 | Elevator/lift |
| `ConditionalScenery` | ConditionalScenery | 0x31 | Conditional scenery |
| `BlockedPath` | BlockedPath | 0x32 | Blocked AI path |
| `Hoverbike` | Hoverbike | 0x33 | Hoverbike vehicle |
| `EndProps` | EndProps | 0x34 | End of props marker |
| `HoverProp` | HoverProp | 0x35 | Hovering prop |
| `Fan` | Fan | 0x36 | Fan object |
| `Hovercar` | Hovercar | 0x37 | Hovercar vehicle |
| `PadEffect` | PadEffect | 0x38 | Pad effect trigger |
| `Chopper` | Chopper | 0x39 | Helicopter |
| `Mine` | Mine | 0x3a | Proximity mine |
| `EscaStep` | EscaStep | 0x3b | Escalator step |
