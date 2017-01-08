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
 * Configuration.cpp
 *
 *  Created on: May 15, 2012
 *      Author: sk
 */

#include <typeinfo>

#include "Configuration.h"
#include "Configurable.h"
#include "Logger.h"

#include "String.h"

using namespace Utils;


using namespace tinyxml2;

/**
 * Initializes members
 *
 * \param configuration_id configuration identifier
 * \param configuration_filename special filename, default ""
 */
Configuration::Configuration(const std::string &class_id, const std::string &instance_id, const std::string &configuration_filename)
    : class_id_(class_id), instance_id_(instance_id), used_(false), configuration_filename_(configuration_filename),
      template_flag_ (false)
{
    logdbg  << "Configuration: constructor: class " << class_id_ << " instance " << instance_id_;
    assert (class_id_.size() != 0);
    assert (instance_id_.size() != 0);
}


Configuration::Configuration(const Configuration &source)
{
    operator=(source);
}

Configuration& Configuration::operator= (const Configuration &source)
{
    class_id_ = source.class_id_;
    /// Instance identifier
    instance_id_ = source.instance_id_; // needs to be overwritten
    /// Flag indicating if configuration has been used by configurable
    used_ = false;
    /// Special XML configuration filename
    configuration_filename_ = source.configuration_filename_;

    template_flag_=source.template_flag_;
    template_name_=source.template_name_;

    /// Container for all parameters (parameter identifier -> ConfigurableParameterBase)
    parameters_bool_ = source.parameters_bool_;
    parameters_int_ = source.parameters_int_;
    parameters_uint_ = source.parameters_uint_;
    parameters_float_ = source.parameters_float_;
    parameters_double_ = source.parameters_double_;
    parameters_string_ = source.parameters_string_;

    /// Container for all added sub-configurables
    sub_configurations_ = source.sub_configurations_;

    // return the existing object
    return *this;
}

Configuration *Configuration::clone ()
{
    return new Configuration (*this);
}

Configuration::~Configuration()
{
    parameters_bool_.clear();
    parameters_int_.clear();
    parameters_uint_.clear();
    parameters_float_.clear();
    parameters_double_.clear();
    parameters_string_.clear();
}

/**
 * Resets all parameters to their default values
 */
void Configuration::resetToDefault ()
{
    logdbg  << "Configuration: resetToDefault: " << instance_id_;

    for (auto it = parameters_bool_.begin(); it != parameters_bool_.end(); it++)
        it->second.resetToDefault();
    for (auto it = parameters_int_.begin(); it != parameters_int_.end(); it++)
        it->second.resetToDefault();
    for (auto it = parameters_uint_.begin(); it != parameters_uint_.end(); it++)
        it->second.resetToDefault();
    for (auto it = parameters_float_.begin(); it != parameters_float_.end(); it++)
        it->second.resetToDefault();
    for (auto it = parameters_double_.begin(); it != parameters_double_.end(); it++)
        it->second.resetToDefault();
    for (auto it = parameters_string_.begin(); it != parameters_string_.end(); it++)
        it->second.resetToDefault();
}

void Configuration::registerParameter (const std::string &parameter_id, bool *pointer, bool default_value)
{
    logdbg  << "Configuration " << instance_id_ << ": registerParameter: bool: " << parameter_id;

    assert (pointer);

    if (parameters_bool_.find(parameter_id) == parameters_bool_.end()) // new parameter, didnt exist in config
    {
        parameters_bool_.insert (std::pair<std::string, ConfigurableParameter<bool> > (parameter_id, ConfigurableParameter<bool>()));
        parameters_bool_.at(parameter_id).parameter_id_=parameter_id;
        parameters_bool_.at(parameter_id).config_value_=default_value;
    }

    parameters_bool_.at(parameter_id).pointer_ = pointer;
    parameters_bool_.at(parameter_id).default_value_ = default_value;
    *(parameters_bool_.at(parameter_id).pointer_) = parameters_bool_.at(parameter_id).config_value_;
    used_=true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter "<< parameter_id << ": bool, value is " << *(parameters_bool_.at(parameter_id).pointer_);
}


void Configuration::registerParameter (const std::string &parameter_id, int *pointer, int default_value)
{
    logdbg  << "Configuration " << instance_id_ << ": registerParameter: int: " << parameter_id;

    assert (pointer);

    if (parameters_int_.find(parameter_id) == parameters_int_.end()) // new parameter, didnt exist in config
    {
        parameters_int_.insert (std::pair<std::string, ConfigurableParameter<int> > (parameter_id, ConfigurableParameter<int>()));
        parameters_int_.at(parameter_id).parameter_id_=parameter_id;
        parameters_int_.at(parameter_id).config_value_=default_value;
    }

    parameters_int_.at(parameter_id).pointer_ = pointer;
    parameters_int_.at(parameter_id).default_value_ = default_value;
    *(parameters_int_.at(parameter_id).pointer_) = parameters_int_.at(parameter_id).config_value_;
    used_=true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter "<< parameter_id << ": int, value is " << *(parameters_int_.at(parameter_id).pointer_);
}

void Configuration::registerParameter (const std::string &parameter_id, unsigned int *pointer, unsigned int default_value)
{
    logdbg  << "Configuration " << instance_id_ << ": registerParameter: unsigned int: " << parameter_id;

    assert (pointer);

    if (parameters_uint_.find(parameter_id) == parameters_uint_.end()) // new parameter, didnt exist in config
    {
        parameters_uint_.insert (std::pair<std::string, ConfigurableParameter<unsigned int> > (parameter_id, ConfigurableParameter<unsigned int>()));
        parameters_uint_.at(parameter_id).parameter_id_=parameter_id;
        parameters_uint_.at(parameter_id).config_value_=default_value;
    }

    parameters_uint_.at(parameter_id).pointer_ = pointer;
    parameters_uint_.at(parameter_id).default_value_ = default_value;
    *(parameters_uint_.at(parameter_id).pointer_) = parameters_uint_.at(parameter_id).config_value_;
    used_=true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter "<< parameter_id << ": unsigned int, value is " << *(parameters_uint_.at(parameter_id).pointer_);
}

void Configuration::registerParameter (const std::string &parameter_id, float *pointer, float default_value)
{
    logdbg  << "Configuration " << instance_id_ << ": registerParameter: float: " << parameter_id;

    assert (pointer);

    if (parameters_float_.find(parameter_id) == parameters_float_.end()) // new parameter, didnt exist in config
    {
        parameters_float_.insert (std::pair<std::string, ConfigurableParameter<float> > (parameter_id, ConfigurableParameter<float>()));
        parameters_float_.at(parameter_id).parameter_id_=parameter_id;
        parameters_float_.at(parameter_id).config_value_=default_value;
    }

    parameters_float_.at(parameter_id).pointer_ = pointer;
    parameters_float_.at(parameter_id).default_value_ = default_value;
    *(parameters_float_.at(parameter_id).pointer_) = parameters_float_.at(parameter_id).config_value_;
    used_=true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter "<< parameter_id << ": float, value is " << *(parameters_float_.at(parameter_id).pointer_);
}

void Configuration::registerParameter (const std::string &parameter_id, double *pointer, double default_value)
{
    logdbg  << "Configuration " << instance_id_ << ": registerParameter: double: " << parameter_id;

    assert (pointer);

    if (parameters_double_.find(parameter_id) == parameters_double_.end()) // new parameter, didnt exist in config
    {
        parameters_double_.insert (std::pair<std::string, ConfigurableParameter<double> > (parameter_id, ConfigurableParameter<double>()));
        parameters_double_.at(parameter_id).parameter_id_=parameter_id;
        parameters_double_.at(parameter_id).config_value_=default_value;
    }

    parameters_double_.at(parameter_id).pointer_ = pointer;
    parameters_double_.at(parameter_id).default_value_ = default_value;
    *(parameters_double_.at(parameter_id).pointer_) = parameters_double_.at(parameter_id).config_value_;
    used_=true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter "<< parameter_id << ": double, value is " << *(parameters_double_.at(parameter_id).pointer_);
}

void Configuration::registerParameter (const std::string &parameter_id, std::string *pointer, const std::string &default_value)
{
    logdbg  << "Configuration " << instance_id_ << ": registerParameter: string: " << parameter_id;

    assert (pointer);

    if (parameters_string_.find(parameter_id) == parameters_string_.end()) // new parameter, didnt exist in config
    {
        parameters_string_.insert (std::pair<std::string, ConfigurableParameter<std::string> > (parameter_id, ConfigurableParameter<std::string>()));
        parameters_string_.at(parameter_id).parameter_id_=parameter_id;
        parameters_string_.at(parameter_id).config_value_=default_value;
    }

    parameters_string_.at(parameter_id).pointer_ = pointer;
    parameters_string_.at(parameter_id).default_value_ = default_value;
    *(parameters_string_.at(parameter_id).pointer_) = parameters_string_.at(parameter_id).config_value_;
    used_=true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter "<< parameter_id << ": string, value is " << *(parameters_string_.at(parameter_id).pointer_);
}

void Configuration::addParameterBool (const std::string &parameter_id, bool default_value)
{
    logdbg  << "Configuration: addParameterBool: parameter " << parameter_id << " default " << default_value;
    if (parameters_bool_.find(parameter_id) != parameters_bool_.end())
    {
        logwrn  << "Configuration "<< instance_id_ <<": addParameterBool: " << parameter_id << " already exists";
        return;
    }

    parameters_bool_.insert (std::pair<std::string, ConfigurableParameter<bool> > (parameter_id, ConfigurableParameter<bool>()));
    parameters_bool_.at(parameter_id).parameter_id_=parameter_id;
    parameters_bool_.at(parameter_id).pointer_=0;
    parameters_bool_.at(parameter_id).default_value_=default_value;
    parameters_bool_.at(parameter_id).config_value_=default_value;
}

void Configuration::addParameterInt (const std::string &parameter_id, int default_value)
{
    logdbg  << "Configuration: addParameterInt: parameter " << parameter_id << " default " << default_value;
    if (parameters_int_.find(parameter_id) != parameters_int_.end())
    {
        logwrn  << "Configuration "<< instance_id_ <<": addParameterInt: " << parameter_id << " already exists";
        return;
    }

    parameters_int_.insert (std::pair<std::string, ConfigurableParameter<int> > (parameter_id, ConfigurableParameter<int>()));
    parameters_int_.at(parameter_id).parameter_id_=parameter_id;
    parameters_int_.at(parameter_id).pointer_=0;
    parameters_int_.at(parameter_id).default_value_=default_value;
    parameters_int_.at(parameter_id).config_value_=default_value;

}

void Configuration::addParameterUnsignedInt (const std::string &parameter_id, unsigned int default_value)
{
    logdbg  << "Configuration: addParameterUnsignedInt: parameter " << parameter_id << " default " << default_value;
    if (parameters_uint_.find(parameter_id) != parameters_uint_.end())
    {
        logwrn  << "Configuration "<< instance_id_ <<": addParameterUnsignedInt: " << parameter_id << " already exists";
        return;
    }

    parameters_uint_.insert (std::pair<std::string, ConfigurableParameter<unsigned int> > (parameter_id, ConfigurableParameter<unsigned int>()));
    parameters_uint_.at(parameter_id).parameter_id_=parameter_id;
    parameters_uint_.at(parameter_id).pointer_=0;
    parameters_uint_.at(parameter_id).default_value_=default_value;
    parameters_uint_.at(parameter_id).config_value_=default_value;
}

void Configuration::addParameterFloat (const std::string &parameter_id, float default_value)
{
    logdbg  << "Configuration: addParameterFloat: parameter " << parameter_id << " default " << default_value;
    if (parameters_float_.find(parameter_id) != parameters_float_.end())
    {
        logwrn  << "Configuration "<< instance_id_ <<": addParameterFloat: " << parameter_id << " already exists";
        return;
    }

    parameters_float_.insert (std::pair<std::string, ConfigurableParameter<float> > (parameter_id, ConfigurableParameter<float>()));
    parameters_float_.at(parameter_id).parameter_id_=parameter_id;
    parameters_float_.at(parameter_id).pointer_=0;
    parameters_float_.at(parameter_id).default_value_=default_value;
    parameters_float_.at(parameter_id).config_value_=default_value;
}

void Configuration::addParameterDouble (const std::string &parameter_id, double default_value)
{
    logdbg  << "Configuration: addParameterDouble: parameter " << parameter_id << " default " << default_value;
    if (parameters_double_.find(parameter_id) != parameters_double_.end())
    {
        logwrn  << "Configuration "<< instance_id_ <<": addParameterDouble: " << parameter_id << " already exists";
        return;
    }

    parameters_double_.insert (std::pair<std::string, ConfigurableParameter<double> > (parameter_id, ConfigurableParameter<double>()));
    parameters_double_.at(parameter_id).parameter_id_=parameter_id;
    parameters_double_.at(parameter_id).pointer_=0;
    parameters_double_.at(parameter_id).default_value_=default_value;
    parameters_double_.at(parameter_id).config_value_=default_value;
}

void Configuration::addParameterString (const std::string &parameter_id, const std::string &default_value)
{
    logdbg  << "Configuration: addParameterString: parameter " << parameter_id << " default " << default_value;
    if (parameters_string_.find(parameter_id) != parameters_string_.end())
    {
        logwrn  << "Configuration "<< instance_id_ <<": addParameterString: " << parameter_id << " already exists";
        return;
    }

    parameters_string_.insert (std::pair<std::string, ConfigurableParameter<std::string> > (parameter_id, ConfigurableParameter<std::string>()));
    parameters_string_.at(parameter_id).parameter_id_=parameter_id;
    parameters_string_.at(parameter_id).pointer_=0;
    parameters_string_.at(parameter_id).default_value_=default_value;
    parameters_string_.at(parameter_id).config_value_=default_value;
}

void Configuration::getParameter (const std::string &parameter_id, bool &value)
{
    if (parameters_bool_.count(parameter_id) == 0)
        throw std::runtime_error ("Configuration: getParameter: bool: unknown parameter id "+parameter_id);

    assert (parameters_bool_.at(parameter_id).getParameterType().compare ("ParameterBool") == 0);

    if (parameters_bool_.at(parameter_id).pointer_ == 0)
        throw std::runtime_error ("Configuration: getParameter: bool "+parameter_id+" not in use");

    value = *(parameters_bool_.at(parameter_id).pointer_);
}

void Configuration::getParameter (const std::string &parameter_id, int &value)
{
    if (parameters_int_.count(parameter_id) == 0)
        throw std::runtime_error ("Configuration: getParameter: int: unknown parameter id "+parameter_id);

    assert (parameters_int_.at(parameter_id).getParameterType().compare ("ParameterInt") == 0);

    if (parameters_int_.at(parameter_id).pointer_ == 0)
        throw std::runtime_error ("Configuration: getParameter: int "+parameter_id+" not in use");

    value = *(parameters_int_.at(parameter_id).pointer_);
}

void Configuration::getParameter (const std::string &parameter_id, unsigned int &value)
{
    if (parameters_uint_.count(parameter_id) == 0)
        throw std::runtime_error ("Configuration: getParameter: uint: unknown parameter id "+parameter_id);

    assert (parameters_uint_.at(parameter_id).getParameterType().compare ("ParameterUnsignedInt") == 0);

    if (parameters_uint_.at(parameter_id).pointer_ == 0)
        throw std::runtime_error ("Configuration: getParameter: uint "+parameter_id+" not in use");

    value = *(parameters_uint_.at(parameter_id).pointer_);
}

void Configuration::getParameter (const std::string &parameter_id, float &value)
{
    if (parameters_float_.count(parameter_id) == 0)
        throw std::runtime_error ("Configuration: getParameter: float: unknown parameter id "+parameter_id);

    assert (parameters_float_.at(parameter_id).getParameterType().compare ("ParameterFloat") == 0);

    if (parameters_float_.at(parameter_id).pointer_ == 0)
        throw std::runtime_error ("Configuration: getParameter: float "+parameter_id+" not in use");

    value = *(parameters_float_.at(parameter_id).pointer_);
}

void Configuration::getParameter (const std::string &parameter_id, double &value)
{
    if (parameters_double_.count(parameter_id) == 0)
        throw std::runtime_error ("Configuration: getParameter: double: unknown parameter id "+parameter_id);

    assert (parameters_double_.at(parameter_id).getParameterType().compare ("ParameterDouble") == 0);

    if (parameters_double_.at(parameter_id).pointer_ == 0)
        throw std::runtime_error ("Configuration: getParameter: double "+parameter_id+" not in use");

    value = *(parameters_double_.at(parameter_id).pointer_);
}
void Configuration::getParameter (const std::string &parameter_id, std::string &value)
{
    if (parameters_string_.count(parameter_id) == 0)
        throw std::runtime_error ("Configuration: getParameter: string: unknown parameter id "+parameter_id);

    assert (parameters_string_.at(parameter_id).getParameterType().compare ("ParameterString") == 0);

    if (parameters_string_.at(parameter_id).pointer_ == 0)
        throw std::runtime_error ("Configuration: getParameter: double "+parameter_id+" not in use");

    value = *(parameters_string_.at(parameter_id).pointer_);
}


void Configuration::parseXMLElement (XMLElement *element)
{
    logdbg  << "Configuration " << instance_id_ << ": parseElement";

    if (strcmp ("SubConfigurationFile", element->Value() ) == 0)
    {
        logdbg  << "Configuration " << instance_id_ << ": parseElement: found file element";
        parseXMLFileElement(element);
    }
    else if (strcmp ("Configuration", element->Value() ) == 0)
    {
        logdbg  << "Configuration " << instance_id_ << ": parseElement: found configuration element";
        parseXMLConfigurationElement(element);
    }
    else
        throw std::runtime_error (std::string("Configuration: parseXMLElement: unknown element ")+element->Value());
}

void Configuration::parseXMLConfigurationElement (tinyxml2::XMLElement *element)
{
    XMLElement * parameter_element;

    for ( parameter_element = element->FirstChildElement(); parameter_element != 0;
          parameter_element = parameter_element->NextSiblingElement())
    {
        logdbg  << "Configuration " << instance_id_ << ": parseXMLConfigurationElement: found element '" << parameter_element->Value() << "'";
        if (strcmp ("ParameterBool", parameter_element->Value() ) == 0)
        {
            logdbg  << "Configuration " << instance_id_ << ": parseXMLConfigurationElement: is ParameterBool";
            ConfigurableParameter<bool> parameter;
            parameter.parseElement(parameter_element);

            assert (parameters_bool_.count(parameter.parameter_id_) == 0);
            parameters_bool_.insert (std::pair <std::string, ConfigurableParameter<bool> > (parameter.parameter_id_, parameter));
        }
        else if (strcmp ("ParameterInt", parameter_element->Value() ) == 0)
        {
            logdbg  << "Configuration " << instance_id_ << ": parseXMLConfigurationElement: is ParameterInt";
            ConfigurableParameter<int> parameter;
            parameter.parseElement(parameter_element);

            assert (parameters_int_.count(parameter.parameter_id_) == 0);
            parameters_int_.insert (std::pair <std::string, ConfigurableParameter<int> > (parameter.parameter_id_, parameter));
        }
        else if (strcmp ("ParameterUnsignedInt", parameter_element->Value() ) == 0)
        {
            logdbg  << "Configuration " << instance_id_ << ": parseXMLConfigurationElement: is ParameterUnsignedInt";
            ConfigurableParameter<unsigned int> parameter;
            parameter.parseElement(parameter_element);

            assert (parameters_uint_.count(parameter.parameter_id_) == 0);
            parameters_uint_.insert (std::pair <std::string, ConfigurableParameter<unsigned int> > (parameter.parameter_id_, parameter));
        }
        else if (strcmp ("ParameterFloat", parameter_element->Value() ) == 0)
        {
            logdbg  << "Configuration " << instance_id_ << ": parseXMLConfigurationElement: is ParameterFloat";
            ConfigurableParameter<float> parameter;
            parameter.parseElement(parameter_element);

            assert (parameters_float_.count(parameter.parameter_id_) == 0);
            parameters_float_.insert (std::pair <std::string, ConfigurableParameter<float> > (parameter.parameter_id_, parameter));
        }
        else if (strcmp ("ParameterDouble", parameter_element->Value() ) == 0)
        {
            logdbg  << "Configuration " << instance_id_ << ": parseXMLConfigurationElement: is ParameterDouble";
            ConfigurableParameter<double> parameter;
            parameter.parseElement(parameter_element);

            assert (parameters_double_.count(parameter.parameter_id_) == 0);
            parameters_double_.insert (std::pair <std::string, ConfigurableParameter<double> > (parameter.parameter_id_, parameter));
        }
        else if (strcmp ("ParameterString", parameter_element->Value() ) == 0)
        {
            logdbg  << "Configuration " << instance_id_ << ": parseXMLConfigurationElement: is ParameterString";
            ConfigurableParameter<std::string> parameter;
            parameter.parseElement(parameter_element);

            assert (parameters_string_.count(parameter.parameter_id_) == 0);
            parameters_string_.insert (std::pair <std::string, ConfigurableParameter<std::string> > (parameter.parameter_id_, parameter));
        }
        else if (strcmp ("Configuration", parameter_element->Value() ) == 0)
        {
            //            loginf << "Configuration: parseXMLElement: parsing configuration " << class_id_ << " instance "
            //                    << instance_id_;

            const char *class_id=0;
            const char *instance_id=0;
            bool template_flag=false;
            const char *template_name=0;

            const XMLAttribute* attribute=parameter_element->FirstAttribute();
            while (attribute)
            {
                //loginf  << "Configuration: parseXMLElement: attribute " << attribute->Name() << "  value "<< attribute->Value();
                if (strcmp ("instance_id", attribute->Name()) == 0)
                    instance_id=attribute->Value();
                else if (strcmp ("class_id", attribute->Name()) == 0)
                    class_id=attribute->Value();
                else if (strcmp ("template", attribute->Name()) == 0)
                {
                    template_flag = true;
                    template_name=attribute->Value();
                }
                else
                    throw std::runtime_error ("Configuration: parseXMLConfigurationElement: unknown attribute");

                attribute=attribute->Next();
            }

            logdbg  << "Configuration: parseXMLConfigurationElement: parsing sub-configuration";

            if (!class_id || !instance_id)
                throw std::runtime_error ("Configuration: parseXMLConfigurationElement: wrong attributes of configuration");

            //            loginf << "Configuration: parseXMLElement: parsing configuration " << class_id_ << " instance "
            //                    << instance_id_ << " found sub-configuration class " << class_id << " instance " << instance_id
            //                    << " template " << template_flag << " template name " << template_name;

            if (template_flag)
            {
                loginf << "Configuration: parseXMLConfigurationElement: found template class " << class_id << " instance "
                       << instance_id;
                assert (configuration_templates_.find(template_name) == configuration_templates_.end());
                configuration_templates_.insert(std::pair <std::string, Configuration> (template_name, Configuration (class_id, instance_id)));
                configuration_templates_.at(template_name).parseXMLElement(parameter_element);
                configuration_templates_.at(template_name).setTemplate(true, template_name);
            }
            else
            {
                std::pair <std::string, std::string> key (class_id, instance_id);
                assert (sub_configurations_.find(key) == sub_configurations_.end());
                sub_configurations_.insert(std::pair <std::pair<std::string, std::string>, Configuration> (key, Configuration (class_id, instance_id)));
                sub_configurations_.at(key).parseXMLElement(parameter_element);
            }
        }
        else if (strcmp ("SubConfigurationFile", parameter_element->Value() ) == 0)
        {
            logdbg  << "Configuration: parseXMLConfigurationElement: is SubConfigurationFile";

            std::string class_id;
            std::string instance_id;
            std::string path;

            const XMLAttribute* attribute=parameter_element->FirstAttribute();
            while (attribute)
            {
                logdbg  << "ConfigurationManager: parseXMLConfigurationElement: attribute '" << attribute->Name()
                        << "'' value '"<< attribute->Value() << "'";
                if (strcmp ("class_id", attribute->Name()) == 0)
                    class_id=attribute->Value();
                else if (strcmp ("instance_id", attribute->Name()) == 0)
                    instance_id=attribute->Value();
                else if (strcmp ("path", attribute->Name()) == 0)
                    path=attribute->Value();
                else
                    throw std::runtime_error (std::string ("Configuration: parseXMLConfigurationElement: unknown attribute ")
                                              +attribute->Name());

                attribute=attribute->Next();
            }

            if (class_id.size() && instance_id.size() && path.size())
            {

                logdbg << "Configuration: parseXMLConfigurationElement: creating new configuration for class " << class_id <<
                          " instance " << instance_id;

                std::pair <std::string, std::string> key (class_id, instance_id);
                assert (sub_configurations_.find(key) == sub_configurations_.end());
                sub_configurations_.insert(std::pair <std::pair<std::string, std::string>, Configuration> (key, Configuration (class_id, instance_id)));
                sub_configurations_.at(key).setConfigurationFilename (path);
                sub_configurations_.at(key).parseXMLElement(parameter_element);
            }
            else
                throw std::runtime_error ("error: Configuration: parseXMLConfigurationElement: configuration misses attributes");


        }
        else
        {
            throw std::runtime_error (std::string("Configuration: parseXMLConfigurationElement: unknown section ")+parameter_element->Value());
        }
    }

    if (template_flag_)
        loginf << "Configuration: parseXMLConfigurationElement: class " << class_id_ << " instance " << instance_id_ <<
                  " is template " << template_name_;

}

void Configuration::parseXMLFileElement (tinyxml2::XMLElement *element)
{
    std::string class_id;
    std::string instance_id;
    std::string path;

    const XMLAttribute* attribute=element->FirstAttribute();
    while (attribute)
    {
        logdbg  << "Configuration: parseXMLFileElement: attribute '" << attribute->Name()
                << "'' value '"<< attribute->Value() << "'";
        if (strcmp ("class_id", attribute->Name()) == 0)
            class_id=attribute->Value();
        else if (strcmp ("instance_id", attribute->Name()) == 0)
            instance_id=attribute->Value();
        else if (strcmp ("path", attribute->Name()) == 0)
            path=attribute->Value();
        else
            throw std::runtime_error (std::string ("Configuration: parseXMLFileElement: unknown attribute ")
                                      +attribute->Name());

        attribute=attribute->Next();
    }

    if (class_id.size() && instance_id.size() && path.size())
    {
        XMLDocument *sub_config_file_doc = new XMLDocument ();

        if (sub_config_file_doc->LoadFile(path.c_str()) == 0)
        {
            XMLElement *element;

            for ( element = sub_config_file_doc->FirstChildElement(); element != 0;
                  element = sub_config_file_doc->NextSiblingElement())
            {
                parseXMLElement (element);
            }
        }
        else
        {
            logerr << "Configuration: parseXMLFileElement: could not load file '" << path << "'";
            throw std::runtime_error ("Configuration: parseXMLFileElement: load error");
        }
        delete sub_config_file_doc;
    }
    else
        throw std::runtime_error ("error: Configuration: parseXMLFileElement: configuration misses attributes");

}

XMLElement *Configuration::generateXMLElement (tinyxml2::XMLDocument *parent_document) const
{
    logdbg  << "Configuration: generateElement: in class " << instance_id_ ;
    assert (parent_document);

    tinyxml2::XMLDocument *document = configuration_filename_.size() > 0 ? new XMLDocument () : parent_document;

    XMLElement *element = document->NewElement("Configuration");

    element->SetAttribute("class_id", class_id_.c_str());
    element->SetAttribute("instance_id", instance_id_.c_str());

    if (template_flag_)
    {
        loginf << "Configuration: generateXMLElement: class " << class_id_ << " instance " << instance_id_ <<
                  " is template " << template_name_;
        element->SetAttribute("template", template_name_.c_str());
    }

    for (auto it = parameters_bool_.begin(); it != parameters_bool_.end(); it++)
    {
        logdbg  << "Configuration: generateElement: bool param " << it->first;
        XMLElement *parameter = document->NewElement(it->second.getParameterType().c_str());
        parameter->SetAttribute(it->second.getParameterId().c_str(), it->second.getParameterValue().c_str());
        element->LinkEndChild(parameter);
    }

    for (auto it = parameters_int_.begin(); it != parameters_int_.end(); it++)
    {
        logdbg  << "Configuration: generateElement: int param " << it->first;
        XMLElement *parameter = document->NewElement(it->second.getParameterType().c_str());
        parameter->SetAttribute(it->second.getParameterId().c_str(), it->second.getParameterValue().c_str());
        element->LinkEndChild(parameter);
    }

    for (auto it = parameters_uint_.begin(); it != parameters_uint_.end(); it++)
    {
        logdbg  << "Configuration: generateElement: uint param " << it->first;
        XMLElement *parameter = document->NewElement(it->second.getParameterType().c_str());
        parameter->SetAttribute(it->second.getParameterId().c_str(), it->second.getParameterValue().c_str());
        element->LinkEndChild(parameter);
    }

    for (auto it = parameters_float_.begin(); it != parameters_float_.end(); it++)
    {
        logdbg  << "Configuration: generateElement: float param " << it->first;
        XMLElement *parameter = document->NewElement(it->second.getParameterType().c_str());
        parameter->SetAttribute(it->second.getParameterId().c_str(), it->second.getParameterValue().c_str());
        element->LinkEndChild(parameter);
    }

    for (auto it = parameters_double_.begin(); it != parameters_double_.end(); it++)
    {
        logdbg  << "Configuration: generateElement: double param " << it->first;
        XMLElement *parameter = document->NewElement(it->second.getParameterType().c_str());
        parameter->SetAttribute(it->second.getParameterId().c_str(), it->second.getParameterValue().c_str());
        element->LinkEndChild(parameter);
    }

    for (auto it = parameters_string_.begin(); it != parameters_string_.end(); it++)
    {
        logdbg  << "Configuration: generateElement: string param " << it->first;
        XMLElement *parameter = document->NewElement(it->second.getParameterType().c_str());
        parameter->SetAttribute(it->second.getParameterId().c_str(), it->second.getParameterValue().c_str());
        element->LinkEndChild(parameter);
    }

    std::map<std::string, Configuration>::const_iterator tit; // coincidence
    for (tit = configuration_templates_.begin(); tit != configuration_templates_.end(); tit++)
    {
        assert (tit->second.getTemplateFlag());
        XMLElement *config = tit->second.generateXMLElement(document);
        element->LinkEndChild(config);
    }

    std::map<std::pair<std::string, std::string>, Configuration >::const_iterator cit;
    for (cit = sub_configurations_.begin(); cit != sub_configurations_.end(); cit++)
    {
        XMLElement *config = cit->second.generateXMLElement(document);
        element->LinkEndChild(config);
    }

    if (configuration_filename_.size() > 0)
    {
        logdbg  << "Configuration: generateElement: saving sub-configuration file '" << configuration_filename_ << "'";

        document->LinkEndChild( element );
        document->SaveFile(configuration_filename_.c_str());
        delete document;

        XMLElement *sub_file_element = parent_document->NewElement( "SubConfigurationFile" );
        sub_file_element->SetAttribute("class_id", class_id_.c_str());
        sub_file_element->SetAttribute("instance_id", instance_id_.c_str());
        sub_file_element->SetAttribute ("path", configuration_filename_.c_str());

        return sub_file_element;
    }
    else
        return element;
}

void Configuration::createSubConfigurables (Configurable *configurable)
{
    logdbg  << "Configuration: createSubConfigurables: config instance " << instance_id_ << " configurable instance " <<
               configurable->getInstanceId()   ;

    std::map<std::pair<std::string, std::string>, Configuration >::iterator it;

    for (it = sub_configurations_.begin(); it != sub_configurations_.end(); it++)
    {
        //ConfigurableDefinition &mos_def = sub_configurables_.at(cnt);
        logdbg << "Configuration: createSubConfigurables: generateSubConfigurable: class_id '" << it->first.first << "' instance_id '"
               << it->first.second <<"'";
        configurable->generateSubConfigurable (it->first.first, it->first.second);
    }

    logdbg  << "Configuration: createSubConfigurables: instance " << instance_id_ << " end";

}

void Configuration::setConfigurationFilename (const std::string &configuration_filename)
{
    assert (configuration_filename.size() > 0);
    configuration_filename_=configuration_filename;
}

bool Configuration::hasConfigurationFilename ()
{
    return configuration_filename_.size() != 0;
}
const std::string &Configuration::getConfigurationFilename ()
{
    assert (hasConfigurationFilename());
    return configuration_filename_;
}

Configuration &Configuration::addNewSubConfiguration (const std::string &class_id, const std::string &instance_id)
{
    std::pair<std::string, std::string> key (class_id, instance_id);
    assert (sub_configurations_.find (key) == sub_configurations_.end());
    sub_configurations_.insert(std::pair <std::pair<std::string, std::string>, Configuration> (key, Configuration (class_id, instance_id)));
    return sub_configurations_.at(key);
}

Configuration &Configuration::addNewSubConfiguration (const std::string &class_id)
{
    int instance_number=-1;

    std::map<std::pair<std::string, std::string>, Configuration >::iterator it;

    for (it = sub_configurations_.begin(); it != sub_configurations_.end(); it++)
    {
        if (it->first.first.compare (class_id) == 0)
        {
            int num = String::getAppendedInt (it->first.second);
            if (num > instance_number)
                instance_number=num;
        }
    }
    instance_number++;

    return addNewSubConfiguration (class_id, class_id+String::intToString (instance_number));
}

Configuration &Configuration::addNewSubConfiguration (Configuration &configuration)
{
    std::pair<std::string, std::string> key (configuration.getClassId(), configuration.getInstanceId());
    assert (sub_configurations_.find (key) == sub_configurations_.end());
    sub_configurations_.insert(std::pair <std::pair<std::string, std::string>, Configuration> (key, configuration));
    return sub_configurations_.at(key);
}

Configuration &Configuration::getSubConfiguration (const std::string &class_id, const std::string &instance_id)
{
    std::pair<std::string, std::string> key (class_id, instance_id);

    if (sub_configurations_.find (key) == sub_configurations_.end())
    {
        loginf << "Configuration instance " << instance_id_ <<": getSubConfiguration: creating new (empty) configuration for class " << class_id <<
                  " instance " << instance_id;
        addNewSubConfiguration(class_id, instance_id);
    }

    assert (sub_configurations_.find (key) != sub_configurations_.end());
    return sub_configurations_.at(key);
}

void Configuration::removeSubConfiguration (const std::string &class_id, const std::string &instance_id)
{
    //    loginf << "Configuration: removeSubConfiguration: me "  << class_id_ << " " << instance_id_
    //            << " you " << class_id << " " << instance_id;

    std::pair<std::string, std::string> key (class_id, instance_id);
    assert (sub_configurations_.find (key) != sub_configurations_.end());
    sub_configurations_.erase(sub_configurations_.find (key));
}

void Configuration::setTemplate (bool template_flag, const std::string &template_name)
{
    template_flag_ = template_flag;
    template_name_ = template_name;

    loginf << "Configuration: setTemplate: " << class_id_ << " instance " << instance_id_ << " flag " << template_flag
           << " name " << template_name;

}

bool Configuration::getSubTemplateNameFree (const std::string &template_name)
{
    return configuration_templates_.find (template_name) == configuration_templates_.end();
}

void Configuration::addSubTemplate (Configuration *configuration, const std::string &template_name)
{
    assert (getSubTemplateNameFree(template_name));
    configuration_templates_.insert (std::pair<std::string, Configuration> (template_name, *configuration));
    configuration_templates_.at(template_name).setTemplate(true, template_name);
    delete configuration;
}

template<> void ConfigurableParameter<bool>::setConfigValue (const tinyxml2::XMLAttribute* attribute)
{
    config_value_ = attribute->BoolValue();
}

template<> void ConfigurableParameter<int>::setConfigValue (const tinyxml2::XMLAttribute* attribute)
{
    config_value_ = attribute->IntValue();
}

template<> void ConfigurableParameter<unsigned int>::setConfigValue (const tinyxml2::XMLAttribute* attribute)
{
    config_value_ = attribute->UnsignedValue();
}

template<> void ConfigurableParameter<float>::setConfigValue (const tinyxml2::XMLAttribute* attribute)
{
    config_value_ = attribute->FloatValue();
}

template<> void ConfigurableParameter<double>::setConfigValue (const tinyxml2::XMLAttribute* attribute)
{
    config_value_ = attribute->DoubleValue();
}

template<> void ConfigurableParameter<std::string>::setConfigValue (const tinyxml2::XMLAttribute* attribute)
{
    config_value_ = attribute->Value();
}

template<> std::string ConfigurableParameter<bool>::getParameterType () const
{
    return "ParameterBool";
}

template<> std::string ConfigurableParameter<int>::getParameterType () const
{
    return "ParameterInt";
}

template<> std::string ConfigurableParameter<unsigned int>::getParameterType () const
{
    return "ParameterUnsignedInt";
}

template<> std::string ConfigurableParameter<float>::getParameterType () const
{
    return "ParameterFloat";
}

template<> std::string ConfigurableParameter<double>::getParameterType () const
{
    return "ParameterDouble";
}

template<> std::string ConfigurableParameter<std::string>::getParameterType () const
{
    return "ParameterString";
}

template<> std::string ConfigurableParameter<bool>::getParameterValue () const
{
    if (pointer_)
        return Utils::String::intToString(*pointer_);
    else
        return Utils::String::intToString(config_value_);
}

template<> std::string ConfigurableParameter<int>::getParameterValue () const
{
    if (pointer_)
        return Utils::String::intToString(*pointer_);
    else
        return Utils::String::intToString(config_value_);
}

template<> std::string ConfigurableParameter<unsigned int>::getParameterValue () const
{
    if (pointer_)
        return Utils::String::uIntToString(*pointer_);
    else
        return Utils::String::uIntToString(config_value_);
}

template<> std::string ConfigurableParameter<float>::getParameterValue () const
{
    if (pointer_)
        return Utils::String::doubleToStringPrecision(*pointer_, 8);
    else
        return Utils::String::doubleToStringPrecision(config_value_, 8);
}

template<> std::string ConfigurableParameter<double>::getParameterValue () const
{
    if (pointer_)
        return Utils::String::doubleToStringPrecision(*pointer_, 12);
    else
        return Utils::String::doubleToStringPrecision(config_value_, 12);
}

template<> std::string ConfigurableParameter<std::string>::getParameterValue () const
{
    if (pointer_)
        return *pointer_;
    else
        return config_value_;
}
