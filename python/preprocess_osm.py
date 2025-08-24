"""
preprocess_osm.py
Extract a road graph using osmnx and write nodes and edges CSV
Requires: osmnx, networkx, pandas
Usage:
  python preprocess_osm.py --place "Pune, India" --outdir data
Or:
  python preprocess_osm.py --pbf path/to/file.pbf --bbox minx miny maxx maxy --outdir data
"""

import argparse
import os
import osmnx as ox
import networkx as nx
import pandas as pd

def graph_to_csv(G, outdir):
    os.makedirs(outdir, exist_ok=True)
    # relabel nodes to 0..n-1
    mapping = {nid:i for i,nid in enumerate(G.nodes())}
    G = nx.relabel_nodes(G, mapping)
    # nodes.csv
    nodes = []
    for nid, data in G.nodes(data=True):
        lat = data.get('y', 0.0)
        lon = data.get('x', 0.0)
        name = data.get('name', '')
        nodes.append((nid, lat, lon, name))
    nodes_df = pd.DataFrame(nodes, columns=['id','lat','lon','name'])
    nodes_df.to_csv(os.path.join(outdir,'cities.csv'), index=False)
    # edges.csv (undirected; combine multiple edges)
    edges = []
    for u,v,data in G.edges(data=True):
        length = data.get('length')
        if length is None: length = data.get('weight',1.0)
        edges.append((u,v,length))
    edges_df = pd.DataFrame(edges, columns=['u','v','w'])
    edges_df.to_csv(os.path.join(outdir,'routes.csv'), index=False)
    print("Wrote", outdir)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--place', type=str, help='place name for osmnx to geocode (e.g. "Pune, India")')
    parser.add_argument('--pbf', type=str, help='optional pbf file (not used in this script)')
    parser.add_argument('--outdir', type=str, default='data')
    args = parser.parse_args()
    if args.place:
        print("Downloading graph for", args.place)
        G = ox.graph_from_place(args.place, network_type='drive')
        G = ox.simplify_graph(G)
        # convert MultiDiGraph to undirected simple graph with lengths
        Gu = ox.get_undirected(G)
        graph_to_csv(Gu, args.outdir)
    else:
        print("Please supply --place 'City, Country'")
        return

if __name__ == "__main__":
    main()
