import matplotlib.pyplot as plt

data = {
    5: [(106,0),(116,2),(127,2),(140,3),(155,0),(170,2),(174,2),(205,2),(219,2),
        (247,1),(250,1),(260,3),(283,3),(295,1),(299,1),(331,3),(334,3),(366,3),(369,3),(414,3)],
    6: [(101,0),(106,2),(153,2),(175,2),(195,2),(229,3),(254,0),(285,2),(292,2),
        (311,2),(338,2),(356,3),(373,3),(404,2)],
    7: [(107,0),(127,3),(164,3),(195,3),(251,3),(280,3),(289,3),(351,2),(367,2),(397,2)],
    8: [(127,0),(160,2),(175,2),(237,3),(255,3),(328,3),(351,3),(399,3),(422,3),(455,3)],
}

colors = {5: "#1f77b4", 6: "#d62728", 7: "#2ca02c", 8: "#9467bd"}

fig, ax = plt.subplots(figsize=(11, 5))

max_tick = max(t for pts in data.values() for t, q in pts)

boost = 48
t = boost
while t <= max_tick + 10:
    ax.axvline(t, color="gray", linestyle="--", linewidth=0.8, alpha=0.5, zorder=0)
    t += boost

for pid, pts in data.items():
    xs = [p[0] for p in pts]
    ys = [p[1] for p in pts]
    ax.step(xs, ys, where="post", color=colors[pid], linewidth=1.5, label=f"PID {pid}", zorder=2)
    ax.scatter(xs, ys, color=colors[pid], s=28, zorder=3)

ax.set_xlabel("Time elapsed (ticks)")
ax.set_ylabel("MLFQ queue level")
ax.set_yticks([0, 1, 2, 3])
ax.set_ylim(-0.3, 3.3)
ax.invert_yaxis()
ax.set_title("MLFQ Queue Level Over Time (dashed lines = 48-tick priority boost)")
ax.legend(loc="upper right", title="Process")
ax.grid(True, axis="y", linestyle=":", alpha=0.4)

plt.text(
    0.95, 0.95, "sudhanshu.rai",
    ha='right', va='top',
    transform=plt.gca().transAxes,
    fontsize=10, color="gray", alpha=0.7
)

plt.tight_layout()
plt.savefig("mlfq_timeline.png", dpi=150)