# UFF Map Editor

Browser-based 3D editor for Perfect Dark test maps (`journal/uff_viewer/`).

## Quick start (recommended — Electron app)

The **Electron launcher** embeds the editor in a native window. It does not depend on an external browser or the shell-based `.app` launcher.

1. Build once (from repo root):

   ```bash
   ./scripts/build-map-editor-electron.sh
   ```

2. **Double-click `Perfect Dark Map Editor (Electron).app`**  
   (repo-root symlink, or `scripts/release/Perfect Dark Map Editor (Electron).app`).

   The app starts `serve_editor.py`, waits for `/api/health`, and opens the map editor in an embedded window. **Quit from the Dock** (Cmd+Q) to stop the Python server.

3. Set options in the **Map options** strip (top-right): Level, Deploy, Mod, Scenario, build flags. Click **Test / Play** (or press **T**) to build assets and launch `./build/pd.arm64 --test-map`.

### Dev mode (no .app build)

```bash
cd journal/uff_viewer/electron
npm install
PD_REPO_ROOT="/path/to/perfect_dark_jonaeru_aio" npm start
```

Or: `./scripts/build-map-editor-electron.sh --dev`

## Legacy shell .app (optional)

The older shell launcher opens your default browser via `open http://127.0.0.1:8765/`:

```bash
./scripts/build-map-editor-app.sh
```

This can fail on iCloud-evicted repos or when the browser does not come to the foreground. Prefer the Electron app above.

## UI map

| Panel | Location | When visible | Purpose |
|-------|----------|--------------|---------|
| **HUD** | Top-left | Always | Layer toggles, legend, geometry counts |
| **Map options** | Top-right | Always | Level, Deploy, Mod, Scenario, box readout, Seg/Deploy/Rebuild/Play flags, Test/Play + Export |
| **View / Mode** | Top-right (below options) | Always | Camera presets, fly mode, edit mode |
| **Edit panel** | Top-right column | Edit mode (`E`) | Pad tools, box sliders, validation, JSON/Python export |
| **Pad properties** | Bottom-right | Edit + pad selected | Position, weapon, scenario fields |
| **Build log** | Bottom-right | Collapsed `<details>` | Terminal command, JSON/.sh fallback |
| **Help** | Bottom-left | Always | Keyboard cheat sheet |

See `UI.md` for layout rules and 1440×900 visibility targets.

## Optional: replay last test

After you have run Test & Play at least once, double-click **`Play Last Test Map.app`** to rerun `journal/uff_viewer/.last_play.sh` (game launch only).

## Manual / developer mode

```bash
python3 journal/uff_viewer/serve_editor.py
# open http://127.0.0.1:8765/
```

## Files

| File | Role |
|------|------|
| `uff_map.html` | Editor UI (Three.js) |
| `serve_editor.py` | Static server + `POST /api/test-map` |
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
