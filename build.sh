#!/usr/bin/env bash
set -e
mkdir -p build bin
g++ -std=gnu++17 -O2 src/main.cpp src/graph.cpp src/io.cpp src/planner.cpp -I src -o bin/route_planner
g++ -std=gnu++17 -O2 src/batch_runner.cpp src/graph.cpp src/io.cpp src/planner.cpp -I src -o bin/batch_runner
echo "Built bin/route_planner and bin/batch_runner"
