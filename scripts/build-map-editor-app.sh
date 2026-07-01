#!/usr/bin/env bash
# Build the Play Last Test Map companion .app (replay last --test-map launch).
#
# The map editor itself is built with ./scripts/build-map-editor-electron.sh
# (outputs scripts/release/Perfect Dark Map Editor.app).
#
# Outputs:
#   scripts/release/Play Last Test Map.app
#
# Usage:
#   ./scripts/build-map-editor-app.sh
#   ./scripts/build-map-editor-app.sh --symlink-at-root   # optional root symlink for Play app

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
RELEASE_DIR="$SCRIPT_DIR/release"
BUILD_DIR="$REPO_ROOT/.tmp-map-editor-app-build"
ICONSET_DIR="$BUILD_DIR/AppIcon.iconset"
SOURCE_PNG="$BUILD_DIR/map-editor-1024.png"
SYMLINK_AT_ROOT=0

while [[ $# -gt 0 ]]; do
	case "$1" in
	--symlink-at-root)
		SYMLINK_AT_ROOT=1
		shift
		;;
	--no-symlink)
		SYMLINK_AT_ROOT=0
		shift
		;;
	-h | --help)
		echo "Usage: $0 [--symlink-at-root]"
		echo ""
		echo "Builds Play Last Test Map.app only. For the editor, run:"
		echo "  ./scripts/build-map-editor-electron.sh"
		exit 0
		;;
	*)
		echo "Unknown option: $1" >&2
		exit 2
		;;
	esac
done

APP_PLAY_NAME="Play Last Test Map"
APP_PLAY_BUNDLE="$RELEASE_DIR/${APP_PLAY_NAME}.app"
EDITOR_ICON="$BUILD_DIR/EditorAppIcon.icns"
PLAY_ICON="$BUILD_DIR/PlayAppIcon.icns"
LEGACY_SHELL_EDITOR="$RELEASE_DIR/Perfect Dark Map Editor.app"

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

	sips -s format png "$input_png" --out "$output_png" >/dev/null
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
	local log_name="PerfectDarkPlayLastTest.log"

	{
		cat <<WRAPPER
#!/bin/bash
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
APP_BUNDLE="\$(cd "\${APP_MACOS}/../.." && pwd -P)"
REPO_ROOT="\${PD_REPO_ROOT:-}"
SERVE_MARKER="journal/uff_viewer/serve_editor.py"
BUNDLED_EDITOR="\${APP_BUNDLE}/Contents/Resources/editor"

export PATH="/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin"

WRAPPER
		tail -n +2 "$SCRIPT_DIR/map-editor-app-common.sh"
		cat <<'WRAPPER_MID'

set -euo pipefail

if [[ -z "$REPO_ROOT" ]]; then
  REPO_ROOT="$(pd_read_baked_repo_root "$APP_BUNDLE" 2>/dev/null || true)"
fi
if [[ -z "$REPO_ROOT" ]]; then
  REPO_ROOT="$(pd_discover_repo_root "$APP_BUNDLE" "$SERVE_MARKER" 2>/dev/null || true)"
fi
if [[ -z "$REPO_ROOT" ]]; then
  fail_dialog "Could not locate the Perfect Dark repository.

Rebuild Play app: ./scripts/build-map-editor-app.sh
Editor app: ./scripts/build-map-editor-electron.sh

Or set PD_REPO_ROOT.

Log: ${LOG_FILE}"
fi

log "REPO_ROOT=${REPO_ROOT}"
REPO_ROOT="$(pd_normalize_repo_root "$REPO_ROOT")"
pd_hydrate_repo_root "$REPO_ROOT" || true

EDITOR_DIR="$(pd_resolve_editor_dir "$REPO_ROOT" "$APP_BUNDLE" 2>/dev/null || true)"
if [[ -z "$EDITOR_DIR" ]]; then
  fail_dialog "Could not locate editor files.

REPO_ROOT: ${REPO_ROOT}

Log: ${LOG_FILE}"
fi

if [[ "$EDITOR_DIR" == "$BUNDLED_EDITOR" ]]; then
  export PD_EDITOR_STATE_DIR="${HOME}/Library/Application Support/PerfectDarkMapEditor"
fi

export PD_REPO_ROOT="$REPO_ROOT"
export PD_EDITOR_DIR="$EDITOR_DIR"
export PYTHONPATH="${REPO_ROOT}:${PYTHONPATH:-}"

WRAPPER_MID
		strip_launcher_for_embed "$SCRIPT_DIR/$launcher_script"
	} >"$macos_exe"
	chmod +x "$macos_exe"

	bundle_editor_into_app "$app_bundle"

	if command -v codesign >/dev/null 2>&1; then
		xattr -cr "$app_bundle" 2>/dev/null || true
		codesign --force --deep --sign - "$app_bundle" 2>&1 || true
	fi

	cp "$icon_icns" "$app_bundle/Contents/Resources/${icon_base}.icns"
	write_info_plist "$app_bundle/Contents/Info.plist" "$executable_name" "$bundle_id" "$display_name" "$icon_base"
}

main() {
	mkdir -p "$RELEASE_DIR" "$BUILD_DIR"

	chmod +x \
		"$SCRIPT_DIR/play-last-test-map.sh" \
		"$SCRIPT_DIR/map-editor-app-common.sh"

	if [[ ! -f "$SOURCE_PNG" ]]; then
		swift "$SCRIPT_DIR/generate-map-editor-icon.swift" "$SOURCE_PNG"
	fi
	if [[ ! -f "$EDITOR_ICON" ]]; then
		build_icns "$SOURCE_PNG" "$EDITOR_ICON"
	fi

	tint_icon_png "$SOURCE_PNG" "$BUILD_DIR/play-icon-1024.png" "0.4"
	build_icns "$BUILD_DIR/play-icon-1024.png" "$PLAY_ICON"

	# Drop legacy shell-based editor .app if present (Electron is canonical).
	if [[ -d "$LEGACY_SHELL_EDITOR" ]] && [[ ! -f "$LEGACY_SHELL_EDITOR/Contents/Resources/app.asar" ]]; then
		rm -rf "$LEGACY_SHELL_EDITOR"
	fi

	install_launcher_app \
		"$APP_PLAY_BUNDLE" \
		"play-last-test-map.sh" \
		"play-last-test-map" \
		"com.perfectdark.jonaeru.play-last-test" \
		"$APP_PLAY_NAME" \
		"$PLAY_ICON" \
		"PlayAppIcon"

	if [[ "$SYMLINK_AT_ROOT" -eq 1 ]]; then
		ln -sfn "$APP_PLAY_BUNDLE" "$REPO_ROOT/${APP_PLAY_NAME}.app"
	fi

	printf '\nBuilt companion app:\n'
	printf '  %s\n' "$APP_PLAY_BUNDLE"
	if [[ "$SYMLINK_AT_ROOT" -eq 1 ]]; then
		printf '  %s -> %s\n' "$REPO_ROOT/${APP_PLAY_NAME}.app" "$APP_PLAY_BUNDLE"
	fi
	printf '\nMap editor: ./scripts/build-map-editor-electron.sh\n'
	printf '  -> scripts/release/Perfect Dark Map Editor.app\n'
}

main "$@"
