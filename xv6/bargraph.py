import matplotlib.pyplot as plt
import numpy as np

schedulers = ["FCFS", "Round Robin (RR)", "Multi-Level Feedback Queue (MLFQ)"]
turnaround = [315.0, 340.8, 325.5]
waiting = [255.4, 240.2, 225.8]
response = [104.5, 57.2, 68.5]

x = np.arange(len(schedulers))
width = 0.25

fig, ax = plt.subplots(figsize=(10, 6))

ax.bar(x - width, turnaround, width, label="Avg Turnaround Time", color="#1f77b4")
ax.bar(x, waiting, width, label="Avg Waiting Time", color="#d62728")
ax.bar(x + width, response, width, label="Avg Response Time", color="#2ca02c")

ax.set_xlabel("Scheduler")
ax.set_ylabel("Ticks")
ax.set_title("Scheduler Comparison: Turnaround, Waiting, and Response Time")
ax.set_xticks(x)
ax.set_xticklabels(schedulers)
ax.legend()
ax.grid(True, axis="y", linestyle=":", alpha=0.4)

plt.text(
    0.95, 0.95, "sudhanshu.rai",
    ha='right', va='top',
    transform=plt.gca().transAxes,
    fontsize=10, color="gray", alpha=0.7
)

plt.tight_layout()
plt.savefig("scheduler_comparison.png", dpi=150)