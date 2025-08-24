import csv

with open('data/routes.csv', newline='') as f:
    reader = csv.DictReader(f)
    invalid = []
    for row in reader:
        dist = float(row['distance'])
        if dist <= 0:
            invalid.append((row['src_id'], row['dst_id'], dist))

if invalid:
    print("Found invalid (zero or negative) distances:")
    for src, dst, dist in invalid:
        print(f"{src} -> {dst}: {dist}")
else:
    print("All route distances are positive and non-zero.")
