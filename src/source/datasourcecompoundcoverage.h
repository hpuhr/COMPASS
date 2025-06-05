#pragma once

#include "rs2gcoordinatesystem.h"

#include <vector>
#include <tuple>
#include <memory>

namespace dbContent
{

class DataSourceCompoundCoverage
{
public:
    DataSourceCompoundCoverage();

    void clear();
    void addRangeCircle (unsigned int ds_id, double center_lat, double center_long, double range_m);

    bool isInside (double pos_lat, double pos_long) const;

    void finalize();
    bool hasCircles() const;

private:
    bool is_finalized_ {false};

    std::vector<std::tuple<unsigned int, double, double, double>> range_circles_;
    std::vector<std::pair<std::unique_ptr<RS2GCoordinateSystem>, double>> range_circles_cs_; // cs -> range
};

}

