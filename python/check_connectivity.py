import csv
from collections import defaultdict, deque

# Load cities
cities = set()
with open('data/cities.csv', newline='') as f:
    reader = csv.DictReader(f)
    for row in reader:
        cities.add(int(row['node_id']))

# Build adjacency list
adj = defaultdict(list)
with open('data/routes.csv', newline='') as f:
    reader = csv.DictReader(f)
    for row in reader:
        src = int(row['src_id'])
        dst = int(row['dst_id'])
        adj[src].append(dst)
        adj[dst].append(src)  # undirected

# BFS to check connectivity
visited = set()
def bfs(start):
    q = deque([start])
    visited.add(start)
    while q:
        u = q.popleft()
        for v in adj[u]:
            if v not in visited:
                visited.add(v)
                q.append(v)

# Start from first city
if cities:
    bfs(next(iter(cities)))
    print(f"Connected: {len(visited)} of {len(cities)} cities reachable.")
    unreachable = cities - visited
    if unreachable:
        print("Unreachable city IDs:", unreachable)
    else:
        print("All cities are connected.")
else:
    print("No cities found.")
