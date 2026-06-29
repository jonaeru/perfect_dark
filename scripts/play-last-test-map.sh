#!/usr/bin/env bash
# Re-runs the last test-map shell script produced by the editor (journal/uff_viewer/.last_test.sh).

set -euo pipefail

readonly APP_TITLE="Perfect Dark — Play Last Test Map"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=map-editor-app-common.sh
source "$SCRIPT_DIR/map-editor-app-common.sh"

REPO_ROOT="${PD_REPO_ROOT:-$(cd "$SCRIPT_DIR/.." && pwd)}"
APP_SUPPORT="${HOME}/Library/Application Support/PerfectDarkMapEditor"
LAST_SCRIPT="$REPO_ROOT/journal/uff_viewer/.last_play.sh"
LAST_JSON="$REPO_ROOT/journal/uff_viewer/.last_test.json"
FALLBACK_SCRIPT="$REPO_ROOT/journal/uff_viewer/.last_test.sh"
# Bundled editor writes artifacts to Application Support when repo viewer is evicted.
if [[ ! -f "$LAST_SCRIPT" && -f "$APP_SUPPORT/.last_play.sh" ]]; then
	LAST_SCRIPT="$APP_SUPPORT/.last_play.sh"
fi
if [[ ! -f "$LAST_JSON" && -f "$APP_SUPPORT/.last_test.json" ]]; then
	LAST_JSON="$APP_SUPPORT/.last_test.json"
fi
if [[ ! -f "$FALLBACK_SCRIPT" && -f "$APP_SUPPORT/.last_test.sh" ]]; then
	FALLBACK_SCRIPT="$APP_SUPPORT/.last_test.sh"
fi
LOG_FILE="$PD_PLAY_LOG"

log() {
	pd_log "$LOG_FILE" "$@"
}

show_dialog() {
	pd_show_dialog "$1" "${2:-$APP_TITLE}"
}

notify() {
	pd_notify "$1" "${2:-$APP_TITLE}"
}

main() {
	log "=== play-last-test-map start (REPO_ROOT=${REPO_ROOT}, PATH=${PATH:-<empty>}) ==="
	pd_hydrate_repo_root "$REPO_ROOT" || true
	cd "$REPO_ROOT"

	if [[ ! -f "$LAST_SCRIPT" ]]; then
		if [[ -f "$FALLBACK_SCRIPT" ]]; then
			LAST_SCRIPT="$FALLBACK_SCRIPT"
			log "Using fallback script ${FALLBACK_SCRIPT}"
		else
			log "ERROR: no last test script (.last_play.sh / .last_test.sh)"
			show_dialog "No last test map found.\n\nUse the map editor and click Test & Play (or Shift+T) at least once, then try again.\n\nExpected script:\njournal/uff_viewer/.last_play.sh\n\nLog: ${LOG_FILE}"
			exit 1
		fi
	fi

	if [[ ! -x "$LAST_SCRIPT" ]]; then
		chmod +x "$LAST_SCRIPT"
	fi

	notify "Launching last test map…" "$APP_TITLE"
	log "Running ${LAST_SCRIPT}"

	# Run build + play pipeline from the saved shell artifact.
	if ! "$LAST_SCRIPT" >>"$LOG_FILE" 2>&1; then
		log "ERROR: ${LAST_SCRIPT} failed"
		show_dialog "Last test map launch failed.\n\nSee log:\n${LOG_FILE}\n\nOr run manually:\n${LAST_SCRIPT}"
		exit 1
	fi

	log "=== play-last-test-map exit ==="
	# .last_test.sh ends with exec on the game binary when --play was used.
}

main "$@"
