# UFF Map Editor

Browser-based 3D editor for Perfect Dark test maps (`journal/uff_viewer/`).

**Map-making wiki (workflow, seg modes, troubleshooting):**
[`docs/MAP_MAKING_WIKI.md`](../../docs/MAP_MAKING_WIKI.md)

## Quick start

1. Build once (from repo root):

   ```bash
   ./scripts/build-map-editor-electron.sh
   ```

   This regenerates `uff_map.html`, copies editor assets into the `.app` bundle (`Contents/Resources/editor/`), and runs `electron-builder`. **After changing editor HTML or Python, rebuild the `.app`** — that is the only step; no manual server restart or browser hard-refresh.

2. **Double-click `Perfect Dark Map Editor.app`** at the repo root (symlink to `scripts/release/`)

   The app spawns the bundled `serve_editor.py`, waits for `/api/health`, and opens the editor in an embedded window. UI is served from the bundle; maps and Test/Play use your repo via baked `PD_REPO_ROOT`. **Quit from the Dock** (Cmd+Q) to stop the Python server.

3. Set options in the **Map options** strip (top-right): Level, Deploy, Mod, Scenario, sims, loadout, game flags. Click **Test / Play** (or press **T**) to build assets and launch `./build/pd.arm64 --test-map`.

The canonical bundle lives under `scripts/release/`; the repo-root `.app` is a symlink created by the build script.

### Dev mode (no .app build)

```bash
cd journal/uff_viewer/electron
npm install
PD_REPO_ROOT="/path/to/perfect_dark_jonaeru_aio" npm start
```

Or: `./scripts/build-map-editor-electron.sh --dev`

Dev mode uses repo `journal/uff_viewer/` directly (live files). For packaged behavior, rebuild the `.app`.

## UI map

| Panel | Location | When visible | Purpose |
|-------|----------|--------------|---------|
| **HUD** | Top-left | Always | Layer toggles, legend, geometry counts |
| **Map options** | Top-right | Always | Level, Deploy, Mod, Scenario, sims, loadout, game flags, **Saved maps** CRUD, Test/Play + Export |
| **View / Mode** | Top-right (below options) | Always | Camera presets, fly mode, edit mode |
| **Edit panel** | Top-right column | Edit mode (`E`) | Pad tools, box sliders, validation, JSON/Python export |
| **Pad properties** | Bottom-right | Edit + pad selected | Position, weapon, scenario fields |
| **Build log** | Bottom-right | Collapsed `<details>` | Terminal command, JSON/.sh fallback |
| **Help** | Bottom-left | Always | Keyboard cheat sheet |

See `UI.md` for layout rules and 1440×900 visibility targets.

## Optional: replay last test

After you have run Test & Play at least once:

```bash
./scripts/build-map-editor-app.sh   # builds Play Last Test Map.app only
```

Double-click **`scripts/release/Play Last Test Map.app`** to rerun `.last_play.sh` (game launch only).

## Manual / developer mode

```bash
python3 journal/uff_viewer/serve_editor.py
# open http://127.0.0.1:8765/
```

Or CLI: `./scripts/launch-map-editor.sh` (opens default browser).

## Saved maps (document workflow)

With the Map Editor `.app` (or manual `serve_editor.py` in dev), maps persist as JSON under **`journal/uff_viewer/maps/`** (gitignored) when the repo is writable; otherwise under **`~/Library/Application Support/PerfectDarkMapEditor/maps/`**. Use the **Map document** section in the top-right **Map options** panel — like a normal document app.

| Action | UI | Shortcut |
|--------|-----|----------|
| **New** | **New** (prompts for name if empty) | **Ctrl+N** |
| **Open** | Select map → **Open** (or double-click list) | **Ctrl+O** |
| **Save** | **Save** (writes disk when server online) | **Ctrl+S** |
| **Save As** | Enter name in **Save as** → **Save As** | — |
| **Delete** | Select map → **Delete** (confirmation) | — |

The current map name appears at the top of the panel; **●** indicates unsaved edits.

REST API (localhost only): `GET/POST /api/maps`, `GET/PUT/DELETE /api/maps/<name>`. When the server is offline, **Save** falls back to browser **LocalStorage** (`pdmap_editor_<name>`) with a clear status message — it does not silently pretend disk save succeeded.

**Important:** Use the Map Editor `.app` or `http://127.0.0.1:8765/` — not a `file://` open of `uff_map.html` — the page needs the local server for disk CRUD and Test/Play.

## Bundled editor (`.app`)

At build time, these files are copied to `Perfect Dark Map Editor.app/Contents/Resources/editor/`:

| File | Role |
|------|------|
| `uff_map.html` | Editor UI (Three.js) — always served from bundle |
| `serve_editor.py` | Static server + `/api/maps` CRUD + `POST /api/test-map` |
| `test_map.py` | Build + deploy + `--play` pipeline |
| `json_to_level.py` | JSON → `src/levels/*.py` helper |
| `manifest.json` | Build timestamp + file list |

Repo `journal/uff_viewer/` remains the source of truth for development; the `.app` ships a snapshot so iCloud eviction or stale HTML cannot break launch.

## Files (repo source)

| File | Role |
|------|------|
| `uff_map.html` | Editor UI (Three.js) |
| `serve_editor.py` | Static server + `/api/maps` CRUD + `POST /api/test-map` |
| `test_map.py` | Build + deploy + `--play` pipeline |
| `electron/` | Electron shell (`main.js`, `package.json`) |
| `gen_uff_viewer.py` | Regenerate HTML from level data |
| `UI.md` | Information architecture + design principles |

## Troubleshooting

- **Log file:** `~/Library/Logs/PerfectDarkMapEditor.log`
- **Python:** Requires 3.10+ (`brew install python`). Set `PD_PYTHON` if needed.
- **Repo path:** Set `PD_REPO_ROOT` if auto-discovery fails (common with iCloud paths containing spaces).
- **iCloud eviction:** Finder → right-click repo folder → **Download Now**, then relaunch.
- Player spawns and objectives should sit at floor height (`Y=0` or pad `Y=10`), not high in the air.
- The editor server only accepts localhost origins.
- If port **8765** is busy, the server auto-picks the next free port in **8765–8775**.
- **Deploy → uff (test-map slot)** unless your level is registered in `stagetable.c`.
