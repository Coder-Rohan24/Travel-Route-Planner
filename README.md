# Travel Planner Project

## Overview
This project is a C++ travel route planner that computes and visualizes optimal paths between cities using advanced graph algorithms (Dijkstra, A*, Bidirectional A*). It supports both custom city/route data and real-world OpenStreetMap (OSM) data. The project includes benchmarking, metrics analysis, and interactive map visualizations.

---

## Features
- Shortest path algorithms: Dijkstra, A*, Bidirectional A*
- Interactive route and network visualizations (Leaflet, Vis.js)
- Batch benchmarking and metrics analysis
- Support for custom CSV data and OSM data
- Python scripts for preprocessing, analysis, and validation

---

## Directory Structure
```
├── bin/                # Compiled executables
├── build.sh            # Build script
├── profile_memory.sh   # Memory profiling script
├── run_batch.sh        # Batch metrics script
├── run_example.sh      # Single route script
├── data/               # City and route CSV files
├── python/             # Python scripts for analysis and preprocessing
├── results/            # Output metrics and visualizations
├── src/                # C++ source code
├── test/               # Unit tests
```

---

## Data Setup

### 1. Using Provided Data
- Edit `data/cities.csv` to add or modify cities (format: node_id,lat,lon,name)
- Edit `data/routes.csv` to add or modify routes (format: src_id,dst_id,distance)

### 2. Using OSM Data
- Place your `.osm.pbf` file in the project directory
- Run preprocessing:
  ```bash
  python3 python/preprocess_osm.py <your_osm_file>
  ```
- This will generate new `cities.csv` and `routes.csv` files

---

## Building the Project

On Linux/macOS:
```bash
./build.sh
```
On Windows (PowerShell):
```powershell
bash build.sh
```

---

## Running the Project

### 1. Single Route Example
```bash
./run_example.sh <source_id> <target_id>
```
- Default: Kabul (0) to Lima (17)
- Output: `results/metrics.csv`, `results/route_map.html`

### 2. Batch Benchmarking
```bash
./run_batch.sh <num_queries> <seed>
```
- Default: 100 queries, seed 12345
- Output: `results/metrics_batch.csv`, metrics summary

### 3. Manual Execution
Compile and run directly:
```bash
# Build
./build.sh
# Run single route
bin/route_planner data/cities.csv data/routes.csv 0 17
# Run batch
bin/batch_runner data/cities.csv data/routes.csv 100 12345
```

---

## Visualization
- Open `results/route_map.html` for route visualization
- Open `results/network_map.html` for network visualization

---

## Metrics Analysis
- Metrics are saved in `results/metrics.csv` and `results/metrics_batch.csv`
- Analyze batch metrics:
  ```bash
  python3 python/analyze_metrics.py results/metrics_batch.csv
  ```

---

## Memory Profiling (Linux/macOS)
- Profile with `/usr/bin/time`:
  ```bash
  ./profile_memory.sh
  ```
- For Valgrind massif:
  ```bash
  valgrind --tool=massif --massif-out-file=results/massif.out bin/route_planner data/cities.csv data/routes.csv 0 17
  ms_print results/massif.out > results/massif.txt
  ```

---

## Customization & Testing
- To test different routes, change the source and target IDs in `run_example.sh` or pass them to `bin/route_planner`
- To use different city/route data, edit `data/cities.csv` and `data/routes.csv`
- To run more or fewer batch queries, change the arguments in `run_batch.sh` or `bin/batch_runner`
- To add new algorithms, extend `src/planner.cpp` and update the runner scripts

---

## Troubleshooting
- If you see `inf` or zero in metrics, check for disconnected cities or bugs in path reconstruction
- For memory errors, use Valgrind or `/usr/bin/time` profiling
- For visualization issues, ensure your browser can open local HTML files and that GeoJSON is generated

---

## Contact & Credits
Developed by Anirban Halder. For questions, open an issue or contact via email.
