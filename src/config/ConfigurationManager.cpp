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

//#include <tinyxml.h>

using namespace tinyxml2;

/**
 * Loads the main configuration filename from Config and calls parseConfigurationFile.
 */
ConfigurationManager::ConfigurationManager()
    : initialized_(false), dummy_configuration_(Configuration ("Dummy", "Dummy0"))
{
}

void ConfigurationManager::init (const std::string &main_config_filename)
{
    assert (!initialized_);
    assert (main_config_filename.size() > 0);

    main_config_filename_ = main_config_filename;
    loginf  << "ConfigurationManager: init: parsing main configuration file '" << main_config_filename_ << "'";
    initialized_ = true;
    parseConfigurationFile (main_config_filename_);
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
 * Assumes a root MainConfiguration, with either a ConfigurationSection or a FileSection, which are
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
            assert (strcmp ("MainConfiguration", doc_main_conf->Value() ) == 0);
            logdbg  << "ConfigurationManager: parseConfigurationFile: found MainConfiguration";

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

    std::map <std::string, XMLDocument *> output_file_documents;
    std::map <std::string, XMLElement *> output_file_root_elements;
    std::map <std::string, XMLElement *> output_file_configuration_section_elements;
    std::map <std::string, XMLElement *> output_file_configurable_section_elements;

    std::map <std::pair<std::string, std::string>, Configurable&>::iterator it;

    logdbg << "ConfigurationManager: saveConfiguration: creating main document";
    XMLDocument *main_document = new XMLDocument ();

    logdbg << "ConfigurationManager: saveConfiguration: creating main document file secion";
    XMLElement *file_section = main_document->NewElement("FileSection") ;

    for (it=root_configurables_.begin(); it != root_configurables_.end(); it++) //iterate over root configurables
    {
        Configurable &root_configurable = it->second;

        XMLDocument *document=0;

        std::string root_config_filename;

        if (root_configurable.getConfiguration().hasConfigurationFilename())
        {
            root_config_filename = root_configurable.getConfiguration().getConfigurationFilename();

            logdbg << "ConfigurationManager: saveConfiguration: adding root configurabke " << root_configurable.getInstanceId()
                   << " filename " << root_config_filename;

            if (output_file_documents.find (root_config_filename) == output_file_documents.end()) // new file
            {
                document = new XMLDocument ();
                output_file_documents[root_config_filename] = document;
                XMLDeclaration* decl = document->NewDeclaration( "1.0");
                document->LinkEndChild( decl );

                XMLElement * root = document->NewElement ("MainConfiguration");
                output_file_root_elements[root_config_filename] = root;
                document->LinkEndChild( root );

                if (root_config_filename.compare(main_config_filename_) == 0) // if main file add filesection
                {
                    root->LinkEndChild(document->NewComment("-----------------------------------------------------------------------------------"));
                    root->LinkEndChild(document->NewComment("----- FileSection: SubConfigurationFiles are to be placed here                -----"));
                    root->LinkEndChild(document->NewComment("-----------------------------------------------------------------------------------"));

                    root->LinkEndChild( file_section );
                }

                // add configurationsection
                root->LinkEndChild(document->NewComment("-----------------------------------------------------------------------------------"));
                root->LinkEndChild(document->NewComment("----- ConfigurationSection: Configurations are to be placed here              -----"));
                root->LinkEndChild(document->NewComment("-----------------------------------------------------------------------------------"));

                XMLElement *configuration_section = document->NewElement( "ConfigurationSection" );
                output_file_configuration_section_elements [root_config_filename] = configuration_section;
                root->LinkEndChild( configuration_section );
            }
        }
        else
            document = main_document;

        XMLElement *configuration_element = root_configurable.getConfiguration().generateXMLElement(document);

        // add configurationpool
        logdbg << "ConfigurationManager: saveConfiguration: for root configurable " << root_configurable.getInstanceId();
        std::string comment = "----- Root configuration: " + it->first.second + " -----";
        output_file_configuration_section_elements.at(root_config_filename)->LinkEndChild(document->NewComment(comment.c_str()));
        output_file_configuration_section_elements.at(root_config_filename)->LinkEndChild(configuration_element);

        logdbg  << "ConfigurationManager: saveConfiguration: configuration " << it->first.second << " appended to file " << root_config_filename;
    }

    std::map <std::pair<std::string, std::string>, Configuration>::iterator root_config_it;

    for (root_config_it = root_configurations_.begin(); root_config_it != root_configurations_.end(); root_config_it++)
    {
        std::pair<std::string, std::string> key = root_config_it->first;

        if (root_configurables_.find (key) == root_configurables_.end())
        {
            logdbg << "ConfigurationManager: saveConfiguration: configuration " << root_config_it->second.getInstanceId()
                    << " unused";

            std::string pool_config_filename;
            if (root_config_it->second.hasConfigurationFilename())
                pool_config_filename = root_config_it->second.getConfigurationFilename();

            if (pool_config_filename.size() == 0) // if no config file, use main
                pool_config_filename = main_config_filename_;

            XMLDocument *document;

            if (output_file_documents.find (pool_config_filename) == output_file_documents.end()) // new file
            {
                document = new XMLDocument ();
                output_file_documents[pool_config_filename] = document;
                XMLDeclaration* decl = document->NewDeclaration("1.0");
                document->LinkEndChild( decl );

                XMLElement * root = document->NewElement("MainConfiguration");
                output_file_root_elements[pool_config_filename] = root;
                document->LinkEndChild( root );

                if (pool_config_filename.compare(main_config_filename_) == 0) // if main file add filesection
                {
                    root->LinkEndChild(document->NewComment("-----------------------------------------------------------------------------------"));
                    root->LinkEndChild(document->NewComment("----- FileSection: SubConfigurationFiles are to be placed here                -----"));
                    root->LinkEndChild(document->NewComment("-----------------------------------------------------------------------------------"));

                    root->LinkEndChild( file_section );
                }

                // add configurationsection
                root->LinkEndChild(document->NewComment("-----------------------------------------------------------------------------------"));
                root->LinkEndChild(document->NewComment("----- ConfigurationSection: Configurations are to be placed here              -----"));
                root->LinkEndChild(document->NewComment("-----------------------------------------------------------------------------------"));

                XMLElement *configuration_section = document->NewElement( "ConfigurationSection" );
                output_file_configuration_section_elements [pool_config_filename] = configuration_section;
                root->LinkEndChild( configuration_section );
            }
            else
                document = output_file_documents[pool_config_filename];

            XMLElement *configuration_element = root_config_it->second.generateXMLElement(document);

            // add configurationpool
            std::string comment = "----- Root configuration: " + root_config_it->first.second + " -----";
            output_file_configuration_section_elements [pool_config_filename]->LinkEndChild(document->NewComment(comment.c_str()));
            output_file_configuration_section_elements [pool_config_filename]->LinkEndChild(configuration_element);

            logdbg  << "ConfigurationManager: saveConfiguration: configuration " << root_config_it->first.second << " appended to file " << pool_config_filename;
        }
    }

    // now file list should be complete (from pools), add all which are not main
    std::map <std::string, XMLDocument *>::iterator file_it;
    for (file_it=output_file_documents.begin(); file_it != output_file_documents.end(); file_it++)
    {
        if (file_it->first.compare(main_config_filename_) != 0)
        {
            XMLElement *sub_file_element = main_document->NewElement( "SubConfigurationFile" );
            sub_file_element->SetAttribute ("path", file_it->first.c_str());
            file_section->LinkEndChild(sub_file_element);
        }
    }

    loginf  << "ConfigurationManager: saveConfiguration: saving configuration files";
    std::map <std::string, XMLDocument *>::iterator doc_it;

    for (doc_it=output_file_documents.begin(); doc_it != output_file_documents.end(); doc_it++)
    {
        logdbg  << "ConfigurationManager: saveConfiguration: saving configuration file '" << doc_it->first << "'";
        doc_it->second->SaveFile(doc_it->first.c_str());
    }
}



