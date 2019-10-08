#include "configurableparameter.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;

template <typename T>
ConfigurableParameter<T>::ConfigurableParameter(const ConfigurableParameter &source)
{
    operator=(source);
}

template <typename T>
ConfigurableParameter<T>& ConfigurableParameter<T>::operator= (const ConfigurableParameter<T>& source)
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

template <typename T>
void ConfigurableParameter<T>::parseElement (const tinyxml2::XMLElement* element)
{
    std::string configuration_id = element->Value();
    const tinyxml2::XMLAttribute* attribute=element->FirstAttribute();
    while (attribute)
    {
        logdbg  << "ConfigurableParameter " << configuration_id << ": parseElement: attribute " << attribute->Name()
                << "  value "<< attribute->Value();

        parameter_id_=attribute->Name();
        setConfigValue (attribute);

        attribute=attribute->Next();
        assert (!attribute);
    }
}

template <typename T>
std::string ConfigurableParameter<T>::getParameterType () const
{
    throw std::runtime_error ("ConfigurableParameter: getParameterType: unknown class type");
}

template <typename T>
const std::string& ConfigurableParameter<T>::getParameterId () const
{
    return parameter_id_;
}

template <typename T>
std::string ConfigurableParameter<T>::getParameterValue () const
{
    throw std::runtime_error ("ConfigurableParameter: getParameterValue: unknown class type");
}

template <typename T>
void ConfigurableParameter<T>::resetToDefault ()
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
    {
        assert (pointer_);
        return std::to_string(*pointer_);
    }
    else
        return std::to_string(config_value_);
}

template<> std::string ConfigurableParameter<int>::getParameterValue () const
{
    if (pointer_)
    {
        assert (pointer_);
        return std::to_string(*pointer_);
    }
    else
        return std::to_string(config_value_);
}

template<> std::string ConfigurableParameter<unsigned int>::getParameterValue () const
{
    if (pointer_)
    {
        assert (pointer_);
        return std::to_string(*pointer_);
    }
    else
        return std::to_string(config_value_);
}

template<> std::string ConfigurableParameter<float>::getParameterValue () const
{
    if (pointer_)
    {
        assert (pointer_);
        return Utils::String::doubleToStringPrecision(*pointer_, 8);
    }
    else
        return Utils::String::doubleToStringPrecision(config_value_, 8);
}

template<> std::string ConfigurableParameter<double>::getParameterValue () const
{
    if (pointer_)
    {
        assert (pointer_);
        return Utils::String::doubleToStringPrecision(*pointer_, 12);
    }
    else
        return Utils::String::doubleToStringPrecision(config_value_, 12);
}

template<> std::string ConfigurableParameter<std::string>::getParameterValue () const
{
    if (pointer_)
    {
        assert (pointer_);
        return *pointer_;
    }
    else
    {
        return config_value_;
    }
}

template class ConfigurableParameter<bool>;
template class ConfigurableParameter<int>;
template class ConfigurableParameter<unsigned int>;
template class ConfigurableParameter<float>;
template class ConfigurableParameter<double>;
template class ConfigurableParameter<std::string>;
