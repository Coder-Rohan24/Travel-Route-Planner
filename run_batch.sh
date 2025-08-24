#!/usr/bin/env bash
set -e
if [ ! -f bin/batch_runner ]; then
  echo "Build first: ./build.sh"
  exit 1
fi
mkdir -p results
NODES="data/cities.csv"
EDGES="data/routes.csv"
NUMQ="${1:-100}"
SEED="${2:-12345}"
echo "Running batch runner: numq=$NUMQ seed=$SEED"
bin/batch_runner "$NODES" "$EDGES" "$NUMQ" "$SEED"
python3 python/analyze_metrics.py results/metrics_batch.csv
