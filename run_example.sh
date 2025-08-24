#!/usr/bin/env bash
set -e
if [ ! -f bin/route_planner ]; then
  echo "Build first: ./build.sh"
  exit 1
fi
mkdir -p results
NODES="data/cities.csv"
EDGES="data/routes.csv"
SRC="${1:-0}"
TGT="${2:-1}"
echo "Running route_planner $NODES $EDGES $SRC $TGT"
bin/route_planner "$NODES" "$EDGES" "$SRC" "$TGT"
echo "See results/metrics.csv and results/route_map.html"
