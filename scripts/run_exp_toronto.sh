#!/bin/bash

DATA_PATH="../data/"
BENCH3D="../build/bin/bench3d_toronto"


REAL_DATA_PATH="${DATA_PATH}real/"

RESULT_PATH="../results/default/"

# mkdir ${RESULT_PATH}

data="t3s"
# # debug
# for index in "ifi"
# do
#     echo "Benchmark ${index} dataset ${data}"
#     ${BENCH3D} ${index} "${REAL_DATA_PATH}$data" 0 range > "${RESULT_PATH}${index}_${data}"
# done
# data="toronto"
for index in "rtree" "rstar" "zm" "mli" "lisa"
do
    echo "Benchmark ${index} dataset ${data}"
    ${BENCH3D} ${index} "${REAL_DATA_PATH}$data" 0 all > "${RESULT_PATH}${index}_${data}"
done

for index in "kdtree" "ann"
do
    echo "Benchmark ${index} dataset ${data}"
    ${BENCH3D} ${index} "${REAL_DATA_PATH}$data" 0 knn > "${RESULT_PATH}${index}_${data}"
done

for index in "ug" "edg" "fs" "ifi" "flood"
do
    echo "Benchmark ${index} dataset ${data}"
    ${BENCH3D} ${index} "${REAL_DATA_PATH}$data" 0 range > "${RESULT_PATH}${index}_${data}"
done
