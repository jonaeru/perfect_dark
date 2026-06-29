#!/usr/bin/env bash
# Starts the uff map editor dev server (serve_editor.py) and opens the browser.
# Intended to be invoked from Perfect Dark Map Editor.app or directly:
#   ./scripts/launch-map-editor.sh
#
# Reuses an already-healthy server (ports 8765–8775) or starts a fresh one with --auto-port.
# Stays in the foreground so Dock Quit (Cmd+Q) stops the server child process.

set -euo pipefail

readonly APP_TITLE="Perfect Dark Map Editor"
readonly DEFAULT_HOST="127.0.0.1"
readonly DEFAULT_PORT="8765"
readonly PORT_MIN="8765"
readonly PORT_MAX="8775"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
# shellcheck source=map-editor-app-common.sh
source "$SCRIPT_DIR/map-editor-app-common.sh"

REPO_ROOT="${PD_REPO_ROOT:-$(cd "$SCRIPT_DIR/.." && pwd)}"
REPO_ROOT="$(pd_normalize_repo_root "$REPO_ROOT")"
VIEWER_DIR="${PD_EDITOR_DIR:-$REPO_ROOT/journal/uff_viewer}"
SERVE_SCRIPT="$VIEWER_DIR/serve_editor.py"
STATE_DIR="${PD_EDITOR_STATE_DIR:-$VIEWER_DIR}"
PID_FILE="$STATE_DIR/.editor_server.pid"
PORT_FILE="$STATE_DIR/.editor_server.port"
SERVER_PID=""
ACTIVE_PORT=""
LOG_FILE="$PD_EDITOR_LOG"

log() {
	pd_log "$LOG_FILE" "$@"
}

notify() {
	pd_notify "$1" "${2:-$APP_TITLE}"
}

show_dialog() {
	pd_show_dialog "$1" "${2:-$APP_TITLE}" "$LOG_FILE"
}

editor_url_for_port() {
	printf 'http://%s:%s/' "$DEFAULT_HOST" "$1"
}

server_healthy_on() {
	pd_editor_health_ok "$DEFAULT_HOST" "$1"
}

read_pid_file() {
	if [[ -f "$PID_FILE" ]]; then
		cat "$PID_FILE" 2>/dev/null || true
	fi
}

read_port_file() {
	if [[ -f "$PORT_FILE" ]]; then
		cat "$PORT_FILE" 2>/dev/null || true
	fi
}

pid_alive() {
	local pid="$1"
	[[ -n "$pid" ]] && kill -0 "$pid" 2>/dev/null
}

cleanup() {
	trap - EXIT INT TERM
	if [[ -n "$SERVER_PID" ]] && pid_alive "$SERVER_PID"; then
		kill "$SERVER_PID" 2>/dev/null || true
		wait "$SERVER_PID" 2>/dev/null || true
	fi
	rm -f "$PID_FILE" "$PORT_FILE"
}
trap cleanup EXIT INT TERM

# Wait until the child server responds on /api/health (port file optional).
wait_for_server_ready() {
	local attempt
	local port_from_file=""
	for attempt in $(seq 1 60); do
		port_from_file="$(read_port_file || true)"
		if [[ -n "$port_from_file" ]] && server_healthy_on "$port_from_file"; then
			ACTIVE_PORT="$port_from_file"
			return 0
		fi
		if ACTIVE_PORT="$(pd_find_healthy_editor_port "$DEFAULT_HOST" "$PORT_MIN" "$PORT_MAX")"; then
			if [[ -n "$SERVER_PID" ]] && pid_alive "$SERVER_PID"; then
				log "Server healthy on port ${ACTIVE_PORT} (port file: ${PORT_FILE})"
				return 0
			fi
		fi
		if [[ -n "$SERVER_PID" ]] && ! pid_alive "$SERVER_PID"; then
			return 1
		fi
		sleep 0.15
	done
	return 1
}

open_existing_session() {
	local url
	url="$(editor_url_for_port "$ACTIVE_PORT")"
	log "Reusing healthy server at ${url}"
	notify "Opening existing editor session at ${url}" "$APP_TITLE"
	if ! open "$url"; then
		log "ERROR: open failed for ${url}"
		pd_show_dialog "Could not open the browser for ${url}.

Log: ${LOG_FILE}" "$APP_TITLE" "$LOG_FILE"
		exit 1
	fi
	exit 0
}

kill_stale_server() {
	local pid="$1"
	log "Stopping stale server PID ${pid}"
	kill "$pid" 2>/dev/null || true
	wait "$pid" 2>/dev/null || true
	rm -f "$PID_FILE" "$PORT_FILE"
}

show_start_failure() {
	local detail
	detail="$(pd_log_tail "$LOG_FILE" 20 | tr '\n' ' ' | sed 's/  */ /g')"
	pd_show_dialog "The map editor server failed to start.

Log: ${LOG_FILE}

Common fix: install Python 3.10+ (brew install python). Finder often uses /usr/bin/python3 (3.9), which is too old for pdmap.

Recent log:
${detail}" "$APP_TITLE" "$LOG_FILE"
}

main() {
	log "=== launch-map-editor start (REPO_ROOT=${REPO_ROOT}, EDITOR_DIR=${VIEWER_DIR}, PATH=${PATH:-<empty>}) ==="

	if ! pd_ensure_local_file "$SERVE_SCRIPT" 12; then
		log "ERROR: serve_editor.py not found or not downloaded at ${SERVE_SCRIPT}"
		show_dialog "Map editor server script not found or still downloading from iCloud.\n\nExpected:\n${SERVE_SCRIPT}\n\nREPO_ROOT: ${REPO_ROOT}\n\nIf using iCloud Drive: Finder → right-click repo folder → Download Now.\n\nOr move the .app inside this repository or rebuild with ./scripts/build-map-editor-app.sh"
		exit 1
	fi

	pd_hydrate_repo_root "$REPO_ROOT"

	local python_bin
	if ! python_bin="$(pd_resolve_python)"; then
		log "ERROR: no Python 3.10+ found (PATH=${PATH:-<empty>})"
		show_dialog "Python 3.10 or newer is required but was not found.\n\nInstall Python 3 (Homebrew: brew install python) or set PD_PYTHON to a 3.10+ interpreter.\n\nLog: ${LOG_FILE}"
		exit 1
	fi
	log "Using python: ${python_bin} ($("$python_bin" --version 2>&1))"

	# Reuse any healthy editor already listening in the port range.
	if ACTIVE_PORT="$(pd_find_healthy_editor_port "$DEFAULT_HOST" "$PORT_MIN" "$PORT_MAX")"; then
		open_existing_session
	fi

	# Stale pid from a prior session: stop it if it never became healthy.
	local existing_pid saved_port
	existing_pid="$(read_pid_file)"
	saved_port="$(read_port_file || echo "$DEFAULT_PORT")"
	if pid_alive "$existing_pid"; then
		if server_healthy_on "$saved_port"; then
			ACTIVE_PORT="$saved_port"
			open_existing_session
		fi
		log "Stale PID ${existing_pid} on port ${saved_port} is not healthy; stopping"
		kill_stale_server "$existing_pid"
	fi

	# Start a fresh server; --auto-port picks 8765 or the next free port in range.
	cd "$REPO_ROOT"
	mkdir -p "$STATE_DIR"
	rm -f "$PORT_FILE"
	log "Starting serve_editor.py on ${DEFAULT_HOST}:${DEFAULT_PORT} (--auto-port) from ${SERVE_SCRIPT} (state=${STATE_DIR})"
	PD_REPO_ROOT="$REPO_ROOT" PD_EDITOR_STATE_DIR="$STATE_DIR" \
		"$python_bin" "$SERVE_SCRIPT" --host "$DEFAULT_HOST" --port "$DEFAULT_PORT" --auto-port >>"$LOG_FILE" 2>&1 &
	SERVER_PID=$!
	echo "$SERVER_PID" >"$PID_FILE"
	log "Server PID ${SERVER_PID}"

	if ! wait_for_server_ready; then
		if [[ -n "$SERVER_PID" ]] && ! pid_alive "$SERVER_PID"; then
			log "ERROR: server process ${SERVER_PID} exited before becoming healthy"
		else
			log "ERROR: server failed health check (PID ${SERVER_PID}, port file ${PORT_FILE})"
		fi
		show_start_failure
		exit 1
	fi

	local url
	url="$(editor_url_for_port "$ACTIVE_PORT")"
	log "Server healthy on port ${ACTIVE_PORT}; opening ${url}"
	notify "Editor running on port ${ACTIVE_PORT} — quit this app from the Dock to stop the server." "$APP_TITLE"
	if ! open "$url"; then
		log "ERROR: open failed for ${url}"
		pd_show_dialog "Editor server is running but the browser could not be opened.

Open manually: ${url}

Log: ${LOG_FILE}" "$APP_TITLE" "$LOG_FILE"
		exit 1
	fi

	# Foreground wait: Cmd+Q on this app sends SIGTERM and cleanup stops python.
	wait "$SERVER_PID" || true
	log "=== launch-map-editor exit ==="
}

main "$@"
