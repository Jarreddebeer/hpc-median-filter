all: serial omp cuda

serial:
	gcc median-filter/qs.helpers.c median-filter/qs.serial.c binning/binning.serial.c helpers.c serial.qs.c -std=c99 -g -o hpc.qs.serial
	gcc median-filter/ccdf.serial.c binning/binning.serial.c helpers.c serial.ccdf.c -std=c99 -g -o hpc.ccdf.serial

omp:
	gcc median-filter/qs.helpers.c median-filter/qs.omp.c binning/binning.omp.c helpers.c omp.c -std=c99 -g -fopenmp -o hpc.omp

cuda:
	nvcc -c -m64 -arch=sm_20 -o median-filter/ccdf.cuda.o -Xcompiler -fPIC median-filter/ccdf.cuda.cu
	gcc median-filter/ccdf.cuda.o binning/binning.serial.c helpers.c -std=c99 -o hpc.cuda -g -lm -lcudart -fopenmp -L/usr/local/cuda/lib64 cuda.c

clean:
	rm hpc.omp hpc.serial
