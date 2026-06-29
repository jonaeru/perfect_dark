#!/usr/bin/env bash
# Build double-clickable macOS .app bundles for the uff map editor workflow.
#
# Outputs:
#   scripts/release/Perfect Dark Map Editor.app
#   scripts/release/Play Last Test Map.app
#   (optional symlink) ./Perfect Dark Map Editor.app
#
# Usage:
#   ./scripts/build-map-editor-app.sh
#   ./scripts/build-map-editor-app.sh --no-symlink

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
RELEASE_DIR="$SCRIPT_DIR/release"
BUILD_DIR="$REPO_ROOT/.tmp-map-editor-app-build"
ICONSET_DIR="$BUILD_DIR/AppIcon.iconset"
SOURCE_PNG="$BUILD_DIR/map-editor-1024.png"
SYMLINK_AT_ROOT=1

while [[ $# -gt 0 ]]; do
	case "$1" in
	--no-symlink)
		SYMLINK_AT_ROOT=0
		shift
		;;
	-h | --help)
		echo "Usage: $0 [--no-symlink]"
		exit 0
		;;
	*)
		echo "Unknown option: $1" >&2
		exit 2
		;;
	esac
done

APP_EDITOR_NAME="Perfect Dark Map Editor"
APP_PLAY_NAME="Play Last Test Map"
APP_EDITOR_BUNDLE="$RELEASE_DIR/${APP_EDITOR_NAME}.app"
APP_PLAY_BUNDLE="$RELEASE_DIR/${APP_PLAY_NAME}.app"
EDITOR_ICON="$BUILD_DIR/EditorAppIcon.icns"
PLAY_ICON="$BUILD_DIR/PlayAppIcon.icns"

write_info_plist() {
	local plist_path="$1"
	local executable="$2"
	local bundle_id="$3"
	local display_name="$4"
	local icon_name="$5"

	cat >"$plist_path" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
	<key>CFBundleDevelopmentRegion</key>
	<string>en</string>
	<key>CFBundleExecutable</key>
	<string>${executable}</string>
	<key>CFBundleIconFile</key>
	<string>${icon_name}</string>
	<key>CFBundleIdentifier</key>
	<string>${bundle_id}</string>
	<key>CFBundleInfoDictionaryVersion</key>
	<string>6.0</string>
	<key>CFBundleName</key>
	<string>${display_name}</string>
	<key>CFBundlePackageType</key>
	<string>APPL</string>
	<key>CFBundleShortVersionString</key>
	<string>1.0</string>
	<key>CFBundleVersion</key>
	<string>1</string>
	<key>LSMinimumSystemVersion</key>
	<string>11.0</string>
	<key>NSHighResolutionCapable</key>
	<true/>
</dict>
</plist>
PLIST
}

build_icns() {
	local png_source="$1"
	local icns_out="$2"
	local iconset="$BUILD_DIR/$(basename "$icns_out" .icns).iconset"

	rm -rf "$iconset"
	mkdir -p "$iconset"

	local size
	for size in 16 32 128 256 512; do
		sips -z "$size" "$size" "$png_source" --out "$iconset/icon_${size}x${size}.png" >/dev/null
		sips -z "$((size * 2))" "$((size * 2))" "$png_source" --out "$iconset/icon_${size}x${size}@2x.png" >/dev/null
	done

	iconutil -c icns "$iconset" -o "$icns_out"
}

tint_icon_png() {
	local input_png="$1"
	local output_png="$2"
	local hue_shift="${3:-0.35}"

	# Duplicate base icon and shift hue for the play shortcut app.
	sips -s format png "$input_png" --out "$output_png" >/dev/null
	# sips has no hue rotate; use a simple overlay via Python if available.
	if command -v python3 >/dev/null 2>&1; then
		python3 - "$input_png" "$output_png" "$hue_shift" <<'PY'
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    Path(sys.argv[2]).write_bytes(Path(sys.argv[1]).read_bytes())
    sys.exit(0)

src, dst, shift = sys.argv[1], sys.argv[2], float(sys.argv[3])
img = Image.open(src).convert("RGBA")
r, g, b, a = img.split()
# Warm tint for play app
merged = Image.merge("RGBA", (
    r.point(lambda p: min(255, int(p * (1.0 + shift * 0.15)))),
    g.point(lambda p: min(255, int(p * (1.0 + shift * 0.05)))),
    b.point(lambda p: min(255, int(p * (0.85 - shift * 0.1)))),
    a,
))
merged.save(dst)
PY
	fi
}

# Drop shebang / duplicate set -euo / SCRIPT_DIR source lines when inlining a launcher script.
strip_launcher_for_embed() {
	local script_path="$1"
	awk '
		/^#!\// { next }
		/^set -euo pipefail$/ { next }
		/^SCRIPT_DIR=/ { next }
		/^# shellcheck source=/ { next }
		/^source "\$SCRIPT_DIR\// { next }
		{ print }
	' "$script_path"
}

# Copy editor server + static UI into the .app so Finder launch survives iCloud eviction.
bundle_editor_into_app() {
	local app_bundle="$1"
	local editor_dir="$app_bundle/Contents/Resources/editor"
	local uff_src="$REPO_ROOT/journal/uff_viewer"

	mkdir -p "$editor_dir"
	for name in serve_editor.py test_map.py json_to_level.py uff_map.html; do
		if [[ ! -f "$uff_src/$name" ]]; then
			echo "Missing editor bundle source: $uff_src/$name" >&2
			exit 1
		fi
		cp "$uff_src/$name" "$editor_dir/$name"
	done
	printf '%s\n' "$REPO_ROOT" >"$app_bundle/Contents/Resources/repo_root.txt"
}

install_launcher_app() {
	local app_bundle="$1"
	local launcher_script="$2"
	local executable_name="$3"
	local bundle_id="$4"
	local display_name="$5"
	local icon_icns="$6"
	local icon_base="$7"

	rm -rf "$app_bundle"
	mkdir -p "$app_bundle/Contents/MacOS" "$app_bundle/Contents/Resources"

	local macos_exe="$app_bundle/Contents/MacOS/$executable_name"
	local log_name="PerfectDarkMapEditor.log"
	if [[ "$launcher_script" == "play-last-test-map.sh" ]]; then
		log_name="PerfectDarkPlayLastTest.log"
	fi

	# Self-contained MacOS executable: repo discovery + embedded launcher (no external sh exec).
	{
		cat <<WRAPPER
#!/bin/bash
# Finder-launched .app entry point — launcher logic is embedded at build time.
set -uo pipefail

LOG_FILE="\${HOME}/Library/Logs/${log_name}"
log() {
  mkdir -p "\$(dirname "\$LOG_FILE")"
  printf '[%s] %s\n' "\$(date '+%Y-%m-%d %H:%M:%S')" "\$*" >>"\$LOG_FILE"
}

fail_dialog() {
  local msg="\$1"
  log "ERROR: \$msg"
  /usr/bin/osascript - "\$msg" "${display_name}" >/dev/null 2>&1 <<'APPLESCRIPT' || true
on run argv
  display dialog (item 1 of argv) with title (item 2 of argv) buttons {"OK"} default button "OK" giving up after 120
end run
APPLESCRIPT
  if [[ -f "\$LOG_FILE" ]]; then
    /usr/bin/open -a TextEdit "\$LOG_FILE" >/dev/null 2>&1 || /usr/bin/open "\$LOG_FILE" >/dev/null 2>&1 || true
  fi
  exit 1
}

log "=== ${executable_name} start (PATH=\${PATH:-<empty>}) ==="

APP_MACOS="\$(cd "\$(dirname "\${BASH_SOURCE[0]}")" && pwd)"
# Resolve symlinks so repo discovery works when the .app is a symlink at repo root.
APP_BUNDLE="\$(cd "\${APP_MACOS}/../.." && pwd -P)"
REPO_ROOT="\${PD_REPO_ROOT:-}"
SERVE_MARKER="journal/uff_viewer/serve_editor.py"
BUNDLED_EDITOR="\${APP_BUNDLE}/Contents/Resources/editor"

export PATH="/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin"

# --- embedded map-editor-app-common.sh (pd_ensure_local_file, pd_read_baked_repo_root, …) ---
WRAPPER
		tail -n +2 "$SCRIPT_DIR/map-editor-app-common.sh"
		cat <<'WRAPPER_MID'

set -euo pipefail

# Resolve repository root: env → baked path → walk-up from .app.
if [[ -z "$REPO_ROOT" ]]; then
  REPO_ROOT="$(pd_read_baked_repo_root "$APP_BUNDLE" 2>/dev/null || true)"
fi
if [[ -z "$REPO_ROOT" ]]; then
  REPO_ROOT="$(pd_discover_repo_root "$APP_BUNDLE" "$SERVE_MARKER" 2>/dev/null || true)"
fi
if [[ -z "$REPO_ROOT" ]]; then
  fail_dialog "Could not locate the Perfect Dark repository.

Rebuild from the repo: ./scripts/build-map-editor-app.sh

Or set PD_REPO_ROOT to the repo path.

Log: ${LOG_FILE}"
fi

log "REPO_ROOT=${REPO_ROOT}"
pd_hydrate_repo_root "$REPO_ROOT" || true

EDITOR_DIR="$(pd_resolve_editor_dir "$REPO_ROOT" "$APP_BUNDLE" 2>/dev/null || true)"
if [[ -z "$EDITOR_DIR" ]]; then
  fail_dialog "Could not locate the map editor server (repo or bundled copy).

REPO_ROOT: ${REPO_ROOT}

Rebuild: ./scripts/build-map-editor-app.sh

Log: ${LOG_FILE}"
fi

if [[ "$EDITOR_DIR" == "$BUNDLED_EDITOR" ]]; then
  log "Using bundled editor at ${EDITOR_DIR} (repo copy unavailable or evicted)"
  export PD_EDITOR_STATE_DIR="${HOME}/Library/Application Support/PerfectDarkMapEditor"
else
  log "Using repo editor at ${EDITOR_DIR}"
fi

export PD_REPO_ROOT="$REPO_ROOT"
export PD_EDITOR_DIR="$EDITOR_DIR"
export PYTHONPATH="${REPO_ROOT}:${PYTHONPATH:-}"

# --- embedded launcher body ---
WRAPPER_MID
		strip_launcher_for_embed "$SCRIPT_DIR/$launcher_script"
	} >"$macos_exe"
	chmod +x "$macos_exe"

	bundle_editor_into_app "$app_bundle"

	# Ad-hoc sign so Gatekeeper allows double-click (strip iCloud/quarantine xattrs first).
	if command -v codesign >/dev/null 2>&1; then
		xattr -cr "$app_bundle" 2>/dev/null || true
		if ! codesign --force --deep --sign - "$app_bundle" 2>&1; then
			# iCloud extended attributes can make the bundle look invalid until cleared again.
			xattr -cr "$app_bundle" 2>/dev/null || true
			codesign --force --deep --sign - "$app_bundle" 2>&1 || printf 'Warning: codesign failed for %s (try: xattr -cr "%s" && codesign --force --deep --sign - "%s")\n' "$app_bundle" "$app_bundle" "$app_bundle" >&2
		fi
	fi

	cp "$icon_icns" "$app_bundle/Contents/Resources/${icon_base}.icns"
	write_info_plist "$app_bundle/Contents/Info.plist" "$executable_name" "$bundle_id" "$display_name" "$icon_base"
}

main() {
	mkdir -p "$RELEASE_DIR" "$BUILD_DIR"

	chmod +x \
		"$SCRIPT_DIR/launch-map-editor.sh" \
		"$SCRIPT_DIR/play-last-test-map.sh" \
		"$SCRIPT_DIR/map-editor-app-common.sh"

	swift "$SCRIPT_DIR/generate-map-editor-icon.swift" "$SOURCE_PNG"
	build_icns "$SOURCE_PNG" "$EDITOR_ICON"

	tint_icon_png "$SOURCE_PNG" "$BUILD_DIR/play-icon-1024.png" "0.4"
	build_icns "$BUILD_DIR/play-icon-1024.png" "$PLAY_ICON"

	install_launcher_app \
		"$APP_EDITOR_BUNDLE" \
		"launch-map-editor.sh" \
		"map-editor-launcher" \
		"com.perfectdark.jonaeru.map-editor" \
		"$APP_EDITOR_NAME" \
		"$EDITOR_ICON" \
		"EditorAppIcon"

	install_launcher_app \
		"$APP_PLAY_BUNDLE" \
		"play-last-test-map.sh" \
		"play-last-test-map" \
		"com.perfectdark.jonaeru.play-last-test" \
		"$APP_PLAY_NAME" \
		"$PLAY_ICON" \
		"PlayAppIcon"

	if [[ "$SYMLINK_AT_ROOT" -eq 1 ]]; then
		ln -sfn "$APP_EDITOR_BUNDLE" "$REPO_ROOT/${APP_EDITOR_NAME}.app"
	fi

	printf '\nBuilt map editor apps:\n'
	printf '  %s\n' "$APP_EDITOR_BUNDLE"
	printf '  %s\n' "$APP_PLAY_BUNDLE"
	if [[ "$SYMLINK_AT_ROOT" -eq 1 ]]; then
		printf '  %s -> %s\n' "$REPO_ROOT/${APP_EDITOR_NAME}.app" "$APP_EDITOR_BUNDLE"
	fi
	printf '\nDouble-click "%s" to start the editor server and open your browser.\n' "$APP_EDITOR_NAME"
}

main "$@"
