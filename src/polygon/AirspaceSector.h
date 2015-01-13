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
 * AirspaceSector.h
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#ifndef AIRSPACESECTOR_H_
#define AIRSPACESECTOR_H_

#include "Configurable.h"
#include "Polygon4DDI.h"
#include "Vector2.h"

class GeographicPoint;
class vec2 {float x,y;};

class AirspaceSector : public Configurable
{
public:
    AirspaceSector(std::string class_id, std::string instance_id, Configurable *parent);
    virtual ~AirspaceSector();

    virtual void generateSubConfigurable (std::string class_id, std::string instance_id);

    std::string getName () { return name_; }
    void setName (std::string name);

    bool hasOwnVolume () { return has_own_volume_; }
    void setHasOwnVolume (bool value) { has_own_volume_ = value; }

    double getHeightMin () { assert (has_own_volume_); return own_height_min_; } // in feet
    void setHeightMin (double value);
    double getHeightMax () { assert (has_own_volume_); return own_height_max_; }
    void setHeightMax (double value);

    void addPoint (double latitude, double longitude);
    void addPoints (std::vector <Vector2> points);
    void addPoints (std::string list);
    void clearPoints ();

    const std::map<std::string, AirspaceSector *> &getSubSectors () { return sectors_; }
    void addAllVolumeSectors (std::vector<AirspaceSector *>& sectors); //adds itself as well
    std::vector <Vector2>& getOwnPoints ();

    bool hasSubSector (std::string name);
    AirspaceSector *addNewSubSector (std::string name);
    void removeSubSector (AirspaceSector *sector);
    void deleteSubSector (std::string name);

    bool isPointInside (double latitude, double longitude, bool debug);
    bool isPointInside (double latitude, double longitude, double height_ft, bool debug);

    bool getUsedForChecking () { return used_for_checking_; }
    void setUsedForChecking (bool value) { used_for_checking_=value; }

    const std::vector <std::pair<double, double> > &getHorizontalPoints () { assert (sectors_.size() == 0); return misnomer_.getPoints(); }
    std::vector <std::pair<double, double> > getVerticalPointsLatitude ();
    std::vector <std::pair<double, double> > getVerticalPointsLongitude ();

    double getLatitudeMin () { return latitude_min_; }
    double getLatitudeMax () { return latitude_max_; }

    double getLongitudeMin () { return longitude_min_; }
    double getLongitudeMax () { return longitude_max_; }

    double getAltitudeMin () { return altitude_min_; }
    double getAltitudeMax () { return altitude_max_; }

    double getLatitudeMinRounded ();
    double getLatitudeMaxRounded ();

    double getLongitudeMinRounded ();
    double getLongitudeMaxRounded ();

    std::vector <Vector2> getPointsBetween (double p1_lat, double p1_long, double p2_lat, double p2_long);

    void rebuildSectorNames ();

protected:
    std::string name_;

    bool has_own_volume_;
    std::map <unsigned int, GeographicPoint *> own_points_config_;
    std::vector < Vector2 > own_points_;
    double own_height_min_; // in feet
    double own_height_max_;

    bool used_for_checking_;

    double latitude_min_;
    double latitude_max_;

    double longitude_min_;
    double longitude_max_;

    double altitude_min_;
    double altitude_max_;

    Polygon4DDI misnomer_;

    std::map <std::string, AirspaceSector*> sectors_;

    void update ();
    void updateMinMax ();

    virtual void checkSubConfigurables ();
    //double distanceFromLineSegmentToPoint( const Vector2 v, const Vector2 w, const Vector2 p, Vector2 * const q);
    //double distanceFromLineSegmentToPoint( double segmentX1, double segmentY1, double segmentX2, double segmentY2, double pX, double pY, double *qX, double *qY );
};

#endif /* AIRSPACESECTOR_H_ */
