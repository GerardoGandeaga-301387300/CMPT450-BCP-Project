import os
import matplotlib.pyplot as plt
import pprint

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
			ipc_i = line.find(ACC_TEMPLATE)
			if ipc_i != -1:
				start = ipc_i + len(ACC_TEMPLATE)
				end = line.find("%", start)
				ipc = line[start:end]
				stats["acc"] = float(ipc)
			# seek MPKI
			ipc_i = line.find(MPKI_TEMPLATE)
			if ipc_i != -1:
				start = ipc_i + len(MPKI_TEMPLATE)
				end = line.find(" ", start)
				ipc = line[start:end]
				stats["mpki"] = float(ipc)
			# seek Avg ROB Occupancy
			ipc_i = line.find(ROB_TEMPLATE)
			if ipc_i != -1:
				start = ipc_i + len(ROB_TEMPLATE)
				end = line.find(" ", start)
				ipc = line[start:end]
				stats["rob"] = float(ipc)

	# pprint.pprint(stats)
	return stats


# model names <BCP_><BP>
MODELS = [
	# vanilla models
	"bimodal",
	"gshare",

	# with confidence predictors
	"bpru_bimodal",
	"bpru_gshare",
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

pprint.pprint(RESULTS)
print("\nEXAMPLE for bpru_bimodal:")
pprint.pprint(RESULTS["bpru_bimodal"])

# iterate through results plot as needed
# generate a new plot for each size
# for size in SIZES:
	


