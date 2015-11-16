#!/bin/bash

for s in 64 128 256 512 1024 2048 4096; do
    echo "----------"
    echo "GRID: ${s}"
    echo "----------"
    for f in 1 5 9 13 17 21; do
        echo "------------"
        echo "FILTER: ${f}"
        echo "------------"
        for i in 1; do
            $1 ${s} ${f} $2
            echo ""
        done
    done
done

