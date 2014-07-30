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
 * ConfigurationManager.h
 *
 *  Created on: May 15, 2012
 *      Author: sk
 */

#ifndef CONFIGURATIONMANAGER_H_
#define CONFIGURATIONMANAGER_H_

#include <tinyxml2.h>
#include <vector>
#include <map>
#include "Singleton.h"
#include "Configuration.h"

class Configurable;

/**
 * @brief Main class for configuration loading, generating and writing.
 *
 * @details Singleton, parses main configuration file using parseConfigurationFile. Each XML configuration file has to consist
 * of a file section (where additional configuration files are defined) and configuration section where all configurations
 * and their sub-configurations are defined.
 *
 * Holds Configurations for all root configurables (no parent). When such a root registers itself, it receives its
 * configuration reference, which is iteratively passed on to its children. When all configurations should be saved,
 * all root configurables pass on their configuration, and unused (root) configurations are saved unmodified.
 *
 */
class ConfigurationManager : public Singleton
{
public:
    /// @brief Destructor
    virtual ~ConfigurationManager();

    /// @brief Registers a configurable as root (no parent)
    Configuration &registerRootConfigurable(Configurable *configurable);
    /// @brief Unregisters a configurable as root
    void unregisterRootConfigurable(Configurable *configurable);

    /// @brief Saves the current configuration
    void saveConfiguration ();

    /// @brief Returns singleton instance
    static ConfigurationManager& getInstance()
    {
        static ConfigurationManager instance;
        return instance;
    }

    /// @brief Returns a dummy configuration which is discarded
    Configuration &getDummyConfiguration () { return dummy_configuration_; }

protected:
    /// Container with all root configurables (class id, instance id) -> Configurable
    std::map <std::pair<std::string, std::string>, Configurable *> root_configurables_;
    /// Container with all root configurations (class id, instance id) -> Configuration
    std::map <std::pair<std::string, std::string>, Configuration> root_configurations_;
    Configuration dummy_configuration_;

    /// @brief Constructor
    ConfigurationManager();

    /// @brief Parses a configuration file
    void parseConfigurationFile (std::string filename);
    /// @brief Parses a file section
    void parseFileSection (tinyxml2::XMLElement *configuration_section_element);
    /// @brief Parses a configuration section
    void parseConfigurationSection (tinyxml2::XMLElement *configuration_section_element, std::string filename);
};

#endif /* CONFIGURATIONMANAGER_H_ */
