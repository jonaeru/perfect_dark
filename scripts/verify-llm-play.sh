#!/usr/bin/env bash
# End-to-end verification for LLM Play: game bridge, /act smoke, one Gemma decision.
#
# Usage:
#   ./scripts/verify-llm-play.sh
#   ./scripts/verify-llm-play.sh --keep   # leave game + adapter running on success
#
# Exit 0 = all checks passed. Logs: ~/Library/Logs/PerfectDarkLlmPlay.log

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
LOG_FILE="${HOME}/Library/Logs/PerfectDarkLlmPlay.log"
LOCK_FILE="${HOME}/Library/Logs/PerfectDarkLlmPlay.lock"
GAME_BIN="$REPO_ROOT/build/pd.arm64"
BRIDGE_URL="http://127.0.0.1:8780"
KEEP_RUNNING=0

while [[ $# -gt 0 ]]; do
	case "$1" in
	--keep) KEEP_RUNNING=1; shift ;;
	-h | --help)
		echo "Usage: $0 [--keep]"
		exit 0
		;;
	*) echo "Unknown option: $1" >&2; exit 2 ;;
	esac
done

log() {
	printf '[verify-llm] %s\n' "$*"
}

fail() {
	printf '[verify-llm] FAIL: %s\n' "$*" >&2
	exit 1
}

cleanup() {
	if [[ "$KEEP_RUNNING" -eq 1 ]]; then
		log "keeping game + adapter running (--keep)"
		return
	fi
	log "stopping test processes"
	pkill -f "${REPO_ROOT}/tools/game_llm/run_adapter.py" 2>/dev/null || true
	pkill -f "${REPO_ROOT}/build/pd.arm64" 2>/dev/null || true
	if command -v lsof >/dev/null 2>&1; then
		lsof -ti :8780 2>/dev/null | xargs kill -9 2>/dev/null || true
	fi
}

trap cleanup EXIT

kill_stale() {
	log "clearing stale game/adapter processes"
	pkill -f "${REPO_ROOT}/tools/game_llm/run_adapter.py" 2>/dev/null || true
	pkill -f "${REPO_ROOT}/build/pd.arm64" 2>/dev/null || true
	if command -v lsof >/dev/null 2>&1; then
		lsof -ti :8780 2>/dev/null | xargs kill -9 2>/dev/null || true
	fi
	sleep 2
}

wait_for_bridge() {
	local deadline=$((SECONDS + 120))
	while ((SECONDS < deadline)); do
		if curl -sf --max-time 2 "${BRIDGE_URL}/api/llm/health" >/dev/null 2>&1; then
			return 0
		fi
		sleep 1
	done
	return 1
}

wait_for_ai() {
	local deadline=$((SECONDS + 180))
	while ((SECONDS < deadline)); do
		local health
		health="$(curl -sf --max-time 2 "${BRIDGE_URL}/api/llm/health" 2>/dev/null || true)"
		if echo "$health" | grep -q '"adapter_connected":1'; then
			log "AI connected"
			return 0
		fi
		sleep 3
	done
	return 1
}

main() {
	cd "$REPO_ROOT"

	LOCK_DIR="${LOCK_FILE}.d"
	if ! mkdir "$LOCK_DIR" 2>/dev/null; then
		log "another verify/repair is running — skip"
		exit 0
	fi
	# Chain lock release with existing cleanup trap.
	trap 'cleanup; rmdir "$LOCK_DIR" 2>/dev/null || true' EXIT

	[[ -x "$GAME_BIN" ]] || fail "missing $GAME_BIN — run: cd build && cmake --build . -j8"

	if ! curl -sf --max-time 3 http://127.0.0.1:11434/api/tags >/dev/null 2>&1; then
		fail "Ollama not running. Open Ollama app, then: ollama pull gemma3:4b"
	fi

	if ! curl -sf --max-time 10 http://127.0.0.1:11434/api/tags | grep -q 'gemma3:4b'; then
		log "pulling gemma3:4b (first run may take several minutes)"
		ollama pull gemma3:4b
	fi

	kill_stale
	python3 "$SCRIPT_DIR/ensure-llm-pd-ini.py" "$REPO_ROOT/pd.ini" >/dev/null
	log "ensuring test-map seg (PDMAP_SEG_MODE=empty — no viewport phantom walls)"
	python3 "$SCRIPT_DIR/ensure-llm-map-assets.py" || fail "test-map seg deploy failed — see MAP_CREATION.md §11.11"

	log "starting game"
	mkdir -p "$(dirname "$LOG_FILE")"
	nohup "$GAME_BIN" --test-map --moddir mods/mod_allinone >>"$LOG_FILE" 2>&1 &
	GAME_PID=$!
	disown "$GAME_PID" 2>/dev/null || true
	log "game pid=$GAME_PID"

	wait_for_bridge || fail "bridge did not start within 120s — see $LOG_FILE"
	log "bridge up"

	log "health check"
	curl -sf --max-time 3 "${BRIDGE_URL}/api/llm/health" | python3 -m json.tool

	log "act smoke"
	curl -sf --max-time 5 -X POST "${BRIDGE_URL}/api/llm/act" \
		-H 'Content-Type: application/json' \
		-d '{"player":{"stick_y":80,"buttons":["Z"]}}' | python3 -m json.tool

	log "adapter --once (Gemma inference, up to ~2 min)"
	python3 "$REPO_ROOT/tools/game_llm/run_adapter.py" --game perfect_dark --once

	log "starting adapter loop"
	nohup python3 "$REPO_ROOT/tools/game_llm/run_adapter.py" --game perfect_dark --backend auto >>"$LOG_FILE" 2>&1 &
	ADAPTER_PID=$!
	disown "$ADAPTER_PID" 2>/dev/null || true
	log "adapter pid=$ADAPTER_PID"

	wait_for_ai || fail "adapter did not connect within 180s — see $LOG_FILE and journal/game_llm/perfect_dark/adapter_status.json"

	log "final health"
	curl -sf --max-time 3 "${BRIDGE_URL}/api/llm/health" | python3 -m json.tool

	log "PASS — LLM Play stack verified"
}

main "$@"
