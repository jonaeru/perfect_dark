#!/bin/bash
# Fixed-interval heartbeat for the map learn engine loop.
# Emits AGENT_LOOP_WAKE_map_learn every MAP_LEARN_INTERVAL_SEC (default 120).
set -euo pipefail
REPO="$(cd "$(dirname "$0")/.." && pwd)"
PIDFILE="$REPO/journal/map_learn/.learn-loop.pid"
INTERVAL="${MAP_LEARN_INTERVAL_SEC:-120}"
PROMPT='Map learn loop: run pdmap learn run; read journal/map_learn/gaps.md; implement next probe or close doc gaps; target deterministic map creation with near-perfect docs.'

mkdir -p "$(dirname "$PIDFILE")"
echo "$$ heartbeat" > "$PIDFILE"

while true; do
  sleep "$INTERVAL"
  echo "AGENT_LOOP_WAKE_map_learn {\"prompt\":\"$PROMPT\",\"iteration\":\"auto\"}"
done
