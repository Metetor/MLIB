#pragma once

#include <cstddef>
#include <string>
#include <chrono>
#include <queue>

#include "../base_index.hpp"
#include "../indexInterface.h"
#include "../rsmi/Constants.h"
#include "../rsmi/RSMI.h"
#include "../rsmi/Point.h"
#include "../rsmi/Mbr.h"
#include "../rsmi/ExpRecorder.h"

#ifdef HEAP_PROFILE
#include <gperftools/heap-profiler.h>
#endif

namespace bench
{
    namespace index
    {

        template <size_t Dim = 2>
        class RSMIWrapper : public BaseIndex
        {
            static_assert(Dim == 2, "RSMI only supports 2-D data!");

            using BenchPoint = point_t<Dim>;
            using BenchBox = box_t<Dim>;
            using BenchPoints = std::vector<point_t<Dim>>;

        public:
            RSMIWrapper(BenchPoints &points, const std::string path_to_model) : N(points.size())
            {
                std::cout << "Construct RSMI index" << std::endl;

                // type cast for RSMI library
                std::vector<rsmientities::Point> rsmi_points;
                rsmi_points.reserve(this->N);

                for (auto &p : points)
                {
                    rsmi_points.emplace_back(std::get<0>(p), std::get<1>(p));
                }

                auto start = std::chrono::steady_clock::now();

#ifdef HEAP_PROFILE
                HeapProfilerStart("rsmi");
#endif

                this->_exp_recorder = new rsmiutils::ExpRecorder();
                this->_rsmi = new RSMI(0, rsmiutils::Constants::MAX_WIDTH);
                this->_rsmi->model_path = path_to_model;
                this->_rsmi->build(*_exp_recorder, rsmi_points);

#ifdef HEAP_PROFILE
                HeapProfilerDump("final");
                HeapProfilerStop();
#endif

                auto end = std::chrono::steady_clock::now();
                build_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
                std::cout << "Build Time: " << get_build_time() << " [ms]" << std::endl;
            }

            ~RSMIWrapper()
            {
                delete this->_rsmi;
                delete this->_exp_recorder;
            }

            inline size_t count()
            {
                return N;
            }

            BenchPoints range_query(BenchBox &box)
            {
                this->_exp_recorder->window_query_results.clear();
                this->_exp_recorder->window_query_results.shrink_to_fit();

                auto min_corner = box.min_corner();
                auto max_corner = box.max_corner();
                // (x1, y1, x2, y2)
                rsmientities::Mbr mbr(std::get<0>(min_corner), std::get<1>(min_corner), std::get<0>(max_corner), std::get<1>(max_corner));

                auto start = std::chrono::steady_clock::now();
                auto rsmi_result = this->_rsmi->acc_window_query(*_exp_recorder, mbr);
                auto end = std::chrono::steady_clock::now();

                range_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                range_count++;

                BenchPoints results;
                for (auto &r : rsmi_result)
                {
                    BenchPoint temp_p;
                    temp_p[0] = r.x;
                    temp_p[1] = r.y;
                    results.emplace_back(temp_p);
                }

                return results;
            }

            BenchPoints knn_query(BenchPoint &point, size_t k)
            {
                // empty the internal queue
                std::priority_queue<rsmientities::Point, std::vector<rsmientities::Point>, rsmiutils::sortForKNN2> temp_pq;
                this->_exp_recorder->pq = temp_pq;

                rsmientities::Point q_point(std::get<0>(point), std::get<1>(point));

                auto start = std::chrono::steady_clock::now();
                std::vector<rsmientities::Point> knn_results = this->_rsmi->acc_kNN_query(*_exp_recorder, q_point, k);
                auto end = std::chrono::steady_clock::now();

                knn_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
                knn_count++;

                BenchPoints results;
                for (auto &r : knn_results)
                {
                    BenchPoint temp_p;
                    temp_p[0] = r.x;
                    temp_p[1] = r.y;
                    results.emplace_back(temp_p);
                }

                return results;
            }

        private:
            size_t N;
            RSMI *_rsmi;
            ExpRecorder *_exp_recorder;
        };

        template <class KEY_TYPE, size_t Dim = 2>
        class RSMIInterface : public IndexInterface<KEY_TYPE, Dim>
        {
            static_assert(Dim == 2, "RSMI only supports 2-D data!");

        public:
            RSMIInterface(std::string path) : _rsmi(nullptr) { path_to_model = path; }
            ~RSMIInterface()
            {
                delete this->_rsmi;
                delete this->_exp_recorder;
            }

            inline size_t count()
            {
                return N;
            }

            void build(std::vector<KEY_TYPE> &points);

            std::vector<KEY_TYPE> range_query(box_t<Dim> &box);

            std::vector<KEY_TYPE> knn_query(KEY_TYPE &q, uint k);

        private:
            size_t N;
            RSMI *_rsmi;
            ExpRecorder *_exp_recorder;
            std::string path_to_model;
        };

        // implementation
        template <class KEY_TYPE, size_t Dim>
        void RSMIInterface<KEY_TYPE, Dim>::build(std::vector<KEY_TYPE> &points)
        {
            std::cout << "Construct RSMI index" << std::endl;
            N = points.size();
            // type cast for RSMI library
            //std::cout << "checkpoint 0" << std::endl;
            std::vector<rsmientities::Point> rsmi_points;
            rsmi_points.reserve(this->N);

            for (auto &p : points)
            {
                rsmi_points.emplace_back(std::get<0>(p), std::get<1>(p));
            }
            //std::cout << "checkpoint 1" << std::endl;
            auto start = std::chrono::steady_clock::now();
            //std::cout << "build begin timer" << std::endl;
#ifdef HEAP_PROFILE
            HeapProfilerStart("rsmi");
#endif

            this->_exp_recorder = new rsmiutils::ExpRecorder();
            this->_rsmi = new RSMI(0, rsmiutils::Constants::MAX_WIDTH);
            this->_rsmi->model_path = path_to_model;
            //std::cout << "checkpoint 2" << std::endl;
            this->_rsmi->build(*_exp_recorder, rsmi_points);
            //std::cout << "checkpoint 3" << std::endl;
#ifdef HEAP_PROFILE
            HeapProfilerDump("final");
            HeapProfilerStop();
#endif

            auto end = std::chrono::steady_clock::now();
            this->build_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            std::cout << "Build Time: " << this->get_build_time() << " [ms]" << std::endl;
        }

        template <class KEY_TYPE, size_t Dim>
        std::vector<KEY_TYPE> RSMIInterface<KEY_TYPE, Dim>::
            range_query(box_t<Dim> &box)
        {
            this->_exp_recorder->window_query_results.clear();
            this->_exp_recorder->window_query_results.shrink_to_fit();

            auto min_corner = box.min_corner();
            auto max_corner = box.max_corner();
            // (x1, y1, x2, y2)
            rsmientities::Mbr mbr(std::get<0>(min_corner), std::get<1>(min_corner), std::get<0>(max_corner), std::get<1>(max_corner));

            auto start = std::chrono::steady_clock::now();
            auto rsmi_result = this->_rsmi->acc_window_query(*_exp_recorder, mbr);
            auto end = std::chrono::steady_clock::now();

            this->range_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            this->range_cnt++;

            std::vector<KEY_TYPE> results;
            for (auto &r : rsmi_result)
            {
                KEY_TYPE temp_p;
                temp_p[0] = r.x;
                temp_p[1] = r.y;
                results.emplace_back(temp_p);
            }

            return results;
        }

        template <class KEY_TYPE, size_t Dim>
        std::vector<KEY_TYPE> RSMIInterface<KEY_TYPE, Dim>::
            knn_query(KEY_TYPE &q, uint k)
        {
            // empty the internal queue
            std::priority_queue<rsmientities::Point, std::vector<rsmientities::Point>, rsmiutils::sortForKNN2> temp_pq;
            this->_exp_recorder->pq = temp_pq;

            rsmientities::Point q_point(std::get<0>(q), std::get<1>(q));

            auto start = std::chrono::steady_clock::now();
            std::vector<rsmientities::Point> knn_results = this->_rsmi->acc_kNN_query(*_exp_recorder, q_point, k);
            auto end = std::chrono::steady_clock::now();

            this->knn_time += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            this->knn_cnt++;

            std::vector<KEY_TYPE> results;
            for (auto &r : knn_results)
            {
                KEY_TYPE temp_p;
                temp_p[0] = r.x;
                temp_p[1] = r.y;
                results.emplace_back(temp_p);
            }

            return results;
        }
    }
}
