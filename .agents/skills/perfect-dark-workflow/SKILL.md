---
name: perfect-dark-workflow
description: Essential commands, build steps, and coordinate system rules for modifying and compiling the Perfect Dark AIO repository.
---

# Perfect Dark Modding Workflow

## 1. Coordinate System
- **Y-Axis**: `+Y` is UP. 
- **Ground Level**: `Y=0` is the standard floor height for flat test maps.
- **Spawns & Objectives**: Player spawns, weapons, and scenario objectives (Capture the Case, King of the Hill) should be placed exactly at the floor height (e.g., `Y=0`). Placing them too high (e.g., `Y=20`) will cause players and bots to die instantly from fall damage upon spawning.

## 2. Asset Generation
- When adding or modifying custom levels (like the Matrix Test Room `uff`), the setup binaries and JSON configurations must be manually regenerated.
- **Command**: `python3 scratch/generate_uff_assets.py` (This bakes the `UsetupuffZ` and `Ump_setupuffZ` binaries directly into the mod directories).

## 3. Building the Game
- **Command**: `make -j8`
- If compilation fails with compiler errors, verify the deployment target or adjust the concurrency (`-j` flag).

## 4. Testing and Running
- **Command Line**: `./build/pd.arm64 --test-map` (Directly loads the test map).
- **Finder**: Use `open -R test_map.command` to reveal the easy-launch script in Finder for the user to double-click.

## 5. C Code Guidelines (N64 Engine)
- **Initialization**: Always check for `NULL` pointers and division-by-zero (e.g., `numpads <= 0`). Bots and multiplayer logic often initialize and execute passes *before* the level setup `playerReset()` finishes parsing, which leaves global arrays and counts empty initially.

## 6. Combat Simulator / Simulants (hard-won rules)
Getting bots into a custom MP arena needs THREE things correct (full detail in `docs/MAP_CREATION.md` §6.1, §9, §11.8–11.10):
1. **Setup must contain an ailist with id `0x1000`** (`mp_init_simulants`, `rebuild_teams`, `rebuild_squadrons`). The engine auto-creates a background chr (chrnum `id-0x60` = 4000) to run any ailist id ≥ `0x1000` at match start; that is what calls `botSpawnAll` to place bots. An empty ailist table = bots fall to `Y≈-99900` and never appear. `pdmap` emits this automatically (`pack_setup` in `tools/pdmap/core.py`).
2. **Set only the player chrslot** (`g_MpSetup.chrslots = 0x01`) before quick-team; pre-setting the simulant bits breaks `mpGetSlotForNewBot()` so no bot configs are created.
3. **Waypoint graph must be symmetric** (bidirectional) or bot pathfinding NULL-derefs in `waypointFindRoute`. `pdmap` makes it symmetric automatically.
- Only **4** bots spawn unless the `MPFEATURE_8BOTS` challenge is unlocked (stock cap, not a bug).
- Weapon/ammo **pickups** spawn from `props[]` directly (independent of the bot pipeline) and render whenever their room is onscreen.

## 7. Debugging the runtime (PC port)
- The PC main loop is `mainTick`/`mainLoop` in **`port/src/pdmain.c`**, NOT `src/lib/main.c` (that one is N64-only and isn't compiled into `pd.arm64`).
- `sysLogPrintf(LOG_NOTE,...)` goes to **stdout (block-buffered)** — lost if you `kill` the process. Use `LOG_WARNING`/`LOG_ERROR` (stderr, unbuffered) for diagnostics you need to survive a kill.
- A one-shot **prop census** (iterate `g_Vars.props[0..maxprops]`, count `active` by `type`, and count `g_Vars.onscreenprops`) is the fastest way to tell "didn't spawn" from "spawned but not rendered/placed".
- Headless launches sometimes fail GL3.0 context creation then fall back to 4.1 core ("using core profile") — that line means GL is fine, not an error. Screenshot via `screencapture -x -o`.
