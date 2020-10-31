/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "configurationmanager.h"

#include <fstream>

#include "config.h"
#include "configurable.h"
#include "files.h"
#include "global.h"
#include "json.hpp"
#include "logger.h"
#include "stringconv.h"

// using namespace tinyxml2;
using namespace nlohmann;

using namespace Utils;

/**
 * Loads the main configuration filename from Config and calls parseConfigurationFile.
 */
ConfigurationManager::ConfigurationManager()
    : initialized_(false)  //, dummy_configuration_(Configuration ("Dummy", "Dummy0"))
{
}

void ConfigurationManager::init(const std::string& main_config_filename)
{
    assert(!initialized_);
    assert(main_config_filename.size() > 0);

    main_config_filename_ = main_config_filename;
    initialized_ = true;

    std::string path_filename = CURRENT_CONF_DIRECTORY + main_config_filename;
    Files::verifyFileExists(path_filename);

    loginf << "ConfigurationManager: init: opening main configuration file '" << path_filename
           << "'";
    parseJSONConfigurationFile(path_filename);
}

ConfigurationManager::~ConfigurationManager()
{
    logdbg << "ConfigurationManager: destructor";
    initialized_ = false;
}

/**
 * Adds Configurable to the root_configurables_ container, return either Configuration from
 * root_configurations_ (if exists) or generates a new one.
 */
Configuration& ConfigurationManager::registerRootConfigurable(Configurable& configurable)
{
    assert(initialized_);

    logdbg << "ConfigurationManager: registerRootConfigurable: " << configurable.instanceId();
    std::pair<std::string, std::string> key(configurable.classId(), configurable.instanceId());
    assert(root_configurables_.find(key) == root_configurables_.end());

    // root_configurables_.insert(key)=configurable;
    root_configurables_.insert(
        std::pair<std::pair<std::string, std::string>, Configurable&>(key, configurable));

    if (root_configurations_.find(key) == root_configurations_.end())  // does not exist
    {
        loginf
            << "ConfigurationManager: getRootConfiguration: creating new configuration for class "
            << configurable.classId() << " instance " << configurable.instanceId();

        root_configurations_.insert(std::pair<std::pair<std::string, std::string>, Configuration>(
            key, Configuration(configurable.classId(), configurable.instanceId())));
    }
    return root_configurations_.at(key);
}

/**
 * Removes configurable from root_configurables_ container.
 */
void ConfigurationManager::unregisterRootConfigurable(Configurable& configurable)
{
    assert(initialized_);

    logdbg << "ConfigurationManager: unregisterRootConfigurable: " << configurable.instanceId();
    std::pair<std::string, std::string> key(configurable.classId(), configurable.instanceId());
    assert(root_configurables_.find(key) != root_configurables_.end());
    root_configurables_.erase(root_configurables_.find(key));
}

void ConfigurationManager::parseJSONConfigurationFile(const std::string& filename)
{
    assert(initialized_);

    logdbg << "ConfigurationManager: parseJSONConfigurationFile: opening '" << filename << "'";
    // XMLDocument *config_file_doc = new XMLDocument ();

    Files::verifyFileExists(filename);
    logdbg << "ConfigurationManager: parseJSONConfigurationFile: opening file '" << filename << "'";

    std::ifstream config_file(filename, std::ifstream::in);

    try
    {
        json config = json::parse(config_file);

        assert(config.is_object());

        std::string class_id;
        std::string instance_id;
        std::string path;

        for (auto& it : config.items())
        {
            if (it.key() == "sub_config_files")
            {
                assert(it.value().is_array());

                for (auto& file_cfg_it : it.value().get<json::array_t>())
                {
                    assert(file_cfg_it.contains("class_id"));
                    assert(file_cfg_it.contains("instance_id"));
                    assert(file_cfg_it.contains("path"));

                    class_id = file_cfg_it.at("class_id");
                    instance_id = file_cfg_it.at("instance_id");
                    path = file_cfg_it.at("path");

                    assert(class_id.size() && instance_id.size() && path.size());

                    std::pair<std::string, std::string> key(class_id, instance_id);
                    assert(root_configurations_.find(key) ==
                           root_configurations_.end());  // should not exist

                    logdbg << "ConfigurationManager: parseJSONConfigurationFile: creating new "
                              "configuration for class "
                           << class_id << " instance " << instance_id;
                    root_configurations_.insert(
                        std::pair<std::pair<std::string, std::string>, Configuration>(
                            key, Configuration(class_id, instance_id)));

                    root_configurations_.at(key).setConfigurationFilename(path);
                    root_configurations_.at(key).parseJSONConfigFile();
                }
            }
            else
                throw std::runtime_error(
                    "ConfigurationManager: parseJSONConfigurationFile: unknown key '" + it.key() +
                    "'");
        }
    }
    catch (json::exception& e)
    {
        logerr << "ConfigurationManager: parseJSONConfigurationFile: could not load file '"
               << filename << "'";
        throw e;
    }
}

void ConfigurationManager::saveConfiguration()
{
    loginf << "ConfigurationManager: saveConfiguration";
    saveJSONConfiguration();
}

void ConfigurationManager::saveJSONConfiguration()
{
    assert(initialized_);

    json main_config;

    logdbg << "ConfigurationManager: saveJSONConfiguration";

    for (auto& it : root_configurables_)  // iterate over root configurables
    {
        loginf << "ConfigurationManager: saveJSONConfiguration: for configurable "
               << it.first.second;
        it.second.configuration().writeJSON(main_config);
        // root_element->LinkEndChild(it.second.configuration().generateXMLElement(document));
    }

    for (auto& it : root_configurations_)  // iterate over root configurations
    {
        if (root_configurables_.find(it.first) ==
            root_configurables_.end())  // unused root configuration, not yet in save_info
        {
            loginf << "ConfigurationManager: saveJSONConfiguration: configuration "
                   << it.second.getInstanceId() << " unused";
            it.second.writeJSON(main_config);
            // root_element->LinkEndChild(it.second.generateXMLElement(document));
        }
    }

    std::string main_config_path = CURRENT_CONF_DIRECTORY + main_config_filename_;
    // String::replace(main_config_path, ".xml", ".json");

    loginf << "ConfigurationManager: saveJSONConfiguration: saving main configuration file '"
           << main_config_path << "'";

    // save file
    std::ofstream file(main_config_path);
    file << main_config.dump(4);

    // document->SaveFile(main_config_path.c_str());
}
