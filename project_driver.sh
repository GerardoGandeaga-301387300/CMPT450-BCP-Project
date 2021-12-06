

# ================================== BPRU ==================================
# Do not edit this section, tested and it works!
echo "Running 3 BRPU models with 5 traces on table sizes 2^6"
./model_driver.sh y bpru_bimodal bpru_gshare bpru_tournament

# compile plot data. Note, that the above command only generates 2^6. 
# the compilation below generates the plots on pre-run configs of the models
# output will be in $REPO/plots/bpru/
echo "Compiling BPRU data..."
python3 ./plots/compile_bpru_results.py
# ================================== BPRU ==================================

# Sonng, you can use the model driver to build and run the 5 benchmarks
# ./model_driver.sh y bpru_bimodal bpru_gshare bpru_tournament