import os
import matplotlib.pyplot as plt
import pprint
import numpy as np

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
	"bpru_bimodal",
	"bpru_gshare",
	"bpru_tournament",
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
# print("\nEXAMPLE for bpru_bimodal:")
# pprint.pprint(RESULTS["bpru_bimodal"])

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

# plot all the branch prediction accuracies for size 2^6 predictor models
fig = plt.figure()
ax = fig.add_subplot(1,1,1)
X = np.arange(len(BENCHMARKS))
W = 0.15
for i,model in enumerate(MODELS):
	acc_stats = get_plottable_stats(6, model, "acc")
	ax.bar(X + i * W, acc_stats, color="C"+str(i), width=W)
	plt.bar(0, 0, color="C"+str(i), label=model)

ax.set_xticks(X + len(X)*W/2)
ax.set_xticklabels(BENCHMARK_LABELS, rotation=10)
plt.legend(bbox_to_anchor=(1,1), loc="upper left", prop={'size': 8})
plt.title("Branch Predictor Accuracy (Model size of 2^6)")
plt.ylabel("Accuracy %")
plt.xlabel("Benchmark")
plt.tight_layout()
plt.show()

