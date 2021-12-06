
# CMPT 450: Branch Confidence Prediction

This project has an implementation of BPRU, SBI, JRS and a composite combination. These models were run on 5 different benchmarks.

In order to run the project you must first download the 5 traces that were used as benchmarks using the the following command:

`$ ./scripts/download_dcp3_traces.sh`

This will create a folder in /data/dcp3_traces which will be used by champsim.

Now in the root of the repo ($REPO) run the following driver. Note: make sure to have pip3 installed with matplotlib, numpy and pandas packages.

`$ ./project_driver.sh`

This will run the models in size 2^6 and recompile the tables and plots used for the reports. Our source implementation can be found the following folders, `$REPO/branch/` and `$REPO/confidence_predictors/`

After running the project driver refer to the `$REPO/plots` folder to view output plots. the `$REPO/RESULTS` folder contains all our gathered tarces we used for the report and for the rest of the tables that werent generated from the driver.