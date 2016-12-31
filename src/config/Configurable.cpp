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
 * Configurable.cpp
 *
 *  Created on: May 15, 2012
 *      Author: sk
 */

#include "Configurable.h"
#include "Configuration.h"
#include "Logger.h"
#include "ConfigurationManager.h"

/**
 * \param class_id Class identifier
 * \param instance_id Instance identifier
 * \param parent Parent, default null (if Singleton)
 * \param configuration_filename special XML configuration filename, default ""
 *
 * Initializes members, adds itself to parent, retrieves filename from parent (if set). Registers itself as child on either
 * the parent or as root configurable, which at the same time sets the configuration reference.
 *
 * \todo Extend registerParameter to template function.
 */
Configurable::Configurable(std::string class_id, std::string instance_id, Configurable *parent, std::string configuration_filename)
: unusable_ (false), class_id_(class_id), instance_id_(instance_id), parent_(parent),
  configuration_ (parent_ ? parent_->registerSubConfigurable(this) : ConfigurationManager::getInstance().registerRootConfigurable(*this))
{
    logdbg  << "Configurable: constructor: class_id " << class_id_ << " instance_id " << instance_id_;

    if (configuration_filename.size() != 0)
    {
        logdbg  << "Configurable: constructor: got filename " << configuration_filename;
        configuration_.setConfigurationFilename (configuration_filename);
    }

    logdbg  << "Configurable: constructor: class_id " << class_id_ << " instance_id " << instance_id_ << " end";
}

/**
 * Sets the unusable flag and sets parameters to null or dummy values.
 */
Configurable::Configurable ()
: unusable_ (true), parent_ (0), configuration_ (ConfigurationManager::getInstance().getDummyConfiguration())
{

}

/**
 * If parent is set, unregisters from it using removeChildConfigurable, if not, calls unregisterRootConfigurable.
 *
 * Displays warning message if undeleted sub-configurables exist.
 */
Configurable::~Configurable()
{
    logdbg  << "Configurable: destructor: class_id " << class_id_ << " instance_id " << instance_id_;

    if (!unusable_)
    {
        if (parent_)
        {
            parent_->removeChildConfigurable (this);
        }
        else
        {
            ConfigurationManager::getInstance().unregisterRootConfigurable(*this);
        }

        if (children_.size() != 0)
        {
            logwrn << "Configurable: destructor: class_id " << class_id_ << " instance_id " << instance_id_ << " still " << children_.size() << " undeleted";
        }
    }
}

void Configurable::registerParameter (std::string parameter_id, bool *pointer, bool default_value)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg << "Configurable " << instance_id_ << ": registerParameter: bool parameter_id " << parameter_id;

    configuration_.registerParameter (parameter_id, pointer, default_value);
}
void Configurable::registerParameter (std::string parameter_id, char *pointer, char default_value)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg << "Configurable " << instance_id_ << ": registerParameter: char parameter_id " << parameter_id;

    configuration_.registerParameter (parameter_id, pointer, default_value);
}
void Configurable::registerParameter (std::string parameter_id, unsigned char *pointer, unsigned char default_value)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg << "Configurable " << instance_id_ << ": registerParameter: unsigned char parameter_id " << parameter_id;

    configuration_.registerParameter (parameter_id, pointer, default_value);
}
void Configurable::registerParameter (std::string parameter_id, short int *pointer, short int default_value)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg << "Configurable " << instance_id_ << ": registerParameter: short int parameter_id " << parameter_id;

    configuration_.registerParameter (parameter_id, pointer, default_value);
}
void Configurable::registerParameter (std::string parameter_id, unsigned short int *pointer, unsigned short int default_value)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg << "Configurable " << instance_id_ << ": registerParameter: unsigned short int parameter_id " << parameter_id;

    configuration_.registerParameter (parameter_id, pointer, default_value);
}

void Configurable::registerParameter (std::string parameter_id, int *pointer, int default_value)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg << "Configurable " << instance_id_ << ": registerParameter: int parameter_id " << parameter_id;
    configuration_.registerParameter (parameter_id, pointer, default_value);
}
void Configurable::registerParameter (std::string parameter_id, unsigned int *pointer, unsigned int default_value)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg << "Configurable " << instance_id_ << ": registerParameter: unsigned int parameter_id " << parameter_id;
    configuration_.registerParameter (parameter_id, pointer, default_value);
}
void Configurable::registerParameter (std::string parameter_id, float *pointer, float default_value)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg << "Configurable " << instance_id_ << ": registerParameter: float parameter_id " << parameter_id;
    configuration_.registerParameter (parameter_id, pointer, default_value);
}
void Configurable::registerParameter (std::string parameter_id, double *pointer, double default_value)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg << "Configurable " << instance_id_ << ": registerParameter: double parameter_id " << parameter_id;
    configuration_.registerParameter (parameter_id, pointer, default_value);
}
void Configurable::registerParameter (std::string parameter_id, std::string *pointer, std::string default_value)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg << "Configurable " << instance_id_ << ": registerParameter: string parameter_id " << parameter_id;
    configuration_.registerParameter (parameter_id, pointer, default_value);
}

Configuration &Configurable::registerSubConfigurable (Configurable *child)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg  << "Configurable: registerSubConfigurable: child " << child->getInstanceId();
    assert (child);
    std::string key = child->getClassId()+child->getInstanceId();

    if (children_.find (key) != children_.end())
    {
        throw std::runtime_error ("Configurable: registerSubConfigurable: child key '"+ key +"' already in use");
    }

    logdbg  << "Configurable " << instance_id_ << ": registerSubConfigurable: " << key;
    children_[key] = child;

    return configuration_.getSubConfiguration(child->getClassId(), child->getInstanceId());
}

void Configurable::removeChildConfigurable (Configurable *child)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg  << "Configurable: removeChildConfigurable: child " << child->getInstanceId();
    assert (child);
    std::string key = child->getClassId()+child->getInstanceId();
    assert (children_.find (key) != children_.end());
    logdbg  << "Configurable " << instance_id_ << ": removeChildConfigurable: " << key;
    children_.erase(children_.find(key));

    configuration_.removeSubConfiguration(child->getClassId(), child->getInstanceId());
}


/**
 * Also iteratively calls resetToDefault on all sub-configurables.
 */
void Configurable::resetToDefault ()
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    logdbg  << "Configurable " << instance_id_ << ": resetToDefault";
    configuration_.resetToDefault();

    std::map <std::string, Configurable*>::iterator it;

    for (it = children_.begin(); it != children_.end(); it++)
    {
        //loginf  << "Configurable " << instance_id_ << ": resetToDefault: child " << it->first;
        Configurable *configurable = it->second;
        configurable->resetToDefault();
    }
}

Configuration &Configurable::addNewSubConfiguration (std::string class_id, std::string instance_id)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    return configuration_.addNewSubConfiguration (class_id, instance_id);
}

Configuration &Configurable::addNewSubConfiguration (std::string class_id)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    return configuration_.addNewSubConfiguration (class_id);
}

Configuration &Configurable::addNewSubConfiguration (Configuration &configuration)
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    return configuration_.addNewSubConfiguration (configuration);
}

void Configurable::createSubConfigurables ()
{
    if (unusable_)
        throw std::runtime_error ("Configurable: registerParameter: configurable unusable");

    configuration_.createSubConfigurables (this);
    checkSubConfigurables ();
}

void Configurable::checkSubConfigurables ()
{
    logerr  << "Configurable: checkSubConfigurables: class " << class_id_ << " failed to override me";
}

void Configurable::generateSubConfigurable (std::string class_id, std::string instance_id)
{
    loginf  << "Configurable: generateSubConfigurable: class " << class_id_ << " failed to override me to generate subconfigurable " << class_id;
}

bool Configurable::hasSubConfigurable(std::string class_id, std::string instance_id)
{
    return ( children_.find(class_id+instance_id) != children_.end() );
}

void Configurable::saveConfigurationAsTemplate (std::string template_name)
{
    assert (parent_);
    parent_->saveTemplateConfiguration(this, template_name);
}

void Configurable::saveTemplateConfiguration (Configurable *child, std::string template_name)
{
    assert (configuration_.getSubTemplateNameFree(template_name));
    configuration_.addSubTemplate(child->getConfiguration().clone(), template_name);
}
