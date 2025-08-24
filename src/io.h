#ifndef IO_H
#define IO_H

#include "graph.h"
#include "planner.h"
#include <string>
#include <vector>
#include <utility>

bool write_geojson(const Graph &g, const std::vector<int> &path, const std::string &outpath);
bool write_leaflet_html(const std::string &geojson_file, const std::string &html_out);
bool write_metrics_csv(const std::string &out_csv,
                       const std::vector<std::pair<std::string, Stats>> &rows);

#endif // IO_H
