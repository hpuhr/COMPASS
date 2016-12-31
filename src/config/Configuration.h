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
 * Configuration.h
 *
 *  Created on: May 15, 2012
 *      Author: sk
 */

#ifndef CONFIGURATION_H_
#define CONFIGURATION_H_

#include <map>
#include <string>
#include <tinyxml2.h>
#include <typeinfo>
#include <vector>
#include "Logger.h"
#include "String.h"

class Configurable;

/**
 * @brief Configuration parameter template class
 *
 * @details Several types of parameters are implement here:
 * "ParameterBool", "ParameterInt", "ParameterUnsignedInt", "ParameterFloat", "ParameterDouble", "ParameterString"
 *
 * For all such types one template class was implemented to unify setting and retrieving the XML configuration.
 */
template <class T> class ConfigurableParameter //: public ConfigurableParameterBase
{
public:
    /// Constructor, initializes members
    ConfigurableParameter () : pointer_(0)
    {}

    /// Copy constructor, uses assignment operator
    ConfigurableParameter(const ConfigurableParameter &source)
    {
        operator=(source);
    }

    /// Assignment operator, make deep copy and discards the pointer.
    virtual ConfigurableParameter& operator= (const ConfigurableParameter &source)
    {
        parameter_id_ = source.parameter_id_;
        pointer_=0;
        if (source.pointer_)
            config_value_ = *source.pointer_;
        else
            config_value_ = source.config_value_;
        default_value_ = source.default_value_;

        return *this;
    }

    /// Desctructor
    virtual ~ConfigurableParameter () {}
    /// Parameter identifier
    std::string parameter_id_;
    /// Template pointer to real value
    T *pointer_;
    /// Template configuration value
    T config_value_;
    /// Template default value as given by registerParameter
    T default_value_;

    /// Parses an element from the XML configuration and sets parameter_id_ and config_value_ (using a stringstream)
    void parseElement (const tinyxml2::XMLElement *element)
    {
        std::string configuration_id = element->Value();
        const tinyxml2::XMLAttribute* attribute=element->FirstAttribute();
        while (attribute)
        {
            logdbg  << "ConfigurableParameter " << configuration_id << ": parseElement: attribute " << attribute->Name() << "  value "<< attribute->Value();

            parameter_id_=attribute->Name();
            setConfigValue (attribute);

            attribute=attribute->Next();
            assert (!attribute);
        }
    }

    /**
     * Returns the parameter type as string
     *
     * \exception std::runtime_error if unknown class is used
     */
    std::string getParameterType ()
    {
        throw std::runtime_error ("ConfigurableParameter: getParameterType: unknown class type");
    }

    /// Returns parameter identifier
    std::string getParameterId ()
    {
        return parameter_id_;
    }

    /// Returns parameter value as string (using a stringstream)
    std::string getParameterValue ()
    {
        throw std::runtime_error ("ConfigurableParameter: getParameterValue: unknown class type");
    }

    /// Sets pointer_ to default value if valid, otherwise sets config_value_ to default_value_
    void resetToDefault ()
    {
        std::stringstream ss;
        ss << "ConfigurableParameter: resetToDefault: parameter '" << parameter_id_ << "' default value '" << default_value_ << "'";

        if (pointer_)
        {
            ss << " ptr not null value '" << *pointer_ << "'";
            *pointer_=default_value_;
        }
        else
        {
            ss << " ptr null";
            config_value_=default_value_;
        }

        loginf  << ss.str();
    }

protected:
    void setConfigValue (const tinyxml2::XMLAttribute* attribute);
};

template<> void ConfigurableParameter<bool>::setConfigValue (const tinyxml2::XMLAttribute* attribute);
template<> void ConfigurableParameter<int>::setConfigValue (const tinyxml2::XMLAttribute* attribute);
template<> void ConfigurableParameter<unsigned int>::setConfigValue (const tinyxml2::XMLAttribute* attribute);
template<> void ConfigurableParameter<float>::setConfigValue (const tinyxml2::XMLAttribute* attribute);
template<> void ConfigurableParameter<double>::setConfigValue (const tinyxml2::XMLAttribute* attribute);
template<> void ConfigurableParameter<std::string>::setConfigValue (const tinyxml2::XMLAttribute* attribute);

template<> std::string ConfigurableParameter<bool>::getParameterType ();
template<> std::string ConfigurableParameter<int>::getParameterType ();
template<> std::string ConfigurableParameter<unsigned int>::getParameterType ();
template<> std::string ConfigurableParameter<float>::getParameterType ();
template<> std::string ConfigurableParameter<double>::getParameterType ();
template<> std::string ConfigurableParameter<std::string>::getParameterType ();

template<> std::string ConfigurableParameter<bool>::getParameterValue ();
template<> std::string ConfigurableParameter<int>::getParameterValue ();
template<> std::string ConfigurableParameter<unsigned int>::getParameterValue ();
template<> std::string ConfigurableParameter<float>::getParameterValue ();
template<> std::string ConfigurableParameter<double>::getParameterValue ();
template<> std::string ConfigurableParameter<std::string>::getParameterValue ();

/**
 * @brief Configuration storage and retrieval container class
 *
 * @details This class is used by a Configurable to store and retrieve parameters to/from an XML configuration system.
 *
 * Is held by a Configurable or can exist on its own.
 *
 * \todo Extend registerParameter to template function.
 * \todo Extend addParameter to template function.
 */
class Configuration
{
public:
    /// @brief Constructor
    Configuration(const std::string &class_id, const std::string &instance_id, const std::string &configuration_filename="");

    /// @brief Copy constructor
    Configuration(const Configuration &source);
    /// @brief Destructor
    virtual ~Configuration();

    Configuration& operator= (const Configuration &source);
    Configuration *clone ();

    /// @brief Registers a boolean parameter
    void registerParameter (const std::string &parameter_id, bool *pointer, bool default_value);
    /// @brief Registers an int parameter
    void registerParameter (const std::string &parameter_id, int *pointer, int default_value);
    /// @brief Registers an unsigned int parameter
    void registerParameter (const std::string &parameter_id, unsigned int *pointer, unsigned int default_value);
    /// @brief Registers a float parameter
    void registerParameter (const std::string &parameter_id, float *pointer, float default_value);
    /// @brief Registers a double parameter
    void registerParameter (const std::string &parameter_id, double *pointer, double default_value);
    /// @brief Registers a string parameter
    void registerParameter (const std::string &parameter_id, std::string *pointer, const std::string &default_value);

    /// @brief Adds a boolean parameter
    void addParameterBool (const std::string &parameter_id, bool default_value);
    /// @brief Adds an integer parameter
    void addParameterInt (const std::string &parameter_id, int default_value);
    /// @brief Adds an unsigned int parameter
    void addParameterUnsignedInt (const std::string &parameter_id, unsigned int default_value);
    /// @brief Adds a float parameter
    void addParameterFloat (const std::string &parameter_id, float default_value);
    /// @brief Adds a double parameter
    void addParameterDouble (const std::string &parameter_id, double default_value);
    /// @brief Adds a string parameter
    void addParameterString (const std::string &, const std::string &default_value);

    /// @brief Writes data value if a boolean parameter to an argument
    void getParameter (const std::string &parameter_id, bool &value);
    /// @brief Writes data value if an integer parameter to an argument
    void getParameter (const std::string &parameter_id, int &value);
    /// @brief Writes data value if an unsigned int parameter to an argument
    void getParameter (const std::string &parameter_id, unsigned int &value);
    /// @brief Writes data value if a float parameter to an argument
    void getParameter (const std::string &parameter_id, float &value);
    /// @brief Writes data value if a double parameter to an argument
    void getParameter (const std::string &parameter_id, double &value);
    /// @brief Writes data value if a string parameter to an argument
    void getParameter (const std::string &parameter_id, std::string &value);

    /// @brief Parses an XML configuration element
    void parseXMLElement (tinyxml2::XMLElement *element);
    /// @brief Generates an XML configuration element
    tinyxml2::XMLElement *generateXMLElement (tinyxml2::XMLDocument *root_document);

    /// @brief Resets all values to their default values
    void resetToDefault ();

    /// @brief Creates added sub-configurables in configurable
    void createSubConfigurables (Configurable *configurable);

    /// @brief Returns flag indicating if configuration has been used by a configurable
    bool getUsed () {return used_; };

    /// @brief Sets special filename for XML configuration
    void setConfigurationFilename (const std::string &configuration_filename);
    /// @brief Returns flag if special filename has been set
    bool hasConfigurationFilename ();
    /// @brief Return special filename
    const std::string & getConfigurationFilename ();

    /// @brief Adds a new sub-configuration and returns reference
    Configuration &addNewSubConfiguration (const std::string &class_id, const std::string &instance_id);
    /// @brief Adds a new sub-configuration and returns reference
    Configuration &addNewSubConfiguration (const std::string &class_id);
    /// @brief Returns a sub-configuration, creates new empty one if non-existing
    Configuration &addNewSubConfiguration (Configuration &configuration);
    /// @brief Returns a specfigied sub-configuration
    Configuration &getSubConfiguration (const std::string &class_id, const std::string &instance_id);
    /// @brief Removes a sub-configuration
    void removeSubConfiguration (const std::string &class_id, const std::string &instance_id);

    /// @brief Returns the instance identifier
    const std::string &getInstanceId () { return instance_id_; }
    /// @brief Returns the class identifier
    const std::string &getClassId () { return class_id_; }

    /// @brief Sets the template flag and name
    void setTemplate (bool template_flag, const std::string &template_name_);
    /// @brief Returns the template flag
    bool getTemplateFlag () { return template_flag_; }
    /// @brief Returns the template name
    const std::string &getTemplateName () { return template_name_; }

    /// @brief Checks if a specific template_name is already taken, true if free
    bool getSubTemplateNameFree (const std::string & template_name);
    /// @brief Adds a template configuration with a name
    void addSubTemplate (Configuration *configuration, const std::string &template_name);

    /// @brief Return contaienr with all configuration templates
    std::map<std::string, Configuration> &getConfigurationTemplates () { return configuration_templates_; }

    // only use in special case of configuration copy
    void setInstanceId (const std::string &instance_id) { instance_id_ = instance_id; }

protected:
    /// Class identifier
    std::string class_id_;
    /// Instance identifier
    std::string instance_id_;
    /// Flag indicating if configuration has been used by configurable
    bool used_;
    /// Special XML configuration filename
    std::string configuration_filename_;

    /// Container for all parameters (parameter identifier -> ConfigurableParameterBase)
    std::map <std::string, ConfigurableParameter<bool> > parameters_bool_;
    std::map <std::string, ConfigurableParameter<int> > parameters_int_;
    std::map <std::string, ConfigurableParameter<unsigned int> > parameters_uint_;
    std::map <std::string, ConfigurableParameter<float> > parameters_float_;
    std::map <std::string, ConfigurableParameter<double> > parameters_double_;
    std::map <std::string, ConfigurableParameter<std::string> > parameters_string_;
    /// Container for all added sub-configurables
    std::map<std::pair<std::string, std::string>, Configuration> sub_configurations_;

    /// Flag which indicates if instance is a template
    bool template_flag_;
    /// Template name, empty if no template
    std::string template_name_;

    /// Container with all configuration templates
    std::map<std::string, Configuration> configuration_templates_;
};

#endif /* CONFIGURATION_H_ */
