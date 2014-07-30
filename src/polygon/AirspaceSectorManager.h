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
 * AirspaceSectorManager.h
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#ifndef AIRSPACESECTORMANAGER_H_
#define AIRSPACESECTORMANAGER_H_

#include "Singleton.h"
#include "Configurable.h"

#include "gdal_priv.h"
#include "ogrsf_frmts.h"

class AirspaceSector;

namespace Check
{
    class TrackStatistic;
};


class AirspaceSectorManager : public Singleton, public Configurable
{
public:
    virtual ~AirspaceSectorManager();

    virtual void generateSubConfigurable (std::string class_id, std::string instance_id);

    const std::map <std::string, AirspaceSector*> &getSectors () { return sectors_; }

    void addNewSector (std::string name);

    bool deleteSectorIfPossible (AirspaceSector *sector);
    void removeSector (AirspaceSector *sector);

    void createNewSectorFromShapefile (std::string path);

    std::map <std::string, bool> &isPointInsideSector (double latitude, double longitude, bool height_given,
            double height_ft, const std::map <std::string, bool> &old_insides, bool debug);
    //not thread safe!

    Check::TrackStatistic *getStatistic (std::string name);

    bool hasSector (std::string name) { return sectors_.find(name) != sectors_.end(); }
    AirspaceSector *getSector (std::string name) { assert (hasSector(name)); return sectors_[name]; }

    void rebuildSectorNames ();

protected:
    std::map <std::string, AirspaceSector*> sectors_;

    std::vector <AirspaceSector*> all_sectors_flat_;
    std::map <std::string, bool> point_inside_sectors_;

    AirspaceSectorManager();

    virtual void checkSubConfigurables ();

    void addLineString( const OGRLineString* obj, AirspaceSector* sector, bool new_line );
    void addMultiLineString( const OGRMultiLineString* obj, AirspaceSector* sector );
    void addLinearRing( const OGRLinearRing* obj, AirspaceSector* sector, bool new_line );
    void addPoly( OGRPolygon* obj, AirspaceSector* sector );
    void addMultiPoly( OGRMultiPolygon* obj, AirspaceSector* sector );
    void addPoint( OGRPoint* obj, AirspaceSector* sector );
    void addMultiPoint( OGRMultiPoint* obj, AirspaceSector* sector );

    void createAllSectorsFlat ();

public:
    static AirspaceSectorManager& getInstance()
    {
        static AirspaceSectorManager instance;
        return instance;
    }
};

#endif /* AIRSPACESECTORMANAGER_H_ */
