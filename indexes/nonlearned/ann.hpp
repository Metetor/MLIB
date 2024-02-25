#pragma once

#include "../ann_1.1.2/include/ANN/ANN.h"
#include "../../utils/type.hpp"
#include "../../utils/common.hpp"
#include "../indexInterface.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <iterator>
#include <new>
#include <vector>
#include <chrono>

#ifdef HEAP_PROFILE
#include <gperftools/heap-profiler.h>
#endif

namespace bench
{
    namespace index
    {
        template <class KEY_TYPE, size_t Dim>
        class ANNKDTreeInterface : public IndexInterface<KEY_TYPE, Dim>
        {
        public:
            ANNKDTreeInterface() : index(nullptr), points_num(0) {}

            ~ANNKDTreeInterface()
            {
                delete index;
                annClose();
            }

            inline size_t count() { return this->points_num; }

            void build(std::vector<KEY_TYPE> &points);
            /// @brief ANNKDTree dosen't support range_query
            /// @param box
            /// @return
            std::vector<KEY_TYPE> range_query(box_t<Dim> &box)
            {
                std::cerr << "Error: range_query is not supported for ANNKDTree." << std::endl;
                return {}; // 返回一个空的结果
            }

            std::vector<KEY_TYPE> knn_query(KEY_TYPE &q, unsigned int k)
            {
                return this->knn_query(q, k, 0.0);
            }

            std::vector<KEY_TYPE> knn_query(KEY_TYPE &q,
                                            unsigned int k, double eps);

        private:
            size_t points_num;
            ANNkd_tree *index; // internal kd-tree index
        };

        // func impl
        template <class KEY_TYPE, size_t Dim>
        void ANNKDTreeInterface<KEY_TYPE, Dim>::build(std::vector<KEY_TYPE> &points)
        {
            std::cout << "Construct ANNKDTree..." << std::endl;
            this->points_num = points.size();

            auto akdtree_pts = annAllocPts(points.size(), Dim);

            for (size_t i = 0; i < points.size(); ++i)
            {
                for (size_t j = 0; j < Dim; ++j)
                {
                    akdtree_pts[i][j] = points[i][j];
                }
            }

            auto start = std::chrono::steady_clock::now();

#ifdef HEAP_PROFILE
            HeapProfilerStart("annkdtree");
#endif

            index = new ANNkd_tree(akdtree_pts, this->points_num, Dim);

#ifdef HEAP_PROFILE
            HeapProfilerDump("final");
            HeapProfilerStop();
#endif

            auto end = std::chrono::steady_clock::now();
            this->build_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            std::cout << "Build Time: " << this->get_build_time() << " [ms]" << std::endl;
        }

        template <class KEY_TYPE, size_t Dim>
        std::vector<KEY_TYPE> ANNKDTreeInterface<KEY_TYPE, Dim>::
            knn_query(KEY_TYPE &q, unsigned int k, double eps)
        {
            std::vector<ANNidx> nn_idx;
            std::vector<ANNdist> nn_dist;
            nn_idx.resize(k);
            nn_dist.resize(k);

            auto start = std::chrono::steady_clock::now();

            index->annkSearch(q.data(), k, &nn_idx[0], &nn_dist[0], eps);
            auto end = std::chrono::steady_clock::now();

            this->knn_cnt++;
            this->knn_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            std::vector<KEY_TYPE> result;
            result.reserve(k);

            for (auto idx : nn_idx)
            {
                KEY_TYPE p;
                auto temp_pt = index->thePoints()[idx];

                for (size_t i = 0; i < Dim; ++i)
                {
                    p[i] = temp_pt[i];
                }

                result.emplace_back(p);
            }

            return result;
        }
        // template <size_t Dim, size_t BucketSize = 1>
        // class ANNBoxDecompositionTree
        // {

        //     using Point = point_t<Dim>;
        //     using Box = box_t<Dim>;
        //     using Points = std::vector<point_t<Dim>>;

        // public:
        //     inline ANNBoxDecompositionTree(Points &points)
        //     {
        //         std::cout << "Construct ANNBDTree..." << std::endl;
        //         this->num_of_points = points.size();

        //         auto bdtree_pts = annAllocPts(points.size(), Dim);

        //         for (size_t i = 0; i < points.size(); ++i)
        //         {
        //             for (size_t j = 0; j < Dim; ++j)
        //             {
        //                 bdtree_pts[i][j] = points[i][j];
        //             }
        //         }

        //         auto start = std::chrono::steady_clock::now();
        //         index = new ANNbd_tree(bdtree_pts, points.size(), Dim, BucketSize);
        //         auto end = std::chrono::steady_clock::now();
        //         std::cout << "Construction time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " [ms]" << std::endl;
        //     }

        //     ~ANNBoxDecompositionTree()
        //     {
        //         delete index;
        //         annClose();
        //     }

        //     inline Points knn_query(Point &q, size_t k, double eps = 0.0)
        //     {
        //         std::vector<ANNidx> nn_idx;
        //         std::vector<ANNdist> nn_dist;
        //         nn_idx.resize(k);
        //         nn_dist.resize(k);

        //         index->annkSearch(&q[0], k, &nn_idx[0], &nn_dist[0], eps);

        //         Points result;
        //         result.reserve(k);

        //         for (auto idx : nn_idx)
        //         {
        //             Point p;
        //             auto temp_pt = index->thePoints()[idx];

        //             for (size_t i = 0; i < Dim; ++i)
        //             {
        //                 p[i] = temp_pt[i];
        //             }

        //             result.emplace_back(p);
        //         }

        //         return result;
        //     }

        //     inline size_t count()
        //     {
        //         return this->num_of_points;
        //     }

        // private:
        //     size_t num_of_points;
        //     ANNbd_tree *index; // internal box decomposition tree index
        // };

    }
}
