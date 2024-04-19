#!/bin/bash

DATA_PATH="../data/"
BENCH2D_DEFAULT="../build/bin/bench2d_default"

SYN_DATA_PATH="${DATA_PATH}synthetic/"
DEFAULT_SYN_DATA_PATH="${DATA_PATH}synthetic/Default/"

RESULT_PATH="../results/default/"

#test
for data in "gaussian_20m_2_1" #"lognormal_20m_2_1"
do
  for index in "ug"
    do
        echo "Benchmark test default (point query) ${index} dataset ${data}"
        ${BENCH2D_DEFAULT} ${index} "${DEFAULT_SYN_DATA_PATH}${data}" 20000000 point >> "${RESULT_PATH}_${data}"
    done
done