#!/bin/bash

#util param
BENCH_BIN="../build/bin/datagen"
ny_csv_name="../data/real/nytaxi.csv"
ny_bin_name="../data/real/nytaxi"
fs_csv_name="../data/real/dataset_TIST2015/fs.csv"
fs_bin_name="../data/real/fs"
toronto_csv_name="../data/Toronto_3D/t3s.csv"
toronto_bin_name="../data/real/t3s"
stock_csv="../data/STOCK/stock.csv"
stock_bin="../data/real/stock"
#nytaxi.csv .csv_to_bin
# echo "nytaxi.csv to bin"
# $BENCH_BIN -t process_csv --infname $iname --fname $fname

#fs.csv .csv_to_bin
# echo "fs.csv to bin"
# $BENCH_BIN -t process_csv --infname $fs_csv_name --fname $fs_bin_name

#toronto.csv to bin
echo "stock to bin"
$BENCH_BIN -t process_csv --infname $stock_csv --fname $stock_bin
