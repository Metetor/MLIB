#!/bin/bash

DATA_PATH="../data/"
BENCH2D_DEFAULT="../build/bin/bench2d_default"
BENCH2D_FS="../build/bin/bench2d_fs"
BENCH2D_OSM="../build/bin/bench2d_osm"

REAL_DATA_PATH="${DATA_PATH}real/"
SYN_DATA_PATH="${DATA_PATH}synthetic/"
DEFAULT_SYN_DATA_PATH="${DATA_PATH}synthetic/Default/"

RESULT_PATH="../results/classical/"

mkdir -p ${RESULT_PATH}

# test classical index range query on synthetic dataset
for data in  "uniform_20m_2_1"
do
  for index in "rtree" "rstar" #"zm" "mli" "lisa" "fs"
   do
       echo "Benchmark classical ${index} dataset ${data}"
       ${BENCH2D_DEFAULT} ${index} "${DEFAULT_SYN_DATA_PATH}$data" 20000000 all >> "${RESULT_PATH}${data}_classical"
   done

#    for index in "kdtree" "ann"
#    do
#        echo "Benchmark ${index} dataset ${data}"
#        ${BENCH2D_DEFAULT} ${index} "${DEFAULT_SYN_DATA_PATH}$data" 20000000 knn >> "${RESULT_PATH}_${data}"
#    done

   for index in "qdtree" "ug" "edg"  #"ifi" "flood"
   do
       echo "Benchmark classical ${index} dataset ${data}"
      ${BENCH2D_DEFAULT} ${index} "${DEFAULT_SYN_DATA_PATH}$data" 20000000 range >> "${RESULT_PATH}${data}_classical"
   done
done