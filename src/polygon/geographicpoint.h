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
 * GeographicPoint.h
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#ifndef GEOGRAPHICPOINT_H_
#define GEOGRAPHICPOINT_H_

#include "Configurable.h"

class GeographicPoint : public Configurable
{
public:
    GeographicPoint(std::string class_id, std::string instance_id, Configurable *parent);
    virtual ~GeographicPoint();

    virtual void generateSubConfigurable (std::string class_id, std::string instance_id);

    void setCoordinates (double latitude, double longitude);
    double getLatitude ();
    double getLongitude ();
    unsigned int getIndex () { return index_; }

protected:
    unsigned int index_;
    double latitude_;
    double longitude_;

    virtual void checkSubConfigurables ();
};

#endif /* GEOGRAPHICPOINT_H_ */
