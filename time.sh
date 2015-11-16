#!/bin/bash

# echo "--- TIMING SERIAL - QS METHOD ---"
# ./time.serial.qs.sh
# echo "--- TIMING SERIAL - CCDF METHOD ---"
# ./time.serial.ccdf.sh
# echo "--- TIMING OMP ---"
# ./time.omp.sh
echo "--- TIMING CUDA ---"
./time.cuda.sh
