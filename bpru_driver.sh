# build the BPRU BP models

# build the specified models
if [ "$#" -le 1 ]; then
	echo "Argument 1 expecting y/n for (y: build and run, n: just run already built models) and Argument 2,3,4...n to be BP models names"
	echo ""
	echo "Available models:"
	ls ./branch | grep ".bpred"
	echo ""
	echo "Only use the base filename without the .bpred extension. i.e. predictor.bpred -> predictor"
	exit 1
fi

if [ "$1" != "y" ] && [ "$1" != "n" ]; then
	echo "Error: Arg1 must be 'y' or 'n'"
	echo "y: to build and run"
	echo "n: to run pre-built"
	exit 1
fi

# build the specified models
if [ "$1" = "y" ]; then
	shift
	for model in "$@"; do
		echo "Building $model..."
		./build_champsim.sh "$model" no no no no lru 1
	done
else
	shift
fi

# run the benchmarks for each model
# benchmarks will be the ones located in your /data/dpc3_traces/
for path in "/data/dpc3_traces"/*; do 
	benchmark=$( basename "$path" );
	echo "Running $benchmark on $# models...";

	for model in "$@"; do
		echo "Running $model..."
		 ./run_champsim.sh "$model"-no-no-no-no-lru-1core 1 10 "$benchmark"
	done
done
