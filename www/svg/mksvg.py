import csv
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates


def plot(ax, t, s, label):
    t0 = t.astype("datetime64[h]")
    t1 = np.unique(t0)
    s1 = np.zeros(t1.size)
    for i, dt in enumerate(t1):
        s1[i] = s[t == dt].mean()

    ax.plot(t, s, color="b", linewidth=0.5)
    ax.plot(t1, s1, color="r")
    ax.set_xlabel("Time")
    ax.set_ylabel(label)
    ax.xaxis.set_major_formatter(
        mdates.ConciseDateFormatter(ax.xaxis.get_major_locator())
    )


def make(path):
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
    N = (2, y.shape[1] // 2 + 1)
    fig, axs = plt.subplots(*N, figsize=(12, 8))
    for i, j in np.ndindex(N):
        k = np.ravel_multi_index((i, j), N)
        if k < y.shape[1]:
            plot(axs[i, j], t, y[:, k], cols[k + 1])
    for ax in axs.flat[y.shape[1] :]:
        ax.remove()
    name = path.name.removesuffix("".join(path.suffixes))
    plt.savefig(name + ".svg")


from pathlib import Path
p = Path("%.csv")
make(p)
