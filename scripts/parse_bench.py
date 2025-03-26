#!/usr/bin/env python3
"""
Simple benchmark JSON parser for services/order-matching-engine/out/bench_results.json
Computes percentiles (p50/p90/p99/p99.9) from iteration real_time values and
writes a small markdown snippet to services/order-matching-engine/bench_summary.md
and prints the snippet to stdout.
"""
import json
import math
import sys
from pathlib import Path

if len(sys.argv) > 1:
    in_path = Path(sys.argv[1])
else:
    in_path = Path('services/order-matching-engine/out/bench_results.json')

if not in_path.exists():
    print(f"Bench JSON not found at {in_path.resolve()}")
    raise SystemExit(2)

j = json.loads(in_path.read_text())

# find iteration entries for the BM_AddOrderAndMatch benchmark
values = []
for b in j.get('benchmarks', []):
    if b.get('run_type') == 'iteration' and 'BM_AddOrderAndMatch' in b.get('name', ''):
        # real_time is in the unit given by time_unit (we'll assume microseconds 'us')
        values.append(float(b.get('real_time')))

if not values:
    print('No iteration values found for BM_AddOrderAndMatch')
    raise SystemExit(3)

values.sort()

def percentile(data, p):
    if not data:
        return None
    k = (len(data)-1) * (p/100.0)
    f = math.floor(k)
    c = math.ceil(k)
    if f == c:
        return data[int(k)]
    d0 = data[int(f)] * (c-k)
    d1 = data[int(c)] * (k-f)
    return d0 + d1

p50 = percentile(values, 50)
p90 = percentile(values, 90)
p99 = percentile(values, 99)
p999 = percentile(values, 99.9)
mean = sum(values)/len(values)

ctx = j.get('context', {})
host = ctx.get('host_name') or ctx.get('host') or 'unknown'
date = ctx.get('date', 'unknown')
cpus = ctx.get('num_cpus', ctx.get('cpus', 'unknown'))

md = f"""
## Benchmark summary (computed)

| Metric | Value |
|---|---:|
| p50 | {p50:.6f} µs |
| p90 | {p90:.6f} µs |
| p99 | {p99:.6f} µs |
| p99.9 | {p999:.6f} µs |
| mean | {mean:.6f} µs |
| samples | {len(values)} |

Context:

- host: `{host}`
- date: `{date}`
- cpus: `{cpus}`
"""

out_path = Path('services/order-matching-engine/bench_summary.md')
out_path.parent.mkdir(parents=True, exist_ok=True)
out_path.write_text(md)
print(md)
print(f"Wrote summary to {out_path}")

if __name__ == '__main__' and len(sys.argv) > 1:
    # If run with explicit path, we already executed above; just exit cleanly
    pass
