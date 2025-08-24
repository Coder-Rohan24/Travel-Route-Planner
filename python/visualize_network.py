import csv
from pyvis.network import Network

# Load cities
city_info = {}
with open('data/cities.csv', newline='') as f:
    reader = csv.DictReader(f)
    for row in reader:
        city_info[int(row['node_id'])] = row['name']

# Build network
net = Network(height='800px', width='100%', notebook=False, directed=False)
for node_id, name in city_info.items():
    net.add_node(node_id, label=name)

with open('data/routes.csv', newline='') as f:
    reader = csv.DictReader(f)
    for row in reader:
        src = int(row['src_id'])
        dst = int(row['dst_id'])
        weight = float(row['distance'])
        net.add_edge(src, dst, value=weight, title=f"{weight} km")

# Use write_html instead of show to avoid notebook issues
net.write_html('results/network_map.html')
print("Visualization saved to results/network_map.html")
