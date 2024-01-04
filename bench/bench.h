#pragma once
#include "query.hpp"
#include "../indexes/indexInterface.h"
#include "../indexes/nonlearned/nonlearned_index.hpp"
#include "../indexes/learned/learned_index.hpp"

#include <iostream>
#include <string>
#include <vector>

#ifndef BENCH_DIM
#define BENCH_DIM 2
#endif

using Point = point_t<BENCH_DIM>;
using Points = std::vector<Point>;

template <class KEY_TYPE, size_t Dim>
IndexInterface<KEY_TYPE, Dim> *get_index(std::string index_type)
{

    IndexInterface<KEY_TYPE, Dim> *index;
    if (index_type == "rtree")
    {
        index = new bench::index::RTreeInterface<KEY_TYPE, Dim>;
    }
    else if (index_type == "rstar")
    {
        //index = new bench::index::RStarInterface<KEY_TYPE, Dim>;
    }
    else
    {
        std::cout << "Could not find a matching index called " << index_type << ".\n";
        exit(0);
    }

    return index;
}

template <class KEY_TYPE, size_t Dim>
class IndexManager
{
    typedef IndexInterface<KEY_TYPE, Dim> IndexInf_t;

private:
    // manage index
    std::string index_type;
    std::string fpath;
    size_t N;
    std::string mode; // query mode:{range,knn,all}
    Points points;

public:
    IndexManager(){};
    inline void parse_args(int argc, char **argv);
    void build_index(IndexInf_t *&idxInf);
    void handle_queries(IndexInf_t *&idxInf);
    void run();
};

template <class KEY_TYPE, size_t Dim>
inline void IndexManager<KEY_TYPE, Dim>::parse_args(int argc, char **argv)
{
    if (argc < 5) // illegal params
    {
        std::cerr << "Usage: <index_name> <data_file_name> <dataset_size> <bench_mode>" << std::endl;
        return;
    }

    index_type = argv[1];
    fpath = argv[2];
    N = std::stoi(argv[3]);
    mode = argv[4];
    // other params waiting to add

    // log output
    std::cout << "====================================" << std::endl;
    std::cout << "Load data: " << fpath << std::endl;
    return;
};

template <class KEY_TYPE, size_t Dim>
void IndexManager<KEY_TYPE, Dim>::build_index(IndexInf_t *&idxInf)
{
    // read data to points
    bench::utils::read_points(points, fpath, N);
    // get index
    idxInf = get_index<KEY_TYPE, Dim>(index_type);
    // build
    idxInf->build(points);
}

template <class KEY_TYPE, size_t Dim>
void IndexManager<KEY_TYPE, Dim>::handle_queries(IndexInf_t *&idxInf)
{
    auto range_queries = bench::query::sample_knn_queries(points);
    auto knn_queries = bench::query::sample_knn_queries(points);

    if (idxInf != nullptr)
    {
        if (mode == "range" || mode == "all")
        {
            // Handle range queries
            bench::query::batch_range_queries(idxInf, range_queries);
        }
        else if (mode == "knn" || mode == "all")
        {
            // KNN queries
            // bench::query::batch_knn_queries(*(index_ptr), knn_queries);
        }
        else
        {
            // should not occur
            std::cout << "Invalid query mode: " << mode << std::endl;
            exit(0);
        }
        return;
    }
    else
    {
        std::cout << "Index is illegal: " << index_type << std::endl;
        exit(0);
    }
}

// run
template <class KEY_TYPE, size_t Dim>
void IndexManager<KEY_TYPE, Dim>::run()
{
    IndexInf_t *idxInf;
    this->build_index(idxInf);
    // this->handle_queries(idxInf);
}