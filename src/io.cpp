#include "io.h"
#include <fstream>
#include <iomanip>
#include <iostream>

bool write_geojson(const Graph &g, const std::vector<int> &path, const std::string &outpath) {
    std::ofstream out(outpath);
    if (!out.is_open()) { std::cerr<<"Failed to open geojson file\n"; return false; }
    const auto &coords = g.get_coords();
    out << std::fixed << std::setprecision(7);
    out << "{ \"type\": \"FeatureCollection\", \"features\": [\n";

    // path line
    out << "{ \"type\":\"Feature\", \"geometry\": { \"type\":\"LineString\", \"coordinates\": [";
    for (size_t i=0;i<path.size();++i) {
        int id = path[i];
        out << "[" << coords[id].second << ", " << coords[id].first << "]";
        if (i+1<path.size()) out << ", ";
    }
    out << "] }, \"properties\": { \"stroke\": \"#FF0000\", \"stroke-width\": 4 } }";

    // Point features for all cities in the path
    for (size_t i = 0; i < path.size(); ++i) {
      int id = path[i];
      std::string color;
      std::string title;
      if (i == 0) {
        color = "#ff0000ff";
        title = "start";
      } else if (i == path.size() - 1) {
        color = "#0000FF";
        title = "target";
      } else {
        color = "#FFA500";
        title = "city";
      }
      out << ",{ \"type\":\"Feature\", \"geometry\": { \"type\":\"Point\", \"coordinates\": [" << coords[id].second << ", " << coords[id].first << "] }, \"properties\": { \"marker-color\":\"" << color << "\", \"title\":\"" << title << "\" } }";
    }
    out << "] }\n";
    out.close();
    return true;
}

bool write_leaflet_html(const std::string &geojson_file, const std::string &html_out) {
    std::ofstream out(html_out);
    if (!out.is_open()) { std::cerr<<"Failed to open html file\n"; return false; }
    out << R"(<!doctype html>
<html>
<head>
  <meta charset="utf-8" />
  <title>Route Planner - Visualization</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <link rel="stylesheet" href="https://unpkg.com/leaflet@1.9.4/dist/leaflet.css" />
  <style> #map { height: 90vh; width: 100%; margin:0; padding:0; } body{ margin:0; padding:0; } </style>
</head>
<body>
<div id="map"></div>
<script src="https://unpkg.com/leaflet@1.9.4/dist/leaflet.js"></script>
<script>
fetch('route.geojson').then(r=>r.json()).then(data=>{
  var map = L.map('map');
  var coords = data.features[0].geometry.coordinates;
  var first = coords[0];
  map.setView([first[1], first[0]], 12);
  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '© OpenStreetMap contributors'
  }).addTo(map);
    L.geoJSON(data, {
      style: function(feature){
        return {color: feature.properties.stroke || '#00FF00', weight: feature.properties["stroke-width"]||3};
      },
      pointToLayer: function(feature, latlng){
        return L.circleMarker(latlng, {
          radius: 6,
          color: feature.properties["marker-color"] || '#FFA500',      // outline color
          fillColor: feature.properties["marker-color"] || '#FFA500',  // fill color
          fillOpacity: 1
        });
      }
    }).addTo(map);
});
</script>
</body>
</html>)";
    out.close();
    return true;
}

bool write_metrics_csv(const std::string &out_csv,
                       const std::vector<std::pair<std::string, Stats>> &rows) {
    std::ofstream out(out_csv);
    if (!out.is_open()) { std::cerr<<"Failed to open metrics csv\n"; return false; }
    out << "label,distance,nodes_expanded,millis,path_len\n";
    for (const auto &p : rows) {
        const Stats &s = p.second;
        out << p.first << "," << s.distance << "," << s.nodes_expanded << "," << s.millis << "," << s.path.size() << "\n";
    }
    out.close();
    return true;
}
