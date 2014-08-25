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
 * AirspaceSectorManager.cpp
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#include "AirspaceSectorManager.h"
#include "AirspaceSector.h"

#include "String.h"

using namespace Utils::String;
AirspaceSectorManager::AirspaceSectorManager()
: Configurable ("AirspaceSectorManager", "AirspaceSectorManager0", 0, "conf/config_airspace.xml")
{
    createSubConfigurables ();

    createAllSectorsFlat();

}

AirspaceSectorManager::~AirspaceSectorManager()
{
    std::map <std::string, AirspaceSector*>::iterator it;
    for (it = sectors_.begin(); it != sectors_.end(); it++)
        delete it->second;
    sectors_.clear();
}

void AirspaceSectorManager::generateSubConfigurable (std::string class_id, std::string instance_id)
{
    if (class_id.compare ("AirspaceSector") == 0)
    {
        AirspaceSector *sector = new AirspaceSector (class_id, instance_id, this);
        assert (sectors_.find(sector->getName()) == sectors_.end());
        sectors_[sector->getName()]=sector;
    }
    else
        throw std::runtime_error ("AirspaceSectorManager: generateSubConfigurable: unknown class_id "+class_id );
}
void AirspaceSectorManager::checkSubConfigurables ()
{
}

void AirspaceSectorManager::addNewSector (std::string name)
{
    Configuration &configuration = addNewSubConfiguration ("AirspaceSector");
    configuration.addParameterString ("name", name);
    generateSubConfigurable(configuration.getClassId(), configuration.getInstanceId());
}

void AirspaceSectorManager::rebuildSectorNames ()
{
    std::map <std::string, AirspaceSector*> old_sectors = sectors_;
    sectors_.clear();

    std::map <std::string, AirspaceSector*>::iterator it;

    for (it = old_sectors.begin(); it != old_sectors.end(); it++)
    {
        assert (sectors_.find(it->second->getName()) == sectors_.end());
        sectors_[it->second->getName()] = it->second;
    }
}

bool AirspaceSectorManager::deleteSectorIfPossible (std::string name)
{
    if (sectors_.find(name) == sectors_.end())
        return false;

    AirspaceSector *sector = sectors_.at(name);
    sectors_.erase (sectors_.find(name));
    delete sector;

    return true;
}

bool AirspaceSectorManager::deleteSectorIfPossible (AirspaceSector *sector)
{
    assert (sector);
    return deleteSectorIfPossible (sector->getName());
}

void AirspaceSectorManager::removeSector (AirspaceSector *sector)
{
    assert (sectors_.find(sector->getName()) != sectors_.end());
    sectors_.erase (sectors_.find(sector->getName()));
}

void AirspaceSectorManager::createNewSectorFromACGXMLFile (std::string path, std::string sector_name)
{
    loginf << "AirspaceSectorManager: createNewSectorFromACGXMLFile: path " << path << " sector name " << sector_name;

    assert (hasSector(sector_name));
    acg_parser_.parse(path, sectors_.at(sector_name));
}

void AirspaceSectorManager::createNewSectorFromShapefile (std::string path)
{
    loginf << "AirspaceSectorManager: createNewSectorFromShapefile: path " << path;
    shapefile_parser_.parse(path);
}


void AirspaceSectorManager::createAllSectorsFlat ()
{
    all_sectors_flat_.clear();
    point_inside_sectors_.clear();

    std::map <std::string, AirspaceSector*>::iterator it;

    for (it = sectors_.begin(); it != sectors_.end(); it++)
    {
        it->second->addAllVolumeSectors(all_sectors_flat_);
    }

    for (unsigned int cnt=0; cnt < all_sectors_flat_.size(); cnt++)
    {
        std::string name = all_sectors_flat_.at(cnt)->getName();
        assert (point_inside_sectors_.find(name) == point_inside_sectors_.end());
        point_inside_sectors_[name]=false;    }
}

std::map <std::string, bool> &AirspaceSectorManager::isPointInsideSector (double latitude, double longitude, bool height_given, double height_ft,
        const std::map <std::string, bool> &old_insides, bool debug)
{
    assert (all_sectors_flat_.size() != 0);
    assert (all_sectors_flat_.size() == point_inside_sectors_.size());

    AirspaceSector *current_sector;

    bool was_inside_defined;
    bool was_inside;

    for (unsigned int cnt=0; cnt < all_sectors_flat_.size(); cnt++)
    {
        current_sector = all_sectors_flat_.at(cnt);
        assert (current_sector);
        std::string name = current_sector->getName();

        assert (point_inside_sectors_.find(name) != point_inside_sectors_.end());

        was_inside = false;
        was_inside_defined = old_insides.find (name) != old_insides.end();

        if (was_inside_defined)
            was_inside = old_insides.at(name);

//        if (debug)
//            loginf << "ASM: checking sector " << name << " was in sector " << ;

//        if (debug && (name == "ALL" || name == "En-Route"))
//        {
//            if (old_insides.find (name) == old_insides.end())
//                loginf << "ASM: checking sector " << name << " was in sector undefined";
//            else
//                loginf << "ASM: checking sector " << name << " was in sector " << old_insides.at(name);
//        }

        if (!current_sector->getUsedForChecking())
        {
            point_inside_sectors_[name] = false;

//            if (debug && (name == "ALL" || name == "En-Route"))
//                loginf << "ASM: checking sector " << name << " not used for checking; false";
        }
        else
        {
            if (height_given)
            {
                point_inside_sectors_[name] = current_sector->isPointInside(latitude, longitude, height_ft, false);

//                if (debug && (name == "ALL" || name == "En-Route"))
//                    loginf << "ASM: checking sector " << name << " height given, using height; " << point_inside_sectors_[name];
//
//                if (debug && was_inside_defined && was_inside && !point_inside_sectors_[name])
//                {
//                    loginf << "ASM: sector " << name << " change, debugging inside";
//                    bool tmp = current_sector->isPointInside(latitude, longitude, height_ft, true);
//                }
            }
            else
            {
                if (was_inside_defined)
                {
//                    bool was_inside = old_insides.at(name);
                    if (was_inside)
                    {
                        point_inside_sectors_[name] = current_sector->isPointInside(latitude, longitude, false);

//                        if (debug && (name == "ALL" || name == "En-Route"))
//                            loginf << "ASM: checking sector " << name << " height not given, was_inside " << was_inside << "; " << point_inside_sectors_[name];
                    }
                    else
                    {
                        point_inside_sectors_[name]=false;

//                        if (debug && (name == "ALL" || name == "En-Route"))
//                            loginf << "ASM: checking sector " << name << " height not given, was_inside not " << was_inside << "; false";
                    }
                }
                else
                {
                    point_inside_sectors_[name]=false; //TODO HACK
                    //logwrn << "AirspaceSectorManager: isPointInsideSector: cannot check on sector " << name << ", this should be handled";

//                    if (debug && (name == "ALL" || name == "En-Route"))
//                        loginf << "ASM: checking sector " << name << " inside unknown; false";

                }
            }
        }
    }

    return point_inside_sectors_;
}

