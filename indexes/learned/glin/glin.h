#include <vector>
#include <geos/geom/Point.h>
#include <geos/index/strtree/SimpleSTRtree.h>
#include <geos/index/strtree/GeometryItemDistance.h>
#include <geos/index/ItemVisitor.h>
#include <geos/geom/Envelope.h>
#include <geos/geom/Coordinate.h>
#include <geos/geom/CoordinateSequence.h>
#include <geos/geom/CoordinateArraySequence.h>
#include <geos/geom/Dimension.h>
#include <geos/geom/PrecisionModel.h>
#include <geos/util/IllegalArgumentException.h>
#include <geos/geom/LineSegment.h>
#include <geos/geom/GeometryFactory.h>
#include <geos/geom/Geometry.h>
#include <geos/geom/Polygon.h>
#include <geos/io/WKTReader.h>
#include <geos/algorithm/Angle.h>

namespace bench
{
    namespace index
    {
        class Glin
        {
            // load space filling curve:parse args:
            void loadCurve(std::vector<geos::geom::Geometry *> geom, double pieceLimitation, std::string curve_type,
                           double cell_xmin, double cell_ymin,
                           double cell_x_intvl, double cell_y_intvl,
                           std::vector<std::tuple<double, double, double, double>> &pieces)
            {
                     
            }
            // glin load:parse params:
            // geom:geometry; pieceLimitation:?curve_type:z_order or other;cell_xmin:x bound min;
            // cell_ymin;cell_x_intvl:?;cell_y_intvl:?;pieces:
            void glin_bulk_load(std::vector<geos::geom::Geometry *> geom, double pieceLimitation,
                                std::string curve_type,
                                double cell_xmin, double cell_ymin,
                                double cell_x_intvl, double cell_y_intvl,
                                std::vector<std::tuple<double, double, double, double>> &pieces)
            {
                // load curve
                load_Curve()
            }
        };
    }
}