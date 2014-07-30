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
 * @brief Configuration parameter base class
 *
 * @details Any class deriving from this class has to override the following functions:
 *
 * @code
 * public:
 *   virtual std::string getParameterType ();
 *   virtual std::string getParameterId ();
 *   virtual std::string getParameterValue ();
 *   virtual void resetToDefault ();
 * @endcode
 */
class ConfigurableParameterBase
{
public:
    /// Constructor
    ConfigurableParameterBase () {}
    /// Destructor
    virtual ~ConfigurableParameterBase () {}

    virtual ConfigurableParameterBase *clone () = 0;

    /// Returns parameter type
    virtual std::string getParameterType ()=0;
    /// Returns parameter identifier
    virtual std::string getParameterId ()=0;
    /// Returns parameter value as string
    virtual std::string getParameterValue ()=0;
    /// Resets parameter to its default value
    virtual void resetToDefault ()=0;
};

/**
 * @brief Configuration parameter template class
 *
 * @details Several types of parameters are implement here:
 * "ParameterBool", "ParameterInt", "ParameterUnsignedInt", "ParameterFloat", "ParameterDouble", "ParameterString"
 *
 * For all such types one template class was implemented to unify setting and retrieving the XML configuration.
 */
template <class T> class ConfigurableParameter : public ConfigurableParameterBase
{
public:
    /// Constructor, initializes members
    ConfigurableParameter () : pointer_(0)
{
        if (typeid(T) != typeid(std::string))
        {
            memset (&config_value_, 0, sizeof (T));
            memset (&default_value_, 0, sizeof (T));
        }
};

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

    /// Clones the current instance with the copy constructor
    virtual ConfigurableParameterBase *clone ()
    {
        return new ConfigurableParameter(*this);
    }

    /// Desctructor
    virtual ~ConfigurableParameter () {};
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

            if (typeid(T) == typeid(bool))
                config_value_ = attribute->BoolValue();
            else if (typeid(T) == typeid(char) || typeid(T) == typeid(short int) || typeid(T) == typeid(int))
                config_value_ = attribute->IntValue();
            else if (typeid(T) == typeid(unsigned char) || typeid(T) == typeid(unsigned short int)
                    || typeid(T) == typeid(unsigned int))
                config_value_ = attribute->UnsignedValue();
            else if (typeid(T) == typeid(float))
                config_value_ = attribute->FloatValue();
            else if (typeid(T) == typeid(double))
                config_value_ = attribute->DoubleValue();
            else
                throw std::runtime_error ("ConfigurableParameter: parseElement: unknown class type");

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
        if (typeid(T) == typeid(bool))
            return "ParameterBool";
        else if (typeid(T) == typeid(char))
            return "ParameterChar";
        else if (typeid(T) == typeid(unsigned char))
            return "ParameterUnsignedChar";
        else if (typeid(T) == typeid(short int))
            return "ParameterShortInt";
        else if (typeid(T) == typeid(unsigned short int))
            return "ParameterUnsignedShortInt";
        else if (typeid(T) == typeid(int))
            return "ParameterInt";
        else if (typeid(T) == typeid(unsigned int))
            return "ParameterUnsignedInt";
        else if (typeid(T) == typeid(float))
            return "ParameterFloat";
        else if (typeid(T) == typeid(double))
            return "ParameterDouble";
        else
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
        std::stringstream ss;

        if (pointer_)
        {
            if (typeid(T) == typeid(float))
                return Utils::String::doubleToStringPrecision(*pointer_, 8);
            else if (typeid(T) == typeid(double))
                return Utils::String::doubleToStringPrecision(*pointer_, 12);

            ss << *pointer_;
        }
        else
            ss << config_value_;

        return ss.str();
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
};

template <> class ConfigurableParameter <std::string>: public ConfigurableParameterBase
{
public:
    /// Constructor, initializes members
    ConfigurableParameter () : pointer_(0)
{
        if (typeid(std::string) != typeid(std::string))
        {
            memset (&config_value_, 0, sizeof (std::string));
            memset (&default_value_, 0, sizeof (std::string));
        }
};

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

    /// Clones the current instance with the copy constructor
    virtual ConfigurableParameterBase *clone ()
    {
        return new ConfigurableParameter(*this);
    }

    /// Desctructor
    virtual ~ConfigurableParameter () {};
    /// Parameter identifier
    std::string parameter_id_;
    /// Template pointer to real value
    std::string *pointer_;
    /// Template configuration value
    std::string config_value_;
    /// Template default value as given by registerParameter
    std::string default_value_;

    /// Parses an element from the XML configuration and sets parameter_id_ and config_value_ (using a stringstream)
    void parseElement (const tinyxml2::XMLElement *element)
    {
        std::string configuration_id = element->Value();
        const tinyxml2::XMLAttribute* attribute=element->FirstAttribute();
        while (attribute)
        {
            logdbg  << "ConfigurableParameter " << configuration_id << ": parseElement: attribute " << attribute->Name() << "  value "<< attribute->Value();

            parameter_id_=attribute->Name();

            config_value_ = attribute->Value();

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
        return "ParameterString";
    }

    /// Returns parameter identifier
    std::string getParameterId ()
    {
        return parameter_id_;
    }

    /// Returns parameter value as string (using a stringstream)
    std::string getParameterValue ()
    {
        if (pointer_)
            return *pointer_;

        return config_value_;
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
};

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
    Configuration(std::string class_id, std::string instance_id, std::string configuration_filename="");
    /// @brief Default constructor for stl containers
    Configuration();
    /// @brief Copy constructor
    Configuration(const Configuration &source);
    /// @brief Destructor
    virtual ~Configuration();

    Configuration& operator= (const Configuration &source);
    Configuration *clone ();

    /// @brief Registers a boolean parameter
    void registerParameter (std::string parameter_id, bool *pointer, bool default_value);
    /// @brief Registers an char parameter
    void registerParameter (std::string parameter_id, char *pointer, char default_value);
    /// @brief Registers an unsigned char parameter
    void registerParameter (std::string parameter_id, unsigned char *pointer, unsigned char default_value);
    /// @brief Registers an short int parameter
    void registerParameter (std::string parameter_id, short int *pointer, short int default_value);
    /// @brief Registers an unsigned short int parameter
    void registerParameter (std::string parameter_id, unsigned short int *pointer, unsigned short int default_value);
    /// @brief Registers an int parameter
    void registerParameter (std::string parameter_id, int *pointer, int default_value);
    /// @brief Registers an unsigned int parameter
    void registerParameter (std::string parameter_id, unsigned int *pointer, unsigned int default_value);
    /// @brief Registers a float parameter
    void registerParameter (std::string parameter_id, float *pointer, float default_value);
    /// @brief Registers a double parameter
    void registerParameter (std::string parameter_id, double *pointer, double default_value);
    /// @brief Registers a string parameter
    void registerParameter (std::string parameter_id, std::string *pointer, std::string default_value);

    /// @brief Adds a boolean parameter
    void addParameterBool (std::string parameter_id, bool default_value);
    /// @brief Adds a char parameter
    void addParameterChar (std::string parameter_id, char default_value);
    /// @brief Adds a unsigned char parameter
    void addParameterUnsignedChar (std::string parameter_id, unsigned char default_value);
    /// @brief Adds a short int parameter
    void addParameterShortInt (std::string parameter_id, short int default_value);
    /// @brief Adds a unsigned short parameter
    void addParameterUnsignedShortInt (std::string parameter_id, unsigned short int default_value);
    /// @brief Adds an integer parameter
    void addParameterInt (std::string parameter_id, int default_value);
    /// @brief Adds an unsigned int parameter
    void addParameterUnsignedInt (std::string parameter_id, unsigned int default_value);
    /// @brief Adds a float parameter
    void addParameterFloat (std::string parameter_id, float default_value);
    /// @brief Adds a double parameter
    void addParameterDouble (std::string parameter_id, double default_value);
    /// @brief Adds a string parameter
    void addParameterString (std::string parameter_id, std::string default_value);

    /// @brief Returns if an parameter exists
    bool existsParameter (std::string parameter_id);

    /// @brief Writes data value if a boolean parameter to an argument
    void getParameter (std::string parameter_id, bool &value);
    /// @brief Writes data value if an integer parameter to an argument
    void getParameter (std::string parameter_id, int &value);
    /// @brief Writes data value if an unsigned int parameter to an argument
    void getParameter (std::string parameter_id, unsigned int &value);
    /// @brief Writes data value if a float parameter to an argument
    void getParameter (std::string parameter_id, float &value);
    /// @brief Writes data value if a double parameter to an argument
    void getParameter (std::string parameter_id, double &value);
    /// @brief Writes data value if a string parameter to an argument
    void getParameter (std::string parameter_id, std::string &value);

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
    void setConfigurationFilename (std::string configuration_filename);
    /// @brief Returns flag if special filename has been set
    bool hasConfigurationFilename ();
    /// @brief Return special filename
    std::string getConfigurationFilename ();

    /// @brief Adds a new sub-configuration and returns reference
    Configuration &addNewSubConfiguration (std::string class_id, std::string instance_id);
    /// @brief Adds a new sub-configuration and returns reference
    Configuration &addNewSubConfiguration (std::string class_id);
    /// @brief Returns a sub-configuration, creates new empty one if non-existing
    Configuration &addNewSubConfiguration (Configuration &configuration);
    /// @brief Returns a specfigied sub-configuration
    Configuration &getSubConfiguration (std::string class_id, std::string instance_id);
    /// @brief Removes a sub-configuration
    void removeSubConfiguration (std::string class_id, std::string instance_id);

    /// @brief Returns the instance identifier
    std::string getInstanceId () { return instance_id_; }
    /// @brief Returns the class identifier
    std::string getClassId () { return class_id_; }

    /// @brief Sets the template flag and name
    void setTemplate (bool template_flag, std::string template_name_);
    /// @brief Returns the template flag
    bool getTemplateFlag () { return template_flag_; }
    /// @brief Returns the template name
    std::string getTemplateName () { return template_name_; }

    /// @brief Checks if a specific template_name is already taken, true if free
    bool getSubTemplateNameFree (std::string template_name);
    /// @brief Adds a template configuration with a name
    void addSubTemplate (Configuration *configuration, std::string template_name);

    /// @brief Return contaienr with all configuration templates
    std::map<std::string, Configuration> &getConfigurationTemplates () { return configuration_templates_; }

    // only use in special case of configuration copy
    void setInstanceId (std::string instance_id) { instance_id_ = instance_id; }

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
    std::map <std::string, ConfigurableParameterBase *> parameters_;
    /// Container for all added sub-configurables
    //std::vector <ConfigurableDefinition> sub_configurables_;
    std::map<std::pair<std::string, std::string>, Configuration > sub_configurations_;

    /// Flag which indicates if instance is a template
    bool template_flag_;
    /// Template name, empty if no template
    std::string template_name_;

    /// Container with all configuration templates
    std::map<std::string, Configuration> configuration_templates_;

    /// Undefined name for default constructor
    static const std::string UNDEFINED_NAME;
};

#endif /* CONFIGURATION_H_ */
