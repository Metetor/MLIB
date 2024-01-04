#pragma once

#include <array>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/adapted/std_array.hpp>
#include <cstddef>
#include <utility>
#include <vector>
#include <boost/geometry.hpp>

BOOST_GEOMETRY_REGISTER_STD_ARRAY_CS(cs::cartesian)

namespace bg = boost::geometry;
namespace bgm = bg::model;

// use static point definition
// the dimension should be determined in compile time for performance consideration
// currently, the max dim is 12 in this benchmark
template <size_t dim>
using point_t = std::array<double, dim>;

template <size_t dim>
using box_t = bgm::box<point_t<dim>>;

template <size_t dim>
using vec_of_point_t = std::vector<point_t<dim>>;
