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
 * GeographicPoint.cpp
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#include "GeographicPoint.h"
#include "Logger.h"

GeographicPoint::GeographicPoint(std::string class_id, std::string instance_id, Configurable *parent)
: Configurable(class_id, instance_id, parent)
{
    registerParameter("index", &index_, 0);
    registerParameter("latitude", &latitude_, 0.0);
    registerParameter("longitude", &longitude_, 0.0);

    assert (latitude_ != longitude_ && longitude_ != 0.0);

    createSubConfigurables ();

}

GeographicPoint::~GeographicPoint()
{

}

void GeographicPoint::generateSubConfigurable (std::string class_id, std::string instance_id)
{
    throw std::runtime_error ("GeographicPoint: generateSubConfigurable: unknown class_id "+class_id );
}

void GeographicPoint::checkSubConfigurables ()
{

}

void GeographicPoint::setCoordinates (double latitude, double longitude)
{
    latitude_=latitude;
    longitude_=longitude;
    //set_=true;
}


double GeographicPoint::getLatitude ()
{
    //assert (set_);
    return latitude_;
}
double GeographicPoint::getLongitude ()
{
    //assert (set_);
    return longitude_;
}
