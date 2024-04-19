#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <iostream>
#include <tuple>
#include <variant>
#include <boost/multi_array.hpp>
#include <vector>
#include <chrono>
#include <random>


#include "../../utils/type.hpp"
#include "../../utils/common.hpp"
#include "../pgm/pgm_index.hpp"
#include "../pgm/pgm_index_variants.hpp"

namespace bench
{
    namespace index
    {
        // the sort dim is always the last dimension
        template <class KEY_TYPE, size_t Dim, size_t K, size_t Eps = 64, size_t SortDim = Dim - 1>
        class FloodInterface : public IndexInterface<KEY_TYPE, Dim>
        {
            using Index = pgm::PGMIndex<double, Eps>;

        public:
            class Bucket
            {
            public:
                std::vector<KEY_TYPE> _local_points;
                // eps for each bucket is fixed to 16 based on a micro benchmark
                pgm::PGMIndex<double, 16> *_local_pgm;

                Bucket() : _local_pgm(nullptr){};

                ~Bucket()
                {
                    delete this->_local_pgm;
                }

                inline void insert(KEY_TYPE &p)
                {
                    this->_local_points.emplace_back(p);
                }

                inline void build()
                {
                    if (_local_points.size() == 0)
                    {
                        return;
                    }
                    // note points are already sorted by SortDim
                    std::vector<double> idx_data;
                    idx_data.reserve(_local_points.size());
                    for (const auto &p : _local_points)
                    {
                        idx_data.emplace_back(std::get<SortDim>(p));
                    }

                    // try sort
                    sort(idx_data.begin(),idx_data.end());
                    _local_pgm = new pgm::PGMIndex<double, 16>(idx_data);
                }

                inline void search(std::vector<KEY_TYPE> &result, box_t<Dim> &box)
                {
                    if (_local_pgm == nullptr)
                    {
                        return;
                    }

                    auto min_key = std::get<SortDim>(box.min_corner());
                    auto max_key = std::get<SortDim>(box.max_corner());
                    auto range_lo = this->_local_pgm->search(min_key);
                    auto range_hi = this->_local_pgm->search(max_key);

                    for (size_t i = range_lo.lo; i < range_hi.hi; ++i)
                    {
                        if (bench::common::is_in_box(this->_local_points[i], box))
                        {
                            result.emplace_back(this->_local_points[i]);
                        }
                    }
                }
            };

            FloodInterface() {}
            ~FloodInterface() {}

            inline size_t count()
            {
                return _data.size();
            }

            inline size_t index_size()
            {
                // size of dimension-level learned index
                size_t cdf_size = 0;
                for (size_t i = 0; i < Dim - 1; ++i)
                {
                    cdf_size += this->indexes[i]->size_in_bytes();
                }

                // size of models within each bucket
                size_t b_size = 0;
                for (size_t i = 0; i < buckets.size(); ++i)
                {
                    if (buckets[i]._local_pgm != nullptr)
                    {
                        b_size += buckets[i]._local_pgm->size_in_bytes();
                    }
                }

                return cdf_size + b_size + count() * sizeof(size_t);
            }

            void build(std::vector<KEY_TYPE> &points);

            std::vector<KEY_TYPE> range_query(box_t<Dim> &box);

            std::vector<KEY_TYPE> knn_query(KEY_TYPE &q, unsigned int k)
            {
                std::cerr << "Error: knn_query is not supported for Flood Index." << std::endl;
                return {}; // 返回一个空的结果
            }
        private:
            std::vector<KEY_TYPE> _data;
            std::array<Index *, Dim - 1> indexes;
            std::array<Bucket, bench::common::ipow(K, Dim - 1)> buckets;
            std::array<size_t, Dim - 1> dim_offset;

            std::array<double, Dim - 1> mins;
            std::array<double, Dim - 1> maxs;

            size_t bucket_size;

            inline void find_intersect_ranges(std::vector<std::pair<size_t, size_t>> &ranges, box_t<Dim> &qbox)
            {
                if (Dim == 2)
                {
                    ranges.emplace_back(get_dim_idx(qbox.min_corner(), 0), get_dim_idx(qbox.max_corner(), 0));
                }
                else
                {
                    // search range on the 1-st dimension
                    ranges.emplace_back(get_dim_idx(qbox.min_corner(), 0), get_dim_idx(qbox.max_corner(), 0));

                    // find all intersect ranges
                    for (size_t i = 1; i < Dim - 1; ++i)
                    {
                        auto start_idx = get_dim_idx(qbox.min_corner(), i);
                        auto end_idx = get_dim_idx(qbox.max_corner(), i);

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
                }
            }

            // locate the bucket on d-th dimension using binary search
            inline size_t get_dim_idx(KEY_TYPE &p, size_t d)
            {
                if (p[d] <= this->mins[d])
                {
                    return 0;
                }
                if (p[d] >= this->maxs[d])
                {
                    return K - 1;
                }
                auto approx_pos = this->indexes[d]->search(p[d]).pos / this->bucket_size;
                return std::min(approx_pos, K - 1);
            }

            inline size_t compute_id(KEY_TYPE &p)
            {
                size_t id = 0;

                for (size_t i = 0; i < Dim - 1; ++i)
                {
                    auto current_idx = get_dim_idx(p, i);
                    id += current_idx * dim_offset[i];
                }

                return id;
            }
        };
        // implenmentation
        template <class KEY_TYPE, size_t Dim, size_t K, size_t Eps, size_t SortDim>
        void FloodInterface<KEY_TYPE, Dim, K, Eps, SortDim>::
            build(std::vector<KEY_TYPE> &points)
        {
            this->_data = points;
            this->bucket_size = (points.size() + K - 1) / K;

            std::cout << "Construct Flood "
                      << "K=" << K << " Epsilon=" << Eps << " SortDim=" << SortDim << std::endl;

            auto start = std::chrono::steady_clock::now();

            // dimension offsets when computing bucket ID
            for (size_t i = 0; i < Dim - 1; ++i)
            {
                this->dim_offset[i] = bench::common::ipow(K, i);
            }

            // sort points by SortDim
            std::sort(_data.begin(), _data.end(), [](auto &p1, auto &p2)
                      { return std::get<SortDim>(p1) < std::get<SortDim>(p2); });

            // boundaries of each dimension
            std::fill(mins.begin(), mins.end(), std::numeric_limits<double>::max());
            std::fill(maxs.begin(), maxs.end(), std::numeric_limits<double>::min());

            // train model on dimension 1 -- Dim-1
            std::vector<double> idx_data;
            idx_data.reserve(points.size());
            for (size_t i = 0; i < Dim - 1; ++i)
            {
                for (const auto &p : _data)
                {
                    mins[i] = std::min(p[i], mins[i]);
                    maxs[i] = std::max(p[i], maxs[i]);

                    idx_data.emplace_back(p[i]);
                }

                std::sort(idx_data.begin(), idx_data.end());

                this->indexes[i] = new Index(idx_data);
                std::cout<<"complete Dim:"<<i<<std::endl;
                idx_data.clear();
            }

            // note data are sorted by SortDim
            for (auto &p : _data)
            {
                buckets[compute_id(p)].insert(p);
            }

            for (auto &b : buckets)
            {
                b.build();
            }

            auto end = std::chrono::steady_clock::now();
            this->build_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "Build Time: " << this->get_build_time() << " [ms]" << std::endl;
            std::cout << "Index Size: " << this->index_size() << " Bytes" << std::endl;
        }

        template <class KEY_TYPE, size_t Dim, size_t LeafNodeCap, size_t MaxElements, size_t sort_dim>
        std::vector<KEY_TYPE> FloodInterface<KEY_TYPE, Dim, LeafNodeCap, MaxElements, sort_dim>::
            range_query(box_t<Dim> &box)
        {
            auto start = std::chrono::steady_clock::now();

            // find all intersected cells
            std::vector<std::pair<size_t, size_t>> ranges;
            find_intersect_ranges(ranges, box);

            // search each cell using local models
            std::vector<KEY_TYPE> result;
            for (auto &range : ranges)
            {
                for (auto idx = range.first; idx <= range.second; ++idx)
                {
                    this->buckets[idx].search(result, box);
                }
            }

            auto end = std::chrono::steady_clock::now();
            this->range_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            this->range_cnt++;

            return result;
        }
    }
}
