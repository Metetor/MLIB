#pragma once

#include "../../utils/type.hpp"
#include "../../utils/common.hpp"
#include "../indexInterface.h"

#include <boost/geometry/index/parameters.hpp>
#include <boost/geometry/index/predicates.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <cstddef>
#include <iterator>
#include <chrono>

namespace bgi = boost::geometry::index;

namespace bench
{
    namespace index
    {
        /// @brief RTreeInterface:inherited from IndexInterface
        /// @tparam MaxElements:RTree Non-Leaf Node can hold Max Entry
        template <class KEY_TYPE, size_t Dim, size_t MaxElements = 128>
        class RTreeInterface : public IndexInterface<KEY_TYPE, Dim>
        {
            using rtree_t = bgi::rtree<point_t<Dim>, bgi::linear<MaxElements>>;

        public:
            RTreeInterface() : index(nullptr) {}

            ~RTreeInterface()
            {
                delete index;
            }

            inline size_t count() { return index->size(); }

            inline size_t index_size()
            {
                return bench::common::get_boost_rtree_statistics(*index);
            }

            void build(std::vector<KEY_TYPE> &points);

            std::vector<KEY_TYPE> range_query(box_t<Dim> &box);

            std::vector<KEY_TYPE> knn_query(KEY_TYPE &q, unsigned int k);

        private:
            rtree_t *index;
        };
        // func impl
        template <class KEY_TYPE, size_t Dim, size_t MaxElements>
        void RTreeInterface<KEY_TYPE, Dim, MaxElements>::
            build(std::vector<KEY_TYPE> &points)
        {
            // Implement the build function for RtreeInterface
            std::cout << "Construct R-tree "
                      << "MaxElements=" << MaxElements << std::endl;

            auto start = std::chrono::steady_clock::now();
            // construct rtree using packing algorithm
            index = new rtree_t(points.begin(), points.end());
            auto end = std::chrono::steady_clock::now();

            this->build_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            std::cout<< "Points Count:"<<this->count()<<std::endl;
            std::cout << "Build Time: " << this->get_build_time() << " [ms]" << std::endl;
            std::cout << "Index Size: " << this->index_size() << " Bytes" << std::endl;
        }

        template <class KEY_TYPE, size_t Dim, size_t MaxElements>
        std::vector<KEY_TYPE> RTreeInterface<KEY_TYPE, Dim, MaxElements>::
            range_query(box_t<Dim> &box)
        {
            auto start = std::chrono::steady_clock::now();
            //
            std::vector<KEY_TYPE> return_values;
            index->query(bgi::covered_by(box), std::back_inserter(return_values));
            //
            auto end = std::chrono::steady_clock::now();
            this->range_cnt++;
            this->range_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            return return_values;
        }

        template <class KEY_TYPE, size_t Dim, size_t MaxElements>
        std::vector<KEY_TYPE> RTreeInterface<KEY_TYPE, Dim, MaxElements>::
            knn_query(KEY_TYPE &q, unsigned int k)

        {
            auto start = std::chrono::steady_clock::now();

            std::vector<KEY_TYPE> return_values;
            index->query(bgi::nearest(q, k), std::back_inserter(return_values));

            auto end = std::chrono::steady_clock::now();

            this->knn_cnt++;
            this->knn_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            return return_values;
        }

        // RStarInterface
        template <class KEY_TYPE, size_t Dim, size_t MaxElements = 128>
        class RStarInterface : public IndexInterface<KEY_TYPE, Dim>
        {
            using rstar_t = bgi::rtree<point_t<Dim>, bgi::rstar<MaxElements>>;

        private:
            rstar_t *index;

        public:
            RStarInterface() : index(nullptr) {}

            ~RStarInterface() { delete index; }

            inline size_t count()
            {
                return index->size();
            }

            inline size_t index_size()
            {
                return bench::common::get_boost_rtree_statistics(*index);
            }

            void build(std::vector<KEY_TYPE> &points);

            std::vector<KEY_TYPE> range_query(box_t<Dim> &box);

            std::vector<KEY_TYPE> knn_query(KEY_TYPE &q, uint k);
        };

        template <class KEY_TYPE, size_t Dim, size_t MaxElements>
        void RStarInterface<KEY_TYPE, Dim, MaxElements>::
            build(std::vector<KEY_TYPE> &points)
        {
            // Implement the build function for RStarInterface
            std::cout << "Construct R*-tree "
                      << "MaxElements=" << MaxElements << std::endl;

            auto start = std::chrono::steady_clock::now();

            index = new rstar_t();
            // rstar init: insert
            for (auto &p : points)
            {
                index->insert(p);
            }

            auto end = std::chrono::steady_clock::now();
            this->build_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

            std::cout << "Build Time: " << this->get_build_time() << " [ms]" << std::endl;
            std::cout << "Index Size: " << this->index_size() << " Bytes" << std::endl;
        }

        template <class KEY_TYPE, size_t Dim, size_t MaxElements>
        std::vector<KEY_TYPE> RStarInterface<KEY_TYPE, Dim, MaxElements>::
            range_query(box_t<Dim> &box)
        {
            auto start = std::chrono::steady_clock::now();

            std::vector<KEY_TYPE> return_values;
            index->query(bgi::covered_by(box), std::back_inserter(return_values));

            auto end = std::chrono::steady_clock::now();

            this->range_cnt++;
            this->range_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            return return_values;
        }

        template <class KEY_TYPE, size_t Dim, size_t MaxElements>
        std::vector<KEY_TYPE> RStarInterface<KEY_TYPE, Dim, MaxElements>::
            knn_query(KEY_TYPE &q, uint k)
        {
            auto start = std::chrono::steady_clock::now();

            std::vector<KEY_TYPE> return_values;
            index->query(bgi::nearest(q, k), std::back_inserter(return_values));

            auto end = std::chrono::steady_clock::now();

            this->knn_cnt++;
            this->knn_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

            return return_values;
        }
    }
}
