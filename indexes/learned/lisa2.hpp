#pragma once

#include <cstddef>
#include <array>
#include <chrono>
#include "../../utils/type.hpp"
#include "../../utils/common.hpp"
#include "../pgm/pgm_index.hpp"

namespace bench
{
    namespace index
    {
        template <class KEY_TYPE, size_t Dim, size_t K, size_t Epsilon = 64>
        class LISA2Interface : public IndexInterface<KEY_TYPE, Dim>
        {
            using Partition = std::array<double, K>;
            using Partitions = std::array<Partition, Dim>;

            using PGMIdx = pgm::PGMIndex<double, Epsilon>;

        public:
            LISA2Interface() : _pgm_ptr(nullptr) {}
            ~LISA2Interface() { delete _pgm_ptr; }

            inline size_t count()
            {
                return this->_data.size();
            }

            inline size_t index_size()
            {
                size_t partition_size = K * Dim * sizeof(double);
                size_t volume_size = this->volumes.size() * sizeof(double);
                size_t pgm_size = this->_pgm_ptr->size_in_bytes();
                return partition_size + volume_size + pgm_size + count() * sizeof(size_t);
            }

            void build(std::vector<KEY_TYPE> &points);

            std::vector<KEY_TYPE> range_query(box_t<Dim> &box);

            std::vector<KEY_TYPE> knn_query(KEY_TYPE &q, uint k);

        private:
            // dimension offsets in bucket array
            std::array<size_t, Dim> dim_offset;

            // bucket boundaries on each dimension
            Partitions partitions;

            // min corner
            std::array<double, Dim> mins;

            // max corner
            std::array<double, Dim> maxs;

            // minimum width
            double min_width;

            // pre-computed grid volumes
            std::array<double, bench::common::ipow(K, Dim)> volumes;

            std::vector<KEY_TYPE> _data;
            // ptr to the underlying 1-d learned index
            PGMIdx *_pgm_ptr;
            // find intitial search range
            inline double initial_knn_range(KEY_TYPE &q, size_t k)
            {
                double min_r = std::numeric_limits<double>::max();

                for (size_t i = 0; i < Dim; ++i)
                {
                    auto idx = get_dim_idx(q, i);
                    if (idx == K - 1)
                    {
                        min_r = std::min(min_r, (this->maxs[i] - q[i]));
                        min_r = std::min(min_r, (q[i] - this->partitions[i][idx]));
                    }
                    else
                    {
                        min_r = std::min(min_r, (this->partitions[i][idx + 1] - q[i]));
                        min_r = std::min(min_r, (q[i] - this->partitions[i][idx]));
                    }
                }

                return min_r;
            }

            void knn_range_helper(std::vector<KEY_TYPE> &result_found, KEY_TYPE &q, double r)
            {
                KEY_TYPE min_p = q;
                KEY_TYPE max_p = q;
                for (size_t i = 0; i < Dim; ++i)
                {
                    min_p[i] -= r;
                    max_p[i] += r;
                }
                box_t<Dim> qbox(min_p, max_p);

                std::vector<std::pair<size_t, size_t>> ranges;
                find_intersect_ranges(ranges, qbox);

                for (const auto &range : ranges)
                {
                    search_range(result_found, static_cast<double>(std::get<0>(range)), static_cast<double>(std::get<1>(range) + 1), qbox);
                }
            }

            // lo and hi are projected values
            inline void search_range(std::vector<KEY_TYPE> &result, double lo, double hi, box_t<Dim> &qbox)
            {
                auto range_lo = this->_pgm_ptr->search(lo);
                auto range_hi = this->_pgm_ptr->search(hi);

                for (size_t i = range_lo.lo; i < range_hi.hi; ++i)
                {
                    if (bench::common::is_in_box(this->_data[i], qbox))
                    {
                        result.emplace_back(this->_data[i]);
                    }
                }

                // auto it_lo = this->_data.begin() + range_lo.lo;
                // auto it_hi = this->_data.begin() + range_hi.hi;

                // for (auto it=it_lo; it!=it_hi; ++it) {
                //     if (bench::common::is_in_box(*it, qbox)) {
                //         result.emplace_back(*it);
                //     }
                // }
            }

            inline void find_intersect_ranges(std::vector<std::pair<size_t, size_t>> &ranges, box_t<Dim> &qbox)
            {
                // search range on the 1-st dimension
                ranges.emplace_back(get_dim_idx(qbox.min_corner(), 0), get_dim_idx(qbox.max_corner(), 0));

                // find all intersect ranges
                for (size_t i = 1; i < Dim; ++i)
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

            inline double project(KEY_TYPE &point)
            {
                size_t id = compute_id(point);
                double vol = 10.0;

                for (size_t i = 0; i < Dim; ++i)
                {
                    auto current_idx = get_dim_idx(point, i);

                    if (point[i] <= this->mins[i])
                    {
                        vol = 0.0;
                    }
                    else if (point[i] >= this->maxs[i])
                    {
                        vol *= (100 * (this->maxs[i] - partitions[i][current_idx]));
                    }
                    else
                    {
                        vol *= (100 * (point[i] - partitions[i][current_idx]));
                    }
                }

                return static_cast<double>(id) + vol / this->volumes[id];
            }

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

            inline size_t compute_id(std::array<size_t, Dim> &idx_comb)
            {
                size_t id = 0;
                for (size_t i = 0; i < Dim; ++i)
                {
                    id += idx_comb[i] * dim_offset[i];
                }
                return id;
            }
        };

        // // implementation
        template <class KEY_TYPE, size_t Dim, size_t K, size_t Epsilon>
        void LISA2Interface<KEY_TYPE, Dim, K, Epsilon>::
            build(std::vector<KEY_TYPE> &points)
        {
            std::cout << "Construct LISA "
                      << "K=" << K << " Epsilon=" << Epsilon << std::endl;

            auto start = std::chrono::steady_clock::now();
            // dimension offsets when computing bucket ID
            for (size_t i = 0; i < Dim; ++i)
            {
                this->dim_offset[i] = bench::common::ipow(K, i);
            }

            auto bucket_size = points.size() / K;

            this->min_width = std::numeric_limits<double>::max();

            // compute equal depth partition boundaries
            for (size_t i = 0; i < Dim; ++i)
            {
                std::vector<double> dim_vector;
                dim_vector.reserve(points.size());

                for (const auto &p : points)
                {
                    dim_vector.emplace_back(p[i]);
                }
                std::sort(dim_vector.begin(), dim_vector.end());

                for (size_t j = 0; j < K; ++j)
                {
                    if (j == 0)
                    {
                        partitions[i][j] = dim_vector[j * bucket_size] - 0.000001; // this is to resolve numerical issues
                    }
                    else
                    {
                        partitions[i][j] = (dim_vector[j * bucket_size] + dim_vector[j * bucket_size + 1]) / 2.0;
                    }
                }

                mins[i] = dim_vector.front();
                maxs[i] = dim_vector.back();

                this->min_width = std::min(this->min_width, (maxs[i] - mins[i]));
            }

            // initialize volumes of grid cells
            std::array<size_t, Dim> comb;
            for (size_t i = 0; i < volumes.size(); ++i)
            {
                auto I = i;
                for (size_t j = 0; j < Dim; ++j)
                {
                    comb[j] = I % K;
                    I /= K;
                }

                auto bucket_id = compute_id(comb);
                double vol = 10.0;
                for (size_t d = 0; d < Dim; ++d)
                {
                    if (comb[d] == (K - 1))
                    {
                        vol *= (100 * (this->maxs[d] - this->partitions[d][comb[d]]));
                    }
                    else
                    {
                        vol *= (100 * (this->partitions[d][comb[d] + 1] - this->partitions[d][comb[d]]));
                    }
                }
                this->volumes[bucket_id] = vol;
            }

            // make sure all volumes are well initialized
            for (auto v : this->volumes)
            {
                assert(v > 0);
            }

            // compute projections and sort points by projections
            std::vector<std::pair<size_t, double>> pid_and_projection;
            pid_and_projection.reserve(points.size());
            size_t pid = 0;
            for (auto &p : points)
            {
                pid_and_projection.emplace_back(pid++, project(p));
            }

            std::sort(pid_and_projection.begin(), pid_and_projection.end(),
                      [](auto p1, auto p2)
                      { return std::get<1>(p1) < std::get<1>(p2); });

            std::vector<double> projections;
            projections.reserve(points.size());
            this->_data.reserve(points.size());

            for (auto &pp : pid_and_projection)
            {
                this->_data.emplace_back(points[std::get<0>(pp)]);
                projections.emplace_back(std::get<1>(pp));
            }

            // make sure there is no duplicate keys,这里似乎并不能确保
            for (size_t i = 0; i < points.size() - 1; ++i)
            {
                if (projections[i] == projections[i + 1])
                {
                    projections[i + 1] = (projections[i] + projections[i + 2]) / 2.0;
                }
            }

            //debug,检查是否是有重复值
            std::vector<double>::iterator p_it;
            p_it=unique(projections.begin(),projections.end());
            if(p_it!=projections.end())
            {
                std::cout<<"存在重复值";
                projections.erase(p_it,projections.end());
            }

            // train 1-D learned index on projections
            this->_pgm_ptr = new PGMIdx(projections);

            auto end = std::chrono::steady_clock::now();
            this->build_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "Build Time: " << this->get_build_time() << " [ms]" << std::endl;
            std::cout << "Index Size: " << this->index_size() << " Bytes" << std::endl;
        }

        template <class KEY_TYPE, size_t Dim, size_t K, size_t Epsilon>
        std::vector<KEY_TYPE> LISA2Interface<KEY_TYPE, Dim, K, Epsilon>::
            range_query(box_t<Dim> &box)
        {
            auto start = std::chrono::steady_clock::now();
            std::vector<std::pair<size_t, size_t>> ranges;
            find_intersect_ranges(ranges, box);

            std::vector<KEY_TYPE> result;
            for (const auto &range : ranges)
            {
                search_range(result, static_cast<double>(std::get<0>(range)), static_cast<double>(std::get<1>(range) + 1), box);
            }

            auto end = std::chrono::steady_clock::now();
            this->range_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            this->range_cnt++;

            return result;
        }

        template <class KEY_TYPE, size_t Dim, size_t K, size_t Epsilon>
        std::vector<KEY_TYPE> LISA2Interface<KEY_TYPE, Dim, K, Epsilon>::
            knn_query(KEY_TYPE &q, uint k)
        {
            auto start = std::chrono::steady_clock::now();

            std::vector<KEY_TYPE> range_result;

            // initialize search range
            double r = initial_knn_range(q, k);
            if (r == 0.0)
            {
                r = this->min_width / (2.0 * K);
            }

            while (1)
            {
                knn_range_helper(range_result, q, r);

                // k results found
                if (range_result.size() >= k)
                {
                    break;
                }

                // increase the search range
                r *= 2;
                range_result.clear();
            }

            auto end = std::chrono::steady_clock::now();
            this->knn_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            this->knn_cnt++;

            std::sort(range_result.begin(), range_result.end(),
                      [&](auto &p1, auto p2)
                      {
                          return bench::common::eu_dist_square(p1, q) < bench::common::eu_dist_square(p2, q);
                      });

            std::vector<KEY_TYPE> knn_result(range_result.begin(), range_result.begin() + k);

            return knn_result;
        }
    }
}
