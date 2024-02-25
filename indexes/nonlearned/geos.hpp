#pragma once

#include <geos/geom/Coordinate.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/index/quadtree/Quadtree.h>
#include <geos/index/strtree/TemplateSTRtree.h>

#ifdef HEAP_PROFILE
#include <gperftools/heap-profiler.h>
#endif

#include <chrono>

#include "../../utils/type.hpp"

namespace bench
{
    namespace index
    {

        struct GEOSGeometry
        {
            std::vector<geos::geom::Geometry *> geometry;
            geos::geom::GeometryFactory::Ptr factory;

            GEOSGeometry()
            {
                factory = geos::geom::GeometryFactory::create();
            }

            ~GEOSGeometry()
            {
                for (geos::geom::Geometry *g : geometry)
                {
                    factory->destroyGeometry(g);
                }
                geometry.clear();
            }
        };

        template <class KEY_TYPE>
        void points_to_geos(std::vector<KEY_TYPE> &points, GEOSGeometry &geos_points)
        {
            geos_points.geometry.reserve(points.size());

            for (auto &p : points)
            {
                geos::geom::Geometry *temp = geos_points.factory->createPoint(geos::geom::Coordinate(std::get<0>(p), std::get<1>(p)));
                geos_points.geometry.push_back(temp);
            }
        }

        template <class KEY_TYPE, size_t Dim = 2>
        class QDTreeInterface : public IndexInterface<KEY_TYPE, Dim>
        {
            using QDTree_t = geos::index::quadtree::Quadtree;

        public:
            QDTreeInterface() : index(nullptr) {}
            ~QDTreeInterface() { delete index; }

            inline size_t count()
            {
                return index->size();
            }

            void build(std::vector<KEY_TYPE> &points);

            std::vector<KEY_TYPE> range_query(box_t<Dim> &box);
            // knn not support
            std::vector<KEY_TYPE> knn_query(KEY_TYPE &q, unsigned int k)
            {
                std::cerr << "Error: knn_query is not supported for QDTree." << std::endl;
                return {}; // 返回一个空的结果
            }

        private:
            QDTree_t *index;
            GEOSGeometry geos_points;
        };

        // func impl
        template <class KEY_TYPE, size_t Dim>
        void QDTreeInterface<KEY_TYPE, Dim>::build(std::vector<KEY_TYPE> &points)
        {
            std::cout << "Construct QDTree "
                      << "Dim=" << Dim << std::endl;

            points_to_geos(points, geos_points);

            auto start = std::chrono::steady_clock::now();

#ifdef HEAP_PROFILE
            HeapProfilerStart("qdtree");
#endif

            index = new QDTree_t();

            for (size_t i = 0; i < points.size(); ++i)
            {
                index->insert(geos_points.geometry[i]->getEnvelopeInternal(), reinterpret_cast<void *>(geos_points.geometry[i]));
            }

#ifdef HEAP_PROFILE
            HeapProfilerDump("final");
            HeapProfilerStop();
#endif

            auto end = std::chrono::steady_clock::now();
            this->build_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "Build Time: " << this->get_build_time() << " [ms]" << std::endl;
        }

        template <class KEY_TYPE, size_t Dim>
        std::vector<KEY_TYPE> QDTreeInterface<KEY_TYPE, Dim>::
            range_query(box_t<Dim> &box)
        {
            // prepare the query geometry
            geos::geom::GeometryFactory::Ptr GF_ptr = geos::geom::GeometryFactory::create();

            geos::geom::Envelope geos_box(box.min_corner()[0], box.max_corner()[0], box.min_corner()[1], box.max_corner()[1]);
            auto query_geometry = GF_ptr->toGeometry(&geos_box);

            // query the geometry envolope
            auto start = std::chrono::steady_clock::now();

            std::vector<void *> results;
            index->query(query_geometry->getEnvelopeInternal(), results);

            auto end = std::chrono::steady_clock::now();

            this->range_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            this->range_cnt++;

            // collect results
            // the geos libaray returns geometries **may** intersect the query rectangle
            std::vector<KEY_TYPE> point_results;
            for (size_t i = 0; i < results.size(); ++i)
            {
                auto pg = static_cast<geos::geom::Geometry *>(results[i]);
                if (query_geometry->contains(pg))
                {
                    point_t<Dim> temp;
                    temp[0] = pg->getCoordinate()->x;
                    temp[1] = pg->getCoordinate()->y;
                    point_results.emplace_back(temp);
                }
            }

            return point_results;
        }
    }
}
