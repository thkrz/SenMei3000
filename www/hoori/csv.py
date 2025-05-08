import csv
import numpy as np


def load(path):
    t = []
    y = []
    with open(path) as f:
        reader = csv.reader(f, delimiter=",")
        cols = next(reader)
        for row in reader:
            t.append(np.datetime64(row[0]))
            y.append([float(n) for n in row[1:]])
    t = np.array(t, dtype=np.datetime64)
    y = np.array(y)
    return cols, t, y


def stats(path):
    cols, t, y = load(path)
    if t.size < 2:
        return {}
    M, N = y.shape
    I = np.min(t[1:] - t[:-1])
    K = int(np.timedelta64(1, "D") / I)
    d = {}
    for i, k in enumerate(cols[1:]):
        m = max(0, M - K)
        d[k] = {
            "before": y[M - 2, i],
            "last": y[M - 1, i],
            "mean": f"{y[m:, i].mean():.2f}",
            "min": f"{y[m:, i].min()}",
            "max": f"{y[m:, i].max()}",
        }
    return {
        "start": str(t[0]),
        "end": str(t[-1]),
        "interval": str(I),
        "info": d,
    }
