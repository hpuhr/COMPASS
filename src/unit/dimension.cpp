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

#include "unit.h"
#include "dimension.h"


Dimension::Dimension(const std::string &class_id, const std::string &instance_id, Configurable *parent)
    : Configurable (class_id, instance_id, parent)
{
    createSubConfigurables();
}

Dimension::~Dimension()
{
    for (auto it : units_)
        delete it.second;
    units_.clear();
}

void Dimension::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id == "Unit")
    {
        Unit *unit = new Unit (class_id, instance_id, *this);
        assert (units_.find(unit->getInstanceId()) == units_.end());
        units_.insert (std::pair <std::string, Unit*> (unit->getInstanceId(), unit));

    }
    else
        throw std::runtime_error ("UnitManager: generateSubConfigurable: unknown class_id "+class_id );
}

void Dimension::addUnit (const std::string &name, double factor, const std::string &definition)
{
    Configuration &config = addNewSubConfiguration ("Unit", name);
    config.addParameterDouble ("factor", factor);
    config.addParameterString ("definition", definition);
    generateSubConfigurable("Unit", name);
}

double Dimension::getFactor (const std::string &unit_source, const std::string &unit_destination) const
{
    logdbg << "Dimension: getFactor: unit src '" << unit_source << "' dst '" << unit_destination << "'";

    assert (units_.find(unit_source) != units_.end());
    assert (units_.find(unit_destination) != units_.end());
    double factor = 1.0;
    logdbg << "Dimension: getFactor: src factor " << units_.at(unit_source)->factor();
    factor /= units_.at(unit_source)->factor();
    logdbg << "Dimension: getFactor: dest factor " << units_.at(unit_destination)->factor();
    factor *= units_.at(unit_destination)->factor();
    return factor;
}
