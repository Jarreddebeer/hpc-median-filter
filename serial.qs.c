#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "helpers.h"
#include "binning/binning.serial.h"
#include "median-filter/qs.h"

// Arguments:
//     <Integer> grid size,
//     <Integer> filter size,
//     <String> binary file name

int main(int argc, char **argv) {

    if (argc != 4) {
        printf("Incorrect number of arguments: %d\n", argc);
        return -1;
    }

    char* binFile;
    float histLen;
    long buffSize; long buffBytes;
    int* in; int* out; int* window;
    int gridLen; int gridSize; int gridBytes; int filtLen; int filtBytes;

    sscanf(argv[1], "%d", &gridLen);
    sscanf(argv[2], "%d", &filtLen);
    binFile = argv[3];

    // window size must be odd.
    if (filtLen % 2 == 0) {
        filtLen++;
        printf("filter size must be odd, increasing to: %d", filtLen);
    }

    // initialise the grid
    histLen = 1.0 / (gridLen+1);
    gridSize = gridLen * gridLen;
    gridBytes = gridSize * sizeof(int);
    in  = (int*) malloc(gridBytes); memset(in,  0, gridBytes);
    out = (int*) malloc(gridBytes); memset(out, 0, gridBytes);

    filtBytes = filtLen * filtLen * sizeof(int);
    window = (int*) malloc(filtBytes);

    // read input file
    clock_t t_rea_s = clock();
    float* buff = readBinaryFile(binFile, &buffBytes, &buffSize);
    clock_t t_rea_e = clock();
    printf("Elapsed reading time: %12.3f\n", (double)(t_rea_e - t_rea_s)/CLOCKS_PER_SEC);

    // perform binning
    clock_t t_bin_s = clock();
    populateHistogram(in, buff, buffSize, histLen, gridLen, gridBytes);
    clock_t t_bin_e = clock();
    printf("Elapsed binning time: %12.3f\n", (double)(t_bin_e - t_bin_s)/CLOCKS_PER_SEC);

    // perform filtering
    clock_t t_fil_s = clock();
    median_filter(in, out, window, gridLen, filtLen);
    clock_t t_fil_e = clock();
    printf("Elapsed filtering time: %12.3f\n", (double)(t_fil_e - t_fil_s)/CLOCKS_PER_SEC);

    // writeArrayToCSV("output/output-serial-qs.csv", out, gridLen, histLen);

    free(in);
    free(out);
    free(window);
}
