#!/bin/bash
#!/bin/bash

DATA_PATH="../data/"
BENCH4D="../build/bin/bench4d_default"


REAL_DATA_PATH="${DATA_PATH}real/"

RESULT_PATH="../results/real/stock/"

mkdir -p ${RESULT_PATH}

data="stock"
# for index in "rtree"
# do
#     echo "Benchmark ${index} dataset ${data}"
#     ${BENCH4D} ${index} "${REAL_DATA_PATH}$data" 0 all > "${RESULT_PATH}${index}_${data}"
# done
for index in "rtree" "rstar" "zm" "mli" "lisa"
do
    echo "Benchmark ${index} dataset ${data}"
    ${BENCH4D} ${index} "${REAL_DATA_PATH}$data" 0 all > "${RESULT_PATH}${index}_${data}"
done

for index in "kdtree" "ann"
do
    echo "Benchmark ${index} dataset ${data}"
    ${BENCH4D} ${index} "${REAL_DATA_PATH}$data" 0 knn > "${RESULT_PATH}${index}_${data}"
done

for index in "ug" "edg" "fs" "ifi" "flood"
do
    echo "Benchmark ${index} dataset ${data}"
    ${BENCH4D} ${index} "${REAL_DATA_PATH}$data" 0 range > "${RESULT_PATH}${index}_${data}"
done

