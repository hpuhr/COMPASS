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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <fstream>
#include <string>
#include <map>
#include "Singleton.h"


/**
 * @brief Provides access to a simple configuration file
 *
 * Configuration file singleton for reading a file containing string identifiers and values
 */
class Config : public Singleton
{
protected:
    bool opened_;
    std::string config_filename_;

    /// Container with all defined identifier -> value pairings
    std::map<std::string,std::string> config_;

    /// @brief Constructor
    Config ();
    /// @brief Returns if a given identifier exists
    bool existsId (std::string id);
    /// @brief Initialises the config class and reads file given by filename
    void loadFile ();

public:
    void init (std::string config_filename);


    /// @brief  Gets value from map by id string and converts it to int
    void getValue (std::string id, int* value);
    /// @brief  Gets value from map by id string and converts it to unsigned int
    void getValue (std::string id, unsigned int* value);
    /// @brief  Gets value from map by id string and converts it to double
    void getValue (std::string id, double* value);
    /// @brief  Gets value from map by id string and doesn't convert it
    void getValue (std::string id, std::string* value);
    /// @brief  Gets value from map by id string and converts it to float
    void getValue (std::string id, float* value);
    /// @brief  Gets value from map by id string and converts it to bool
    void getValue (std::string id, bool* value);

    /// @brief Destructor
    virtual ~Config();

    /// @brief Returns singleton instance. Important: Call init before getting any values
    static Config& getInstance()
    {
        static Config instance;
        return instance;
    }
};

#endif
