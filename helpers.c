#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// read binary file of points

float* readBinaryFile(
    char* binFile,
    long* buffBytes,
    long* buffSize)
{
    // read binary file into buffer
    FILE* bfile = fopen(binFile, "rb");
    if (bfile == NULL) {
        printf("could not read binary file %s.", binFile);
        exit (2);
    }
    fseek(bfile, 0L, SEEK_END);
    *buffBytes = (long) ftell(bfile);
    *buffSize = (long) (*buffBytes / sizeof(float));
    float* buff = (float*) malloc(*buffBytes);
    fseek(bfile, 0L, SEEK_SET);
    size_t newBuffLen = fread(buff, sizeof(float), *buffSize, bfile);
    if (newBuffLen == 0) {
        printf("Error loading binary file.");
        exit (2);
    }
    fclose(bfile);
    return buff;
}

// write array to csv file

int writeArrayToCSV(
    char* outName,
    int* out,
    int gridLen,
    float histLen)
{
    // FILE *f = fopen(outName, "w");
    // if (f == NULL) return -1;

    // print column headers
    for (int x = 0; x < gridLen-1; x++) {
        //fprintf(f, "%.9g,", histLen * x);
        printf("%.9g,", histLen * x);
    }
    // fprintf(f, "%.9g\n", histLen * (gridLen-1));
    printf("%.9g\n", histLen * (gridLen-1));
    // print the columns
    for (int y = 0; y < gridLen; y++)
    {
        // fprintf(f, "%.9g,", histLen * y);
        printf("%.9g,", histLen * y);
        for (int x = 0; x < gridLen-1; x++)
        {
            // fprintf(f, "%d,", out[y * gridLen + x]);
            printf("%d,", out[y * gridLen + x]);
        }
        // fprintf(f, "%d\n", out[y * gridLen + gridLen-1]);
        printf("%d\n", out[y * gridLen + gridLen-1]);
    }
    // fclose(f);
    return 1;
}
