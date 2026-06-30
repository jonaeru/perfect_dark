#!/usr/bin/env bash
# Launched by LLM Play.app — starts game + Gemma adapter.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=map-editor-app-common.sh
source "$SCRIPT_DIR/map-editor-app-common.sh"
# shellcheck source=ensure-ollama-external-drive.sh
source "$SCRIPT_DIR/ensure-ollama-external-drive.sh"

readonly APP_TITLE="LLM Play"
readonly APP_TITLE_AI="LLM Play — AI Controlling"
readonly APP_TITLE_WAIT="LLM Play — Waiting for AI"
readonly APP_TITLE_BRIDGE="LLM Play — Bridge On"
readonly LOG_FILE="${HOME}/Library/Logs/PerfectDarkLlmPlay.log"
readonly LLM_HEALTH_URL="http://127.0.0.1:8780/api/llm/health"

log() {
	pd_log "$LOG_FILE" "$@"
}

# macOS notification with optional subtitle (LLM status in the title bar text).
llm_notify() {
	local message="$1"
	local title="$2"
	local subtitle="${3:-}"
	if command -v osascript >/dev/null 2>&1; then
		if [[ -n "$subtitle" ]]; then
			osascript -e "display notification $(printf '%q' "$message") with title $(printf '%q' "$title") subtitle $(printf '%q' "$subtitle")" >/dev/null 2>&1 || true
		else
			pd_notify "$message" "$title"
		fi
	else
		printf '%s: %s\n' "$title" "$message" >&2
	fi
}

llm_fetch_health() {
	curl -sf --max-time 2 "$LLM_HEALTH_URL" 2>/dev/null || true
}

llm_health_field() {
	local json="$1"
	local field="$2"
	python3 -c 'import json,sys; d=json.loads(sys.argv[1]); print(d.get(sys.argv[2], 0))' "$json" "$field" 2>/dev/null || echo 0
}

llm_wait_for_adapter() {
	local max_wait="${1:-45}"
	local i health connected
	for i in $(seq 1 "$max_wait"); do
		health="$(llm_fetch_health)"
		[[ -n "$health" ]] || break
		connected="$(llm_health_field "$health" adapter_connected)"
		if [[ "$connected" == "1" ]]; then
			return 0
		fi
		sleep 1
	done
	return 1
}

# Keep the .app alive in the Dock and refresh the notification title while AI is active.
llm_status_supervisor() {
	local last_connected=-1
	local tick=0

	while true; do
		local health connected
		health="$(llm_fetch_health)"
		if [[ -z "$health" ]]; then
			llm_notify "Game closed or bridge stopped." "$APP_TITLE" "Session ended"
			return 0
		fi

		connected="$(llm_health_field "$health" adapter_connected)"
		if [[ "$connected" == "1" && "$last_connected" != "1" ]]; then
			llm_notify "Hands off the controls — Gemma is playing." "$APP_TITLE_AI" "Player + bots"
		elif [[ "$connected" == "1" ]]; then
			tick=$((tick + 1))
			if (( tick % 10 == 0 )); then
				llm_notify "Still controlling player and bots." "$APP_TITLE_AI" "Gemma active"
			fi
		elif [[ "$connected" == "0" && "$last_connected" != "0" ]]; then
			llm_notify "Bridge is up; waiting for the first AI action…" "$APP_TITLE_WAIT" "Start Ollama if needed"
		fi
		last_connected="$connected"
		sleep 3
	done
}

main() {
	REPO_ROOT="${PD_REPO_ROOT:-$(cd "$SCRIPT_DIR/.." && pwd)}"
	REPO_ROOT="$(pd_normalize_repo_root "$REPO_ROOT")"
	pd_hydrate_repo_root "$REPO_ROOT" || true
	cd "$REPO_ROOT"

	log "=== LLM Play start (REPO_ROOT=$REPO_ROOT) ==="

	local ollama_models_root
	if ollama_models_root="$(pd_ensure_ollama_external_drive 2>&1)"; then
		log "Ollama models: $ollama_models_root"
	else
		pd_show_dialog "External dev drive or Ollama model path is unavailable.

Mount: /Volumes/4TEx Dev main
Or set PD_OLLAMA_MODELS to your ollama-models folder.

$ollama_models_root

Log: $LOG_FILE" "LLM Play — Error" "$LOG_FILE"
		exit 1
	fi

	local game="$REPO_ROOT/build/pd.arm64"
	if [[ ! -x "$game" ]]; then
		pd_show_dialog "Build the game first:

cd build && cmake .. && cmake --build . -j8

Log: $LOG_FILE" "$APP_TITLE" "$LOG_FILE"
		exit 1
	fi

	local py
	py="$(pd_resolve_python)" || {
		pd_show_dialog "Python 3.10+ required for the LLM adapter.

Install: brew install python
Or set PD_PYTHON

Log: $LOG_FILE" "$APP_TITLE" "$LOG_FILE"
		exit 1
	}

	# PC port reads [LLM] section keys (Enabled, PlayerMask), not flat LLM.* lines.
	local ini_changes
	ini_changes="$("$py" "$REPO_ROOT/scripts/ensure-llm-pd-ini.py" "$REPO_ROOT/pd.ini" 2>&1)" || {
		pd_show_dialog "Could not update pd.ini [LLM] section.

$ini_changes

Log: $LOG_FILE" "LLM Play — Error" "$LOG_FILE"
		exit 1
	}
	log "pd.ini [LLM]: $ini_changes"

	log "ensuring test-map seg (PDMAP_SEG_MODE=empty — no viewport phantom walls)"
	"$py" "$REPO_ROOT/scripts/ensure-llm-map-assets.py" >>"$LOG_FILE" 2>&1 || {
		pd_show_dialog "Could not deploy empty bg_uff.seg for LLM Play.

See MAP_CREATION.md §11.11 and log: $LOG_FILE" "LLM Play — Error" "$LOG_FILE"
		exit 1
	}

	pd_notify "Starting game bridge…" "$APP_TITLE_BRIDGE"

	if ! curl -sf --max-time 2 "$LLM_HEALTH_URL" >/dev/null 2>&1; then
		log "Launching game"
		nohup "$game" --test-map --moddir mods/mod_allinone >>"$LOG_FILE" 2>&1 &
		local i
		for i in $(seq 1 90); do
			curl -sf --max-time 2 "$LLM_HEALTH_URL" >/dev/null 2>&1 && break
			sleep 1
		done
	fi

	if ! curl -sf --max-time 2 "$LLM_HEALTH_URL" >/dev/null 2>&1; then
		pd_show_dialog "Game bridge did not start.

Make sure Ollama is optional for launch — the game window must open.
See log: $LOG_FILE" "LLM Play — Error" "$LOG_FILE"
		exit 1
	fi

	llm_notify "Bridge ready — starting Gemma adapter." "$APP_TITLE_BRIDGE" "Perfect Dark is running"

	log "Starting adapter ($py)"
	nohup "$py" "$REPO_ROOT/tools/game_llm/run_adapter.py" --game perfect_dark --backend auto >>"$LOG_FILE" 2>&1 &

	local dialog_title="$APP_TITLE_WAIT"
	local dialog_body="Bridge is on. Waiting for Gemma to send the first action.

• Models path: $ollama_models_root
• If empty, run: ollama pull gemma3:4b
• Log: $LOG_FILE"

	if llm_wait_for_adapter 45; then
		dialog_title="$APP_TITLE_AI"
		dialog_body="AI is controlling the game now.

• Do not touch keyboard/controller to watch Gemma play
• Dock/notifications show: LLM Play — AI Controlling
• Quit Perfect Dark when finished

Log: $LOG_FILE"
		llm_notify "Gemma is driving player + bots." "$APP_TITLE_AI" "Session active"
	else
		llm_notify "Adapter started but no AI actions yet — check Ollama." "$APP_TITLE_WAIT" "Models: $ollama_models_root"
	fi

	pd_show_dialog "$dialog_body" "$dialog_title" "$LOG_FILE"

	log "=== LLM Play ready (supervisor active) ==="
	llm_status_supervisor
}

main "$@"
