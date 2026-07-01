# Release v0.1.0 — moyoteg/perfect_dark fork

**Tag:** `v0.1.0`  
**Branch:** `port-mods/all-in-one`  
**Platform:** macOS arm64 only (Apple Silicon)

## Highlights

- **All-in-One mod bundle** (`mods/mod_allinone`) plus personal deploy slot (`mods/mod_moyoteg`).
- **Combat Simulator custom maps:** `my_arena` (all six scenarios) and `testarena` (Combat + King of the Hill).
- **Perfect Dark Map Editor** desktop app for JSON → build → deploy → Test/Play.
- **Deterministic pdmap pipeline** (`tools/pdmap.py from-json`, `validate`, `register`).
- **BYO ROM** — no game assets are bundled in this release.

## Credits

Built on [jonaeru/perfect_dark](https://github.com/jonaeru/perfect_dark) (`port-mods/all-in-one`) and the upstream [fgsfdsfgs/perfect_dark](https://github.com/fgsfdsfgs/perfect_dark) PC port. Map tooling and custom arenas by moyoteg.

## Build from source (macOS arm64)

1. Clone and checkout the tag:
   ```bash
   git clone https://github.com/moyoteg/perfect_dark.git
   cd perfect_dark
   git checkout v0.1.0
   ```
2. Provide your own NTSC-final ROM (`pd.ntsc-final.z64`, md5 `e03b088b6ac9e0080440efed07c1e40f`):
   ```bash
   mkdir -p data
   ln -sf /path/to/your/pd.ntsc-final.z64 data/pd.ntsc-final.z64
   ln -sf data/pd.ntsc-final.z64 pd.ntsc-final.z64
   ```
3. Install deps: `brew install cmake sdl2 zlib python3`
4. Build:
   ```bash
   cmake -G"Unix Makefiles" -Bbuild -DCMAKE_OSX_ARCHITECTURES=arm64 .
   cmake --build build --target pd -j8
   ```
5. Build the map editor (optional):
   ```bash
   ./scripts/build-map-editor-electron.sh
   # → scripts/release/Perfect Dark Map Editor.app
   ```

## Run with custom maps

Always pass the mod directory:

```bash
./build/pd.arm64 --moddir mods/mod_allinone
```

**Combat Sim → Custom Maps:** `My Arena`, `Test Arena`

**Direct boot (smoke test, skip intro):**

```bash
./build/pd.arm64 --moddir mods/mod_allinone --boot-stage 0x80 --skip-intro   # My Arena
./build/pd.arm64 --moddir mods/mod_allinone --boot-stage 0x81 --skip-intro   # Test Arena
```

**Editor Test/Play shortcut:**

```bash
./build/pd.arm64 --test-map --moddir mods/mod_allinone
```

## Map validation

```bash
python3 tools/pdmap.py validate my_arena
python3 tools/pdmap.py validate testarena
```

## Out of scope for v0.1.0

- Prebuilt game binaries or mod zips in GitHub Releases (source + editor app only).
- Linux / Intel macOS builds.
- LLM Play tooling.
- Upstream merge to jonaeru/origin.

## Documentation

- [`docs/MAP_MAKING_WIKI.md`](docs/MAP_MAKING_WIKI.md) — start here (§15 pre-flight checklist).
- [`docs/MAP_CREATION.md`](docs/MAP_CREATION.md) — binary reference.
- [`docs/MAP_DETERMINISTIC_SPEC.md`](docs/MAP_DETERMINISTIC_SPEC.md) — pdmap pipeline contract.
