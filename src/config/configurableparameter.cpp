#include "configurableparameter.h"

#include "logger.h"
#include "stringconv.h"

using namespace Utils;

template <typename T>
ConfigurableParameter<T>::ConfigurableParameter(const ConfigurableParameter& source)
{
    operator=(source);
}

template <typename T>
ConfigurableParameter<T>& ConfigurableParameter<T>::operator=(
    const ConfigurableParameter<T>& source)
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
std::string ConfigurableParameter<T>::getParameterType() const
{
    throw std::runtime_error("ConfigurableParameter: getParameterType: unknown class type");
}

template <typename T>
const std::string& ConfigurableParameter<T>::getParameterId() const
{
    return parameter_id_;
}

template <typename T>
T ConfigurableParameter<T>::getParameterValue() const
{
    if (pointer_)
    {
        assert(pointer_);
        return *pointer_;
    }
    else
        return config_value_;
}

template <typename T>
std::string ConfigurableParameter<T>::getParameterValueString() const
{
    throw std::runtime_error("ConfigurableParameter: getParameterValue: unknown class type");
}

template <typename T>
void ConfigurableParameter<T>::resetToDefault()
{
    std::stringstream ss;
    ss << "ConfigurableParameter: resetToDefault: parameter '" << parameter_id_
       << "' default value '" << default_value_ << "'";

    if (pointer_)
    {
        ss << " ptr not null value '" << *pointer_ << "'";
        *pointer_ = default_value_;
    }
    else
    {
        ss << " ptr null";
        config_value_ = default_value_;
    }

    loginf << ss.str();
}

template <>
std::string ConfigurableParameter<bool>::getParameterType() const
{
    return "ParameterBool";
}

template <>
std::string ConfigurableParameter<int>::getParameterType() const
{
    return "ParameterInt";
}

template <>
std::string ConfigurableParameter<unsigned int>::getParameterType() const
{
    return "ParameterUnsignedInt";
}

template <>
std::string ConfigurableParameter<float>::getParameterType() const
{
    return "ParameterFloat";
}

template <>
std::string ConfigurableParameter<double>::getParameterType() const
{
    return "ParameterDouble";
}

template <>
std::string ConfigurableParameter<std::string>::getParameterType() const
{
    return "ParameterString";
}

template <>
std::string ConfigurableParameter<nlohmann::json>::getParameterType() const
{
    return "ParameterJSON";
}

template <>
std::string ConfigurableParameter<bool>::getParameterValueString() const
{
    if (pointer_)
    {
        assert(pointer_);
        return std::to_string(*pointer_);
    }
    else
        return std::to_string(config_value_);
}

template <>
std::string ConfigurableParameter<int>::getParameterValueString() const
{
    if (pointer_)
    {
        assert(pointer_);
        return std::to_string(*pointer_);
    }
    else
        return std::to_string(config_value_);
}

template <>
std::string ConfigurableParameter<unsigned int>::getParameterValueString() const
{
    if (pointer_)
    {
        assert(pointer_);
        return std::to_string(*pointer_);
    }
    else
        return std::to_string(config_value_);
}

template <>
std::string ConfigurableParameter<float>::getParameterValueString() const
{
    if (pointer_)
    {
        assert(pointer_);
        return Utils::String::doubleToStringPrecision(*pointer_, 8);
    }
    else
        return Utils::String::doubleToStringPrecision(config_value_, 8);
}

template <>
std::string ConfigurableParameter<double>::getParameterValueString() const
{
    if (pointer_)
    {
        assert(pointer_);
        return Utils::String::doubleToStringPrecision(*pointer_, 12);
    }
    else
        return Utils::String::doubleToStringPrecision(config_value_, 12);
}

template <>
std::string ConfigurableParameter<std::string>::getParameterValueString() const
{
    if (pointer_)
    {
        assert(pointer_);
        return *pointer_;
    }
    else
    {
        return config_value_;
    }
}

template <>
std::string ConfigurableParameter<nlohmann::json>::getParameterValueString() const
{
    if (pointer_)
    {
        assert(pointer_);
        return pointer_->dump(4);
    }
    else
    {
        return config_value_.dump(4);
    }
}

template class ConfigurableParameter<bool>;
template class ConfigurableParameter<int>;
template class ConfigurableParameter<unsigned int>;
template class ConfigurableParameter<float>;
template class ConfigurableParameter<double>;
template class ConfigurableParameter<std::string>;
template class ConfigurableParameter<nlohmann::json>;
