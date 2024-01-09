#pragma once

#include <cstddef>
#include <chrono>
#include "../../utils/type.hpp"
#include "../../utils/common.hpp"
#include "../base_index.hpp"
#include "../indexInterface.h"
namespace bench
{
    namespace index
    {

        // a naive baseline FullScan
        template <size_t dim>
        struct FullScan : public BaseIndex
        {
            using Point = point_t<dim>;
            using Box = box_t<dim>;
            using Points = std::vector<point_t<dim>>;

            Points &_data;

            FullScan(Points &points) : _data(points)
            {
            }

            inline size_t count()
            {
                return _data.size();
            }

            // linear scan O(N)
            Points range_query(Box &box)
            {
                auto start = std::chrono::steady_clock::now();
                Points result;
                for (auto p : _data)
                {
                    if (bench::common::is_in_box(p, box))
                    {
                        result.emplace_back(p);
                    }
                }
                auto end = std::chrono::steady_clock::now();
                range_count++;
                range_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                return result;
            }

            //linear scan using a priority queue O(N log K)
                Points knn_query(Point &q, size_t k)
                {
                    using QueueElement = std::pair<Point, double>;
                    auto cmp = [&q](QueueElement &e1, QueueElement &e2)
                    { return bench::common::eu_dist_square(e1.first, q) < e2.second; };
                    std::priority_queue<QueueElement, std::vector<QueueElement>, decltype(cmp)> queue(cmp);

                    auto start = std::chrono::steady_clock::now();
                    for (auto p : _data)
                    {
                        if (queue.size() < k)
                        {
                            queue.push(std::make_pair(p, bench::common::eu_dist_square(p, q)));
                        }
                        else
                        {
                            QueueElement const &top_element = queue.top();
                            double new_dist = bench::common::eu_dist_square(p, q);
                            if (new_dist < top_element.second)
                            {
                                queue.pop();
                                queue.push(std::make_pair(p, new_dist));
                            }
                        }
                    }
                    auto end = std::chrono::steady_clock::now();
                    knn_count++;
                    knn_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

                    Points result;
                    result.reserve(k);
                    while (!queue.empty())
                    {
                        QueueElement const &top_element = queue.top();
                        result.emplace_back(top_element.first);
                        queue.pop();
                    }

                    return result;
                }
        };

    //     template <class KEY_TYPE, size_t Dim>
    //     class FSInterface : public IndexInterface<KEY_TYPE, Dim>
    //     {
    //     public:
    //         FSInterface() {}
    //         ~FSInterface() {}

    //         inline size_t count()
    //         {
    //             return this->_data.size();
    //         }

    //         void build(const std::vector<KEY_TYPE> &points)
    //         {
    //             this->_data = points;
    //         }

    //         std::vector<KEY_TYPE> range_query(const box_t<Dim> &box);
    //         std::vector<KEY_TYPE> knn_query(const KEY_TYPE &q, unsigned int k);

    //     private:
    //         std::vector<KEY_TYPE> _data;
    //     };

    //     // func impl
    //     template <class KEY_TYPE, size_t Dim>
    //     std::vector<KEY_TYPE> FSInterface<KEY_TYPE, Dim>::
    //         range_query(const box_t<Dim> &box)
    //     {
    //         auto start = std::chrono::steady_clock::now();

    //         std::vector<KEY_TYPE> result;
    //         for (auto p : this->_data)
    //         {
    //             if (bench::common::is_in_box(p, box))
    //             {
    //                 result.emplace_back(p);
    //             }
    //         }

    //         auto end = std::chrono::steady_clock::now();

    //         this->range_cnt++;
    //         this->range_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    //         return result;
    //     }

    //     template <class KEY_TYPE, size_t Dim>
    //     std::vector<KEY_TYPE> FSInterface<KEY_TYPE, Dim>::
    //         knn_query(const KEY_TYPE &q, unsigned int k)
    //     {
    //         using QueueElement = std::pair<KEY_TYPE, double>;
    //         // 定义cmp函数
    //         auto cmp = [&q](QueueElement &e1, QueueElement &e2)
    //         {
    //             return bench::common::eu_dist_square(e1.first, q) < e2.second;
    //         };

    //         std::priority_queue<QueueElement, std::vector<QueueElement>, decltype(cmp)> queue(cmp);

    //         auto start = std::chrono::steady_clock::now();

    //         for (auto p : _data)
    //         {
    //             if (queue.size() < k)
    //             {
    //                 queue.push(std::make_pair(p, bench::common::eu_dist_square(p, q)));
    //             }
    //             else
    //             {
    //                 QueueElement const &top_element = queue.top();
    //                 double new_dist = bench::common::eu_dist_square(p, q);
    //                 if (new_dist < top_element.second)
    //                 {
    //                     queue.pop();
    //                     queue.push(std::make_pair(p, new_dist));
    //                 }
    //             }
    //         }

    //         auto end = std::chrono::steady_clock::now();
    //         this->knn_cnt++;
    //         this->knn_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    //         std::vector<KEY_TYPE> result;
    //         result.reserve(k);
    //         while (!queue.empty())
    //         {
    //             QueueElement const &top_element = queue.top();
    //             result.emplace_back(top_element.first);
    //             queue.pop();
    //         }

    //         return result;
    //     }
    }
}