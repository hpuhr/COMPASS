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
 * AirspaceSector.cpp
 *
 *  Created on: Nov 25, 2013
 *      Author: sk
 */

#include "AirspaceSector.h"
#include "GeographicPoint.h"
#include "Configuration.h"
#include "AirspaceSectorManager.h"

#include <algorithm>
#include "String.h"

using namespace Utils::String;

AirspaceSector::AirspaceSector(std::string class_id, std::string instance_id, Configurable *parent)
 : Configurable(class_id, instance_id, parent)
{
    registerParameter("name", &name_, "");
    registerParameter("own_volume", &has_own_volume_, false);
    registerParameter("own_height_min", &own_height_min_, -1.0);
    registerParameter("own_height_max", &own_height_max_, -1.0);
    registerParameter ("used_for_checking", &used_for_checking_, false);

    if (name_.size() == 0)
    {
        logerr << "AirspaceSector: constructor: name is null, setting to instance id '" << instance_id << "'";
        name_=instance_id;
    }
    misnomer_.setAltitudeMinMax(own_height_min_, own_height_max_);

    createSubConfigurables ();

    update();
}

AirspaceSector::~AirspaceSector()
{
    std::vector<AirspaceSector *>::iterator it;
    for (it = sub_sectors_.begin(); it != sub_sectors_.end(); it++)
        delete *it;
    sub_sectors_.clear();

    clearPoints ();
}

void AirspaceSector::checkSubConfigurables ()
{

}

void AirspaceSector::generateSubConfigurable (std::string class_id, std::string instance_id)
{
    if (class_id.compare ("AirspaceSector") == 0)
    {
        AirspaceSector *sector = new AirspaceSector (class_id, instance_id, this);
        //assert (sub_sectors_.find (sector->getName()) == sub_sectors_.end());
        sub_sectors_.push_back(sector);
        //loginf << "AirspaceSector: generateSubConfigurable: name " << name_ << " has new sub sector " << sector->getName();
    }
    else if (class_id.compare ("GeographicPoint") == 0)
    {
        GeographicPoint *point = new GeographicPoint (class_id, instance_id, this);
        assert (own_points_config_.find(point->getIndex()) == own_points_config_.end());
        own_points_config_[point->getIndex()] = point;
        //own_points_config_.push_back (point);
        //own_points_.push_back (std::pair<double, double> (point->getLatitude(), point->getLongitude()));
        //misnomer_.addPoint(point->getLatitude(), point->getLongitude());
    }
    else
        throw std::runtime_error ("AirspaceSectorManager: generateSubConfigurable: unknown class_id "+class_id );
}

void AirspaceSector::update ()
{
    misnomer_.clearPoints();
    own_points_.clear();

    std::map <unsigned int, GeographicPoint *>::iterator it;

    for (it = own_points_config_.begin(); it != own_points_config_.end(); it++)
    {
        own_points_.push_back (std::pair<double, double> (it->second->getLatitude(), it->second->getLongitude()));
        misnomer_.addPoint(it->second->getLatitude(), it->second->getLongitude());
    }

}

void AirspaceSector::addPoint (double latitude, double longitude)
{
    has_own_volume_=true;
    Configuration &configuration = addNewSubConfiguration ("GeographicPoint");
    configuration.addParameterDouble ("latitude", latitude);
    configuration.addParameterDouble ("longitude", longitude);
    configuration.addParameterUnsignedInt ("index", own_points_config_.size());
    generateSubConfigurable(configuration.getClassId(), configuration.getInstanceId());

    update();
}

void AirspaceSector::addPoints (std::string list)
{
    assert (has_own_volume_);

    list.erase (std::remove(list.begin(), list.end(), ' '), list.end()); //remove blanks

    std::vector<std::string>line_splits =  split(list, '\n');

    std::vector<std::string>::iterator it;
    std::vector<std::string>::iterator it2;

    std::string latitude_str, longitude_str;

    for (it = line_splits.begin(); it != line_splits.end(); it++)
    {
        std::vector<std::string>number_splits =  split(*it, ',');

        loginf << "AirspaceSector: addPoints: split '" << *it << "' into " << number_splits.size() << " pieces";

        if (number_splits.size() % 2 == 1)
        {
            logerr << "AirspaceSector: addPoints: parsing error at '" << *it << "'";
            return;
        }

        for (it2 = number_splits.begin(); it2 != number_splits.end(); it2 += 2)
        {
            latitude_str = *it2;
            longitude_str = *(it2+1);
            addPoint (doubleFromString(latitude_str), doubleFromString(longitude_str));
        }
    }

    update();
}

void AirspaceSector::clearPoints ()
{
    std::map <unsigned int, GeographicPoint *>::iterator it;

    for (it = own_points_config_.begin(); it != own_points_config_.end(); it++)
    {
        delete it->second;
    }
    own_points_config_.clear();
    own_points_.clear();
    misnomer_.clearPoints();
}

AirspaceSector *AirspaceSector::addNewSubSector (std::string name)
{
    Configuration &configuration = addNewSubConfiguration ("AirspaceSector");
    configuration.addParameterString ("name", name);
    generateSubConfigurable(configuration.getClassId(), configuration.getInstanceId());
    return sub_sectors_.back();
}

void AirspaceSector::removeSubSector (AirspaceSector *sector)
{
    assert (find (sub_sectors_.begin(), sub_sectors_.end(), sector) != sub_sectors_.end());
    sub_sectors_.erase(find (sub_sectors_.begin(), sub_sectors_.end(), sector));
}

void AirspaceSector::addAllVolumeSectors (std::vector<AirspaceSector *>& sectors)
{
    if (has_own_volume_)
    {
        assert (find (sectors.begin(), sectors.end(), this) == sectors.end());
        sectors.push_back(this);
    }

    std::vector<AirspaceSector *>::iterator it;
    for (it = sub_sectors_.begin(); it != sub_sectors_.end(); it++)
        (*it)->addAllVolumeSectors(sectors);
}

std::vector < std::pair<double, double> >& AirspaceSector::getOwnPoints ()
{
    assert (has_own_volume_);

//    if (own_points_.size() != own_points_config_.size())
//    {
//        own_points_.clear();
//        std::map <unsigned int, GeographicPoint *>::iterator it;
//
//        for (it = own_points_config_.begin(); it != own_points_config_.end(); it++)
//            own_points_.push_back (std::pair<double, double> (it->second->getLatitude(), it->second->getLongitude()));
//    }

    return own_points_;
}

void AirspaceSector::setHeightMin (double value)
{
    own_height_min_=value;
    misnomer_.setAltitudeMinMax(own_height_min_, own_height_max_);
}

void AirspaceSector::setHeightMax (double value)
{
    own_height_max_=value;
    misnomer_.setAltitudeMinMax(own_height_min_, own_height_max_);
}

bool AirspaceSector::isPointInside (double latitude, double longitude, double height_ft, bool debug)
{
    if (!misnomer_.isFinalized())
        misnomer_.finalize(name_);

    return misnomer_.inside(latitude, longitude, height_ft, debug);
}

bool AirspaceSector::isPointInside (double latitude, double longitude, bool debug)
{
    if (!misnomer_.isFinalized())
        misnomer_.finalize(name_);

    return misnomer_.inside(latitude, longitude, debug);
}

void AirspaceSector::setName (std::string name)
{
    name_ = name;

    AirspaceSectorManager::getInstance().rebuildSectorNames();
}
