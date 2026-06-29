# Map Editor UI — Information Architecture

## Problem (pass-5b regression)

Map option controls (Mod, Scenario, Deploy as, Level name, Box, build flags) lived inside `#testPanel` at **bottom-right** with `max-height: 62vh` and `overflow-y:auto`. On 1440×900 that panel overlapped `#panelRight` (view/mode, also top-right) and required scrolling past Export/status to discover options. Users saw only **Test / Play** shortcut buttons, not the option menus.

## Control surfaces (audit)

| Surface | Location | Mode | Purpose |
|---------|----------|------|---------|
| `#hud` | Top-left | Always | Layers, legend, counts |
| `#mapOptions` | **Top-right** | Always | Mod, scenario, deploy, level, box, flags, Test/Play + Export |
| `#panelRight` | Top-right (below options) | Always | View presets, fly/edit mode |
| `#editPanel` | Top-right column | Edit only | Tools, box sliders, validation, JSON/Python export |
| `#props` | Bottom-right | Edit + selection | Selected pad properties |
| `#testOutput` | Bottom-right | Always (collapsed) | Command log, terminal fallback |
| `#help` | Bottom-left | Always | Keyboard cheat sheet |

## 1440×900 — must fit without scroll

**Always visible (no scroll):**

1. Map options strip (~150px): Level, Deploy, Mod, Scenario (2×2 grid); Box + simulants readout; Seg / Deploy / Rebuild / Play flags (one row); Test/Play + Export buttons.
2. View/mode panel (~200px) directly below options strip.
3. Left HUD + bottom help as today.

**Collapsed by default:** terminal command log, advanced export, validation warnings detail.

## Design principles

- **Succinct labels:** Level, Deploy, Mod, Scenario — tooltips carry detail.
- **Native controls:** `<select>` and compact checkbox row; no custom widgets.
- **One action cluster:** Test/Play and Export adjacent to the options they use.
- **No edit-mode gate:** Map options work in orbit and fly modes; server at `:8765` optional for launch.
- **Stable IDs:** `#testLevelName`, `#testDeployAs`, `#testMod`, `#testScenario` for automation.

## Implementation

- `gen_uff_viewer.py` → `#mapOptions` panel + slim `#testOutput` details block.
- `#panelRight` offset below `#mapOptions`; edit mode shifts both left of `#editPanel`.
- Regenerate `uff_map.html`; validate with Playwright at 1440×900.
