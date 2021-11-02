#!/bin/bash

mkdir -p /data/dpc3_traces
wget -P /data/dpc3_traces -c https://hpca23.cse.tamu.edu/champsim-traces/speccpu/600.perlbench_s-210B.champsimtrace.xz
wget -P /data/dpc3_traces -c https://hpca23.cse.tamu.edu/champsim-traces/speccpu/602.gcc_s-734B.champsimtrace.xz
wget -P /data/dpc3_traces -c https://hpca23.cse.tamu.edu/champsim-traces/speccpu/625.x264_s-18B.champsimtrace.xz
wget -P /data/dpc3_traces -c https://hpca23.cse.tamu.edu/champsim-traces/speccpu/641.leela_s-800B.champsimtrace.xz
wget -P /data/dpc3_traces -c https://hpca23.cse.tamu.edu/champsim-traces/speccpu/648.exchange2_s-1699B.champsimtrace.xz
# TODO: We can add more later