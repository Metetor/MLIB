#pragma once

#include "../../utils/type.hpp"
#include "../../utils/common.hpp"
#include "../base_index.hpp"

#include <algorithm>
#include <array>
#include <boost/geometry/geometries/box.hpp>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <limits>
#include <ostream>
#include <streambuf>
#include <tuple>
#include <utility>
#include <vector>
#include <iostream>
#include <chrono>

namespace bench
{
    namespace index
    {

        // uniform K by K ... grid
        template <class KEY_TYPE, size_t Dim, size_t K>
        class UGInterface : public IndexInterface<KEY_TYPE, Dim>
        {
        public:
            UGInterface() {}
            ~UGInterface() {}

            inline size_t count() { return this->point_num; }
            inline size_t index_size()
            {
                return Dim * (3 * sizeof(double) + sizeof(size_t)) + this->buckets.size() * sizeof(std::vector<KEY_TYPE>);
            }

            void build(const std::vector<KEY_TYPE> &points);

            std::vector<KEY_TYPE> range_query(const box_t<Dim> &box);

            std::vector<KEY_TYPE> knn_query(const KEY_TYPE &q, unsigned int k)
            {
                std::cerr << "Error: knn_query is not supported for Uniform_Grid Index." << std::endl;
                return {}; // 返回一个空的结果
            }

        private:
            size_t point_num;
            std::array<std::vector<KEY_TYPE>, common::ipow(K, Dim)> buckets;
            std::array<double, Dim> mins;
            std::array<double, Dim> maxs;
            std::array<double, Dim> widths;
            std::array<size_t, Dim> dim_offset;

            // compute the index on d-th dimension of a given point
            inline size_t get_dim_idx(const KEY_TYPE &p, const size_t &d)
            {
                if (p[d] <= mins[d])
                {
                    return 0;
                }
                else if (p[d] >= maxs[d])
                {
                    return K - 1;
                }
                else
                {
                    return (size_t)((p[d] - mins[d]) / widths[d]);
                }
            }

            // compute the bucket ID of a given point
            inline size_t compute_id(KEY_TYPE &p)
            {
                size_t id = 0;

                for (size_t i = 0; i < Dim; ++i)
                {
                    auto current_idx = get_dim_idx(p, i);
                    id += current_idx * dim_offset[i];
                }

                return id;
            }
        };

        // func impl
        template <class KEY_TYPE, size_t Dim, size_t K>
        void UGInterface<KEY_TYPE, Dim, K>::build(const std::vector<KEY_TYPE> &points)
        {
            std::cout << "Construct Uniform Grid K=" << K << std::endl;
            auto start = std::chrono::steady_clock::now();

            this->point_num = points.size();
            // dimension offsets when computing bucket ID
            for (size_t i = 0; i < Dim; ++i)
            {
                this->dim_offset[i] = bench::common::ipow(K, i);
            }
            // insert points to buckets
            for (auto p : points)
            {
                buckets[compute_id(p)].emplace_back(p);
            }

            auto end = std::chrono::steady_clock::now();
            this->build_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "Build Time: " << this->get_build_time() << " [ms]" << std::endl;
            std::cout << "Index Size: " << this->index_size() << " Bytes" << std::endl;
        }

        template <class KEY_TYPE, size_t Dim, size_t K>
        std::vector<KEY_TYPE> UGInterface<KEY_TYPE, Dim, K>::
            range_query(const box_t<Dim> &box)
        {
            auto start = std::chrono::steady_clock::now();

            // bucket ranges that intersect the query box
            std::vector<std::pair<size_t, size_t>> ranges;

            // search range on the 1-st dimension
            ranges.emplace_back(std::make_pair(get_dim_idx(box.min_corner(), 0), get_dim_idx(box.max_corner(), 0)));

            // find all intersect ranges
            for (size_t i = 1; i < Dim; ++i)
            {
                auto start_idx = get_dim_idx(box.min_corner(), i);
                auto end_idx = get_dim_idx(box.max_corner(), i);

                std::vector<std::pair<size_t, size_t>> temp_ranges;
                for (auto idx = start_idx; idx <= end_idx; ++idx)
                {
                    for (size_t j = 0; j < ranges.size(); ++j)
                    {
                        temp_ranges.emplace_back(std::make_pair(ranges[j].first + idx * dim_offset[i], ranges[j].second + idx * dim_offset[i]));
                    }
                }

                // update the range vector
                ranges = temp_ranges;
            }

            // Points candidates;
            std::vector<KEY_TYPE> result;

            // find candidate points
            for (auto range : ranges)
            {
                auto start_idx = range.first;
                auto end_idx = range.second;

                for (auto idx = start_idx; idx <= end_idx; ++idx)
                {
                    for (auto cand : this->buckets[idx])
                    {
                        if (bench::common::is_in_box(cand, box))
                        {
                            result.emplace_back(cand);
                        }
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
