/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Polygon4DDI.h
 *
 *  Created on: Oct 1, 2013
 *      Author: sk
 */

#ifndef POLYGON4DDI_H_
#define POLYGON4DDI_H_

#include <vector>
#include <string>
#include <cassert>

enum POLYGON_SELECTION {POLYGON_FIR_UPPER=0, POLYGON_FIR_EAST, POLYGON_FIR_WEST, POLYGON_FIR_TMA_VIENNA, POLYGON_FIR_TMA_GRAZ,
    POLYGON_FIR_TMA_LINZ, POLYGON_FIR_TMA_SALZBURG, POLYGON_FIR_TMA_KLAGENFURT, POLYGON_FIR_TMA_INNSBRUCK};

class Polygon4DDI
{
public:
    Polygon4DDI();
    virtual ~Polygon4DDI();

    void addPoint (double latitude, double longitude);
    void setAltitudeMinMax (double min, double max);
    void finalize (std::string name);
    bool isFinalized () { return finalized_; }

    bool inside (double p_lat, double p_long, bool debug);
    bool inside (double p_lat, double p_long, double altitude, bool debug);

    void clearPoints ();

    const std::vector< std::pair<double, double> > &getPoints () { return points_; }

    double getLatitudeMin () { assert (finalized_); return latitude_min_; }
    double getLatitudeMax () { assert (finalized_); return latitude_max_; }

    double getLongitudeMin () { assert (finalized_); return longitude_min_; }
    double getLongitudeMax () { assert (finalized_); return longitude_max_; }

protected:
    bool finalized_;
    std::string name_;

    double altitude_min_;
    double altitude_max_;

    double latitude_min_;
    double latitude_max_;

    double longitude_min_;
    double longitude_max_;

    std::vector < std::pair<double, double> > points_;
    double **points_array_;
    unsigned int points_array_size_;

    inline int isLeft(double p0_lat, double p0_long, double p1_lat, double p1_long, double p2_lat, double p2_long)
    {
        double calc = ((p1_long - p0_long) * (p2_lat - p0_lat)
                - (p2_long - p0_long) * (p1_lat - p0_lat));
        if (calc > 0)
            return 1;
        else if (calc < 0)
            return -1;
        else
            return 0;
    }
};

#endif /* POLYGON4DDI_H_ */
