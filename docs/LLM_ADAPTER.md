# Perfect Dark — Game LLM integration

Perfect Dark is the **reference implementation** of the generic [Game LLM engine](../tools/game_llm/README.md).
The reusable sidecar lives in `tools/game_llm/`; this game adds the in-game bridge and `adapters/perfect_dark/`.

## Architecture

```
pd.arm64  <--HTTP-->  tools/game_llm/run_adapter.py --game perfect_dark  -->  Ollama / MLX
   |                              |
port/llm_bridge.c (protocol v1)   adapters/perfect_dark/ (prompts + schema)
port/llm_player.c / llm_bot.c
src/game/bot.c
```

The game ticks at 60 Hz; the adapter decides at ~1 Hz and actions are held until `ActionTtlMs` expires.

## Quick start

1. `cd build && cmake .. && cmake --build . -j8`
2. `ollama pull gemma3:4b`
3. Double-click **`LLM Play.app`** — Electron control panel (build once: `./scripts/build-llm-play-app.sh`)

The window auto-starts the session. Title and status pill show **AI controlling** when Gemma is driving player + bots. **Hands off keyboard and mouse** — the AI replaces your input entirely while active.

**Important:** The game must be rebuilt after bridge/input changes (`cd build && cmake --build . -j8`). Keyboard/mouse-only setups require the input merge fix in `port/src/input.c`.

**Verify everything works** (kills stale processes, starts game, runs one Gemma decision, confirms AI connected):

```bash
./scripts/verify-llm-play.sh
```

Logs: `~/Library/Logs/PerfectDarkLlmPlay.log`

## Setup

### Build the port

```bash
mkdir -p build && cd build
cmake ..
cmake --build . -j8
```

### Bridge config

**LLM Play.app** writes the `[LLM]` section in `pd.ini` on every launch:

```ini
[LLM]
Enabled=1
Port=8780
PlayerMask=1
BotMask=15
```

`PlayerMask=1` — human player 1. `BotMask=15` — simulant slots 0–3.

**LLM Play.app** also runs `scripts/ensure-llm-map-assets.py` on every session start (and
`scripts/verify-llm-play.sh` runs it before the smoke test). That redeploys `bg_uff.seg` with
`PDMAP_SEG_MODE=empty` so in-box FPS never loads a stale wall seg (viewport phantom sheet;
see `MAP_CREATION.md` §11.11–11.12).

### Gemma backend

Models should live on the external dev drive (`/Volumes/4TEx Dev main/LLM-Data/ollama-models`).
**LLM Play.app** runs `scripts/ensure-ollama-external-drive.sh` on launch to symlink
`~/.ollama/models` and `Application Support/Ollama` there (Finder apps do not read `~/.zshrc`).

```bash
# One-time manual wiring (same as LLM Play):
source scripts/ensure-ollama-external-drive.sh && pd_ensure_ollama_external_drive

# Register gemma3:4b on the external store (writes manifests + uses existing blobs when possible):
ollama pull gemma3:4b
# Apple Silicon optional: pip install mlx-lm
```

## CLI

```bash
python3 tools/game_llm/run_adapter.py --game perfect_dark
python3 tools/game_llm/verify_bridge.py perfect_dark
python3 tools/game_llm/run_adapter.py --list-games
```

Legacy path `tools/llm_adapter/run_adapter.py` still works (defaults to `perfect_dark`).

| Flag / env | Default | Purpose |
|------------|---------|---------|
| `--game` / `GAME_LLM_ADAPTER` | `perfect_dark` | Adapter id |
| `--backend` / `GAME_LLM_BACKEND` | `auto` | `auto`, `ollama`, `mlx` |
| `--game-url` / `GAME_LLM_URL` | `http://127.0.0.1:8780` | Bridge URL |
| `--interval-ms` | `1000` | Decision interval |

## HTTP API

Documented in [tools/game_llm/protocols/BRIDGE_API.md](../tools/game_llm/protocols/BRIDGE_API.md).

Perfect Dark action JSON:

```json
{
  "player": {"stick_x": 0, "stick_y": 90, "buttons": ["Z"]},
  "bots": [{"slot": 0, "move_fwd": 0.8, "aim_yaw_delta": 3.5, "fire": true, "target_id": 2}]
}
```

## Manual smoke test (no LLM)

```bash
curl -s http://127.0.0.1:8780/api/llm/health | python3 -m json.tool
curl -s -X POST http://127.0.0.1:8780/api/llm/act \
  -H 'Content-Type: application/json' \
  -d @tools/game_llm/adapters/perfect_dark/fixtures/sample_act.json
```

## Add another game

1. In-game bridge — `tools/game_llm/protocols/BRIDGE_API.md`
2. Python adapter — `tools/game_llm/protocols/ADAPTER.md` (copy `adapters/_template/`)
3. Run — `python3 tools/game_llm/run_adapter.py --game <id>`

## Files

| Path | Role |
|------|------|
| `tools/game_llm/` | Generic engine + registry |
| `tools/game_llm/adapters/perfect_dark/` | PD prompts, schema, fixtures |
| `port/src/llm_*.c` | In-game bridge (protocol v1) |
| `LLM Play.app` | Electron control panel (build: `./scripts/build-llm-play-app.sh`) |
| `scripts/ensure-llm-map-assets.py` | Redeploy empty `bg_uff.seg` before LLM Play (phantom wall guard) |
| `tools/game_llm/electron/` | Electron UI source |
| `tools/llm_adapter/` | Deprecated shim — use `tools/game_llm/` |

Decision logs: `journal/game_llm/perfect_dark/decisions.jsonl`
