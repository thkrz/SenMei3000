import matplotlib.pyplot as plt
import numpy as np

N = 35040
x = np.linspace(1, N, num=N)


def f(x):
    return np.sin(x) + np.random.normal(scale=0.1, size=len(x))

y = f(x)


t = np.datetime64("2025-01-29T10:00")
dt = np.timedelta64(15, "m")
for i in range(N):
    print(t, end="\r\n")
    print("{:.2f}".format(12 - 0.5 * y[i]), end="\r\n")
    print("{:.2f}".format(50 - np.random.normal(scale=10)), end="\r\n")
    print("{:.2f}".format(15 - np.random.normal(scale=10)), end="\r\n")
    for c in "01ab":
        m = max(0.0, 20 - np.random.normal(scale=10))
        T = max(0.0, 10 - np.random.normal(scale=10))
        print(f"{c}+{m:.4f}+{T:.4f}", end="\r\n")
    print("\r\n", end="")
    t += dt

# plt.plot(x, 12-0.5*f(x))
# plt.show()
