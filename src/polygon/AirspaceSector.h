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

class GeographicPoint;

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

    double getHeightMin () { return own_height_min_; } // in feet
    void setHeightMin (double value);
    double getHeightMax () { return own_height_max_; }
    void setHeightMax (double value);

    void addPoint (double latitude, double longitude);
    void addPoints (std::string list);
    void clearPoints ();

    std::vector<AirspaceSector *> &getSubSectors () { return sub_sectors_; }
    void addAllVolumeSectors (std::vector<AirspaceSector *>& sectors); //adds itself as well
    std::vector < std::pair<double, double> >& getOwnPoints ();

    AirspaceSector *addNewSubSector (std::string name);
    void removeSubSector (AirspaceSector *sector);

    bool isPointInside (double latitude, double longitude, bool debug);
    bool isPointInside (double latitude, double longitude, double height_ft, bool debug);

    bool getUsedForChecking () { return used_for_checking_; }
    void setUsedForChecking (bool value) { used_for_checking_=value; }

    const std::vector <std::pair<double, double> > &getPoints () { return misnomer_.getPoints(); }

    double getLatitudeMinRounded () { return misnomer_.getLatitudeMinRounded(); }
    double getLatitudeMaxRounded () { return misnomer_.getLatitudeMaxRounded(); }

    double getLongitudeMinRounded () { return misnomer_.getLongitudeMinRounded(); }
    double getLongitudeMaxRounded () { return misnomer_.getLongitudeMaxRounded(); }

protected:
    std::string name_;

    bool has_own_volume_;
    std::map <unsigned int, GeographicPoint *> own_points_config_;
    std::vector < std::pair<double, double> > own_points_;
    double own_height_min_; // in feet
    double own_height_max_;

    bool used_for_checking_;

    Polygon4DDI misnomer_;

    std::vector<AirspaceSector *> sub_sectors_;

    void update ();

    virtual void checkSubConfigurables ();
};

#endif /* AIRSPACESECTOR_H_ */
