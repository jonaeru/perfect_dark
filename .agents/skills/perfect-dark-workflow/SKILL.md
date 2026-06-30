---
name: perfect-dark-workflow
description: Essential commands, build steps, and coordinate system rules for modifying and compiling the Perfect Dark AIO repository.
---

# Perfect Dark Modding Workflow

**Master map-making guide:** [`docs/MAP_MAKING_WIKI.md`](../../docs/MAP_MAKING_WIKI.md)
(deploy, seg modes, viewport artifacts, Test/Play). Deep binary reference:
[`docs/MAP_CREATION.md`](../../docs/MAP_CREATION.md).

## 1. Coordinate System
- **Y-Axis**: `+Y` is UP.
- **Floor tiles**: `Y=0` is the standard floor height for flat box arenas.
- **Pads (spawns, weapons, props)**: Place at **`Y=10`** (slightly above floor). The ground search requires `floor_y < pad_y`; `Y=0` pads fall through. Do not use `Y=20+` — fall damage on spawn.

## 2. Asset Generation
- **Command**: `python3 tools/pdmap.py build uff --seg --deploy`
- Box arenas: Test/Play uses `PDMAP_SEG_MODE=empty` (no visible seg — avoids viewport sheet). Use `full` only when you want wall preview and accept possible near-plane clip in FPS.
- Always pass `--moddir mods/mod_allinone` when playing custom assets.

## 3. Building the Game
- **Command**: `make -j8`

## 4. Testing and Running
- **Command Line**: `./build/pd.arm64 --test-map --moddir mods/mod_allinone`
- **Map Editor**: `Perfect Dark Map Editor.app` → Test / Play (T)
- **Editor backend**: `python3 journal/uff_viewer/test_map.py map.json --level uff --seg --deploy --play`

## 5. C Code Guidelines (N64 Engine)
- Always check for `NULL` pointers and division-by-zero. Bots may initialize before level setup finishes.

## 6. Combat Simulator / Simulants (hard-won rules)
Three requirements (detail in wiki §8 and `MAP_CREATION.md` §11.8–11.10):
1. Setup ailist **id `0x1000`** (`mp_init_simulants`, …). `pdmap` emits this via `add_loadout_intro`.
2. **`g_MpSetup.chrslots = 0x01`** only before quick-team (`--test-map` in `title.c`).
3. **Symmetric waypoint graph** (pdmap adds reverse edges).
- Stock cap: **4 bots** unless `MPFEATURE_8BOTS` unlocked.

## 7. Seg / viewport pitfalls
- **Phantom wall at origin** (collidable, bullet holes): bad `G_VTX(24)` — rebuild with per-face `G_VTX(4)` (`python3 tools/pdmap.py build uff --seg --deploy`).
- **Dark sheet on screen** (visual only): in-box near-plane clip — use `PDMAP_SEG_MODE=empty`.
- Never deploy stale seg (unchecking **Seg** in editor skips rebuild).

## 8. Debugging the runtime (PC port)
- Main loop: `port/src/pdmain.c` (`mainTick`/`mainLoop`).
- `LOG_WARNING`/`LOG_ERROR` → stderr (survives kill). `LOG_NOTE` → stdout buffer.
- Prop census: iterate `g_Vars.props`, count `active` by `type`.
