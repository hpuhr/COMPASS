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
#include "quantity.h"
#include "logger.h"

UnitManager::UnitManager()
: Configurable ("UnitManager", "UnitManager0", 0, "conf/units.xml")
{
    createSubConfigurables ();
}

UnitManager::~UnitManager()
{
  for (auto it : quantities_)
    delete it.second;
  quantities_.clear();
}

void UnitManager::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id == "Quantity")
    {
        Quantity *quantity = new Quantity (class_id, instance_id, this);
        assert (quantities_.find(quantity->getInstanceId()) == quantities_.end());
        quantities_.insert (std::pair <std::string, Quantity*> (quantity->getInstanceId(), quantity));
    }
    else
        throw std::runtime_error ("UnitManager: generateSubConfigurable: unknown class_id "+class_id );
}

void UnitManager::checkSubConfigurables ()
{
    if (quantities_.count("Length") == 0)
    {
        addNewSubConfiguration ("Quantity", "Length");
        generateSubConfigurable("Quantity", "Length");

        quantities_.at("Length")->addUnit ("Meter", 1.0, "");
        quantities_.at("Length")->addUnit ("Kilometer", 1.0/1000.0, "");
        quantities_.at("Length")->addUnit ("Mile", 1.0/1609.344, "");
        quantities_.at("Length")->addUnit ("Nautical Mile", 1.0/1852.0, "");
    }

    if (quantities_.count("Time") == 0)
    {
        addNewSubConfiguration ("Quantity", "Time");
        generateSubConfigurable("Quantity", "Time");

        quantities_.at("Time")->addUnit ("Second", 1.0, "");
        quantities_.at("Time")->addUnit ("Minute", 1.0/60.0, "");
        quantities_.at("Time")->addUnit ("Hour", 1.0/3600.0, "");
        quantities_.at("Time")->addUnit ("MilliSeconds", 1000.0, "");
        quantities_.at("Time")->addUnit ("V7Time", 128.0, "");
        quantities_.at("Time")->addUnit ("V6Time", 4096.0, "");
    }
}




