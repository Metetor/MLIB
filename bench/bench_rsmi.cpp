#include "../utils/datautils.hpp"
#include "../utils/common.hpp"
#include "query.hpp"

#include "../indexes/learned/rsmi.hpp"

#include "../indexes/nonlearned/fullscan.hpp"

#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>

#ifndef BENCH_DIM
#define BENCH_DIM 2
#endif

using Point = point_t<BENCH_DIM>;
using Box = box_t<BENCH_DIM>;
using Points = std::vector<point_t<BENCH_DIM>>;

const std::string MODEL_PATH = "../model/";

// extract filename from a path
std::string get_filename(const std::string &path)
{
    auto idx = path.find_last_of('/') + 1;
    return path.substr(idx);
}
//
int main(int argc, char **argv)
{
    typedef IndexInterface<point_t<BENCH_DIM>, BENCH_DIM> IndexInf_t;
    assert(argc >= 4);
    //parse args
    std::string index = argv[1];   // index name
    std::string fname = argv[2];   // data file name
    size_t N = std::stoi(argv[3]); // dataset size
    std::string mode = argv[4];    // bench mode

    std::cout << "====================================" << std::endl;
    std::cout << "Load data: " << fname << std::endl;
    //read points
    Points points;
    bench::utils::read_points(points, fname, N);
    //model path
    std::string pure_fname = get_filename(fname);
    std::string model_path = MODEL_PATH + pure_fname;
    model_path.push_back('/');

    std::cout<<model_path;
    if (!boost::filesystem::is_directory(model_path))
    {
        boost::filesystem::create_directory(model_path);
    }

    std::cout << "Will save/load model to/from " << model_path << std::endl;

#ifdef HEAP_PROFILE
    // directly exit if heap profiling is enabled
    bench::index::RSMIWrapper<BENCH_DIM> rsmi(points, model_path);
    return 0;
#endif

#ifndef HEAP_PROFILE
    IndexInf_t *rsmi=new bench::index::RSMIInterface<point_t<BENCH_DIM>, BENCH_DIM>(model_path);


    rsmi->build(points);
    if (mode.compare("range") == 0)
    {
        auto range_queries = bench::query::sample_range_queries(points);
        bench::query::batch_range_queries(rsmi, range_queries);
        return 0;
    }

    if (mode.compare("knn") == 0)
    {
        auto knn_queries = bench::query::sample_knn_queries(points);
        bench::query::batch_knn_queries(rsmi, knn_queries);
        return 0;
    }

    if (mode.compare("all") == 0)
    {
        auto range_queries = bench::query::sample_range_queries(points);
        auto knn_queries = bench::query::sample_knn_queries(points);

        bench::query::batch_range_queries(rsmi, range_queries);
        bench::query::batch_knn_queries(rsmi, knn_queries);
        return 0;
    }
#endif
    return 0;
}
