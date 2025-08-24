#include "graph.h"
#include "planner.h"
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

int main() {
    std::string nodes_csv = "data/cities.csv";
    std::string edges_csv = "data/routes.csv";
    int source = 0; // Kabul
    int target = 17; // Lima
    int runs = 10000; 

    Graph g;
    if (!g.load_nodes_csv(nodes_csv)) {
        std::cerr << "Failed to load nodes\n";
        return 1;
    }
    if (!g.load_edges_csv(edges_csv)) {
        std::cerr << "Failed to load edges\n";
        return 2;
    }
    const auto &names = g.get_names();

    auto print_path = [&](const std::vector<int> &path) {
        std::cout << "Path: ";
        for (size_t i = 0; i < path.size(); ++i) {
            std::cout << path[i] << " (" << names[path[i]] << ")";
            if (i+1 < path.size()) std::cout << " -> ";
        }
        std::cout << "\n";
    };

    // Dijkstra benchmark
    auto t0 = std::chrono::high_resolution_clock::now();
    Stats sd;
    for (int i = 0; i < runs; ++i) {
        sd = dijkstra_search(g, source, target);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    auto d_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "Dijkstra avg time: " << (double)d_us/runs << " us over " << runs << " runs\n";
    print_path(sd.path);

    // A* benchmark
    t0 = std::chrono::high_resolution_clock::now();
    Stats sa;
    for (int i = 0; i < runs; ++i) {
        sa = astar_search(g, source, target);
    }
    t1 = std::chrono::high_resolution_clock::now();
    auto a_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "A* avg time: " << (double)a_us/runs << " us over " << runs << " runs\n";
    print_path(sa.path);

    // Bidirectional A* benchmark
    t0 = std::chrono::high_resolution_clock::now();
    Stats sb;
    for (int i = 0; i < runs; ++i) {
        sb = bidir_astar_search(g, source, target);
    }
    t1 = std::chrono::high_resolution_clock::now();
    auto b_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    std::cout << "Bidirectional A* avg time: " << (double)b_us/runs << " us over " << runs << " runs\n";
    print_path(sb.path);

    return 0;
}
