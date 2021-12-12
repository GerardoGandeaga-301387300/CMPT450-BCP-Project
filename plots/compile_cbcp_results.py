import os
from typing import Collection
import matplotlib.pyplot as plt
import pprint
import numpy as np
import pandas as pd

def get_stats(benchmark, size, model):
	"""
	This function will return a map with important stats
	from the final trace we can use for plotting.

	Stats:
	- Cumulative IPC
	- Branch prediction Accuracy
	- MPKI
	- Avg ROB Occupancy as Mispredict
	"""
	IPC_TEMPLATE = "cumulative IPC: "
	ACC_TEMPLATE = "Branch Prediction Accuracy: "
	MPKI_TEMPLATE = "MPKI: "
	ROB_TEMPLATE = "Average ROB Occupancy at Mispredict: "

	stats = {}

	with open("./RESULTS/{}/{}-{}-no-no-no-no-lru-1core.txt".format(size, benchmark, model), "r") as f:
		lines = f.readlines()
		for line in lines:
			# seek IPC
			ipc_i = line.find(IPC_TEMPLATE)
			if ipc_i != -1:
				start = ipc_i + len(IPC_TEMPLATE)
				end = line.find(" ", start)
				ipc = line[start:end]
				stats["ipc"] = float(ipc)
			# seek Accuracy
			acc_i = line.find(ACC_TEMPLATE)
			if acc_i != -1:
				start = acc_i + len(ACC_TEMPLATE)
				end = line.find("%", start)
				ipc = line[start:end]
				stats["acc"] = float(ipc)
			# seek MPKI
			mpki_i = line.find(MPKI_TEMPLATE)
			if mpki_i != -1:
				start = mpki_i + len(MPKI_TEMPLATE)
				end = line.find(" ", start)
				ipc = line[start:end]
				stats["mpki"] = float(ipc)
			# seek Avg ROB Occupancy
			rob_i = line.find(ROB_TEMPLATE)
			if rob_i != -1:
				start = rob_i + len(ROB_TEMPLATE)
				end = line.find(" ", start)
				ipc = line[start:end]
				stats["rob"] = float(ipc)

	# pprint.pprint(stats)
	return stats

# TODO: Add your models here!
# model names <BCP_><BP>
MODELS = [
	# vanilla models
	"bimodal",
	"gshare",
	"tournament",

	# with confidence predictors
	"cbcp_bimodal",
	"cbcp_gshare",
	"cbcp_tournament",
]
# model sizes 2^N
SIZES = [6, 10, 14, 24]

BENCHMARKS = [
	"600.perlbench_s-210B.champsimtrace.xz",
	"602.gcc_s-734B.champsimtrace.xz",
	"625.x264_s-18B.champsimtrace.xz",
	"641.leela_s-800B.champsimtrace.xz",
	"648.exchange2_s-1699B.champsimtrace.xz",
]

"""
RESULTS is a map with the following structure
{
	"model": {
		size: {
			"600.perlbench_s-210B.champsimtrace.xz": { acc: float, mpki: float, rob: float, IPC: float }
			"602.gcc_s-734B.champsimtrace.xz": { acc: float, mpki: float, rob: float, IPC: float }
			"625.x264_s-18B.champsimtrace.xz": { acc: float, mpki: float, rob: float, IPC: float }
			"641.leela_s-800B.champsimtrace.xz": { acc: float, mpki: float, rob: float, IPC: float }
			"648.exchange2_s-1699B.champsimtrace.xz": { acc: float, mpki: float, rob: float, IPC: float }
		}
		.
		.
		.
	}
	.
	.
	.
}
"""
# Compile results
RESULTS = {}
for model in MODELS:
	RESULTS[model] = {}
	for size in SIZES: 
		RESULTS[model][size] = {}
		for benchmark in BENCHMARKS:
			RESULTS[model][size][benchmark] = get_stats(benchmark, size, model)

# pprint.pprint(RESULTS)
# print("\nEXAMPLE for cbcp_bimodal:")
# pprint.pprint(RESULTS["cbcp_bimodal"])

# ----------------------------------- PLOTTING ----------------------------------- 
# iterate through results plot as needed
# generate a new plot for each size

# Get the xtick label names
BENCHMARK_LABELS = [bm[bm.find('.')+1:] for bm in BENCHMARKS]
BENCHMARK_LABELS = [bm[:bm.find('.')] for bm in BENCHMARK_LABELS]

def get_plottable_stats(size, model, stat):
	"""
	Get all the stat values from the specified model and
	model size
	"""
	model_stats = RESULTS[model][size]
	stats = [model_stats[bm][stat] for bm in BENCHMARKS]
	return stats

# pprint.pprint(get_plottable_stats(6, "bimodal", "acc"))
# pprint.pprint(get_plottable_stats(6, "bimodal", "mpki"))
# pprint.pprint(get_plottable_stats(6, "bimodal", "rob"))

def create_table(file_name, size, stat):
	file_name = "./plots/cbcp/{}_{}_{}.png".format(file_name, stat, size)
	fig, ax = plt.subplots()
	fig.patch.set_visible(False)
	ax.axis("off")
	ax.axis("tight")
	data = []
	for i,model in enumerate(MODELS):
		acc_stats = get_plottable_stats(size, model, stat)
		data.append(np.array(acc_stats))
		# ax.bar(X + i * W, acc_stats, color="C"+str(i), width=W)
		# plt.bar(0, 0, color="C"+str(i), label=model)
	data[0] = np.append(data[0], np.mean(data[0]))
	data[1] = np.append(data[1], np.mean(data[1]))
	data[2] = np.append(data[2], np.mean(data[2]))
	data[3] = np.append(data[3], np.mean(data[3]))
	data[4] = np.append(data[4], np.mean(data[4]))
	data[5] = np.append(data[5], np.mean(data[5]))
	data = np.transpose(data)
	# pprint.pprint(data)
	df = pd.DataFrame(data, columns=MODELS)
	table = ax.table(cellText=df.values, colLabels=df.columns, rowLabels=[*BENCHMARK_LABELS, "Mean"], loc="center", cellLoc="center", rowLoc="right")
	fig.tight_layout()
	plt.savefig(file_name, dpi=600)


#region ------------- cbcp Plot -------------

# prpgram specs plots, these values were computed
program_ips_hists = {
	"ips": [961, 55, 159, 800, 272],
	"hist": [1050, 64, 187, 1069, 420],
}
fig, ax = plt.subplots()
# ax = fig.add_subplot(1,1,1)
ax.plot(BENCHMARK_LABELS, program_ips_hists["ips"], color="b", marker="o", label="Unique Branch IPs")
ax.plot(BENCHMARK_LABELS, program_ips_hists["hist"], color="r", marker="^", linestyle="dashed", label="Unique Branch Outcome Combinations")
ax.set_xticklabels(BENCHMARK_LABELS, rotation=10)
plt.ylabel("# of Branch IPs")
plt.title("Unique Instructions Against Branch Combinations")
plt.tight_layout()
plt.legend()
plt.savefig("./plots/cbcp/benchmark_ip_hist.png", dpi=600)

# actual cbcp plots
create_table("cbcp", 6,  "acc")
create_table("cbcp", 10, "acc")
create_table("cbcp", 14, "acc")
create_table("cbcp", 24, "acc")
create_table("cbcp", 6,  "ipc")
create_table("cbcp", 10, "ipc")
create_table("cbcp", 14, "ipc")
create_table("cbcp", 24, "ipc")

#endregion 
