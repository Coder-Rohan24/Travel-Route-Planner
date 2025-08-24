#include "graph.h"
#include <fstream>
#include <sstream>
#include <iostream>

static inline std::string trim(const std::string &s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b-a+1);
}

bool Graph::ensure_size(int n) {
    if ((int)coords.size() >= n) return true;
    coords.resize(n, {0.0,0.0});
    names.resize(n);
    adj.resize(n);
    return true;
}

bool Graph::load_nodes_csv(const std::string &nodes_csv) {
    std::ifstream in(nodes_csv);
    if (!in.is_open()) { std::cerr << "Failed to open: " << nodes_csv << "\n"; return false; }
    std::string line;
    bool header_checked = false;
    int max_id = -1;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        // header detection: skip first non-numeric line
        if (!header_checked) {
            std::string first = line.substr(0,line.find(','));
            bool numeric = true;
            for(char c: first) if (!(c=='-'||c=='.'||isdigit(c))) { numeric=false; break; }
            if (!numeric) { header_checked = true; continue; }
            header_checked = true;
        }
        int id; double lat, lon;
        if (!(ss >> id)) continue;
        if (ss.peek() == ',') ss.get();
        if (!(ss >> lat)) continue;
        if (ss.peek() == ',') ss.get();
        if (!(ss >> lon)) continue;
        if (ss.peek() == ',') ss.get();
        std::string rest; std::getline(ss, rest);
        std::string name = trim(rest);
        if (id > max_id) max_id = id;
        ensure_size(max_id+1);
        coords[id] = {lat, lon};
        names[id] = name;
    }
    if (max_id >= 0) ensure_size(max_id+1);
    return true;
}

bool Graph::load_edges_csv(const std::string &edges_csv, bool undirected) {
    std::ifstream in(edges_csv);
    if (!in.is_open()) { std::cerr << "Failed to open: " << edges_csv << "\n"; return false; }
    std::string line;
    bool header_checked = false;
    int line_num = 0;
    while (std::getline(in, line)) {
        ++line_num;
        if (line.empty()) continue;
        // Skip header
        if (!header_checked) {
            std::stringstream ss(line);
            std::string first;
            std::getline(ss, first, ',');
            bool numeric = true;
            for(char c: first) if (!(c=='-'||c=='.'||isdigit(c))) { numeric=false; break; }
            if (!numeric) { header_checked = true; continue; }
            header_checked = true;
        }
        std::stringstream ss(line);
        std::string u_str, v_str, w_str;
        if (!std::getline(ss, u_str, ',')) { std::cerr << "Parse error at line " << line_num << "\n"; continue; }
        if (!std::getline(ss, v_str, ',')) { std::cerr << "Parse error at line " << line_num << "\n"; continue; }
        if (!std::getline(ss, w_str, ',')) { std::cerr << "Parse error at line " << line_num << "\n"; continue; }
        int u = -1, v = -1;
        double w = 0.0;
        try {
            u = std::stoi(u_str);
            v = std::stoi(v_str);
            w = std::stod(w_str);
        } catch (...) {
            std::cerr << "Parse error at line " << line_num << ": " << line << "\n";
            continue;
        }
        if (u < 0 || v < 0) continue;
        int needed = std::max(u,v)+1;
        ensure_size(needed);
        adj[u].push_back(Edge(v,w));
        if (undirected) adj[v].push_back(Edge(u,w));
    }
    return true;
}
