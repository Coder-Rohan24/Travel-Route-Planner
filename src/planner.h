#ifndef PLANNER_H
#define PLANNER_H

#include "graph.h"
#include <vector>

struct Stats {
    double distance = 0.0;
    size_t nodes_expanded = 0;
    long long millis = 0;
    std::vector<int> path;
};

// algorithms
Stats dijkstra_search(const Graph &g, int s, int t);
Stats astar_search(const Graph &g, int s, int t);
Stats bidir_astar_search(const Graph &g, int s, int t);

#endif // PLANNER_H
