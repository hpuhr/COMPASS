#include "datasourcecompoundcoverage.h"
#include "ogrcoordinatesystem.h"
#include "logger.h"

using namespace std;

namespace dbContent
{

DataSourceCompoundCoverage::DataSourceCompoundCoverage()
{
}

void DataSourceCompoundCoverage::clear()
{
    range_circles_.clear();
    range_circles_cs_.clear();

    is_finalized_ = false;
}

void DataSourceCompoundCoverage::addRangeCircle (unsigned int ds_id, double center_lat, double center_long, double range_m)
{
    range_circles_.push_back(std::tuple<unsigned int, double, double, double> {
                                 ds_id, center_lat, center_long, range_m});
}

bool DataSourceCompoundCoverage::isInside (double pos_lat, double pos_long) const
{
    assert (is_finalized_);

    // if no info, true
    if (!range_circles_cs_.size())
    {
        logdbg << "DataSourceCompoundCoverage: isInside: no circ, true";

        return true;
    }

    // check inside any

    bool ok;
    double x_pos_m;
    double y_pos_m;
    double pos_rng_m;

    for (auto& rng_circ : range_circles_cs_)
    {
        ok = rng_circ.first->wgs842Cartesian(pos_lat, pos_long, x_pos_m, y_pos_m);

        if (ok)
        {
            pos_rng_m = sqrt(pow(x_pos_m, 2) + pow(y_pos_m, 2));

            if (pos_rng_m <= rng_circ.second)
            {
                logdbg << "DataSourceCompoundCoverage: isInside: inside circ, true";
                return true;
            }
            else
                logdbg << "DataSourceCompoundCoverage: isInside: outside circ, range "
                       << pos_rng_m << " max " << rng_circ.second << ", false";
        }
    }

    logdbg << "DataSourceCompoundCoverage: isInside: not inside any circ, false";
    return false;
}

void DataSourceCompoundCoverage::finalize()
{
    assert (!is_finalized_);

    for (auto& rng_circ : range_circles_)
        range_circles_cs_.emplace_back(
                    std::pair<std::unique_ptr<OGRCoordinateSystem>, double>
                    {new OGRCoordinateSystem(get<0>(rng_circ), get<1>(rng_circ), get<2>(rng_circ), 0),
                     get<3>(rng_circ)});

    is_finalized_ = true;
}

bool DataSourceCompoundCoverage::hasCircles() const
{
    assert (is_finalized_);

    return range_circles_cs_.size();
}

}