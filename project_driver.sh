

# ================================== BPRU ==================================
# Do not edit this section, tested and it works!
echo "Running 3 BRPU models with 5 traces on table sizes 2^6"
./model_driver.sh y bpru_bimodal bpru_gshare bpru_tournament
./model_driver.sh y sbi_bimodal sbi_gshare sbi_tournament
./model_driver.sh y cbcp_bimodal cbcp_gshare cbcp_tournament

# compile plot data. Note, that the above command only generates 2^6. 
# the compilation below generates the plots on pre-run configs of the models
# output will be in $REPO/plots/bpru/
echo "Compiling BPRU data..."
python3 ./plots/compile_bpru_results.py

echo "Compiling SBI data..."
python3 ./plots/compile_sbi_results.py

echo "Compiling Composite Confidence Estimator (CBCP) data..."
python3 ./plots/compile_cbcp_results.py