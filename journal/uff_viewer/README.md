# UFF Map Editor

Browser-based 3D editor for Perfect Dark test maps (`journal/uff_viewer/`).

## Quick start (no terminal)

1. Build the macOS launcher once (from repo root):

   ```bash
   ./scripts/build-map-editor-app.sh
   ```

2. **Double-click `Perfect Dark Map Editor.app`**  
   (in the repo root symlink, or `scripts/release/Perfect Dark Map Editor.app`).

   The app starts the local dev server, opens `http://127.0.0.1:8765/`, and shows a notification. **Quit the app from the Dock** (Cmd+Q) to stop the server.

3. Set options in the **Map options** strip (top-right): Level, Deploy, Mod, Scenario, build flags. Click **Test / Play** (or press **T**) to build assets and launch `./build/pd.arm64 --test-map`.

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
| `gen_uff_viewer.py` | Regenerate HTML from level data |
| `UI.md` | Information architecture + design principles |

## Notes

- Player spawns and objectives should sit at floor height (`Y=0` or pad `Y=10`), not high in the air.
- The editor server only accepts localhost origins.
- If the app says the server is already running, another session is using port **8765** — the app opens the existing tab instead of starting a duplicate.
- **Deploy → uff (test-map slot)** unless your level is registered in `stagetable.c`.
