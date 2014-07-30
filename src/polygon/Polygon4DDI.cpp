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
 * Polygon4DDI.cpp
 *
 *  Created on: Oct 1, 2013
 *      Author: sk
 */

#include "Polygon4DDI.h"
#include "Logger.h"
#include <math.h>

Polygon4DDI::Polygon4DDI()
: finalized_(false), altitude_min_(0), altitude_max_(0)
{
}

Polygon4DDI::~Polygon4DDI()
{
    if (finalized_)
    {
        for(unsigned int i = 0; i < points_array_size_; ++i) {
            delete [] points_array_[i];
        }
        delete [] points_array_;
        points_array_=0;
    }
}

void Polygon4DDI::finalize (std::string name)
{
    name_=name;
    assert (!finalized_);
    assert (points_.size() > 0);
    points_.push_back(points_.at(0)); // complete polygon

    //points_.push_back(points_.at(0)); //add first
    points_array_size_ = points_.size();
    //points_array_ = new double [points_array_size_] [2];

    points_array_ = new double*[points_array_size_];
    for(unsigned int i = 0; i < points_array_size_; ++i) {
        points_array_[i] = new double[2];
    }

    for (unsigned int cnt=0; cnt < points_.size(); cnt++)
    {
        points_array_[cnt] [0] = points_.at(cnt).first;
        points_array_[cnt] [1] = points_.at(cnt).second;
        //loginf << "UGA " << name_ << " Point " << cnt << " lat " << points_array_[cnt] [0] << " long " << points_array_[cnt] [1];
    }

    finalized_=true;
}

void Polygon4DDI::addPoint (double latitude, double longitude)
{
    assert (!finalized_);

    if (points_.size() == 0)
    {
        latitude_min_=latitude;
        latitude_max_=latitude;
        longitude_min_=longitude;
        longitude_max_=longitude;
    }
    else
    {
        if (latitude < latitude_min_)
            latitude_min_=latitude;
        if (latitude > latitude_max_)
            latitude_max_=latitude;

        if (longitude < longitude_min_)
            longitude_min_=longitude;
        if (longitude > longitude_max_)
            longitude_max_=longitude;
    }

    points_.push_back (std::pair <double, double > (latitude, longitude));
}

void Polygon4DDI::setAltitudeMinMax (double min, double max)
{
    altitude_min_=min;
    altitude_max_=max;
}

bool Polygon4DDI::inside (double p_lat, double p_long, double altitude, bool debug)
{
    assert (finalized_);

    if (altitude_min_ == altitude_max_ && altitude_min_ == 0)
    {
        if (debug)
            loginf << "Polygon4DDI: inside: no height defined, using 2d";

        return inside (p_lat, p_long, debug);
    }
    else
    {
        if (altitude >= altitude_min_ && altitude <= altitude_max_) // altitude ok
        {
            if (debug)
                loginf << "Polygon4DDI: inside: within defined altitude";

            if (p_lat >= latitude_min_ && p_lat <= latitude_max_ &&
                    p_long >= longitude_min_ && p_long <= longitude_max_) // wihtin bounding rectangle
            {
                if (debug)
                    loginf << "Polygon4DDI: inside: within bounding rectangle";
                return inside (p_lat, p_long, debug);
            }
            else
            {
                if (debug)
                    loginf << "Polygon4DDI: inside: outside of bounding rect " << p_lat << ", " << p_long
                        << " [" << latitude_min_ << ", " << longitude_min_ << "] "
                        << " [" << latitude_max_ << ", " << longitude_max_ << "] ";
                return false;
            }
        }
        else
        {
            if (debug)
                loginf << "Polygon4DDI: inside: altitude " << altitude <<" out of [" << altitude_min_ << ","
                    << altitude_max_ << "]";
            return false;
        }
    }
}

bool Polygon4DDI::inside (double p_lat, double p_long, bool debug)
{
    assert (finalized_);

    int n = points_array_size_ - 1; //poly.Count();

    if (debug)
        loginf << "Polygon4DDI: inside: 2d n " << n;

    int wn = 0;    // the winding number counter

    // loop through all edges of the polygon
    for (int i = 0; i < n; i++)
    {   // edge from V[i] to V[i+1]
        if (points_array_[i][0] <= p_lat)
        {         // start y <= P.y
            if (points_array_[i + 1][0] > p_lat)      // an upward crossing
                if (isLeft(points_array_[i][0], points_array_[i][1], points_array_[i + 1][0],
                        points_array_[i+1][1], p_lat, p_long) > 0)  // P left of edge
                    ++wn;            // have a valid up intersect
        }
        else
        {                       // start y > P.y (no test needed)
            if (points_array_[i + 1][0] <= p_lat)     // a downward crossing
                if (isLeft(points_array_[i][0], points_array_[i][1], points_array_[i + 1][0],
                        points_array_[i+1][1], p_lat, p_long) < 0)  // P right of edge
                    --wn;            // have a valid down intersect
        }
    }

    if (debug)
            loginf << "Polygon4DDI: inside: 2d " << name_ << " lat " << p_lat << " long " << p_long << ((wn != 0) ? " inside" : " outside" );

    if (wn != 0)
        return true;
    else
        return false;

//    int i, j, c = 0;
//    for (i = 0, j = nvert-1; i < nvert; j = i++) {
//      if ( ((verty[i]>testy) != (verty[j]>testy)) &&
//       (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
//         c = !c;
//    }
//    return c;

//        int i, j, c = 0;
//
//        for (i = 0, j = nvert-1; i < nvert; j = i++) {
//          if ( ((vert_long[i]>test_long) != (vert_long[j]>test_long)) &&
//           (test_lat < (vert_lat[j]-vert_lat[i]) * (test_long-vert_long[i]) / (vert_long[j]-vert_long[i]) + vert_lat[i]) )
//             c = !c;
//        }
//        return c;

//    int i, j, c = 0;
//
//    for (i = 0, j = n; i < points_array_size_; j = i++)
//    {
//      if ( ((points_array_[i][1] >= p_long) != (points_array_[j][1] >= p_long)) &&
//       (p_lat <= (points_array_[j][0]-points_array_[i][0]) * (p_long-points_array_[i][1]) / (points_array_[j][1]-points_array_[i][1]) + points_array_[i][0]) )
//         c = !c;
//    }
//
//    bool inside = !(c%2);
//
//    if (debug)
//        loginf << "Polygon4DDI: inside: 2d " << name_ << " lat " << p_lat << " long " << p_long << " inside " << inside;
//
//    return inside;

}

//bool Polygon4DDI::inside (double p_lat, double p_long)
//{
//    assert (finalized_);
//    //assert ((altitude_min_ == altitude_max_ && altitude_min_ == 0));
//    //List<LatLong> poly;
//    int n = points_.size() - 1; //poly.Count();
//
////    poly.Add(new LatLong { Lat = poly[0].Lat, Lon = poly[0].Lon });
////    LatLong[] v = poly.ToArray();
//
//    int wn = 0;    // the winding number counter
//
//    // loop through all edges of the polygon
//    for (int i = 0; i < n; i++)
//    {   // edge from V[i] to V[i+1]
//        if (points_.at(i).first <= p_lat)
//        {         // start y <= P.y
//            if (points_.at(i + 1).first > p_lat)      // an upward crossing
//                if (isLeft(points_.at(i).first, points_.at(i).second, points_.at(i + 1).first,
//                        points_.at(i+1).second, p_lat, p_long) > 0)  // P left of edge
//                    ++wn;            // have a valid up intersect
//        }
//        else
//        {                       // start y > P.y (no test needed)
//            if (points_.at(i + 1).first <= p_lat)     // a downward crossing
//                if (isLeft(points_.at(i).first, points_.at(i).second, points_.at(i + 1).first,
//                        points_.at(i+1).second, p_lat, p_long) < 0)  // P right of edge
//                    --wn;            // have a valid down intersect
//        }
//    }
//
////    loginf << name_ << " lat " << p_lat << " long " << p_long << ((wn != 0) ? " inside" : " outside" );
//
//    if (wn != 0)
//        return true;
//    else
//        return false;
//}

//int Polygon4DDI::isLeft(double p0_lat, double p0_long, double p1_lat, double p1_long, double p2_lat, double p2_long)
//{
//    double calc = ((p1_long - p0_long) * (p2_lat - p0_lat)
//            - (p2_long - p0_long) * (p1_lat - p0_lat));
//    if (calc > 0)
//        return 1;
//    else if (calc < 0)
//        return -1;
//    else
//        return 0;
//}

void Polygon4DDI::clearPoints ()
{
    points_.clear();
    finalized_=false;
}


double Polygon4DDI::getLatitudeMinRounded ()
{
    assert (finalized_);
    //loginf << "Polygon4DDI::getLatitudeMinRounded: lat min " << latitude_min_ << " returning " << floor(2*latitude_min_)/2;
    return floor(4*latitude_min_)/4;
}
double Polygon4DDI::getLatitudeMaxRounded ()
{
    assert (finalized_);
    //loginf << "Polygon4DDI::getLatitudeMaxRounded: lat max " << latitude_max_ << " returning " << ceil(2*latitude_max_)/2;
    return ceil(4*latitude_max_)/4;
}

double Polygon4DDI::getLongitudeMinRounded ()
{
    assert (finalized_);
    //loginf << "Polygon4DDI::getLongitudeMinRounded: lon min " << longitude_min_ << " returning " << floor(2*longitude_min_)/2;
    return floor(4*longitude_min_)/4;
}
double Polygon4DDI::getLongitudeMaxRounded ()
{
    assert (finalized_);
    //loginf << "Polygon4DDI::getLongitudeMaxRounded: lon max " << longitude_max_ << " returning " << ceil(2*longitude_max_)/2;
    return ceil(4*longitude_max_)/4;
}
