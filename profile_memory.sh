#!/usr/bin/env bash
# example memory profiling with /usr/bin/time -v
if [ ! -f bin/route_planner ]; then
  echo "Build first: ./build.sh"
  exit 1
fi
mkdir -p results
/usr/bin/time -v bin/route_planner data/cities.csv data/routes.csv 0 1 2> results/time_profile.txt
echo "Wrote results/time_profile.txt"
/usr/bin/time -v bin/batch_runner data/cities.csv data/routes.csv 100 12345 2> results/batch_time_profile.txt
echo "Wrote results/batch_time_profile.txt"
# For massif (valgrind), run manually as it is heavy:
# valgrind --tool=massif --massif-out-file=results/massif.out bin/route_planner data/cities.csv data/routes.csv 0 1
# ms_print results/massif.out > results/massif.txt
