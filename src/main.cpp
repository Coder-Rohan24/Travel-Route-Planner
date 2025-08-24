#include "graph.h"
#include "planner.h"
#include "io.h"
#include <iostream>
#include <filesystem>
#include <iomanip>

// simple driver: single query; prints metrics, produces visualization
int main(int argc, char** argv) {
    std::cout << "Travel Route Planner (Dijkstra, A*, Bidirectional A*)\n";
    if (argc < 5) {
        std::cout << "Usage: " << argv[0] << " <cities.csv> <routes.csv> <source_id> <target_id>\n";
        return 1;
    }
    std::string nodes_csv = argv[1];
    std::string edges_csv = argv[2];
    int source = std::stoi(argv[3]);
    int target = std::stoi(argv[4]);

    Graph g;
    if (!g.load_nodes_csv(nodes_csv)) return 2;
    if (!g.load_edges_csv(edges_csv)) return 3;
    std::cout << "Loaded graph: nodes=" << g.num_nodes() << " edges(approx)=";
    size_t edgecount = 0;
    for (const auto &v : g.adjacency()) edgecount += v.size();
    std::cout << edgecount/2 << " (undirected)\n";

    // Print adjacency list
    const auto &adj = g.adjacency();
    const auto &names = g.get_names();
    std::cout << "Adjacency List:\n";
    for (size_t i = 0; i < adj.size(); ++i) {
        std::cout << i << " (" << names[i] << "): ";
        for (const auto &e : adj[i]) {
            std::cout << e.to << " (" << names[e.to] << ") w=" << e.w << ", ";
        }
        std::cout << "\n";
    }

    auto print_path = [&](const std::vector<int> &path) {
        std::cout << "Path: ";
        for (size_t i = 0; i < path.size(); ++i) {
            std::cout << path[i] << " (" << names[path[i]] << ")";
            if (i+1 < path.size()) std::cout << " -> ";
        }
        std::cout << "\n";
    };

    auto run_and_print = [&](const std::string &label, Stats st) {
        std::cout << label << ": dist=" << std::fixed << std::setprecision(6) << st.distance
                  << " nodes=" << st.nodes_expanded << " ms=" << st.millis << " path_len=" << st.path.size() << "\n";
        print_path(st.path);
    };

    Stats sd = dijkstra_search(g, source, target);
    run_and_print("DIJKSTRA", sd);

    Stats sa = astar_search(g, source, target);
    run_and_print("ASTAR", sa);

    Stats sb = bidir_astar_search(g, source, target);
    run_and_print("BIDIR_ASTAR", sb);

    std::filesystem::create_directories("results");
    std::string geo = "results/route.geojson";
    if (!write_geojson(g, sb.path.empty() ? sa.path : sb.path, geo)) {
        std::cerr << "Failed to write geojson\n";
    } else {
        write_leaflet_html("route.geojson", "results/route_map.html");
        std::cout << "Visualization: results/route_map.html\n";
    }

    std::vector<std::pair<std::string, Stats>> rows;
    rows.push_back({"dijkstra", sd});
    rows.push_back({"astar", sa});
    rows.push_back({"bidir_astar", sb});
    write_metrics_csv("results/metrics.csv", rows);
    std::cout << "Metrics saved to results/metrics.csv\n";

    return 0;
}
