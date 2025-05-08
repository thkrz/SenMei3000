import numpy as np
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

from . import csv

plt.rc("font", family="serif")
plt.rc("xtick", labelsize="x-small")
plt.rc("ytick", labelsize="x-small")


def plot(ax, t, s, label):
    ax.plot(t, s, color="black", linewidth=0.5)
    ax.set_ylabel(label)


def make(path):
    cols, t, y = csv.load(path)
    N = y.shape[1]
    shape = (N, 1)
    fig, axs = plt.subplots(
        *shape, figsize=(10, N * 2), sharex=True, layout="constrained"
    )
    for i in range(N):
        plot(axs[i], t, y[:, i], cols[i + 1])
    ax = axs[N - 1]
    ax.set_xlabel("Time")
    ax.xaxis.set_major_formatter(
        mdates.ConciseDateFormatter(ax.xaxis.get_major_locator())
    )
    name = path.name.removesuffix("".join(path.suffixes))
    plt.savefig(path.parent / (path.stem + ".svg"))
