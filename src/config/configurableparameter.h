#ifndef CONFIGURABLPPARAMETER_H
#define CONFIGURABLPPARAMETER_H

#include "logger.h"
#include "stringconv.h"

#include <tinyxml2.h>
#include <string>

class Configurable;

/**
 * @brief Configuration parameter template class
 *
 * @details Several types of parameters are implement here:
 * "ParameterBool", "ParameterInt", "ParameterUnsignedInt", "ParameterFloat", "ParameterDouble", "ParameterString"
 *
 * For all such types one template class was implemented to unify setting and retrieving the XML configuration.
 */
template <class T> class ConfigurableParameter
{
public:
    /// Constructor, initializes members
    explicit ConfigurableParameter () {}

    /// Copy constructor, uses assignment operator
    ConfigurableParameter(const ConfigurableParameter &source)
    {
        operator=(source);
    }

    /// Assignment operator, make deep copy and discards the pointer.
    virtual ConfigurableParameter& operator= (const ConfigurableParameter& source)
    {
        parameter_id_ = source.parameter_id_;
        pointer_ = nullptr;
        if (source.pointer_)
            config_value_ = *source.pointer_;
        else
            config_value_ = source.config_value_;
        default_value_ = source.default_value_;

        return *this;
    }

    /// Destructor
    virtual ~ConfigurableParameter () {}

    /// Parameter identifier
    std::string parameter_id_;
    /// Template pointer to real value
    T* pointer_{nullptr};
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
            logdbg  << "ConfigurableParameter " << configuration_id << ": parseElement: attribute "
                    << attribute->Name() << "  value "<< attribute->Value();

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
    std::string getParameterType () const
    {
        throw std::runtime_error ("ConfigurableParameter: getParameterType: unknown class type");
    }


    /// Returns parameter identifier
    const std::string& getParameterId () const
    {
        return parameter_id_;
    }

    /// Returns parameter value as string (using a stringstream)
    std::string getParameterValue () const
    {
        throw std::runtime_error ("ConfigurableParameter: getParameterValue: unknown class type");
    }


    /// Sets pointer_ to default value if valid, otherwise sets config_value_ to default_value_
    void resetToDefault ()
    {
        std::stringstream ss;
        ss << "ConfigurableParameter: resetToDefault: parameter '" << parameter_id_ << "' default value '"
           << default_value_ << "'";

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
    void setConfigValue (const tinyxml2::XMLAttribute* attribute) { assert (false); }
};


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
        return std::to_string(*pointer_);
    else
        return std::to_string(config_value_);
}

template<> std::string ConfigurableParameter<int>::getParameterValue () const
{
    if (pointer_)
        return std::to_string(*pointer_);
    else
        return std::to_string(config_value_);
}

template<> std::string ConfigurableParameter<unsigned int>::getParameterValue () const
{
    if (pointer_)
        return std::to_string(*pointer_);
    else
        return std::to_string(config_value_);
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

#endif // CONFIGURABLPPARAMETER_H
