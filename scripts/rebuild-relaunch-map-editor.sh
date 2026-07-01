#!/usr/bin/env bash
# Rebuild Perfect Dark Map Editor (Electron) and relaunch.
#
# Usage:
#   ./scripts/rebuild-relaunch-map-editor.sh
#
# Safe to invoke from Desktop shortcut — resolves repo root from this script's path.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
APP_PATH="$SCRIPT_DIR/release/Perfect Dark Map Editor.app"
APP_NAME="Perfect Dark Map Editor"

log() {
	printf '[rebuild-map-editor] %s\n' "$*"
}

fail() {
	printf '[rebuild-map-editor] FAIL: %s\n' "$*" >&2
	exit 1
}

kill_stale_processes() {
	log "Stopping stale Map Editor process (if running)"
	if command -v osascript >/dev/null 2>&1; then
		osascript -e "quit app \"${APP_NAME}\"" >/dev/null 2>&1 || true
	fi
	pkill -f "${REPO_ROOT}/journal/uff_viewer/serve_editor.py" 2>/dev/null || true
	sleep 1
}

main() {
	cd "$REPO_ROOT"
	log "Repo: $REPO_ROOT"

	kill_stale_processes

	log "Regenerating uff_map.html …"
	if ! python3 "$REPO_ROOT/journal/uff_viewer/gen_uff_viewer.py"; then
		fail "gen_uff_viewer.py exited with an error"
	fi

	log "Building Map Editor.app (this may take a minute) …"
	if ! "$SCRIPT_DIR/build-map-editor-electron.sh" --no-symlink; then
		fail "build-map-editor-electron.sh exited with an error"
	fi

	if [[ ! -d "$APP_PATH" ]]; then
		fail "Expected app not found: $APP_PATH"
	fi

	log "Launching: $APP_PATH"
	if ! open "$APP_PATH"; then
		fail "open failed for $APP_PATH"
	fi

	log "SUCCESS — Map Editor rebuilt and launched."
}

main "$@"
