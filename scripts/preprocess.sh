#!/bin/bash

#util param
BENCH_BIN="../build/bin/datagen"
ny_csv_name="../data/real/nytaxi.csv"
ny_bin_name="../data/real/nytaxi"
fs_csv_name="../data/real/dataset_TIST2015/fs.csv"
fs_bin_name="../data/real/fs"
toronto_csv_name="../data/Toronto_3D/t3s.csv"
toronto_bin_name="../data/real/t3s"
#nytaxi.csv .csv_to_bin
# echo "nytaxi.csv to bin"
# $BENCH_BIN -t process_csv --infname $iname --fname $fname

#fs.csv .csv_to_bin
# echo "fs.csv to bin"
# $BENCH_BIN -t process_csv --infname $fs_csv_name --fname $fs_bin_name

#toronto.csv to bin
echo "nytaxi(remove duplicate key) to bin"
$BENCH_BIN -t process_csv --infname $ny_csv_name --fname $ny_bin_name
