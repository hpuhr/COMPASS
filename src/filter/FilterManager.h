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
 * FilterManager.h
 *
 *  Created on: Jan 15, 2012
 *      Author: sk
 */

#ifndef FILTERMANAGER_H_
#define FILTERMANAGER_H_

#include <map>
#include <vector>
#include <string>
#include "Singleton.h"
#include "Global.h"
#include "Configurable.h"

class DBFilter;


/**
 * @brief Manages all filters and generates SQL conditions
 *
 * Generates DBFilters from configuration and SensorFilters for all DBOs with data and data sources.
 * GUI classes operate on this class for setting and retrieval of the filter configuration.
 * Other modules can retrieve the filter SQL conditions when a loading process is triggered.
 *
 * \todo Generalize SQL condition w.r.t. RDL schema
 */
class FilterManager : public Singleton, public Configurable
{
public:
    /// @brief Destructor
    virtual ~FilterManager();

    /// @brief Returns flag if an active filter was changed
    bool getChanged ();
    /// @brief Sets the changed flag
    void setChanged ();
    /// @brief Clears the changed flag
    void clearChanged ();

    /// @brief Returns of data of a DBO should be loaded
    bool getLoad (DB_OBJECT_TYPE type);
    /// @brief Sets if data of a DBO should be loaded
    void setLoad (DB_OBJECT_TYPE type, bool show);

    /// @brief Returns the SQL condition for a DBO and sets all used variable names
    std::string getSQLCondition (DB_OBJECT_TYPE type, std::vector<std::string> &variable_names);

    /// @brief Returns number of existing filters
    unsigned int getNumFilters ();
    /// @brief Returns filter at a given index
    DBFilter *getFilter (unsigned int index);

    virtual void generateSubConfigurable (std::string class_id, std::string instance_id);

    /// @brief Resets all filters
    void reset ();

    /// @brief Deletes a given filter
    void deleteFilter (DBFilter *filter);

protected:
    /// Container with all load flags
    std::map<DB_OBJECT_TYPE, bool*> load_;
    /// Changed flag
    bool changed_;
    /// Database definition, resets if changed
    std::string db_id_;
    /// Container with all DBFilters
    std::vector <DBFilter*> filters_;

    /// @brief Constructor
    FilterManager();
    /// @brief Returns the SQL condition for a DBO and sets all used variable names
    std::string getActiveFilterSQLCondition (DB_OBJECT_TYPE type, std::vector<std::string> &variable_names);

    virtual void checkSubConfigurables ();

public:
    static FilterManager& getInstance()
    {
        static FilterManager instance;
        return instance;
    }
};

#endif /* FILTERMANAGER_H_ */
