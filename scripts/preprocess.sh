#!/bin/bash

#util param
BENCH_BIN="../build/bin/datagen"
iname="../data/real/nytaxi.csv"
fname="../data/real/nytaxi"

#nytaxi.csv .csv_to_bin
echo "nytaxi.csv to bin"
$BENCH_BIN -t process_csv --infname $iname --fname $fname

