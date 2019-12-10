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

#include <fstream>

#include "configurationmanager.h"
#include "configurable.h"
#include "logger.h"
#include "config.h"
#include "global.h"
#include "files.h"
#include "stringconv.h"
#include "json.hpp"

using namespace tinyxml2;
using namespace nlohmann;

using namespace Utils;

/**
 * Loads the main configuration filename from Config and calls parseConfigurationFile.
 */
ConfigurationManager::ConfigurationManager()
    : initialized_(false) //, dummy_configuration_(Configuration ("Dummy", "Dummy0"))
{
}

void ConfigurationManager::init (const std::string& main_config_filename)
{
    assert (!initialized_);
    assert (main_config_filename.size() > 0);

    main_config_filename_ = main_config_filename;
    initialized_ = true;

    std::string path_filename = CURRENT_CONF_DIRECTORY+main_config_filename;
    Files::verifyFileExists(path_filename);

    loginf << "ConfigurationManager: init: opening main configuration file '" << path_filename << "'";
    parseJSONConfigurationFile (path_filename);
}

ConfigurationManager::~ConfigurationManager()
{
    logdbg  << "ConfigurationManager: destructor";
    initialized_ = false;
}

/**
 * Adds Configurable to the root_configurables_ container, return either Configuration from root_configurations_
 * (if exists) or generates a new one.
 */
Configuration& ConfigurationManager::registerRootConfigurable(Configurable& configurable)
{
    assert (initialized_);

    logdbg  << "ConfigurationManager: registerRootConfigurable: " << configurable.instanceId();
    std::pair<std::string, std::string> key (configurable.classId(), configurable.instanceId());
    assert (root_configurables_.find (key) == root_configurables_.end());

    //root_configurables_.insert(key)=configurable;
    root_configurables_.insert( std::pair<std::pair<std::string, std::string>, Configurable &> (key, configurable) );

    if (root_configurations_.find (key) == root_configurations_.end()) // does not exist
    {
        loginf << "ConfigurationManager: getRootConfiguration: creating new configuration for class "
                << configurable.classId() << " instance " << configurable.instanceId();

        root_configurations_.insert( std::pair<std::pair<std::string, std::string>, Configuration>
                                     (key, Configuration (configurable.classId(), configurable.instanceId())) );

    }
    return root_configurations_.at(key);
}

/**
 * Removes configurable from root_configurables_ container.
 */
void ConfigurationManager::unregisterRootConfigurable(Configurable &configurable)
{
    assert (initialized_);

    logdbg  << "ConfigurationManager: unregisterRootConfigurable: " << configurable.instanceId();
    std::pair<std::string, std::string> key (configurable.classId(), configurable.instanceId());
    assert (root_configurables_.find(key) != root_configurables_.end());
    root_configurables_.erase (root_configurables_.find(key));
}

/**
 * Assumes a root Configuration, which is parsed with the appropriate functions.
 */
void ConfigurationManager::parseXMLConfigurationFile (const std::string& filename)
{
    assert (initialized_);

    logdbg  << "ConfigurationManager: parseXMLConfigurationFile: opening '" << filename << "'";
    XMLDocument *config_file_doc = new XMLDocument ();

    Files::verifyFileExists(filename);
    logdbg  << "ConfigurationManager: parseXMLConfigurationFile: opening file '" << filename << "'";

    if (config_file_doc->LoadFile(filename.c_str()) == 0)
    {
        XMLElement *doc_main_conf;
        XMLElement *main_conf_child;

        for ( doc_main_conf = config_file_doc->FirstChildElement(); doc_main_conf != 0;
                doc_main_conf = doc_main_conf->NextSiblingElement())
        {
            assert (strcmp ("Configuration", doc_main_conf->Value() ) == 0);
            logdbg  << "ConfigurationManager: parseXMLConfigurationFile: found Configuration";

            for (main_conf_child = doc_main_conf->FirstChildElement(); main_conf_child != 0;
                 main_conf_child = main_conf_child->NextSiblingElement())
            {
                logdbg  << "ConfigurationManager: parseXMLConfigurationFile: found element '" << main_conf_child->Value() << "'";
                if (strcmp ("SubConfigurationFile", main_conf_child->Value() ) == 0)
                {
                    logdbg  << "ConfigurationManager: parseXMLConfigurationFile: is SubConfigurationFile";

                    std::string class_id;
                    std::string instance_id;
                    std::string path;

                    const XMLAttribute* attribute=main_conf_child->FirstAttribute();
                    while (attribute)
                    {
                        logdbg  << "ConfigurationManager: parseXMLConfigurationFile: attribute '" << attribute->Name()
                                        << "'' value '"<< attribute->Value() << "'";
                        if (strcmp ("class_id", attribute->Name()) == 0)
                            class_id=attribute->Value();
                        else if (strcmp ("instance_id", attribute->Name()) == 0)
                            instance_id=attribute->Value();
                        else if (strcmp ("path", attribute->Name()) == 0)
                            path=attribute->Value();
                        else
                            throw std::runtime_error (std::string ("ConfigurationManager: parseXMLConfigurationFile: unknown attribute ")
                                +attribute->Name());

                        attribute=attribute->Next();
                    }

                    if (class_id.size() && instance_id.size() && path.size())
                    {
                        std::pair<std::string, std::string> key (class_id, instance_id);
                        assert (root_configurations_.find (key) == root_configurations_.end()); // should not exist

                        logdbg << "ConfigurationManager: parseXMLConfigurationFile: creating new configuration for class " << class_id <<
                                " instance " << instance_id;
                        root_configurations_.insert (std::pair<std::pair<std::string, std::string>, Configuration> (key, Configuration (class_id, instance_id)));
                        root_configurations_.at(key).setConfigurationFilename (path);
                        root_configurations_.at(key).parseXMLElement(main_conf_child);
                    }
                    else
                        throw std::runtime_error ("error: ConfigurationManager: parseXMLConfigurationFile: configuration misses attributes");
                }
                else
                {
                    throw std::runtime_error (std::string("ConfigurationManager: parseXMLConfigurationFile: unknown section '")+main_conf_child->Value()+"'");
                }
            }
        }
    }
    else
    {
        logerr << "ConfigurationManager: parseXMLConfigurationFile: could not load file '" << filename<< "'";
        throw std::runtime_error ("ConfigurationManager: parseXMLConfigurationFile: load error");
    }
    logdbg  << "ConfigurationManager: parseXMLConfigurationFile: file '" << filename << "' done";
    delete config_file_doc;
}

void ConfigurationManager::parseJSONConfigurationFile (const std::string& filename)
{
    assert (initialized_);

    logdbg  << "ConfigurationManager: parseJSONConfigurationFile: opening '" << filename << "'";
    //XMLDocument *config_file_doc = new XMLDocument ();

    Files::verifyFileExists(filename);
    logdbg  << "ConfigurationManager: parseJSONConfigurationFile: opening file '" << filename << "'";

    std::ifstream config_file (filename, std::ifstream::in);

    try
    {
        json config = json::parse(config_file);

        assert (config.is_object());

        std::string class_id;
        std::string instance_id;
        std::string path;

        for (auto& it : config.items())
        {
            if (it.key() == "sub_config_files")
            {
                assert (it.value().is_array());

                for (auto& file_cfg_it : it.value().get<json::array_t>())
                {
                    assert (file_cfg_it.contains("class_id"));
                    assert (file_cfg_it.contains("instance_id"));
                    assert (file_cfg_it.contains("path"));

                    class_id = file_cfg_it.at("class_id");
                    instance_id = file_cfg_it.at("instance_id");
                    path = file_cfg_it.at("path");

                    assert (class_id.size() && instance_id.size() && path.size());

                    std::pair<std::string, std::string> key (class_id, instance_id);
                    assert (root_configurations_.find (key) == root_configurations_.end()); // should not exist

                    logdbg << "ConfigurationManager: parseJSONConfigurationFile: creating new configuration for class " << class_id <<
                              " instance " << instance_id;
                    root_configurations_.insert (std::pair<std::pair<std::string, std::string>, Configuration>
                                                 (key, Configuration (class_id, instance_id)));

                    root_configurations_.at(key).setConfigurationFilename (path);
                    root_configurations_.at(key).parseJSONConfigFile();
                }
            }
            else
                throw std::runtime_error ("ConfigurationManager: parseJSONConfigurationFile: unknown key '"
                                          +it.key()+"'");
        }

    }
    catch (json::exception& e)
    {
        logerr << "ConfigurationManager: parseJSONConfigurationFile: could not load file '" << filename << "'";
        throw e;
    }


//    if (config_file_doc->LoadFile(filename.c_str()) == 0)
//    {
//        XMLElement *doc_main_conf;
//        XMLElement *main_conf_child;

//        for ( doc_main_conf = config_file_doc->FirstChildElement(); doc_main_conf != 0;
//                doc_main_conf = doc_main_conf->NextSiblingElement())
//        {
//            assert (strcmp ("Configuration", doc_main_conf->Value() ) == 0);
//            logdbg  << "ConfigurationManager: parseJSONConfigurationFile: found Configuration";

//            for (main_conf_child = doc_main_conf->FirstChildElement(); main_conf_child != 0;
//                 main_conf_child = main_conf_child->NextSiblingElement())
//            {
//                logdbg  << "ConfigurationManager: parseJSONConfigurationFile: found element '" << main_conf_child->Value() << "'";
//                if (strcmp ("SubConfigurationFile", main_conf_child->Value() ) == 0)
//                {
//                    logdbg  << "ConfigurationManager: parseJSONConfigurationFile: is SubConfigurationFile";

//                    std::string class_id;
//                    std::string instance_id;
//                    std::string path;

//                    const XMLAttribute* attribute=main_conf_child->FirstAttribute();
//                    while (attribute)
//                    {
//                        logdbg  << "ConfigurationManager: parseJSONConfigurationFile: attribute '" << attribute->Name()
//                                        << "'' value '"<< attribute->Value() << "'";
//                        if (strcmp ("class_id", attribute->Name()) == 0)
//                            class_id=attribute->Value();
//                        else if (strcmp ("instance_id", attribute->Name()) == 0)
//                            instance_id=attribute->Value();
//                        else if (strcmp ("path", attribute->Name()) == 0)
//                            path=attribute->Value();
//                        else
//                            throw std::runtime_error (std::string ("ConfigurationManager: parseConfigurationFile: unknown attribute ")
//                                +attribute->Name());

//                        attribute=attribute->Next();
//                    }

//                    if (class_id.size() && instance_id.size() && path.size())
//                    {
//                        std::pair<std::string, std::string> key (class_id, instance_id);
//                        assert (root_configurations_.find (key) == root_configurations_.end()); // should not exist

//                        logdbg << "ConfigurationManager: parseJSONConfigurationFile: creating new configuration for class " << class_id <<
//                                " instance " << instance_id;
//                        root_configurations_.insert (std::pair<std::pair<std::string, std::string>, Configuration> (key, Configuration (class_id, instance_id)));
//                        root_configurations_.at(key).setConfigurationFilename (path);
//                        root_configurations_.at(key).parseXMLElement(main_conf_child);
//                    }
//                    else
//                        throw std::runtime_error ("error: ConfigurationManager: parseJSONConfigurationFile: configuration misses attributes");
//                }
//                else
//                {
//                    throw std::runtime_error (std::string("ConfigurationManager: parseJSONConfigurationFile: unknown section '")+main_conf_child->Value()+"'");
//                }
//            }
//        }
//    }
//    else
//    {
//        logerr << "ConfigurationManager: parseJSONConfigurationFile: could not load file '" << filename<< "'";
//        throw std::runtime_error ("ConfigurationManager: parseJSONConfigurationFile: load error");
//    }
//    logdbg  << "ConfigurationManager: parseJSONConfigurationFile: file '" << filename << "' done";
//    delete config_file_doc;
}



void ConfigurationManager::saveConfiguration ()
{
    loginf << "ConfigurationManager: saveConfiguration NOT ACTIVE";
  // TODO deactivated
//    saveXMLConfiguration();
//    saveJSONConfiguration();
}

void ConfigurationManager::saveXMLConfiguration ()
{
    assert (initialized_);

    loginf << "ConfigurationManager: saveXMLConfiguration";

    tinyxml2::XMLDocument *document = new tinyxml2::XMLDocument ();
    XMLDeclaration* decl = document->NewDeclaration( "1.0");
    document->LinkEndChild( decl );

    tinyxml2::XMLElement *root_element = document->NewElement ("Configuration");
    document->LinkEndChild(root_element);

    for (auto it : root_configurables_) //iterate over root configurables
    {
        logdbg << "ConfigurationManager: saveConfiguration: for configurable " << it.first.second;
        root_element->LinkEndChild(it.second.configuration().generateXMLElement(document));
    }

    for (auto it : root_configurations_) // iterate over root configurations
    {
        if (root_configurables_.find (it.first) == root_configurables_.end()) // unused root configuration, not yet in save_info
        {
            logdbg << "ConfigurationManager: saveConfiguration: configuration " << it.second.getInstanceId()
                    << " unused";
            root_element->LinkEndChild(it.second.generateXMLElement(document));
        }
    }

    std::string main_config_path = CURRENT_CONF_DIRECTORY+main_config_filename_;
    loginf  << "ConfigurationManager: saveConfiguration: saving main configuration file '" << main_config_path << "'";
    document->SaveFile(main_config_path.c_str());

    delete document;
}
void ConfigurationManager::saveJSONConfiguration ()
{
    assert (initialized_);

    json main_config;

    loginf << "ConfigurationManager: saveJSONConfiguration";

    for (auto& it : root_configurables_) //iterate over root configurables
    {
        loginf << "ConfigurationManager: saveJSONConfiguration: for configurable " << it.first.second;
        it.second.configuration().generateJSON(main_config);
        //root_element->LinkEndChild(it.second.configuration().generateXMLElement(document));
    }

    for (auto& it : root_configurations_) // iterate over root configurations
    {
        if (root_configurables_.find (it.first) == root_configurables_.end()) // unused root configuration, not yet in save_info
        {
            loginf << "ConfigurationManager: saveJSONConfiguration: configuration " << it.second.getInstanceId()
                    << " unused";
            it.second.generateJSON(main_config);
            //root_element->LinkEndChild(it.second.generateXMLElement(document));
        }
    }

    std::string main_config_path = CURRENT_CONF_DIRECTORY+main_config_filename_;
    //String::replace(main_config_path, ".xml", ".json");

    loginf  << "ConfigurationManager: saveJSONConfiguration: saving main configuration file '" << main_config_path << "'";

    // save file
    std::ofstream file(main_config_path);
    file << main_config.dump(4);

    //document->SaveFile(main_config_path.c_str());
}

