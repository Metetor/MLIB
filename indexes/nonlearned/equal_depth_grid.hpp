#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "../indexInterface.h"
#include "../../utils/type.hpp"
#include "../../utils/common.hpp"

namespace bench
{
    namespace index
    {
        template <class KEY_TYPE, size_t Dim, size_t K>
        class EDGInterface : public IndexInterface<KEY_TYPE, Dim>
        {
            using Partition = std::array<double, K>;
            using Partitions = std::array<Partition, Dim>;

        public:
            EDGInterface() {}
            ~EDGInterface() {}

            inline size_t count()
            {
                return this->N;
            }
            inline size_t index_size()
            {
                return Dim * K * sizeof(double) + Dim * sizeof(size_t) + buckets.size() * sizeof(std::vector<KEY_TYPE>);
            }

            void build(std::vector<KEY_TYPE> &points);
            std::vector<KEY_TYPE> range_query(box_t<Dim> &box);
            std::vector<KEY_TYPE> knn_query(KEY_TYPE &q, unsigned int k)
            {
                std::cerr << "Error: knn_query is not supported for Equal_Depth_Grid Index." << std::endl;
                return {}; // 返回一个空的结果
            }

            void print_partitions()
            {
                for (auto &partition : partitions)
                {
                    for (auto p : partition)
                    {
                        std::cout << p << " ";
                    }
                    std::cout << std::endl;
                }
            }

        private:
            size_t N;
            std::array<std::vector<KEY_TYPE>, bench::common::ipow(K, Dim)> buckets;
            std::array<size_t, Dim> dim_offset;
            Partitions partitions; // bucket boundaries on each dimension

            // locate the bucket on d-th dimension using binary search
            inline size_t get_dim_idx(KEY_TYPE &p, size_t d)
            {
                if (p[d] <= partitions[d][0])
                {
                    return 0;
                }
                else
                {
                    auto upper = std::upper_bound(partitions[d].begin(), partitions[d].end(), p[d]);
                    return (size_t)(upper - partitions[d].begin() - 1);
                }
            }

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

        template <class KEY_TYPE, size_t Dim, size_t K>
        void EDGInterface<KEY_TYPE, Dim, K>::build(std::vector<KEY_TYPE> &points)
        {
            std::cout << "Construct Euqal-Depth Grid K=" << K << std::endl;
            auto start = std::chrono::steady_clock::now();

            // dimension offsets when computing bucket ID
            for (size_t i = 0; i < Dim; ++i)
            {
                this->dim_offset[i] = bench::common::ipow(K, i);
            }

            this->N = points.size();
            auto bucket_size = N / K;

            // compute equal depth partition boundaries
            for (size_t i = 0; i < Dim; ++i)
            {
                std::vector<double> dim_vector;
                dim_vector.reserve(N);

                for (const auto &p : points)
                {
                    dim_vector.emplace_back(p[i]);
                }
                std::sort(dim_vector.begin(), dim_vector.end());

                for (size_t j = 0; j < K; ++j)
                {
                    partitions[i][j] = dim_vector[j * bucket_size];
                }
            }

            for (auto &p : points)
            {
                buckets[compute_id(p)].emplace_back(p);
            }

            auto end = std::chrono::steady_clock::now();
            this->build_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "Build Time: " << this->get_build_time() << " [ms]" << std::endl;
            std::cout << "Index Size: " << this->index_size() << " Bytes" << std::endl;
        }

        template <class KEY_TYPE, size_t Dim, size_t K>
        std::vector<KEY_TYPE> EDGInterface<KEY_TYPE, Dim, K>::
            range_query(box_t<Dim> &box)
        {
            auto start = std::chrono::steady_clock::now();
            // bucket ranges that intersect the query box
            std::vector<std::pair<size_t, size_t>> ranges;

            // search range on the 1-st dimension
            ranges.emplace_back(get_dim_idx(box.min_corner(), 0), get_dim_idx(box.max_corner(), 0));

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
                        temp_ranges.emplace_back(ranges[j].first + idx * dim_offset[i], ranges[j].second + idx * dim_offset[i]);
                    }
                }

                // update the range vector
                ranges = temp_ranges;
            }

            // Points candidates;
            std::vector<KEY_TYPE> result;

            // find candidate points
            for (auto &range : ranges)
            {
                auto start_idx = range.first;
                auto end_idx = range.second;

                for (auto idx = start_idx; idx <= end_idx; ++idx)
                {
                    for (auto &cand : this->buckets[idx])
                    {
                        if (bench::common::is_in_box(cand, box))
                        {
                            result.emplace_back(cand);
                        }
                    }
                }
            }

            auto end = std::chrono::steady_clock::now();
            this->range_cnt++;
            this->range_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            return result;
        }
    }
}
