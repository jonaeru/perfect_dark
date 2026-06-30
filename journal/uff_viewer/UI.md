# Map Editor UI — Information Architecture (pass-10)

Production-quality chrome for the Perfect Dark map editor. Canonical source: `gen_uff_viewer.py` → `uff_map.html` → Electron `.app`.

## Problem (pass-9 failure mode)

Stacking file bar, play grid, and collapsible build sections inside `#mapOptions` competed with the 3D canvas. Dropdown-based file workflows felt like a web form, not a Mac document app. Incremental compaction could not fix the fundamental IA mistake: **too many jobs in one fixed panel**.

## Design rationale (pass-10)

**Separate surfaces by job** — the same pattern as Xcode, Sketch, or any native Mac editor:

| Job | Surface | What the user sees |
|-----|---------|-------------------|
| **Document** | macOS menu bar (Electron) or 28px browser strip | New, Open, Save, Save As, Export, Delete |
| **Canvas** | Full viewport | 3D view; `#hud` for layers/counts only |
| **Inspector** | `#editPanel` (edit mode only) | Tools, box sliders, validation, export/import |
| **Run** | `#runBar` (top-right, ~44px) | Mod, Scenario, ▶ Play, ⚙ Build Settings |
| **Status** | `#statusPill` (bottom-center) | Map name, dirty dot, server hint |

File operations **never** appear in the canvas overlay stack. One save path (`Save` / ⌘S). Session LocalStorage is **File → Revert to Cached**, not a primary toolbar button.

Advanced build flags (level, deploy, sims, loadout, game options, seg/deploy/rebuild) live in **Build Settings…** modal — opened from Play menu or ⚙ on the run bar.

---

## Control surfaces

| Surface | Location | Mode | Purpose |
|---------|----------|------|---------|
| Application menu | macOS menu bar (Electron) | Always | File, Edit, View, Play |
| `#browserMenuBar` | Top (browser only) | Always | Same actions when no Electron menu |
| `#runBar` | Top-right | Always | Mod, Scenario, Play, Build Settings |
| `#statusPill` | Bottom-center | Always | Document name · dirty · server |
| `#editPanel` | Top-right column | Edit only | Pad tools, box, validation |
| `#panelRight` | Below run bar | Always | View presets, fly/edit mode |
| `#hud` | Top-left | Always | Layers, legend, pad counts |
| `#buildSettingsModal` | Center overlay | On demand | All test/build options |
| `#testOutput` | Bottom-right | Collapsed | Build log, terminal fallback |

---

## File workflow

| Action | UI | Behavior |
|--------|-----|----------|
| **New** | File → New Map · ⌘N | Prompt name → starter template → server POST or LocalStorage |
| **Open file…** | File → Open… · ⌘O | Native `dialog.showOpenDialog` (Electron) or hidden file input |
| **Open saved** | File → Open Saved… | Prompt pick from server / LocalStorage catalog |
| **Drag-drop** | Canvas | Drop `.json` → parse → `loadMap()` |
| **Save** | File → Save · ⌘S | PUT `/api/maps/<name>` or LocalStorage |
| **Save As** | File → Save As… · ⌘⇧S | Prompt new name → PUT/create |
| **Export JSON** | File → Export JSON… | Download current map JSON |
| **Revert to Cached** | File → Revert to Cached | Load browser LocalStorage backup |
| **Delete** | File → Delete Map… | DELETE `/api/maps/<name>` + LocalStorage |

---

## Run workflow

| Action | UI | Behavior |
|--------|-----|----------|
| **Test Map** | Play → Test Map · `T` · ▶ Play | Validate, build, deploy, launch |
| **Export Assets** | Play → Export Assets · `X` | Build + deploy, no launch |
| **Build Settings** | Play → Build Settings… · ⚙ | Modal with level/deploy/sims/loadout/flags |

---

## Keyboard shortcuts

| Shortcut | Action |
|----------|--------|
| `⌘N` | New map |
| `⌘O` | Open file… |
| `⌘S` | Save |
| `⌘⇧S` | Save As |
| `⌘Z` / `⌘⇧Z` | Undo / Redo |
| `E` | Toggle edit mode |
| `F` | Toggle fly mode |
| `T` | Test / Play |
| `X` | Export assets |
| `Del` | Delete selected pad (edit) |
| `G` | Toggle snap grid (edit) |
| `M` | Toggle minimap (edit) |

---

## 1440×900 constraints

- Run bar height ≤60px; total primary chrome ≤120px vertical.
- Play button visible without scrolling.
- `#panelRight` at fixed `top: 64px` (88px in browser mode with menu strip).
- Build Settings modal for advanced options — not inline clutter.

---

## Implementation

| File | Role |
|------|------|
| `gen_uff_viewer.py` | HTML/CSS/JS — run bar, status pill, modal, menu bridge |
| `electron/main.js` | Native application menu + IPC |
| `electron/preload.js` | `openJsonFile`, `onMenuAction` |
| `serve_editor.py` | `/api/maps` CRUD (unchanged) |

Regenerate: `python3 journal/uff_viewer/gen_uff_viewer.py`  
Build app: `./scripts/build-map-editor-electron.sh`

---

## Validation checklist (pass-10)

| # | Check | Method | Status |
|---|-------|--------|--------|
| 1 | HTML regenerated | `python3 gen_uff_viewer.py` | ✅ PASS |
| 2 | Electron app builds | `./scripts/build-map-editor-electron.sh` | ✅ PASS |
| 3 | UI screenshot 1440×900 | Playwright → `journal/uff_evidence/ui_v2_*.png` | ✅ PASS |
| 4 | No `#mapsPanel` file bar | Playwright | ✅ PASS |
| 5 | Run bar h≤60px, chrome ≤120px | Playwright measure | ✅ PASS |
| 6 | Open file loads fixture | Playwright + `fixtures/minimal_map.json` | ✅ PASS |
| 7 | Save → reload → Open | Playwright API flow | ✅ PASS |
| 8 | Build Settings modal | Playwright | ✅ PASS |
| 9 | `curl /api/maps` | HTTP 200 + JSON | ✅ PASS |

Validated 2026-06-29. Re-run: `python3 journal/uff_viewer/validate_ui_v2.py` (requires `serve_editor.py` on :8765).

---

## User guide — standard workflow

1. **Launch** — Double-click `Perfect Dark Map Editor.app`.
2. **Open from disk** — **File → Open…** (or drag `.json` onto the canvas).
3. **Edit** — **View → Toggle Edit Mode** or press `E`; place pads on the floor.
4. **Save** — **File → Save** or ⌘S (writes to `journal/uff_viewer/maps/<name>.json`).
5. **Test** — Set Mod/Scenario on the run bar → **▶ Play** or **Play → Test Map**.
6. **Advanced** — **Play → Build Settings…** for deploy slot, sims, loadout, build flags.
