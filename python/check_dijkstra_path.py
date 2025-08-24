import csv
from collections import defaultdict, deque

# Load cities
city_info = {}
with open('data/cities.csv', newline='') as f:
    reader = csv.DictReader(f)
    for row in reader:
        city_info[int(row['node_id'])] = row['name']

# Build adjacency list
adj = defaultdict(list)
with open('data/routes.csv', newline='') as f:
    reader = csv.DictReader(f)
    for row in reader:
        src = int(row['src_id'])
        dst = int(row['dst_id'])
        weight = float(row['distance'])
        adj[src].append((dst, weight))
        adj[dst].append((src, weight))  # undirected

# Print adjacency list
print('Adjacency List:')
for node, edges in adj.items():
    print(f"{node} ({city_info[node]}): {[f'{dst} ({city_info[dst]}) w={w}' for dst, w in edges]}")

# Dijkstra's algorithm to get path
import heapq

def dijkstra(adj, start, end):
    heap = [(0, start, [])]
    visited = set()
    while heap:
        cost, node, path = heapq.heappop(heap)
        if node in visited:
            continue
        visited.add(node)
        path = path + [node]
        if node == end:
            return cost, path
        for neighbor, weight in adj[node]:
            if neighbor not in visited:
                heapq.heappush(heap, (cost + weight, neighbor, path))
    return float('inf'), []

# Example: Find path from Kabul (0) to Lima (17)
cost, path = dijkstra(adj, 0, 17)
print(f"\nShortest path from Kabul (0) to Lima (17):")
print(f"Total distance: {cost}")
print(f"Path: {[f'{node} ({city_info[node]})' for node in path]}")
