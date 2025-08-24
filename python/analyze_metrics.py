# analyze_metrics.py
# reads results/metrics.csv or results/metrics_batch.csv and outputs summary: mean, median, p90, p99
import sys, csv, statistics

def read_metric(file):
    data = {}
    with open(file) as f:
        r = csv.DictReader(f)
        for row in r:
            label = row['label']
            if label not in data: data[label] = {'times':[], 'nodes':[], 'dist':[]}
            data[label]['dist'].append(float(row['distance']))
            data[label]['nodes'].append(int(row['nodes_expanded']))
            data[label]['times'].append(int(float(row['millis'])))
    return data

def pct(v,p):
    v=sorted(v)
    if not v: return 0
    idx = (p/100.0)*(len(v)-1)
    lo = int(idx); hi = min(lo+1, len(v)-1)
    frac = idx-lo
    return v[lo]*(1-frac) + v[hi]*frac

if __name__=='__main__':
    if len(sys.argv)<2:
        print("Usage: python analyze_metrics.py results/metrics_batch.csv")
        sys.exit(1)
    data = read_metric(sys.argv[1])
    for k,v in data.items():
        times = v['times']
        print(k, "count=", len(times))
        print(" mean time ms=", statistics.mean(times))
        print(" p50=", pct(times,50), "p90=", pct(times,90), "p99=", pct(times,99))
        print(" mean nodes=", statistics.mean(v['nodes']))
        print()
