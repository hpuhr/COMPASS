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

#include "unitmanager.h"
#include "dimension.h"
#include "logger.h"

UnitManager::UnitManager()
: Configurable ("UnitManager", "UnitManager0", 0, "units.xml")
{
    createSubConfigurables ();
}

UnitManager::~UnitManager()
{
  for (auto it : dimensions_)
    delete it.second;
  dimensions_.clear();
}

void UnitManager::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id == "Dimension")
    {
        Dimension *dimension = new Dimension (class_id, instance_id, this);
        assert (dimensions_.find(dimension->getInstanceId()) == dimensions_.end());
        dimensions_.insert (std::pair <std::string, Dimension*> (dimension->getInstanceId(), dimension));
    }
    else
        throw std::runtime_error ("UnitManager: generateSubConfigurable: unknown class_id "+class_id );
}

void UnitManager::checkSubConfigurables ()
{
    if (dimensions_.count("Length") == 0)
    {
        addNewSubConfiguration ("Dimension", "Length");
        generateSubConfigurable("Dimension", "Length");

        dimensions_.at("Length")->addUnit ("Meter", 1.0, "");
        dimensions_.at("Length")->addUnit ("Kilometer", 1.0/1000.0, "");
        dimensions_.at("Length")->addUnit ("Mile", 1.0/1609.344, "");
        dimensions_.at("Length")->addUnit ("Nautical Mile", 1.0/1852.0, "");
    }

    if (dimensions_.count("Time") == 0)
    {
        addNewSubConfiguration ("Dimension", "Time");
        generateSubConfigurable("Dimension", "Time");

        dimensions_.at("Time")->addUnit ("Second", 1.0, "");
        dimensions_.at("Time")->addUnit ("Minute", 1.0/60.0, "");
        dimensions_.at("Time")->addUnit ("Hour", 1.0/3600.0, "");
        dimensions_.at("Time")->addUnit ("MilliSeconds", 1000.0, "");
        dimensions_.at("Time")->addUnit ("V7Time", 128.0, "");
        dimensions_.at("Time")->addUnit ("V6Time", 4096.0, "");
    }

    if (dimensions_.count("Position") == 0)
    {
        addNewSubConfiguration ("Dimension", "Position");
        generateSubConfigurable("Dimension", "Position");

        dimensions_.at("Position")->addUnit ("Meter", 1.0, "");
        dimensions_.at("Position")->addUnit ("Kilometer", 1.0/1000.0, "");
        dimensions_.at("Position")->addUnit ("Mile", 1.0/1609.344, "");
        dimensions_.at("Position")->addUnit ("Nautical Mile", 1.0/1852.0, "");
        dimensions_.at("Position")->addUnit ("WGS 84", 0, "");
    }
}




