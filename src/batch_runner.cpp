// batch_runner.cpp
// runs multiple queries (random or from a file) and writes aggregated metrics
#include "graph.h"
#include "planner.h"
#include "io.h"
#include <iostream>
#include <random>
#include <vector>
#include <algorithm>
#include <fstream>
#include <chrono>
#include <numeric>

static double percentile(std::vector<long long> &v, double p) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    double idx = (p/100.0) * (v.size()-1);
    size_t lo = (size_t)std::floor(idx), hi=(size_t)std::ceil(idx);
    if (lo==hi) return (double)v[lo];
    double frac = idx - lo;
    return v[lo] * (1-frac) + v[hi]*frac;
}

int main(int argc,char** argv) {
    std::cout<<"Batch runner: runs 100 queries (default). Usage:\n";
    std::cout<<argv[0]<<" <cities.csv> <routes.csv> [num_queries] [seed]\n";
    if (argc < 3) return 1;
    std::string nodes_csv = argv[1], edges_csv = argv[2];
    int numq = (argc>=4) ? std::stoi(argv[3]) : 100;
    unsigned seed = (argc>=5) ? std::stoul(argv[4]) : (unsigned)std::chrono::system_clock::now().time_since_epoch().count();

    Graph g;
    if (!g.load_nodes_csv(nodes_csv)) return 2;
    if (!g.load_edges_csv(edges_csv)) return 3;
    int n = g.num_nodes();
    if (n<2) { std::cerr<<"Not enough nodes\n"; return 4; }

    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> uid(0,n-1);

    std::vector<long long> dij_times, astar_times, bidir_times;
    std::vector<size_t> dij_nodes, astar_nodes, bidir_nodes;
    std::vector<double> dij_dist, astar_dist, bidir_dist;
    std::vector<size_t> dij_pathlen, astar_pathlen, bidir_pathlen;

    for (int i=0;i<numq;i++){
        int s = uid(rng), t = uid(rng);
        if (s==t) { i--; continue; }
        Stats sd = dijkstra_search(g,s,t);
        Stats sa = astar_search(g,s,t);
        Stats sb = bidir_astar_search(g,s,t);

    dij_times.push_back(sd.millis); dij_nodes.push_back(sd.nodes_expanded); dij_dist.push_back(sd.distance); dij_pathlen.push_back(sd.path.size());
    astar_times.push_back(sa.millis); astar_nodes.push_back(sa.nodes_expanded); astar_dist.push_back(sa.distance); astar_pathlen.push_back(sa.path.size());
    bidir_times.push_back(sb.millis); bidir_nodes.push_back(sb.nodes_expanded); bidir_dist.push_back(sb.distance); bidir_pathlen.push_back(sb.path.size());

        if (!std::isfinite(sb.distance)) {
            std::cerr << "[DEBUG] Bidirectional A* failed for query " << i
                      << ": src=" << s << " (" << g.get_names()[s] << ")"
                      << ", tgt=" << t << " (" << g.get_names()[t] << ")"
                      << ". Dijkstra distance=" << sd.distance << ", A* distance=" << sa.distance << "\n";
        }

        if ((i+1)%10==0) std::cout<<"Completed "<<(i+1)<<"/"<<numq<<"\n";
    }

    auto dump_stats = [&](const std::string &label, std::vector<long long> &times, std::vector<size_t> &nodes, std::vector<double> &dist){
        double mean_time = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
        double mean_nodes = std::accumulate(nodes.begin(), nodes.end(), 0.0) / nodes.size();
        double mean_dist = std::accumulate(dist.begin(), dist.end(), 0.0) / dist.size();
        std::cout<<label<<" mean_time_ms="<<mean_time<<" mean_nodes="<<mean_nodes<<" mean_dist="<<mean_dist<<"\n";
        std::cout<<label<<" p50_time="<<percentile(times,50)<<" p90_time="<<percentile(times,90)<<" p99_time="<<percentile(times,99)<<"\n";
    };

    dump_stats("DIJKSTRA", dij_times, dij_nodes, dij_dist);
    dump_stats("ASTAR", astar_times, astar_nodes, astar_dist);
    dump_stats("BIDIR_ASTAR", bidir_times, bidir_nodes, bidir_dist);

    // store metrics CSV
    std::vector<std::pair<std::string, Stats>> rows;
    // average rows
    Stats sdavg; sdavg.distance = std::accumulate(dij_dist.begin(), dij_dist.end(), 0.0)/dij_dist.size(); sdavg.nodes_expanded = (size_t)(std::accumulate(dij_nodes.begin(), dij_nodes.end(), 0.0)/dij_nodes.size()); sdavg.millis = (long long)(std::accumulate(dij_times.begin(), dij_times.end(), 0.0)/dij_times.size()); sdavg.path = std::vector<int>(std::max((size_t)1, (size_t)(std::accumulate(dij_pathlen.begin(), dij_pathlen.end(), 0.0)/dij_pathlen.size())), -1);
    Stats saavg; saavg.distance = std::accumulate(astar_dist.begin(), astar_dist.end(), 0.0)/astar_dist.size(); saavg.nodes_expanded = (size_t)(std::accumulate(astar_nodes.begin(), astar_nodes.end(), 0.0)/astar_nodes.size()); saavg.millis = (long long)(std::accumulate(astar_times.begin(), astar_times.end(), 0.0)/astar_times.size()); saavg.path = std::vector<int>(std::max((size_t)1, (size_t)(std::accumulate(astar_pathlen.begin(), astar_pathlen.end(), 0.0)/astar_pathlen.size())), -1);
    Stats sbavg; sbavg.distance = std::accumulate(bidir_dist.begin(), bidir_dist.end(), 0.0)/bidir_dist.size(); sbavg.nodes_expanded = (size_t)(std::accumulate(bidir_nodes.begin(), bidir_nodes.end(), 0.0)/bidir_nodes.size()); sbavg.millis = (long long)(std::accumulate(bidir_times.begin(), bidir_times.end(), 0.0)/bidir_times.size()); sbavg.path = std::vector<int>(std::max((size_t)1, (size_t)(std::accumulate(bidir_pathlen.begin(), bidir_pathlen.end(), 0.0)/bidir_pathlen.size())), -1);
    rows.push_back({"dijkstra_avg", sdavg});
    rows.push_back({"astar_avg", saavg});
    rows.push_back({"bidir_astar_avg", sbavg});
    write_metrics_csv("results/metrics_batch.csv", rows);
    std::cout<<"Wrote results/metrics_batch.csv\n";
    return 0;
}
