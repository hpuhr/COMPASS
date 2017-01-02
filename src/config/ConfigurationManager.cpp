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
 * ConfigurationManager.cpp
 *
 *  Created on: May 15, 2012
 *      Author: sk
 */

#include "ConfigurationManager.h"
#include "Configurable.h"
#include "Logger.h"
#include "Config.h"

#include <fstream>

using namespace tinyxml2;

/**
 * Loads the main configuration filename from Config and calls parseConfigurationFile.
 */
ConfigurationManager::ConfigurationManager()
    : initialized_(false) //, dummy_configuration_(Configuration ("Dummy", "Dummy0"))
{
}

void ConfigurationManager::init (const std::string &main_config_filename)
{
    assert (!initialized_);
    assert (main_config_filename.size() > 0);

    main_config_filename_ = main_config_filename;
    initialized_ = true;

    std::ifstream grab(main_config_filename.c_str());

    //check file exists
    if (grab)
    {
        loginf  << "ConfigurationManager: init: parsing main configuration file '" << main_config_filename_ << "'";
        parseConfigurationFile (main_config_filename_);
    }
    else
        logwrn << "ConfigurationManager::init: main configuration file does not exist";

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
Configuration &ConfigurationManager::registerRootConfigurable(Configurable &configurable)
{
    assert (initialized_);

    logdbg  << "ConfigurationManager: registerRootConfigurable: " << configurable.getInstanceId();
    std::pair<std::string, std::string> key (configurable.getClassId(), configurable.getInstanceId());
    assert (root_configurables_.find (key) == root_configurables_.end());

    //root_configurables_.insert(key)=configurable;
    root_configurables_.insert( std::pair<std::pair<std::string, std::string>, Configurable &> (key, configurable) );

    if (root_configurations_.find (key) == root_configurations_.end()) // does not exist
    {
        loginf << "ConfigurationManager: getRootConfiguration: creating new configuration for class "
                << configurable.getClassId() << " instance " << configurable.getInstanceId();
        //root_configurations_.insert(key) = Configuration (configurable.getClassId(), configurable.getInstanceId());
        root_configurations_.insert( std::pair<std::pair<std::string, std::string>, Configuration>
                                     (key, Configuration (configurable.getClassId(), configurable.getInstanceId())) );

    }
    return root_configurations_.at(key);
}

/**
 * Removes configurable from root_configurables_ container.
 */
void ConfigurationManager::unregisterRootConfigurable(Configurable &configurable)
{
    assert (initialized_);

    logdbg  << "ConfigurationManager: unregisterRootConfigurable: " << configurable.getInstanceId();
    std::pair<std::string, std::string> key (configurable.getClassId(), configurable.getInstanceId());
    assert (root_configurables_.find(key) != root_configurables_.end());
    root_configurables_.erase (root_configurables_.find(key));
}

/**
 * Assumes a root Configuration, with either a ConfigurationSection or a FileSection, which are
 * parsed with the appropriate functions.
 */
void ConfigurationManager::parseConfigurationFile (std::string filename)
{
    assert (initialized_);

    logdbg  << "ConfigurationManager: parseConfigurationFile: opening '" << filename << "'";
    XMLDocument *config_file_doc = new XMLDocument ();

    if (config_file_doc->LoadFile(filename.c_str()) == 0)
    {
        logdbg  << "ConfigurationManager: parseConfigurationFile: file '" << filename << "' opened";

        XMLElement *doc_main_conf;
        XMLElement *main_conf_child;

        for ( doc_main_conf = config_file_doc->FirstChildElement(); doc_main_conf != 0;
                doc_main_conf = doc_main_conf->NextSiblingElement())
        {
            assert (strcmp ("Configuration", doc_main_conf->Value() ) == 0);
            logdbg  << "ConfigurationManager: parseConfigurationFile: found Configuration";

            for (main_conf_child = doc_main_conf->FirstChildElement(); main_conf_child != 0; main_conf_child = main_conf_child->NextSiblingElement())
            {
                logdbg  << "ConfigurationManager: parseConfigurationFile: found element '" << main_conf_child->Value() << "'";
                if (strcmp ("ConfigurationSection", main_conf_child->Value() ) == 0)
                {
                    logdbg  << "ConfigurationManager: parseConfigurationFile: is ConfigurationSection";
                    parseConfigurationSection (main_conf_child, filename);
                }
                else if (strcmp ("FileSection", main_conf_child->Value() ) == 0)
                {
                    logdbg  << "ConfigurationManager: parseConfigurationFile: is FileSection";
                    parseFileSection (main_conf_child);
                }
                else
                {
                    throw std::runtime_error (std::string("ConfigurationManager: parseConfigurationFile: unknown section '")+main_conf_child->Value()+"'");
                }
            }
        }
    }
    else
    {
        logerr << "ConfigurationManager: parseConfigurationFile: could not load file '" << filename << "'";
        throw std::runtime_error ("ConfigurationManager: parseConfigurationFile: load error");
    }
    logdbg  << "ConfigurationManager: parseConfigurationFile: file '" << filename << "' read";
    delete config_file_doc;
}

/**
 * Assumes a list of SubConfigurationFile nodes, which define paths on which parseConfigurationFile is called.
 */
void ConfigurationManager::parseFileSection (XMLElement *configuration_section_element)
{
    assert (initialized_);
    logdbg  << "ConfigurationManager: parseFileSection";

    XMLElement * pool_child;

    for ( pool_child = configuration_section_element->FirstChildElement(); pool_child != 0; pool_child = pool_child->NextSiblingElement())
    {
        logdbg  << "ConfigurationManager: parseFileSection: found element '" << pool_child->Value() << "'";

        if (strcmp ("SubConfigurationFile", pool_child->Value() ) == 0)
        {
            logdbg  << "ConfigurationManager: parseFileSection: is SubConfigurationFile";

            const char *path=0;

            const XMLAttribute* attribute=pool_child->FirstAttribute();
            while (attribute)
            {
                logdbg  << "ConfigurationManager: parseFileSection: attribute " << attribute->Name() << "  value "<< attribute->Value();
                if (strcmp ("path", attribute->Name()) == 0)
                    path=attribute->Value();
                else
                    throw std::runtime_error ("ConfigurationManager: parseFileSection: unknown attribute");

                attribute=attribute->Next();
            }

            if (path)
            {
                logdbg  << "ConfigurationManager: parseFileSection: parsing sub configuration file '" << path << "'";
                parseConfigurationFile (path);
            }
            else
                throw std::runtime_error ("error: ConfigurationManager: parseFileSection: configuration misses attributes");

        }
        else
        {
            throw std::runtime_error (std::string("ConfigurationManager: parseFileSection: unknown section '")+pool_child->Value()+"'");
        }
    }
    logdbg  << "ConfigurationManager: parseFileSection: done";
}

/**
 * Assumes a Configuration tags, which hold the root configurables & configurations. If a correct class id and
 * instance id is supplied, a new root configuration is generated, which the parses its element using parseXMLElement.
 */
void ConfigurationManager::parseConfigurationSection (XMLElement *configuration_section_element, std::string filename)
{
    assert (initialized_);
    logdbg  << "ConfigurationManager: parseConfigurationSection";

    XMLElement * configuration_element;

    for ( configuration_element = configuration_section_element->FirstChildElement(); configuration_element != 0;
            configuration_element = configuration_element->NextSiblingElement())
    {
        logdbg  << "ConfigurationManager: parseConfigurationSection: found element '" << configuration_element->Value() << "'";
        if (strcmp ("Configuration", configuration_element->Value() ) == 0)
        {
            logdbg  << "ConfigurationManager: parseConfigurationSection: is Configuration";

            const char *class_id=0;
            const char *instance_id=0;

            const XMLAttribute* attribute=configuration_element->FirstAttribute();
            while (attribute)
            {
                logdbg  << "ConfigurationManager: parseConfigurationSection: attribute " << attribute->Name()
                                << "  value "<< attribute->Value();
                if (strcmp ("class_id", attribute->Name()) == 0)
                    class_id=attribute->Value();
                else if (strcmp ("instance_id", attribute->Name()) == 0)
                    instance_id=attribute->Value();
                else
                    throw std::runtime_error (std::string ("ConfigurationManager: parseConfigurationSection: unknown attribute ")
                        +attribute->Name());

                attribute=attribute->Next();
            }

            if (class_id && instance_id)
            {
                std::pair<std::string, std::string> key (class_id, instance_id);
                assert (root_configurations_.find (key) == root_configurations_.end()); // should not exist

                logdbg << "ConfigurationManager: parseConfigurationSection: creating new configuration for class " << class_id <<
                        " instance " << instance_id;
                root_configurations_.insert (std::pair<std::pair<std::string, std::string>, Configuration> (key, Configuration (class_id, instance_id)));
                root_configurations_.at(key).setConfigurationFilename (filename);
                root_configurations_.at(key).parseXMLElement(configuration_element);
            }
            else
                throw std::runtime_error ("error: ConfigurationManager: parseConfigurationSection: configuration misses attributes");

        }
        else
        {
            throw std::runtime_error ("ConfigurationManager: parseConfigurationSection: unknown section");
        }
    }
    logdbg  << "ConfigurationManager: parseConfigurationSection: done";
}

void ConfigurationManager::saveConfiguration ()
{
    assert (initialized_);

    save_info_.clear();

    //std::map <std::pair<std::string, std::string>, Configurable&>::iterator it;

    logdbg << "ConfigurationManager: saveConfiguration: creating main document";
    save_info_ [main_config_filename_] = ConfigurationSaveInformation (main_config_filename_, true);

    for (auto it : root_configurables_) //iterate over root configurables
    {
        Configurable &root_configurable = it.second;

        std::string output_config_filename = root_configurable.getConfiguration().hasConfigurationFilename() ?
                    root_configurable.getConfiguration().getConfigurationFilename() : main_config_filename_;

        logdbg << "ConfigurationManager: saveConfiguration: adding root configurable " << root_configurable.getInstanceId()
               << " to filename " << output_config_filename;

        if (save_info_.find (output_config_filename) == save_info_.end()) // new file, can not happen to main config
        {
            logdbg << "ConfigurationManager: saveConfiguration: creating new save information for " << root_configurable.getInstanceId();
            save_info_ [output_config_filename] = ConfigurationSaveInformation (output_config_filename, false);
        }
        save_info_.at(output_config_filename).addConfigurationSection(root_configurable.getInstanceId(), root_configurable.getConfiguration());
    }

    for (auto it : root_configurations_) // iterate over root configurations
    {
        std::pair<std::string, std::string> key = it.first;

        if (root_configurables_.find (key) == root_configurables_.end()) // unused root configuration, not yet in save_info
        {
            logdbg << "ConfigurationManager: saveConfiguration: configuration " << it.second.getInstanceId()
                    << " unused";

            std::string pool_config_filename = it.second.hasConfigurationFilename() ?
                        it.second.getConfigurationFilename() : main_config_filename_;

            logdbg << "ConfigurationManager: saveConfiguration: adding unused root configuration " << it.first.second
                   << " to filename " << pool_config_filename;

            if (save_info_.find (pool_config_filename) == save_info_.end()) // new file, can not happen to main config
            {
                logdbg << "ConfigurationManager: saveConfiguration: creating new save information for " << it.second.getInstanceId();
                save_info_ [pool_config_filename] = ConfigurationSaveInformation (pool_config_filename, false);
            }

            save_info_.at(pool_config_filename).addConfigurationSection(it.second.getInstanceId(), it.second);
        }
    }

    // now file list should be complete (from pools), add all which are not main
    for (auto it : save_info_)
        if (it.first.compare(main_config_filename_) != 0)
            save_info_.at(main_config_filename_).addSubConfigurationFile (it.first);

    loginf  << "ConfigurationManager: saveConfiguration: saving configuration files";
    for (auto it : save_info_)
        it.second.save();

    save_info_.clear();
}

ConfigurationSaveInformation::ConfigurationSaveInformation ()
    : document_ (nullptr), root_element_(nullptr), file_element_(nullptr), configuration_section_element_(nullptr),
      configurable_section_element_(nullptr)
{

}

ConfigurationSaveInformation::ConfigurationSaveInformation (const std::string &output_filename, bool is_main_configuration)
    : output_filename_(output_filename), is_main_configuration_(is_main_configuration), document_ (nullptr), root_element_(nullptr),
      file_element_(nullptr), configuration_section_element_(nullptr), configurable_section_element_(nullptr)
{
    assert (output_filename.size() > 0);
    init ();
}

ConfigurationSaveInformation::~ConfigurationSaveInformation ()
{

}

void ConfigurationSaveInformation::init ()
{
    logdbg << "ConfigurationSaveInformation: init: " << output_filename_;
    document_ = new XMLDocument ();

    XMLDeclaration* decl = document_->NewDeclaration( "1.0");
    document_->LinkEndChild( decl );

    root_element_ = document_->NewElement ("Configuration");
    document_->LinkEndChild( root_element_ );

    if (is_main_configuration_) // if main file add filesection
    {
        logdbg << "ConfigurationSaveInformation: init: adding file section to " << output_filename_;

        file_element_ = document_->NewElement("FileSection") ;
        file_element_->LinkEndChild(document_->NewComment("-----------------------------------------------------------------------------------"));
        file_element_->LinkEndChild(document_->NewComment("----- FileSection: SubConfigurationFiles are to be placed here                -----"));
        file_element_->LinkEndChild(document_->NewComment("-----------------------------------------------------------------------------------"));

        root_element_->LinkEndChild( file_element_ );
    }

    logdbg << "ConfigurationManager: saveConfiguration: adding configuration section to " << output_filename_;

    // add configurationsection
    configuration_section_element_ = document_->NewElement( "ConfigurationSection" );
    configuration_section_element_->LinkEndChild(document_->NewComment("-----------------------------------------------------------------------------------"));
    configuration_section_element_->LinkEndChild(document_->NewComment("----- ConfigurationSection: Configurations are to be placed here              -----"));
    configuration_section_element_->LinkEndChild(document_->NewComment("-----------------------------------------------------------------------------------"));

}


void ConfigurationSaveInformation::addConfigurationSection (const std::string &instance_id, const Configuration &configuration)
{
    logdbg << "ConfigurationSaveInformation: addConfigurationSection: for configurable " << instance_id;
    assert (document_);

    // add configurationpool
    std::string comment = "----- Root configuration: " + instance_id + " -----";
    configuration_section_element_->LinkEndChild(document_->NewComment(comment.c_str()));

    configuration_section_element_->LinkEndChild(configuration.generateXMLElement(document_));
    //configuration_section_element_->LinkEndChild(configuration_section_element_);

    root_element_->LinkEndChild( configuration_section_element_ );

    logdbg  << "ConfigurationManager: saveConfiguration: configuration " << instance_id << " appended to file " << output_filename_;

}

void ConfigurationSaveInformation::addSubConfigurationFile (const std::string &output_filename)
{
    logdbg  << "ConfigurationSaveInformation: addSubConfigurationFile: adding sub configuration file " << output_filename << " in " << output_filename;
    assert (document_);
    XMLElement *sub_file_element = document_->NewElement( "SubConfigurationFile" );
    sub_file_element->SetAttribute ("path", output_filename.c_str());
    file_element_->LinkEndChild(sub_file_element);
}

void ConfigurationSaveInformation::save ()
{
    logdbg  << "ConfigurationSaveInformation: save: saving configuration file '" << output_filename_ << "'";
    assert (document_);
    document_->SaveFile(output_filename_.c_str());
}
