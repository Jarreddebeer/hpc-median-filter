// quick-select sort and return the middle item

int getPivot(
    int* values,
    int start,
    int stop)
{
    int pivot = values[start];
    int i = start;
    int j = stop+1;
    while (1) {
        while (values[++i] < pivot) if (i == stop) break;
        while (values[--j] >= pivot) if (j == start) break;
        if (i >= j) break;
        int tmp = values[i];
        values[i] = values[j];
        values[j] = tmp;
    }
    // place the pivot back
    int tmp = values[j];
    values[j] = pivot;
    values[start] = tmp;
    return j;
}

// Get the median from an array of ints

int getMedian(
    int* values,
    int size)
{
    int middle = size / 2;
    int start = 0;
    int stop = size-1;
    int pivot = getPivot(values, start, stop);
    while (pivot != middle && start < stop) {
        if (pivot > middle) stop = pivot-1; // median is in left half
        else start = pivot+1;               // median is in right half
        pivot = getPivot(values, start, stop);
    }
    return values[pivot];
}
