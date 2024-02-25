#pragma once

#include <cstddef>
#include <vector>
#include <utility>
#include <algorithm>
#include <iostream>
#include <chrono>

#include <boost/geometry/index/rtree.hpp>
#include <boost/math/statistics/linear_regression.hpp>

#include "../base_index.hpp"
#include "../indexInterface.h"
#include "../../utils/type.hpp"

namespace bgi = boost::geometry::index;
using boost::math::statistics::simple_ordinary_least_squares;

namespace bench
{
    namespace index
    {

        // implementation of the IF-IndexInterface by augumenting boost rtree
        // the original paper uses linear interpolation whose error is generally large
        // instead, we train a simple linear regression model as a trade-off

        template <class KEY_TYPE, size_t Dim, size_t LeafNodeCap = 2000, size_t MaxElements = 32, size_t sort_dim = 0>
        class IFIndexInterface : public IndexInterface<KEY_TYPE, Dim>
        {

        public:
            // class of Leaf Node
            class LeafNode
            {
            public:
                std::vector<size_t> _ids;
                std::vector<KEY_TYPE> _local_points;
                size_t count;
                // the maximum prediction error
                size_t max_err;
                // predicted pos = slope * point[sort_dim] + intercept
                double slope;
                double intercept;

                LeafNode(std::vector<size_t> ids, std::vector<KEY_TYPE> &points) : _ids(ids), count(ids.size())
                {
                    std::vector<std::pair<size_t, double>> id_and_vals;
                    std::vector<double> vals, ys;

                    _local_points.reserve(count);

                    id_and_vals.reserve(count);
                    vals.resize(this->count);
                    ys.resize(this->count);

                    for (auto id : _ids)
                    {
                        id_and_vals.emplace_back(std::make_pair(id, points[id][sort_dim]));
                    }

                    // sort the array by sort_dim
                    std::sort(id_and_vals.begin(), id_and_vals.end(),
                              [](auto p1, auto p2)
                              { return std::get<1>(p1) < std::get<1>(p2); });

                    // un-tie pairs
                    for (size_t i = 0; i < count; ++i)
                    {
                        _ids[i] = std::get<0>(id_and_vals[i]);
                        vals[i] = std::get<1>(id_and_vals[i]);
                        ys[i] = static_cast<double>(i);
                    }

                    // update local points in the bucket
                    for (auto id : _ids)
                    {
                        _local_points.emplace_back(points[id]);
                    }

                    // train a linear regression model using ordinary least square
                    auto [a, b] = simple_ordinary_least_squares(vals, ys);
                    intercept = a;
                    slope = b;

                    // compute the max error
                    max_err = 0;
                    for (size_t i = 0; i < count; ++i)
                    {
                        auto pred = predict(vals[i]);
                        auto err = (pred >= i) ? (pred - i) : (i - pred);
                        max_err = (err > max_err) ? err : max_err;
                    }
                }

                void print_model()
                {
                    std::cout << "f(x) = " << this->intercept << " + " << this->slope << " * x Max Error: " << this->max_err << std::endl;
                }

                inline size_t predict(double val)
                {
                    double guess = slope * val + intercept;
                    if (guess < 0)
                    {
                        return 0;
                    }
                    if (guess > (count - 1))
                    {
                        return count - 1;
                    }
                    return static_cast<size_t>(guess);
                }
            };

            using pack_rtree_t = bgi::rtree<std::pair<KEY_TYPE, size_t>, bgi::linear<LeafNodeCap>>;
            using index_rtree_t = bgi::rtree<std::pair<box_t<Dim>, LeafNode>, bgi::linear<MaxElements>>;

            IFIndexInterface():index(nullptr){}
            ~IFIndexInterface() { delete index; }

            inline size_t count()
            {
                return this->_points.size();
            }

            // the index size is the sum of rtree and all identifiers
            inline size_t index_size()
            {
                auto index_size = bench::common::get_boost_rtree_statistics(*(this->index));
                return index_size + count() * sizeof(size_t);
            }

            void build(std::vector<KEY_TYPE> &points);

            std::vector<KEY_TYPE> range_query(box_t<Dim> &box);

            std::vector<KEY_TYPE> knn_query(KEY_TYPE &q, unsigned int k)
            {
                std::cerr << "Error: knn_query is not supported for IFindex Index." << std::endl;
                return {}; // 返回一个空的结果
            }

        private:
            std::vector<KEY_TYPE> _points;
            index_rtree_t *index;

            inline box_t<Dim> compute_mbr(std::vector<size_t> &ids, std::vector<KEY_TYPE> &points)
            {
                KEY_TYPE mins, maxs;
                std::fill(mins.begin(), mins.end(), std::numeric_limits<double>::max());
                std::fill(maxs.begin(), maxs.end(), std::numeric_limits<double>::min());

                for (size_t i = 0; i < Dim; ++i)
                {
                    for (auto id : ids)
                    {
                        mins[i] = std::min(points[id][i], mins[i]);
                        maxs[i] = std::max(points[id][i], maxs[i]);
                    }
                }

                return box_t<Dim>(mins, maxs);
            }

            inline size_t predict(const LeafNode &leaf, double val)
            {
                double guess = leaf.slope * val + leaf.intercept;
                if (guess < 0)
                {
                    return 0;
                }
                if (guess > (leaf.count - 1))
                {
                    return leaf.count - 1;
                }
                return static_cast<size_t>(guess);
            }

            inline std::pair<size_t, size_t> search_leaf(const LeafNode &leaf, box_t<Dim> &box)
            {
                size_t pred_min = predict(leaf, box.min_corner()[sort_dim]);
                size_t pred_max = predict(leaf, box.max_corner()[sort_dim]);
                size_t lo = (leaf.max_err > pred_min) ? 0 : (pred_min - leaf.max_err);
                size_t hi = ((pred_max + leaf.max_err + 1) > leaf.count) ? (leaf.count - 1) : (pred_max + leaf.max_err);
                return std::make_pair(lo, hi);
            }
        };
        // implenmentation
        template <class KEY_TYPE, size_t Dim, size_t LeafNodeCap, size_t MaxElements, size_t sort_dim>
        void IFIndexInterface<KEY_TYPE, Dim, LeafNodeCap, MaxElements, sort_dim>::
            build(std::vector<KEY_TYPE> &points)
        {
            std::cout << "Construct IFIndex (on Rtree) "
                      << "LeafNodeCap=" << LeafNodeCap
                      << " MaxElements=" << MaxElements << " sort_dim=" << sort_dim << std::endl;
            _points = points;
            auto start = std::chrono::steady_clock::now();

            std::vector<std::pair<KEY_TYPE, size_t>> point_with_id;
            point_with_id.reserve(points.size());
            size_t cnt = 0;
            for (auto &p : points)
            {
                point_with_id.emplace_back(p, (cnt++));
            }

            // run STR algorithm to bulk-load points
            pack_rtree_t temp_rt(point_with_id.begin(), point_with_id.end());

            // group leaf nodes
            std::vector<std::pair<box_t<Dim>, LeafNode>> idx_data;
            idx_data.reserve((points.size() / LeafNodeCap) + 1);
            cnt = 0;
            std::vector<size_t> temp_ids;
            temp_ids.reserve(LeafNodeCap);
            for (auto it = temp_rt.begin(); it != temp_rt.end(); ++it)
            {
                temp_ids.emplace_back(std::get<1>(*it));
                if ((++cnt) % LeafNodeCap == 0)
                {
                    idx_data.emplace_back(compute_mbr(temp_ids, points), LeafNode(temp_ids, points));
                    temp_ids.clear();
                }
            }

            if (temp_ids.size() != 0)
            {
                idx_data.emplace_back(compute_mbr(temp_ids, points), LeafNode(temp_ids, points));
                temp_ids.clear();
            }

            // build index rtree
            index = new index_rtree_t(idx_data.begin(), idx_data.end());

            auto end = std::chrono::steady_clock::now();
            this->build_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "Build Time: " << this->get_build_time() << " [ms]" << std::endl;
            std::cout << "Index Size: " << this->index_size() << " Bytes" << std::endl;
        }

        template <class KEY_TYPE, size_t Dim, size_t LeafNodeCap, size_t MaxElements, size_t sort_dim>
        std::vector<KEY_TYPE> IFIndexInterface<KEY_TYPE, Dim, LeafNodeCap, MaxElements, sort_dim>::
            range_query(box_t<Dim> &box)
        {
            auto start = std::chrono::steady_clock::now();

            std::vector<KEY_TYPE> result;
            // for leaf nodes covered by the query box
            // directly insert points to the result set
            for (auto it = index->qbegin(bgi::covered_by(box)); it != index->qend(); ++it)
            {
                for (auto p : std::get<1>(*it)._local_points)
                {
                    result.emplace_back(p);
                }
            }

            // for leaf nodes overlaps the query box
            // check whether the points are in the query range
            for (auto it = index->qbegin(bgi::overlaps(box)); it != index->qend(); ++it)
            {
                const LeafNode &leaf = std::get<1>(*it);
                auto [lo, hi] = search_leaf(leaf, box);
                for (auto i = lo; i <= hi; ++i)
                {
                    KEY_TYPE temp_p = leaf._local_points[i];
                    if (bench::common::is_in_box(temp_p, box))
                    {
                        result.emplace_back(temp_p);
                    }
                }
            }

            auto end = std::chrono::steady_clock::now();

            this->range_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            this->range_cnt++;

            return result;
        }
    }
}
