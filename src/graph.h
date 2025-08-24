#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <string>
#include <utility>

struct Edge {
    int to;
    double w;
    Edge(int _to=-1,double _w=0.0): to(_to), w(_w) {}
};

class Graph {
public:
    Graph() = default;
    bool load_nodes_csv(const std::string &nodes_csv);
    bool load_edges_csv(const std::string &edges_csv, bool undirected = true);

    int num_nodes() const { return (int)coords.size(); }
    const std::vector<std::pair<double,double>>& get_coords() const { return coords; } // (lat, lon)
    const std::vector<std::string>& get_names() const { return names; }
    const std::vector<std::vector<Edge>>& adjacency() const { return adj; }

private:
    std::vector<std::pair<double,double>> coords;
    std::vector<std::string> names;
    std::vector<std::vector<Edge>> adj;

    bool ensure_size(int n);
};

#endif // GRAPH_H
