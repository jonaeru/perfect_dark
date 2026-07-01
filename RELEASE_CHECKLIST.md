# v0.1.0 release checklist (evidence)

Date: 2026-06-30  
Branch: `port-mods/all-in-one` @ pre-tag  
Platform: macOS arm64

## §15 Pre-flight (wiki)

| Check | Result |
|-------|--------|
| `pdmap validate my_arena` | 0 errors |
| `pdmap validate testarena` | 0 errors (1 WARN: no CTF Case pads — expected; Combat + KOTH only) |
| `pdmap build my_arena --seg --deploy` | OK → mod_allinone + mod_moyoteg |
| `pdmap build testarena --seg --deploy` | OK → mod_allinone + mod_moyoteg |
| `cmake --build build --target pd` | OK (`build/pd.arm64`) |
| Map Editor `.app` build | OK (`scripts/release/Perfect Dark Map Editor.app`) |

## Smoke commands (manual)

```bash
./build/pd.arm64 --moddir mods/mod_allinone --boot-stage 0x80 --skip-intro
./build/pd.arm64 --moddir mods/mod_allinone --boot-stage 0x81 --skip-intro
```

## Evidence screenshots

Stored under `journal/uff_evidence/` (committed):

- `uff_matrix_room_playable.png` — Matrix/box arena playable
- `edit_play_flow.png`, `test_play_options_flow.png` — editor Test/Play
- `editor_options_visible.png`, `ui_overhaul_verify.png` — editor UI
- `uff_full_box_stable.png`, `uff_box_default.png` — box arena stability

## Stage IDs

- `STAGE_MY_ARENA` = `0x80` — menu label "My Arena"
- `STAGE_TESTARENA` = `0x81` — menu label "Test Arena"
