#!/usr/bin/env bash
# Shared helpers for Perfect Dark map editor .app launchers (Finder-safe PATH, logging).

readonly PD_LOG_DIR="${HOME}/Library/Logs"
readonly PD_EDITOR_LOG="${PD_LOG_DIR}/PerfectDarkMapEditor.log"
readonly PD_PLAY_LOG="${PD_LOG_DIR}/PerfectDarkPlayLastTest.log"

pd_ensure_log_dir() {
	mkdir -p "$PD_LOG_DIR"
}

pd_log() {
	local log_file="$1"
	shift
	pd_ensure_log_dir
	printf '[%s] %s\n' "$(date '+%Y-%m-%d %H:%M:%S')" "$*" >>"$log_file"
}

# Prefer Python 3.10+ (pdmap uses PEP 604 unions). GUI/Finder PATH often exposes only /usr/bin/python3 (3.9).
pd_resolve_python() {
	local candidates=()
	local seen="|"
	local candidate

	if [[ -n "${PD_PYTHON:-}" && -x "${PD_PYTHON}" ]]; then
		candidates+=("$PD_PYTHON")
	fi

	# python.org framework installs and user pip --user bins are common on macOS but absent from Finder PATH.
	local framework_py="/Library/Frameworks/Python.framework/Versions/Current/bin/python3"
	local user_py
	for candidate in \
		/opt/homebrew/bin/python3 \
		/usr/local/bin/python3 \
		"$framework_py" \
		"${HOME}/Library/Python/3.14/bin/python3" \
		"${HOME}/Library/Python/3.13/bin/python3" \
		"${HOME}/Library/Python/3.12/bin/python3" \
		"${HOME}/Library/Python/3.11/bin/python3" \
		"${HOME}/Library/Python/3.10/bin/python3" \
		"$(command -v python3 2>/dev/null || true)" \
		/usr/bin/python3; do
		[[ -n "$candidate" && -x "$candidate" ]] || continue
		case "$seen" in *"|${candidate}|"*) continue ;; esac
		seen="${seen}${candidate}|"
		candidates+=("$candidate")
	done

	for candidate in "${candidates[@]}"; do
		if "$candidate" -c 'import sys; sys.exit(0 if sys.version_info >= (3, 10) else 1)' 2>/dev/null; then
			printf '%s' "$candidate"
			return 0
		fi
	done

	return 1
}

# Open the editor log in TextEdit so Finder failures are visible after a brief dialog.
pd_open_log_in_editor() {
	local log_file="$1"
	[[ -f "$log_file" ]] || return 0
	if command -v open >/dev/null 2>&1; then
		open -a TextEdit "$log_file" >/dev/null 2>&1 || open "$log_file" >/dev/null 2>&1 || true
	fi
}

pd_show_dialog() {
	local message="$1"
	local title="${2:-Perfect Dark Map Editor}"
	local log_file="${3:-}"
	if command -v osascript >/dev/null 2>&1; then
		osascript - "$message" "$title" >/dev/null 2>&1 <<'APPLESCRIPT' || true
on run argv
	display dialog (item 1 of argv) with title (item 2 of argv) buttons {"OK"} default button "OK" giving up after 120
end run
APPLESCRIPT
	else
		printf '%s: %s\n' "$title" "$message" >&2
	fi
	if [[ -n "$log_file" ]]; then
		pd_open_log_in_editor "$log_file"
	fi
}

pd_notify() {
	local message="$1"
	local title="${2:-Perfect Dark Map Editor}"
	if command -v osascript >/dev/null 2>&1; then
		osascript -e "display notification $(printf '%q' "$message") with title $(printf '%q' "$title")" >/dev/null 2>&1 || true
	fi
}

# Return 0 when the editor health endpoint responds on host:port.
pd_editor_health_ok() {
	local host="$1"
	local port="$2"
	curl -sf --max-time 2 "http://${host}:${port}/api/health" >/dev/null 2>&1
}

# First port in [min,max] with a healthy editor server, or empty string.
pd_find_healthy_editor_port() {
	local host="$1"
	local min_port="$2"
	local max_port="$3"
	local port
	for port in $(seq "$min_port" "$max_port"); do
		if pd_editor_health_ok "$host" "$port"; then
			printf '%s' "$port"
			return 0
		fi
	done
	return 1
}

pd_log_tail() {
	local log_file="$1"
	local lines="${2:-15}"
	if [[ -f "$log_file" ]]; then
		tail -n "$lines" "$log_file" 2>/dev/null || true
	fi
}

# iCloud Drive may leave placeholders that pass -f/-x but cannot be read or executed.
# Retry with brctl download, parent-dir hydration, and Finder touch when available.
pd_ensure_local_file() {
	local file_path="$1"
	local max_attempts="${2:-10}"
	local attempt
	local parent_dir

	[[ -n "$file_path" ]] || return 1

	for attempt in $(seq 1 "$max_attempts"); do
		if [[ -f "$file_path" ]] && [[ -r "$file_path" ]] && head -c 1 "$file_path" >/dev/null 2>&1; then
			return 0
		fi
		parent_dir="$(dirname "$file_path")"
		if command -v brctl >/dev/null 2>&1; then
			brctl download "$file_path" 2>/dev/null || true
			[[ -d "$parent_dir" ]] && brctl download "$parent_dir" 2>/dev/null || true
		fi
		# Later attempts: ask Finder to materialize the placeholder (works when brctl alone stalls).
		if [[ "$attempt" -ge 3 ]] && command -v osascript >/dev/null 2>&1; then
			osascript - "$file_path" >/dev/null 2>&1 <<'APPLESCRIPT' || true
on run argv
	set p to POSIX file (item 1 of argv)
	try
		tell application "Finder" to update p
	end try
end run
APPLESCRIPT
		fi
		sleep 0.35
	done

	return 1
}

# Read repo path baked into the .app at build time (Contents/Resources/repo_root.txt).
pd_read_baked_repo_root() {
	local app_bundle="$1"
	local repo_file="${app_bundle}/Contents/Resources/repo_root.txt"
	local baked=""

	[[ -f "$repo_file" ]] || return 1
	IFS= read -r baked <"$repo_file" || return 1
	baked="${baked#"${baked%%[![:space:]]*}"}"
	baked="${baked%"${baked##*[![:space:]]}"}"
	[[ -n "$baked" ]] || return 1
	printf '%s' "$baked"
}

# Walk up from the .app bundle looking for journal/uff_viewer/serve_editor.py.
pd_discover_repo_root() {
	local app_bundle="$1"
	local serve_marker="${2:-journal/uff_viewer/serve_editor.py}"
	local candidate="$app_bundle"

	for _ in 1 2 3 4 5 6 7 8 9 10; do
		if pd_ensure_local_file "${candidate}/${serve_marker}" 3; then
			printf '%s' "$candidate"
			return 0
		fi
		local parent
		parent="$(dirname "$candidate")"
		[[ "$parent" == "$candidate" ]] && break
		candidate="$parent"
	done
	return 1
}

# Prefer repo copy of the editor; fall back to bundled Contents/Resources/editor/.
pd_resolve_editor_dir() {
	local repo_root="$1"
	local app_bundle="$2"
	local repo_viewer="${repo_root}/journal/uff_viewer"
	local bundled="${app_bundle}/Contents/Resources/editor"

	if pd_ensure_local_file "${repo_viewer}/serve_editor.py" 8; then
		printf '%s' "$repo_viewer"
		return 0
	fi
	if [[ -f "${bundled}/serve_editor.py" ]] && [[ -r "${bundled}/serve_editor.py" ]]; then
		printf '%s' "$bundled"
		return 0
	fi
	return 1
}

# Nudge key repo paths off iCloud before pdmap / game binary access.
pd_hydrate_repo_root() {
	local repo_root="$1"
	[[ -n "$repo_root" ]] || return 1
	pd_ensure_local_file "${repo_root}/tools/pdmap/core.py" 8 || true
	pd_ensure_local_file "${repo_root}/Makefile" 4 || true
	return 0
}
