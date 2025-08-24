#include <iostream>
#include "planner.h"
#include <queue>
#include <limits>
#include <cmath>
#include <chrono>
#include <algorithm>
#include <unordered_map>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static inline double haversine_km(double lat1,double lon1,double lat2,double lon2){
    // returns distance in kilometers
    const double R = 6371.0;
    double toRad = M_PI / 180.0;
    double dLat = (lat2 - lat1) * toRad;
    double dLon = (lon2 - lon1) * toRad;
    double a = std::sin(dLat/2)*std::sin(dLat/2) +
               std::cos(lat1*toRad)*std::cos(lat2*toRad) *
               std::sin(dLon/2)*std::sin(dLon/2);
    double c = 2*std::atan2(std::sqrt(a), std::sqrt(1-a));
    return R * c;
}

static std::vector<int> reconstruct_parent(const std::vector<int>& parent, int s, int t) {
    std::vector<int> path;
    if (t < 0 || t >= (int)parent.size()) return {};
    int cur = t;
    while (cur != -1) {
        path.push_back(cur);
        if (cur == s) break;
        cur = parent[cur];
    }
    if (path.empty() || path.back() != s) return {};
    std::reverse(path.begin(), path.end());
    return path;
}

/* Dijkstra: lazy PQ (stale entries skipped)
   Returns Stats with nodes_expanded and time (ms)
*/
Stats dijkstra_search(const Graph &g, int s, int t) {
    const auto &adj = g.adjacency();
    const int n = (int)adj.size();
    Stats st;
    if (s<0||s>=n||t<0||t>=n) return st;
    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> dist(n, INF);
    std::vector<int> parent(n, -1);
    using PQ = std::pair<double,int>;
    std::priority_queue<PQ, std::vector<PQ>, std::greater<PQ>> pq;
    dist[s] = 0.0;
    pq.push({0.0, s});
    size_t expanded = 0;

    auto t0 = std::chrono::high_resolution_clock::now();
    while (!pq.empty()) {
        auto [d,u] = pq.top(); pq.pop();
        if (d != dist[u]) continue; // stale
        expanded++;
        if (u == t) break;
        for (const auto &e : adj[u]) {
            if (dist[u] + e.w < dist[e.to]) {
                dist[e.to] = dist[u] + e.w;
                parent[e.to] = u;
                pq.push({dist[e.to], e.to});
            }
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    st.distance = dist[t];
    st.nodes_expanded = expanded;
    st.millis = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    st.path = reconstruct_parent(parent, s, t);
    return st;
}

/* A* using Haversine heuristic (lat/lon in degrees)
*/
Stats astar_search(const Graph &g, int s, int t) {
    const auto &adj = g.adjacency();
    const auto &coords = g.get_coords(); // lat,lon
    const int n = (int)adj.size();
    Stats st;
    if (s<0||s>=n||t<0||t>=n) return st;
    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> gscore(n, INF), fscore(n, INF);
    std::vector<int> parent(n, -1);
    std::vector<char> closed(n, 0);

    auto h = [&](int u)->double {
        // return haversine_km(coords[u].first, coords[u].second, coords[t].first, coords[t].second);
        double dx = coords[u].first - coords[t].first;
        double dy = coords[u].second - coords[t].second;
        return std::sqrt(dx*dx + dy*dy);
    };

    struct Node { double f; int v; };
    struct Cmp { bool operator()(const Node &a, const Node &b) const { return a.f > b.f; } };
    std::priority_queue<Node, std::vector<Node>, Cmp> open;
    gscore[s] = 0.0;
    fscore[s] = h(s);
    open.push({fscore[s], s});
    size_t expanded = 0;
    auto t0 = std::chrono::high_resolution_clock::now();

    while (!open.empty()) {
        Node cur = open.top(); open.pop();
        int u = cur.v;
        if (closed[u]) continue;
        closed[u] = 1;
        expanded++;
        if (u == t) break;
        for (const auto &e : adj[u]) {
            int v = e.to;
            if (closed[v]) continue;
            double tentative = gscore[u] + e.w;
            if (tentative < gscore[v]) {
                gscore[v] = tentative;
                parent[v] = u;
                fscore[v] = tentative + h(v);
                open.push({fscore[v], v});
            }
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    st.distance = gscore[t];
    st.nodes_expanded = expanded;
    st.millis = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    st.path = reconstruct_parent(parent, s, t);
    return st;
}

/* Bidirectional A*:
   - Run A* (forward from s, backward from t) with heuristic h_f(u)=h(u,t), h_b(u)=h(u,s).
   - Maintain g_f,g_b maps and parent arrays.
   - When searches meet, compute best path via meeting node.
   - Stop criterion: when min_f_in_open_forward + min_f_in_open_backward >= best_found
   Note: uses Haversine heuristic.
*/
Stats bidir_astar_search(const Graph &g, int s, int t) {
    const auto &adj = g.adjacency();
    const auto &coords = g.get_coords();
    const int n = (int)adj.size();
    Stats st;
    if (s<0||s>=n||t<0||t>=n) return st;
    if (s==t) { st.distance = 0; st.nodes_expanded = 0; st.millis = 0; st.path = {s}; return st; }

    auto h = [&](int u, int goal)->double {
        return haversine_km(coords[u].first, coords[u].second, coords[goal].first, coords[goal].second);
    };

    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> g_f(n, INF), g_b(n, INF);
    std::vector<int> p_f(n, -1), p_b(n, -1);
    std::vector<char> closed_f(n,0), closed_b(n,0);
    std::vector<char> seen(n,0);

    struct Node { double f; int v; };
    struct Cmp { bool operator()(const Node &a, const Node &b) const { return a.f > b.f; } };
    std::priority_queue<Node, std::vector<Node>, Cmp> open_f, open_b;

    g_f[s] = 0.0; open_f.push({g_f[s] + h(s, t), s});
    g_b[t] = 0.0; open_b.push({g_b[t] + h(t, s), t});
    double best_path = INF;
    int meeting_node = -1;
    size_t expanded = 0;

    auto relax_neighbors = [&](int u, const std::vector<double>& g_from,
                               std::vector<double>& g_other, std::vector<int>& parent_from,
                               const std::priority_queue<Node, std::vector<Node>, Cmp> &dummy){
        // noop; left for clarity
    };

    auto t0 = std::chrono::high_resolution_clock::now();
    while (!open_f.empty() && !open_b.empty()) {
        double top_f_f = open_f.empty() ? INF : open_f.top().f;
        double top_f_b = open_b.empty() ? INF : open_b.top().f;

        if (top_f_f + top_f_b >= best_path) break;

        // Expand forward
        Node cur_f = open_f.top(); open_f.pop();
        int u_f = cur_f.v;
        if (closed_f[u_f]) continue;
        closed_f[u_f] = 1;
        expanded++;
        for (const auto &e : adj[u_f]) {
            int v = e.to;
            double tentative = g_f[u_f] + e.w;
            if (tentative < g_f[v]) {
                g_f[v] = tentative;
                p_f[v] = u_f;
                double fscore = tentative + h(v, t);
                open_f.push({fscore, v});
            }
            if (closed_b[v]) {
                double cand = g_f[v] + g_b[v];
                if (cand < best_path) { best_path = cand; meeting_node = v; }
            }
        }

        // Expand backward
        Node cur_b = open_b.top(); open_b.pop();
        int u_b = cur_b.v;
        if (closed_b[u_b]) continue;
        closed_b[u_b] = 1;
        expanded++;
        for (const auto &e : adj[u_b]) {
            int v = e.to;
            double tentative = g_b[u_b] + e.w;
            if (tentative < g_b[v]) {
                g_b[v] = tentative;
                p_b[v] = u_b;
                double fscore = tentative + h(v, s);
                open_b.push({fscore, v});
            }
            if (closed_f[v]) {
                double cand = g_f[v] + g_b[v];
                if (cand < best_path) { best_path = cand; meeting_node = v; }
            }
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    st.nodes_expanded = expanded;
    st.millis = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    if (best_path == INF) {
        std::cerr << "[BidirA*] No meeting node found.\n";
        st.distance = INF;
        st.path = {};
        return st;
    }
    // std::cerr << "[BidirA*] Meeting node: " << meeting_node << ", best_path: " << best_path << "\n";
    int meet = meeting_node;
    // build left path from s to meet using p_f
    std::vector<int> left;
    for (int cur = meet; cur != -1; cur = p_f[cur]) left.push_back(cur);
    std::reverse(left.begin(), left.end());
    // std::cerr << "[BidirA*] Left path: ";
    // for (size_t i = 0; i < left.size(); ++i) std::cerr << left[i] << (i+1<left.size()?" -> ":"");
    // std::cerr << "\n";
    // build right path from meet to t using p_b (but p_b points towards t)
    std::vector<int> right;
    for (int cur = p_b[meet]; cur != -1; cur = p_b[cur]) right.push_back(cur);
    // std::cerr << "[BidirA*] Right path: ";
    // for (size_t i = 0; i < right.size(); ++i) std::cerr << right[i] << (i+1<right.size()?" -> ":"");
    // std::cerr << "\n";
    st.path = left;
    st.path.insert(st.path.end(), right.begin(), right.end());
    st.distance = best_path;
    return st;
}
