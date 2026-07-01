#!/usr/bin/env bash
# Deterministic map validation launcher (registered stages + learn curriculum).
#
# Validation order (step index):
#   0       my_arena   (--boot-stage 0x80)
#   1       testarena  (--boot-stage 0x81)
#   2–12    learn_01 … learn_11 via play-learn-step (curriculum steps 1–11)
#
# Usage:
#   ./scripts/validate-maps.sh              # launch current step (from state file)
#   ./scripts/validate-maps.sh next         # advance state and launch
#   ./scripts/validate-maps.sh step 0       # jump to step and launch
#   ./scripts/validate-maps.sh my_arena     # launch by name
#   ./scripts/validate-maps.sh status       # show step + checklist hint
#   ./scripts/validate-maps.sh list         # all steps
#   ./scripts/validate-maps.sh step 3 --no-play   # deploy only (learn steps)
#
set -euo pipefail

REPO="$(cd "$(dirname "$0")/.." && pwd)"
STATE_FILE="$REPO/journal/map_learn/.validation_step"
LOG_DIR="$REPO/journal/map_learn"
PD="$REPO/build/pd.arm64"
MODDIR="$REPO/mods/mod_allinone"
ROM="$REPO/data/pd.ntsc-final.z64"

TOTAL_STEPS=12

ensure_rom() {
	if [[ -f "$ROM" ]]; then
		return 0
	fi
	local fb=(
		"/Users/moigutierrez/Library/CloudStorage/GoogleDrive-moyoteg@gmail.com/My Drive/Games/N64 Decomp/PerfectDarkShared/pd.ntsc-final.z64"
		"/Users/moigutierrez/Library/Mobile Documents/com~apple~CloudDocs/Work/Personal/Games/perfect_dark/pd.ntsc-final.z64"
	)
	for p in "${fb[@]}"; do
		if [[ -f "$p" ]]; then
			mkdir -p "$REPO/data"
			cp "$p" "$ROM" && return 0
		fi
	done
	echo "ERROR: missing ROM at $ROM (see README_MOD_INFO.md)" >&2
	exit 1
}

ensure_binary() {
	if [[ ! -x "$PD" ]]; then
		echo "ERROR: build $PD first (make -j8)" >&2
		exit 1
	fi
}

read_state() {
	if [[ -f "$STATE_FILE" ]]; then
		tr -d '[:space:]' < "$STATE_FILE"
	else
		echo "0"
	fi
}

write_state() {
	local n="$1"
	mkdir -p "$(dirname "$STATE_FILE")"
	echo "$n" > "$STATE_FILE"
}

step_label() {
	case "$1" in
		0) echo "my_arena (registered stage STAGE_MY_ARENA 0x80)" ;;
		1) echo "testarena (registered stage STAGE_TESTARENA 0x81)" ;;
		*)
			local learn=$((10#$1 - 1))
			printf "learn_%02d (curriculum step %d, uff test slot)\n" "$learn" "$learn"
			;;
	esac
}

step_hint() {
	case "$1" in
		0)
			cat <<'EOF'
Verify my_arena:
  • You spawn in the 5000×3000 box (four corner spawns); floor is solid at Y=0 — no fall-through.
  • Center pickups: AR34 (+Z side) and Shotgun (−Z); rifle/shotgun ammo crates on ±X.
  • Walk the perimeter: invisible/collision walls match the box; no phantom origin wall.
EOF
			;;
		1)
			cat <<'EOF'
Verify testarena:
  • Registered stage loads (boot 0x81) without crash; flat arena playable.
  • Spawns and floor behave like other custom registered stages.
EOF
			;;
		*)
			local learn=$((10#$1 - 1))
			echo "See journal/map_learn/CURRICULUM.md — curriculum step $learn ($(step_label "$1"))."
			;;
	esac
}

resolve_step_arg() {
	local arg="$1"
	local lower
	lower="$(printf '%s' "$arg" | tr '[:upper:]' '[:lower:]')"
	case "$lower" in
		my_arena|my-arena|myarena) echo 0 ;;
		testarena|test_arena) echo 1 ;;
		next) echo "__next__" ;;
		status|list) echo "$arg" ;;
		step) echo "__step__" ;;
		"") echo "__current__" ;;
		*)
			if [[ "$arg" =~ ^[0-9]+$ ]]; then
				echo "$arg"
			else
				echo "ERROR: unknown step '$arg' (try: my_arena, testarena, 0–12, next, status)" >&2
				exit 1
			fi
			;;
	esac
}

launch_registered() {
	local boot_hex="$1"
	local name="$2"
	ensure_rom
	ensure_binary
	cd "$REPO"
	local log="$LOG_DIR/validate_launch_${name}.log"
	echo "Launching $name → $PD --moddir mods/mod_allinone --boot-stage $boot_hex --skip-intro"
	nohup "$PD" --moddir "$MODDIR" --boot-stage "$boot_hex" --skip-intro \
		>>"$log" 2>&1 &
	echo "$!" > "$LOG_DIR/.validate_${name}.pid"
	echo "  log: ${log#$REPO/}"
	echo "  pid: $(cat "$LOG_DIR/.validate_${name}.pid")"
}

launch_step() {
	local step="$1"
	shift || true
	local extra=("$@")

	if [[ "$step" -lt 0 || "$step" -gt "$TOTAL_STEPS" ]]; then
		echo "ERROR: step must be 0–$TOTAL_STEPS" >&2
		exit 1
	fi

	write_state "$step"
	echo "=== Validation step $step / $TOTAL_STEPS ==="
	echo "  $(step_label "$step")"
	echo ""
	step_hint "$step"
	echo ""

	if [[ "$step" -eq 0 ]]; then
		launch_registered "0x80" "my_arena"
	elif [[ "$step" -eq 1 ]]; then
		launch_registered "0x81" "testarena"
	else
		local learn=$((step - 1))
		cd "$REPO"
		exec "$REPO/scripts/play-learn-step.sh" "$learn" "${extra[@]}"
	fi
}

cmd_list() {
	local i
	for i in $(seq 0 "$TOTAL_STEPS"); do
		printf "%2d  %s\n" "$i" "$(step_label "$i")"
	done
	echo ""
	echo "State file: ${STATE_FILE#$REPO/} ($(read_state))"
}

cmd_status() {
	local cur
	cur="$(read_state)"
	echo "Current validation step: $cur / $TOTAL_STEPS"
	echo "  $(step_label "$cur")"
	echo ""
	step_hint "$cur"
	echo ""
	echo "Launch:  $REPO/scripts/validate-maps.sh"
	echo "Next:    $REPO/scripts/validate-maps.sh next"
}

main() {
	local arg1="${1:-}"
	local resolved
	resolved="$(resolve_step_arg "${arg1:-}")"

	case "$resolved" in
		list)
			cmd_list
			;;
		status)
			cmd_status
			;;
		__next__)
			local cur next
			cur="$(read_state)"
			next=$((cur + 1))
			if [[ "$next" -gt "$TOTAL_STEPS" ]]; then
				echo "Validation complete (step $TOTAL_STEPS). Reset with: validate-maps.sh step 0" >&2
				exit 0
			fi
			shift || true
			launch_step "$next" "$@"
			;;
		__step__)
			if [[ -z "${2:-}" ]]; then
				echo "Usage: validate-maps.sh step N" >&2
				exit 1
			fi
			local n
			n="$(resolve_step_arg "$2")"
			shift 2 || true
			launch_step "$n" "$@"
			;;
		__current__)
			shift || true
			launch_step "$(read_state)" "$@"
			;;
		*)
			shift || true
			launch_step "$resolved" "$@"
			;;
	esac
}

main "$@"
