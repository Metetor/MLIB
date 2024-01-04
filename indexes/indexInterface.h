#pragma once
#include "../utils/type.hpp"
#include <vector>

/// @brief Base class IndexInterface:Inherited by concrete Index-Interface objects
/// @tparam KEY_TYPE:key type,maybe point
/// @tparam dim:data dimensions
template <class KEY_TYPE, size_t Dim>
class IndexInterface
{
    typedef unsigned int uint;

public:
    virtual size_t index_size() = 0;

    virtual void build(const std::vector<KEY_TYPE> &points) = 0;
    //virtual void range_query(const box_t<Dim> &box) = 0;
    //virtual void knn_query(const KEY_TYPE &q, uint k) = 0;
    // to be added:insert,remove,update and son on

    inline size_t get_build_time() { return build_time; }

protected:
    // unit [ms]
    size_t build_time;
    // unit [us]
    size_t point_time;
    // unit [us]
    size_t range_time;
    // unit [us]
    size_t knn_time;

    size_t point_cnt;
    size_t range_cnt;
    size_t knn_cnt;
};