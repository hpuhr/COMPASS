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

#ifndef UNITMANAGER_H_
#define UNITMANAGER_H_

#include "configurable.h"
#include "singleton.h"

class Quantity;

/**
 * @brief Holds and manages all units
 *
 * All units have to call registerUnit.
 */
class UnitManager : public Configurable, public Singleton
{
public:
    /// @brief Destructor
    virtual ~UnitManager();

    bool hasQuanity (const std::string &name) { return quantities_.count(name) > 0; }

    /// @brief Returns unit with a given name
    const Quantity &quanity (const std::string &name) { return *quantities_.at(name); }
    /// @brief Return container with all units
    const std::map <std::string, Quantity*> &quantities () { return quantities_; }

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

protected:
    /// Container with all units (unit name (length, time) -> unit)
    std::map <std::string, Quantity*> quantities_;

    virtual void checkSubConfigurables ();

    /// @brief Constructor
    UnitManager();

public:
    static UnitManager& getInstance()
    {
        static UnitManager instance;
        return instance;
    }
};

#endif /* UNITMANAGER_H_ */
