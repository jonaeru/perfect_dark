#!/usr/bin/env bash
# Build the Electron-based Perfect Dark Map Editor macOS .app bundle.
#
# Outputs:
#   scripts/release/Perfect Dark Map Editor (Electron).app
#   (optional symlink) ./Perfect Dark Map Editor (Electron).app
#
# Usage:
#   ./scripts/build-map-editor-electron.sh
#   ./scripts/build-map-editor-electron.sh --no-symlink
#   ./scripts/build-map-editor-electron.sh --dev   # npm start only (no .app)

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
ELECTRON_DIR="$REPO_ROOT/journal/uff_viewer/electron"
RELEASE_DIR="$SCRIPT_DIR/release"
APP_NAME="Perfect Dark Map Editor (Electron)"
APP_BUNDLE="$RELEASE_DIR/${APP_NAME}.app"
BUILD_DIR="$REPO_ROOT/.tmp-map-editor-app-build"
SYMLINK_AT_ROOT=1
DEV_ONLY=0

while [[ $# -gt 0 ]]; do
	case "$1" in
	--no-symlink)
		SYMLINK_AT_ROOT=0
		shift
		;;
	--dev)
		DEV_ONLY=1
		shift
		;;
	-h | --help)
		echo "Usage: $0 [--no-symlink] [--dev]"
		exit 0
		;;
	*)
		echo "Unknown option: $1" >&2
		exit 2
		;;
	esac
done

# Bake repo root for packaged .app (handles iCloud path with spaces).
write_repo_config() {
	printf '{"repoRoot": "%s"}\n' "$REPO_ROOT" >"$ELECTRON_DIR/repo-config.json"
}

prepare_icon() {
	mkdir -p "$ELECTRON_DIR/build"
	local icon_src="$BUILD_DIR/EditorAppIcon.icns"
	if [[ ! -f "$icon_src" ]]; then
		# Reuse shell-app icon generator when missing.
		if [[ -f "$SCRIPT_DIR/generate-map-editor-icon.swift" ]]; then
			mkdir -p "$BUILD_DIR"
			local png="$BUILD_DIR/map-editor-1024.png"
			if [[ ! -f "$png" ]]; then
				swift "$SCRIPT_DIR/generate-map-editor-icon.swift" "$png"
			fi
			local iconset="$BUILD_DIR/EditorAppIcon.iconset"
			rm -rf "$iconset"
			mkdir -p "$iconset"
			local size
			for size in 16 32 128 256 512; do
				sips -z "$size" "$size" "$png" --out "$iconset/icon_${size}x${size}.png" >/dev/null
				sips -z "$((size * 2))" "$((size * 2))" "$png" --out "$iconset/icon_${size}x${size}@2x.png" >/dev/null
			done
			iconutil -c icns "$iconset" -o "$icon_src"
		fi
	fi
	if [[ -f "$icon_src" ]]; then
		cp "$icon_src" "$ELECTRON_DIR/build/icon.icns"
	fi
}

install_built_app() {
	local dist_root="$ELECTRON_DIR/node_modules/.cache/electron-builder"
	local built=""
	# electron-builder --mac dir writes under directories.output in package.json
	local out_dir="$ELECTRON_DIR/../../scripts/release/electron-dist"
	out_dir="$(cd "$ELECTRON_DIR" && node -p "require('./package.json').build.directories.output" 2>/dev/null || echo '../../../scripts/release/electron-dist')"
	out_dir="$(cd "$ELECTRON_DIR" && cd "$out_dir" && pwd)"

	if [[ -d "$out_dir/mac" ]]; then
		built="$(find "$out_dir/mac" -maxdepth 1 -name '*.app' -print -quit 2>/dev/null || true)"
	fi
	if [[ -z "$built" && -d "$out_dir" ]]; then
		built="$(find "$out_dir" -maxdepth 2 -name '*.app' -print -quit 2>/dev/null || true)"
	fi

	if [[ -z "$built" || ! -d "$built" ]]; then
		echo "electron-builder did not produce a .app under $out_dir" >&2
		exit 1
	fi

	mkdir -p "$RELEASE_DIR"
	rm -rf "$APP_BUNDLE"
	cp -R "$built" "$APP_BUNDLE"

	# Also bake repo root into the packaged Resources for runtime discovery.
	mkdir -p "$APP_BUNDLE/Contents/Resources"
	printf '%s\n' "$REPO_ROOT" >"$APP_BUNDLE/Contents/Resources/repo_root.txt"
	cp "$ELECTRON_DIR/repo-config.json" "$APP_BUNDLE/Contents/Resources/repo-config.json" 2>/dev/null || true

	if command -v codesign >/dev/null 2>&1; then
		xattr -cr "$APP_BUNDLE" 2>/dev/null || true
		codesign --force --deep --sign - "$APP_BUNDLE" 2>&1 || true
	fi
}

main() {
	if ! command -v node >/dev/null 2>&1; then
		echo "Node.js is required. Install via brew install node or nvm." >&2
		exit 1
	fi
	if ! command -v npm >/dev/null 2>&1; then
		echo "npm is required." >&2
		exit 1
	fi

	write_repo_config
	prepare_icon

	cd "$ELECTRON_DIR"
	echo "Installing npm dependencies in $ELECTRON_DIR ..."
	npm install

	if [[ "$DEV_ONLY" -eq 1 ]]; then
		echo ""
		echo "Dev mode — launch with:"
		echo "  cd \"$ELECTRON_DIR\" && PD_REPO_ROOT=\"$REPO_ROOT\" npm start"
		exit 0
	fi

	echo "Building macOS .app with electron-builder ..."
	npm run build

	install_built_app

	if [[ "$SYMLINK_AT_ROOT" -eq 1 ]]; then
		ln -sfn "$APP_BUNDLE" "$REPO_ROOT/${APP_NAME}.app"
	fi

	printf '\nBuilt Electron map editor app:\n'
	printf '  %s\n' "$APP_BUNDLE"
	if [[ "$SYMLINK_AT_ROOT" -eq 1 ]]; then
		printf '  %s -> %s\n' "$REPO_ROOT/${APP_NAME}.app" "$APP_BUNDLE"
	fi
	printf '\nDouble-click "%s" — editor opens in an embedded window (no external browser).\n' "$APP_NAME"
	printf 'Dev fallback: cd journal/uff_viewer/electron && PD_REPO_ROOT="%s" npm start\n' "$REPO_ROOT"
}

main "$@"
