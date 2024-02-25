#include "../utils/datautils.hpp"
#include "../utils/common.hpp"

#include "bench.h"

#include <cstddef>
#include <string>
#include <stdlib.h>

#ifndef BENCH_DIM
#define BENCH_DIM 2
#endif

#ifndef PARTITION_NUM
#define PARTITION_NUM 10
#endif

#ifndef INDEX_ERROR_THRESHOLD
#define INDEX_ERROR_THRESHOLD 64
#endif

using Point = point_t<BENCH_DIM>;
using Points = std::vector<Point>;
using Box = box_t<BENCH_DIM>;

int main(int argc, char **argv)
{
    std::cout<<"enter main\n";
    IndexManager<Point, BENCH_DIM> idx_m;

    idx_m.parse_args(argc, argv);
    std::cout<<"parse_args end\n";
    idx_m.run();

    return 0;
}
