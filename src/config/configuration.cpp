/*
*  This file is part of ATSDB.
*
*  ATSDB is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  ATSDB is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.

*  You should have received a copy of the GNU General Public License
*  along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "configuration.h"

#include <fstream>
#include <typeinfo>

#include "configurable.h"
#include "files.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;

using namespace nlohmann;

/*
 *  Initializes members
 *
 *  \param configuration_id configuration identifier
 *  \param configuration_filename special filename, default ""
 */
Configuration::Configuration(const std::string& class_id, const std::string& instance_id,
                             const std::string& configuration_filename)
    : class_id_(class_id),
      instance_id_(instance_id),
      configuration_filename_(configuration_filename)
{
    logdbg << "Configuration: constructor: class " << class_id_ << " instance " << instance_id_;
    assert(class_id_.size() != 0);
    assert(instance_id_.size() != 0);
}

Configuration::Configuration(const Configuration& source) { operator=(source); }

// Configuration& Configuration::operator= (const Configuration& source)
//{
//    class_id_ = source.class_id_;
//    /// Instance identifier
//    instance_id_ = source.instance_id_; // needs to be overwritten
//    /// Flag indicating if configuration has been used by configurable
//    used_ = false;
//    /// Special XML configuration filename
//    configuration_filename_ = source.configuration_filename_;

//    template_flag_=source.template_flag_;
//    template_name_=source.template_name_;

//    /// Container for all parameters (parameter identifier -> ConfigurableParameterBase)
//    parameters_bool_ = source.parameters_bool_;
//    parameters_int_ = source.parameters_int_;
//    parameters_uint_ = source.parameters_uint_;
//    parameters_float_ = source.parameters_float_;
//    parameters_double_ = source.parameters_double_;
//    parameters_string_ = source.parameters_string_;

//    /// Container for all added sub-configurables
//    sub_configurations_ = source.sub_configurations_;

//    // return the existing object
//    return* this;
//}

// Configuration* Configuration::clone ()
//{
//    return new Configuration (*this);
//}

Configuration::~Configuration()
{
    parameters_bool_.clear();
    parameters_int_.clear();
    parameters_uint_.clear();
    parameters_float_.clear();
    parameters_double_.clear();
    parameters_string_.clear();
    parameters_json_.clear();
}

/*
 *  Resets all parameters to their default values
 */
void Configuration::resetToDefault()
{
    logdbg << "Configuration: resetToDefault: " << instance_id_;

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
    for (auto it = parameters_json_.begin(); it != parameters_json_.end(); it++)
        it->second.resetToDefault();
}

void Configuration::registerParameter(const std::string& parameter_id, bool* pointer,
                                      bool default_value)
{
    logdbg << "Configuration " << instance_id_ << ": registerParameter: bool: " << parameter_id;

    assert(pointer);

    if (org_config_parameters_.contains(parameter_id) &&
        parameters_bool_.find(parameter_id) == parameters_bool_.end())
    // if exists in org json config and not yet stored in parameters
    {
        parameters_bool_.insert(std::make_pair(parameter_id, ConfigurableParameter<bool>()));
        assert(org_config_parameters_.at(parameter_id).is_boolean());
        parameters_bool_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_bool_.at(parameter_id).config_value_ = org_config_parameters_.at(parameter_id);
    }

    if (parameters_bool_.find(parameter_id) ==
        parameters_bool_.end())  // new parameter, didnt exist in config
    {
        parameters_bool_.insert(std::pair<std::string, ConfigurableParameter<bool>>(
            parameter_id, ConfigurableParameter<bool>()));
        parameters_bool_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_bool_.at(parameter_id).config_value_ = default_value;
    }

    parameters_bool_.at(parameter_id).pointer_ = pointer;
    parameters_bool_.at(parameter_id).default_value_ = default_value;
    *(parameters_bool_.at(parameter_id).pointer_) = parameters_bool_.at(parameter_id).config_value_;
    used_ = true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter " << parameter_id
           << ": bool, value is " << *(parameters_bool_.at(parameter_id).pointer_);
}

void Configuration::registerParameter(const std::string& parameter_id, int* pointer,
                                      int default_value)
{
    logdbg << "Configuration " << instance_id_ << ": registerParameter: int: " << parameter_id;

    assert(pointer);

    if (org_config_parameters_.contains(parameter_id) &&
        parameters_int_.find(parameter_id) == parameters_int_.end())
    // if exists in org json config and not yet stored in parameters
    {
        parameters_int_.insert(std::make_pair(parameter_id, ConfigurableParameter<int>()));
        assert(org_config_parameters_.at(parameter_id).is_number());
        parameters_int_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_int_.at(parameter_id).config_value_ = org_config_parameters_.at(parameter_id);
    }

    if (parameters_int_.find(parameter_id) ==
        parameters_int_.end())  // new parameter, didnt exist in config
    {
        parameters_int_.insert(std::pair<std::string, ConfigurableParameter<int>>(
            parameter_id, ConfigurableParameter<int>()));
        parameters_int_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_int_.at(parameter_id).config_value_ = default_value;
    }

    parameters_int_.at(parameter_id).pointer_ = pointer;
    parameters_int_.at(parameter_id).default_value_ = default_value;
    *(parameters_int_.at(parameter_id).pointer_) = parameters_int_.at(parameter_id).config_value_;
    used_ = true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter " << parameter_id
           << ": int, value is " << *(parameters_int_.at(parameter_id).pointer_);
}

void Configuration::registerParameter(const std::string& parameter_id, unsigned int* pointer,
                                      unsigned int default_value)
{
    logdbg << "Configuration " << instance_id_
           << ": registerParameter: unsigned int: " << parameter_id;

    assert(pointer);

    if (org_config_parameters_.contains(parameter_id) &&
        parameters_uint_.find(parameter_id) == parameters_uint_.end())
    // if exists in org json config and not yet stored in parameters
    {
        parameters_uint_.insert(
            std::make_pair(parameter_id, ConfigurableParameter<unsigned int>()));
        assert(org_config_parameters_.at(parameter_id).is_number_unsigned());
        parameters_uint_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_uint_.at(parameter_id).config_value_ = org_config_parameters_.at(parameter_id);
    }

    if (parameters_uint_.find(parameter_id) ==
        parameters_uint_.end())  // new parameter, didnt exist in config
    {
        parameters_uint_.insert(std::pair<std::string, ConfigurableParameter<unsigned int>>(
            parameter_id, ConfigurableParameter<unsigned int>()));

        parameters_uint_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_uint_.at(parameter_id).config_value_ = default_value;
    }

    parameters_uint_.at(parameter_id).pointer_ = pointer;
    parameters_uint_.at(parameter_id).default_value_ = default_value;
    *(parameters_uint_.at(parameter_id).pointer_) = parameters_uint_.at(parameter_id).config_value_;
    used_ = true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter " << parameter_id
           << ": unsigned int, value is " << *(parameters_uint_.at(parameter_id).pointer_);
}

void Configuration::registerParameter(const std::string& parameter_id, float* pointer,
                                      float default_value)
{
    logdbg << "Configuration " << instance_id_ << ": registerParameter: float: " << parameter_id;

    assert(pointer);

    if (org_config_parameters_.contains(parameter_id) &&
        parameters_float_.find(parameter_id) == parameters_float_.end())
    // if exists in org json config and not yet stored in parameters
    {
        parameters_float_.insert(std::make_pair(parameter_id, ConfigurableParameter<float>()));
        assert(org_config_parameters_.at(parameter_id).is_number_float());
        parameters_float_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_float_.at(parameter_id).config_value_ = org_config_parameters_.at(parameter_id);
    }

    if (parameters_float_.find(parameter_id) ==
        parameters_float_.end())  // new parameter, didnt exist in config
    {
        parameters_float_.insert(std::pair<std::string, ConfigurableParameter<float>>(
            parameter_id, ConfigurableParameter<float>()));

        parameters_float_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_float_.at(parameter_id).config_value_ = default_value;
    }

    parameters_float_.at(parameter_id).pointer_ = pointer;
    parameters_float_.at(parameter_id).default_value_ = default_value;
    *(parameters_float_.at(parameter_id).pointer_) =
        parameters_float_.at(parameter_id).config_value_;
    used_ = true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter " << parameter_id
           << ": float, value is " << *(parameters_float_.at(parameter_id).pointer_);
}

void Configuration::registerParameter(const std::string& parameter_id, double* pointer,
                                      double default_value)
{
    logdbg << "Configuration " << instance_id_ << ": registerParameter: double: " << parameter_id;

    assert(pointer);

    if (org_config_parameters_.contains(parameter_id) &&
        parameters_double_.find(parameter_id) == parameters_double_.end())
    // if exists in org json config and not yet stored in parameters
    {
        parameters_double_.insert(std::make_pair(parameter_id, ConfigurableParameter<double>()));
        assert(org_config_parameters_.at(parameter_id).is_number_float());
        parameters_double_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_double_.at(parameter_id).config_value_ = org_config_parameters_.at(parameter_id);
    }

    if (parameters_double_.find(parameter_id) ==
        parameters_double_.end())  // new parameter, didnt exist in config
    {
        parameters_double_.insert(std::pair<std::string, ConfigurableParameter<double>>(
            parameter_id, ConfigurableParameter<double>()));
        parameters_double_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_double_.at(parameter_id).config_value_ = default_value;
    }

    parameters_double_.at(parameter_id).pointer_ = pointer;
    parameters_double_.at(parameter_id).default_value_ = default_value;
    *(parameters_double_.at(parameter_id).pointer_) =
        parameters_double_.at(parameter_id).config_value_;
    used_ = true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter " << parameter_id
           << ": double, value is " << *(parameters_double_.at(parameter_id).pointer_);
}

void Configuration::registerParameter(const std::string& parameter_id, std::string* pointer,
                                      const std::string& default_value)
{
    logdbg << "Configuration " << instance_id_ << ": registerParameter: string: " << parameter_id;

    assert(pointer);

    if (org_config_parameters_.contains(parameter_id) &&
        parameters_string_.find(parameter_id) == parameters_string_.end())
    // if exists in org json config and not yet stored in parameters
    {
        parameters_string_.insert(
            std::make_pair(parameter_id, ConfigurableParameter<std::string>()));
        assert(org_config_parameters_.at(parameter_id).is_string());
        parameters_string_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_string_.at(parameter_id).config_value_ = org_config_parameters_.at(parameter_id);
    }

    if (parameters_string_.find(parameter_id) ==
        parameters_string_.end())  // new parameter, didnt exist in config
    {
        parameters_string_.insert(std::pair<std::string, ConfigurableParameter<std::string>>(
            parameter_id, ConfigurableParameter<std::string>()));

        parameters_string_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_string_.at(parameter_id).config_value_ = default_value;
    }

    parameters_string_.at(parameter_id).pointer_ = pointer;
    parameters_string_.at(parameter_id).default_value_ = default_value;
    *(parameters_string_.at(parameter_id).pointer_) =
        parameters_string_.at(parameter_id).config_value_;
    used_ = true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter " << parameter_id
           << ": string, value is " << *(parameters_string_.at(parameter_id).pointer_);
}

void Configuration::registerParameter(const std::string& parameter_id, nlohmann::json* pointer,
                                      const nlohmann::json& default_value)
{
    logdbg << "Configuration " << instance_id_ << ": registerParameter: json: " << parameter_id;

    assert(pointer);

    if (org_config_parameters_.contains(parameter_id) &&
        parameters_json_.find(parameter_id) == parameters_json_.end())
    // if exists in org json config and not yet stored in parameters
    {
        parameters_json_.insert(
            std::make_pair(parameter_id, ConfigurableParameter<nlohmann::json>()));
        //assert(org_config_parameters_.at(parameter_id).is_string());
        parameters_json_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_json_.at(parameter_id).config_value_ = org_config_parameters_.at(parameter_id);
    }

    if (parameters_json_.find(parameter_id) ==
        parameters_json_.end())  // new parameter, didnt exist in config
    {
        parameters_json_.insert(std::pair<std::string, ConfigurableParameter<nlohmann::json>>(
            parameter_id, ConfigurableParameter<nlohmann::json>()));

        parameters_json_.at(parameter_id).parameter_id_ = parameter_id;
        parameters_json_.at(parameter_id).config_value_ = default_value;
    }

    parameters_json_.at(parameter_id).pointer_ = pointer;
    parameters_json_.at(parameter_id).default_value_ = default_value;
    *(parameters_json_.at(parameter_id).pointer_) =
        parameters_json_.at(parameter_id).config_value_;
    used_ = true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter " << parameter_id
           << ": json, value is " << *(parameters_json_.at(parameter_id).pointer_);
}

void Configuration::updateParameterPointer(const std::string& parameter_id, bool* pointer)
{
    logdbg << "Configuration " << instance_id_
           << ": updateParameterPointer: bool: " << parameter_id;

    assert(pointer);

    assert(parameters_bool_.find(parameter_id) != parameters_bool_.end());

    parameters_bool_.at(parameter_id).pointer_ = pointer;
    used_ = true;
}

void Configuration::updateParameterPointer(const std::string& parameter_id, int* pointer)
{
    logdbg << "Configuration " << instance_id_ << ": updateParameterPointer: int: " << parameter_id;

    assert(pointer);
    assert(parameters_int_.find(parameter_id) != parameters_int_.end());

    parameters_int_.at(parameter_id).pointer_ = pointer;
    used_ = true;
}

void Configuration::updateParameterPointer(const std::string& parameter_id, unsigned int* pointer)
{
    logdbg << "Configuration " << instance_id_
           << ": updateParameterPointer: uint: " << parameter_id;

    assert(pointer);
    assert(parameters_uint_.find(parameter_id) != parameters_uint_.end());

    parameters_uint_.at(parameter_id).pointer_ = pointer;
    used_ = true;
}

void Configuration::updateParameterPointer(const std::string& parameter_id, float* pointer)
{
    logdbg << "Configuration " << instance_id_
           << ": updateParameterPointer: float: " << parameter_id;

    assert(pointer);
    assert(parameters_float_.find(parameter_id) != parameters_float_.end());

    parameters_float_.at(parameter_id).pointer_ = pointer;
    used_ = true;
}

void Configuration::updateParameterPointer(const std::string& parameter_id, double* pointer)
{
    logdbg << "Configuration " << instance_id_
           << ": updateParameterPointer: double: " << parameter_id;

    assert(pointer);
    assert(parameters_double_.find(parameter_id) != parameters_double_.end());

    parameters_double_.at(parameter_id).pointer_ = pointer;
    used_ = true;
}

void Configuration::updateParameterPointer(const std::string& parameter_id, std::string* pointer)
{
    logdbg << "Configuration " << instance_id_
           << ": updateParameterPointer: string: " << parameter_id;

    assert(pointer);
    assert(parameters_string_.find(parameter_id) != parameters_string_.end());

    parameters_string_.at(parameter_id).pointer_ = pointer;
    used_ = true;
}

void Configuration::updateParameterPointer(const std::string& parameter_id, nlohmann::json* pointer)
{
    logdbg << "Configuration " << instance_id_
           << ": updateParameterPointer: json: " << parameter_id;

    assert(pointer);
    assert(parameters_json_.find(parameter_id) != parameters_json_.end());

    parameters_json_.at(parameter_id).pointer_ = pointer;
    used_ = true;
}

void Configuration::addParameterBool(const std::string& parameter_id, bool default_value)
{
    logdbg << "Configuration: addParameterBool: parameter " << parameter_id << " default "
           << default_value;
    if (parameters_bool_.find(parameter_id) != parameters_bool_.end())
    {
        logwrn << "Configuration " << instance_id_ << ": addParameterBool: " << parameter_id
               << " already exists";
        return;
    }

    parameters_bool_.insert(std::pair<std::string, ConfigurableParameter<bool>>(
        parameter_id, ConfigurableParameter<bool>()));

    parameters_bool_.at(parameter_id).parameter_id_ = parameter_id;
    parameters_bool_.at(parameter_id).pointer_ = nullptr;
    parameters_bool_.at(parameter_id).default_value_ = default_value;
    parameters_bool_.at(parameter_id).config_value_ = default_value;
}

void Configuration::addParameterInt(const std::string& parameter_id, int default_value)
{
    logdbg << "Configuration: addParameterInt: parameter " << parameter_id << " default "
           << default_value;
    if (parameters_int_.find(parameter_id) != parameters_int_.end())
    {
        logwrn << "Configuration " << instance_id_ << ": addParameterInt: " << parameter_id
               << " already exists";
        return;
    }

    parameters_int_.insert(std::pair<std::string, ConfigurableParameter<int>>(
        parameter_id, ConfigurableParameter<int>()));

    parameters_int_.at(parameter_id).parameter_id_ = parameter_id;
    parameters_int_.at(parameter_id).pointer_ = nullptr;
    parameters_int_.at(parameter_id).default_value_ = default_value;
    parameters_int_.at(parameter_id).config_value_ = default_value;
}

void Configuration::addParameterUnsignedInt(const std::string& parameter_id,
                                            unsigned int default_value)
{
    logdbg << "Configuration: addParameterUnsignedInt: parameter " << parameter_id << " default "
           << default_value;
    if (parameters_uint_.find(parameter_id) != parameters_uint_.end())
    {
        logwrn << "Configuration " << instance_id_ << ": addParameterUnsignedInt: " << parameter_id
               << " already exists";
        return;
    }

    parameters_uint_.insert(std::pair<std::string, ConfigurableParameter<unsigned int>>(
        parameter_id, ConfigurableParameter<unsigned int>()));
    parameters_uint_.at(parameter_id).parameter_id_ = parameter_id;
    parameters_uint_.at(parameter_id).pointer_ = nullptr;
    parameters_uint_.at(parameter_id).default_value_ = default_value;
    parameters_uint_.at(parameter_id).config_value_ = default_value;
}

void Configuration::addParameterFloat(const std::string& parameter_id, float default_value)
{
    logdbg << "Configuration: addParameterFloat: parameter " << parameter_id << " default "
           << default_value;
    if (parameters_float_.find(parameter_id) != parameters_float_.end())
    {
        logwrn << "Configuration " << instance_id_ << ": addParameterFloat: " << parameter_id
               << " already exists";
        return;
    }

    parameters_float_.insert(std::pair<std::string, ConfigurableParameter<float>>(
        parameter_id, ConfigurableParameter<float>()));

    parameters_float_.at(parameter_id).parameter_id_ = parameter_id;
    parameters_float_.at(parameter_id).pointer_ = nullptr;
    parameters_float_.at(parameter_id).default_value_ = default_value;
    parameters_float_.at(parameter_id).config_value_ = default_value;
}

void Configuration::addParameterDouble(const std::string& parameter_id, double default_value)
{
    logdbg << "Configuration: addParameterDouble: parameter " << parameter_id << " default "
           << default_value;
    if (parameters_double_.find(parameter_id) != parameters_double_.end())
    {
        logwrn << "Configuration " << instance_id_ << ": addParameterDouble: " << parameter_id
               << " already exists";
        return;
    }

    parameters_double_.insert(std::pair<std::string, ConfigurableParameter<double>>(
        parameter_id, ConfigurableParameter<double>()));

    parameters_double_.at(parameter_id).parameter_id_ = parameter_id;
    parameters_double_.at(parameter_id).pointer_ = nullptr;
    parameters_double_.at(parameter_id).default_value_ = default_value;
    parameters_double_.at(parameter_id).config_value_ = default_value;
}

void Configuration::addParameterString(const std::string& parameter_id,
                                       const std::string& default_value)
{
    logdbg << "Configuration: addParameterString: parameter " << parameter_id << " default "
           << default_value;
    if (parameters_string_.find(parameter_id) != parameters_string_.end())
    {
        logwrn << "Configuration " << instance_id_ << ": addParameterString: " << parameter_id
               << " already exists";
        return;
    }

    parameters_string_.insert(std::pair<std::string, ConfigurableParameter<std::string>>(
        parameter_id, ConfigurableParameter<std::string>()));

    parameters_string_.at(parameter_id).parameter_id_ = parameter_id;
    parameters_string_.at(parameter_id).pointer_ = nullptr;
    parameters_string_.at(parameter_id).default_value_ = default_value;
    parameters_string_.at(parameter_id).config_value_ = default_value;
}

void Configuration::addParameterJSON(const std::string& parameter_id,
                                       const nlohmann::json& default_value)
{
    logdbg << "Configuration: addParameterJSON: parameter " << parameter_id << " default "
           << default_value;
    if (parameters_json_.find(parameter_id) != parameters_json_.end())
    {
        logwrn << "Configuration " << instance_id_ << ": addParameterJSON: " << parameter_id
               << " already exists";
        return;
    }

    parameters_json_.insert(std::pair<std::string, ConfigurableParameter<nlohmann::json>>(
        parameter_id, ConfigurableParameter<nlohmann::json>()));

    parameters_json_.at(parameter_id).parameter_id_ = parameter_id;
    parameters_json_.at(parameter_id).pointer_ = nullptr;
    parameters_json_.at(parameter_id).default_value_ = default_value;
    parameters_json_.at(parameter_id).config_value_ = default_value;
}

void Configuration::getParameter(const std::string& parameter_id, bool& value)
{
    if (parameters_bool_.count(parameter_id) == 0)
        throw std::runtime_error("Configuration: getParameter: bool: unknown parameter id " +
                                 parameter_id);

    assert(parameters_bool_.at(parameter_id).getParameterType().compare("ParameterBool") == 0);

    if (parameters_bool_.at(parameter_id).pointer_ == nullptr)
        throw std::runtime_error("Configuration: getParameter: bool " + parameter_id +
                                 " not in use");

    value = *(parameters_bool_.at(parameter_id).pointer_);
}

void Configuration::getParameter(const std::string& parameter_id, int& value)
{
    if (parameters_int_.count(parameter_id) == 0)
        throw std::runtime_error("Configuration: getParameter: int: unknown parameter id " +
                                 parameter_id);

    assert(parameters_int_.at(parameter_id).getParameterType().compare("ParameterInt") == 0);

    if (parameters_int_.at(parameter_id).pointer_ == nullptr)
        throw std::runtime_error("Configuration: getParameter: int " + parameter_id +
                                 " not in use");

    value = *(parameters_int_.at(parameter_id).pointer_);
}

void Configuration::getParameter(const std::string& parameter_id, unsigned int& value)
{
    if (parameters_uint_.count(parameter_id) == 0)
        throw std::runtime_error("Configuration: getParameter: uint: unknown parameter id " +
                                 parameter_id);

    assert(parameters_uint_.at(parameter_id).getParameterType().compare("ParameterUnsignedInt") ==
           0);

    if (parameters_uint_.at(parameter_id).pointer_ == nullptr)
        throw std::runtime_error("Configuration: getParameter: uint " + parameter_id +
                                 " not in use");

    value = *(parameters_uint_.at(parameter_id).pointer_);
}

void Configuration::getParameter(const std::string& parameter_id, float& value)
{
    if (parameters_float_.count(parameter_id) == 0)
        throw std::runtime_error("Configuration: getParameter: float: unknown parameter id " +
                                 parameter_id);

    assert(parameters_float_.at(parameter_id).getParameterType().compare("ParameterFloat") == 0);

    if (parameters_float_.at(parameter_id).pointer_ == nullptr)
        throw std::runtime_error("Configuration: getParameter: float " + parameter_id +
                                 " not in use");

    value = *(parameters_float_.at(parameter_id).pointer_);
}

void Configuration::getParameter(const std::string& parameter_id, double& value)
{
    if (parameters_double_.count(parameter_id) == 0)
        throw std::runtime_error("Configuration: getParameter: double: unknown parameter id " +
                                 parameter_id);

    assert(parameters_double_.at(parameter_id).getParameterType().compare("ParameterDouble") == 0);

    if (parameters_double_.at(parameter_id).pointer_ == nullptr)
        throw std::runtime_error("Configuration: getParameter: double " + parameter_id +
                                 " not in use");

    value = *(parameters_double_.at(parameter_id).pointer_);
}

void Configuration::getParameter(const std::string& parameter_id, std::string& value)
{
    if (parameters_string_.count(parameter_id) == 0)
        throw std::runtime_error("Configuration: getParameter: string: unknown parameter id " +
                                 parameter_id);

    assert(parameters_string_.at(parameter_id).getParameterType().compare("ParameterString") == 0);

    if (parameters_string_.at(parameter_id).pointer_ == nullptr)
        throw std::runtime_error("Configuration: getParameter: string " + parameter_id +
                                 " not in use");

    value = *(parameters_string_.at(parameter_id).pointer_);
}

void Configuration::getParameter(const std::string& parameter_id, nlohmann::json& value)
{
    if (parameters_json_.count(parameter_id) == 0)
        throw std::runtime_error("Configuration: getParameter: json: unknown parameter id " +
                                 parameter_id);

    assert(parameters_json_.at(parameter_id).getParameterType().compare("ParameterJSON") == 0);

    if (parameters_json_.at(parameter_id).pointer_ == nullptr)
        throw std::runtime_error("Configuration: getParameter: string " + parameter_id +
                                 " not in use");

    value = *(parameters_json_.at(parameter_id).pointer_);
}

bool Configuration::hasParameterConfigValueBool(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_bool_.find(parameter_id) == parameters_bool_.end())
        // if exists in org json config and not yet stored in parameters
        return true;

    return parameters_bool_.count(parameter_id);
}

bool Configuration::getParameterConfigValueBool(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_bool_.find(parameter_id) == parameters_bool_.end())
    // if exists in org json config and not yet stored in parameters
    {
        assert(org_config_parameters_.at(parameter_id).is_boolean());
        return org_config_parameters_.at(parameter_id).get<bool>();
    }

    assert(parameters_bool_.count(parameter_id));
    assert(parameters_bool_.at(parameter_id).getParameterType().compare("ParameterBool") == 0);

    return parameters_bool_.at(parameter_id).config_value_;
}

bool Configuration::hasParameterConfigValueInt(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_int_.find(parameter_id) == parameters_int_.end())
        // if exists in org json config and not yet stored in parameters
        return true;

    return parameters_int_.count(parameter_id);
}

int Configuration::getParameterConfigValueInt(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_int_.find(parameter_id) == parameters_int_.end())
    // if exists in org json config and not yet stored in parameters
    {
        assert(org_config_parameters_.at(parameter_id).is_number_integer());
        return org_config_parameters_.at(parameter_id).get<int>();
    }

    assert(parameters_int_.count(parameter_id));
    assert(parameters_int_.at(parameter_id).getParameterType().compare("ParameterInt") == 0);

    return parameters_int_.at(parameter_id).config_value_;
}

bool Configuration::hasParameterConfigValueUint(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_uint_.find(parameter_id) == parameters_uint_.end())
        // if exists in org json config and not yet stored in parameters
        return true;

    return parameters_uint_.count(parameter_id);
}

unsigned int Configuration::getParameterConfigValueUint(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_uint_.find(parameter_id) == parameters_uint_.end())
    // if exists in org json config and not yet stored in parameters
    {
        assert(org_config_parameters_.at(parameter_id).is_number_unsigned());
        return org_config_parameters_.at(parameter_id).get<unsigned int>();
    }

    assert(parameters_uint_.count(parameter_id));
    assert(parameters_uint_.at(parameter_id).getParameterType().compare("ParameterUnsignedInt") ==
           0);

    return parameters_uint_.at(parameter_id).config_value_;
}

bool Configuration::hasParameterConfigValueFloat(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_float_.find(parameter_id) == parameters_float_.end())
        // if exists in org json config and not yet stored in parameters
        return true;

    return parameters_float_.count(parameter_id);
}

float Configuration::getParameterConfigValueFloat(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_float_.find(parameter_id) == parameters_float_.end())
    // if exists in org json config and not yet stored in parameters
    {
        assert(org_config_parameters_.at(parameter_id).is_number_float());
        return org_config_parameters_.at(parameter_id).get<float>();
    }

    assert(parameters_float_.count(parameter_id));
    assert(parameters_float_.at(parameter_id).getParameterType().compare("ParameterFloat") == 0);

    return parameters_float_.at(parameter_id).config_value_;
}

bool Configuration::hasParameterConfigValueDouble(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_double_.find(parameter_id) == parameters_double_.end())
        // if exists in org json config and not yet stored in parameters
        return true;

    return parameters_double_.count(parameter_id);
}

double Configuration::getParameterConfigValueDouble(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_double_.find(parameter_id) == parameters_double_.end())
    // if exists in org json config and not yet stored in parameters
    {
        assert(org_config_parameters_.at(parameter_id).is_number_float());
        return org_config_parameters_.at(parameter_id).get<double>();
    }

    assert(parameters_double_.count(parameter_id));
    assert(parameters_double_.at(parameter_id).getParameterType().compare("ParameterDouble") == 0);

    return parameters_double_.at(parameter_id).config_value_;
}

bool Configuration::hasParameterConfigValueString(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_string_.find(parameter_id) == parameters_string_.end())
        // if exists in org json config and not yet stored in parameters
        return true;

    return parameters_string_.count(parameter_id);
}

std::string Configuration::getParameterConfigValueString(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_string_.find(parameter_id) == parameters_string_.end())
    // if exists in org json config and not yet stored in parameters
    {
        assert(org_config_parameters_.at(parameter_id).is_string());
        return org_config_parameters_.at(parameter_id).get<std::string>();
    }

    assert(parameters_string_.count(parameter_id));
    assert(parameters_string_.at(parameter_id).getParameterType().compare("ParameterString") == 0);

    return parameters_string_.at(parameter_id).config_value_;
}

nlohmann::json Configuration::getParameterConfigValueJSON(const std::string& parameter_id)
{
    if (org_config_parameters_.contains(parameter_id) &&
        parameters_json_.find(parameter_id) == parameters_json_.end())
    // if exists in org json config and not yet stored in parameters
    {
        //assert(org_config_parameters_.at(parameter_id).is_string());
        return org_config_parameters_.at(parameter_id);
    }

    assert(parameters_json_.count(parameter_id));
    assert(parameters_json_.at(parameter_id).getParameterType().compare("ParameterJSON") == 0);

    return parameters_json_.at(parameter_id).config_value_;
}

void Configuration::parseJSONConfigFile()
{
    assert(hasConfigurationFilename());

    std::string file_path = CURRENT_CONF_DIRECTORY + configuration_filename_;

    Files::verifyFileExists(file_path);
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": parseJSONConfigFile: opening file '" << file_path << "'";

    std::ifstream config_file(file_path, std::ifstream::in);

    try
    {
        json config = json::parse(config_file);
        parseJSONConfig(config);
    }
    catch (json::exception& e)
    {
        logerr << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
               << ": parseJSONConfigFile: could not load file '" << file_path << "'";
        throw e;
    }
}

void Configuration::parseJSONConfig(nlohmann::json& config)
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": parseJSONConfig";

    assert(config.is_object());

    for (auto& it : config.items())
    {
        if (it.value() == nullptr)  // empty
            continue;

        if (it.key() == "parameters")
        {
            assert(it.value().is_object());
            parseJSONParameters(it.value());
        }
        else if (it.key() == "sub_config_files")
        {
            std::string class_id;
            std::string instance_id;
            std::string path;

            assert(it.value().is_array());

            for (auto& file_cfg_it : it.value().get<json::array_t>())
            {
                assert(file_cfg_it.contains("class_id"));
                assert(file_cfg_it.contains("instance_id"));
                assert(file_cfg_it.contains("path"));

                class_id = file_cfg_it.at("class_id");
                instance_id = file_cfg_it.at("instance_id");
                path = file_cfg_it.at("path");

                assert(class_id.size() && instance_id.size() && path.size());
                parseJSONSubConfigFile(class_id, instance_id, path);
            }
        }
        else if (it.key() == "sub_configs")
        {
            assert(it.value().is_object());
            parseJSONSubConfigs(it.value());
        }
        else
            throw std::runtime_error("Configuration class_id" + class_id_ + " instance_id " +
                                     instance_id_ + ": parseJSONConfig: unknown key '" + it.key() +
                                     "'");
    }
}

void Configuration::parseJSONSubConfigFile(const std::string& class_id,
                                           const std::string& instance_id, const std::string& path)
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": parseJSONSubConfigFile: class_id " << class_id << " instance_id " << instance_id
           << " path '" << path << "'";

    std::pair<std::string, std::string> key(class_id, instance_id);
    assert(sub_configurations_.find(key) == sub_configurations_.end());  // should not exist

    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": parseJSONConfigurationFile: creating new configuration for class " << class_id
           << " instance " << instance_id;
    sub_configurations_.insert(std::pair<std::pair<std::string, std::string>, Configuration>(
        key, Configuration(class_id, instance_id)));
    sub_configurations_.at(key).setConfigurationFilename(path);
    sub_configurations_.at(key).parseJSONConfigFile();
}

void Configuration::parseJSONParameters(nlohmann::json& parameters_config)
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": parseJSONParameters";

    // is object
    assert(parameters_config.is_object());

    // store paramaters in member
    for (auto& it : parameters_config.items())
    {
        //assert(it.value().is_primitive());
        assert(!org_config_parameters_.contains(it.key()));
        // logdbg << "param key " << it.key() << " value '" << it.value() << "'";
        org_config_parameters_[it.key()] = it.value();
    }
}

void Configuration::parseJSONSubConfigs(nlohmann::json& sub_configs_config)
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": parseJSONSubConfigs";

    // is object
    assert(sub_configs_config.is_object());

    std::string class_id;
    std::string instance_id;

    // sub-configs in member
    for (auto& sub_cfg_class_it : sub_configs_config.items())
    {
        assert(sub_cfg_class_it.value().is_object());
        class_id = sub_cfg_class_it.key();

        for (auto& sub_cfg_instance_it : sub_cfg_class_it.value().items())
        {
            assert(sub_cfg_instance_it.value().is_object());
            instance_id = sub_cfg_instance_it.key();

            //        assert (!org_config_sub_configs_.contains(it.key()));
            //        loginf << "sub-config key " << it.key();
            //        org_config_sub_configs_[it.key()] = std::move(it.value()); // move out, might
            //        be big

            std::pair<std::string, std::string> key(class_id, instance_id);
            assert(sub_configurations_.find(key) == sub_configurations_.end());  // should not exist

            logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
                   << ": parseJSONSubConfigs: creating new configuration for class " << class_id
                   << " instance " << instance_id;
            sub_configurations_.insert(
                std::pair<std::pair<std::string, std::string>, Configuration>(
                    key, Configuration(class_id, instance_id)));
            sub_configurations_.at(key).parseJSONConfig(sub_cfg_instance_it.value());
        }
    }
}

// writes full json config or sub-file to parent
void Configuration::writeJSON(nlohmann::json& parent_json) const
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": writeJSON";

    json config;  // my config
    generateJSON(config);

    if (configuration_filename_.size() > 0)  // if we had custom filename
    {
        std::string file_path = CURRENT_CONF_DIRECTORY + configuration_filename_;

        logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
               << ": writeJSON: saving sub-configuration file '" << file_path << "'";
        // Files::verifyFileExists(file_path);

        // save file
        std::ofstream file(file_path);
        file << config.dump(4);

        if (!parent_json.contains("sub_config_files"))
            parent_json["sub_config_files"] = json::array();

        assert(parent_json["sub_config_files"].is_array());

        json sub_file_json = json::object();
        sub_file_json["class_id"] = class_id_;
        sub_file_json["instance_id"] = instance_id_;
        sub_file_json["path"] = configuration_filename_;

        parent_json["sub_config_files"][parent_json["sub_config_files"].size()] = sub_file_json;
    }
    else  // add full config to parent
    {
        parent_json["sub_configs"][class_id_][instance_id_] = std::move(config);
    }
}

// generates the full json config
void Configuration::generateJSON(nlohmann::json& target) const
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": generateJSON: writing into '" << target.dump(4) << "'";

    json& param_config = target["parameters"];

    // original parameters, in case config was not used

    for (auto& par_it : org_config_parameters_.items())
    {
        param_config[par_it.key()] = par_it.value();
    }

    // overwrite new parameter values
    for (auto& par_it : parameters_bool_)
    {
        logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
               << ": generateJSON: writing bool '" << par_it.second.getParameterId() << "'";
        // assert (!param_config.contains(par_it.second.getParameterId()));
        param_config[par_it.second.getParameterId()] = par_it.second.getParameterValue();
    }

    for (auto& par_it : parameters_int_)
    {
        logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
               << ": generateJSON: writing int '" << par_it.second.getParameterId() << "'";
        // assert (!param_config.contains(par_it.second.getParameterId()));
        param_config[par_it.second.getParameterId()] = par_it.second.getParameterValue();
    }

    for (auto& par_it : parameters_uint_)
    {
        logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
               << ": generateJSON: writing uint '" << par_it.second.getParameterId() << "'";
        // assert (!param_config.contains(par_it.second.getParameterId()));
        param_config[par_it.second.getParameterId()] = par_it.second.getParameterValue();
    }

    for (auto& par_it : parameters_float_)
    {
        logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
               << ": generateJSON: writing float '" << par_it.second.getParameterId() << "'";
        // assert (!param_config.contains(par_it.second.getParameterId()));
        param_config[par_it.second.getParameterId()] = par_it.second.getParameterValue();
    }

    for (auto& par_it : parameters_double_)
    {
        logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
               << ": generateJSON: writing double '" << par_it.second.getParameterId() << "'";
        // assert (!param_config.contains(par_it.second.getParameterId()));
        param_config[par_it.second.getParameterId()] = par_it.second.getParameterValue();
    }

    for (auto& par_it : parameters_string_)
    {
        logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
               << ": generateJSON: writing string '" << par_it.second.getParameterId() << "'";
        // assert (!param_config.contains(par_it.second.getParameterId()));
        param_config[par_it.second.getParameterId()] = par_it.second.getParameterValue();
    }

    for (auto& par_it : parameters_json_)
    {
        logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
               << ": generateJSON: writing json '" << par_it.second.getParameterId() << "'";
        // assert (!param_config.contains(par_it.second.getParameterId()));
        param_config[par_it.second.getParameterId()] = par_it.second.getParameterValue();
    }

    for (auto& config_it : sub_configurations_)
    {
        config_it.second.writeJSON(target);
    }
}

void Configuration::createSubConfigurables(Configurable* configurable)
{
    logdbg << "Configuration: createSubConfigurables: config instance " << instance_id_
           << " configurable instance " << configurable->instanceId();

    std::map<std::pair<std::string, std::string>, Configuration>::iterator it;
    // TODO what if map changed (deleting of config) during iteration
    for (it = sub_configurations_.begin(); it != sub_configurations_.end(); it++)
    {
        // ConfigurableDefinition& mos_def = sub_configurables_.at(cnt);
        logdbg << "Configuration: createSubConfigurables: generateSubConfigurable: class_id '"
               << it->first.first << "' instance_id '" << it->first.second << "'";
        configurable->generateSubConfigurable(it->first.first, it->first.second);
    }

    logdbg << "Configuration: createSubConfigurables: instance " << instance_id_ << " end";
}

void Configuration::setConfigurationFilename(const std::string& configuration_filename)
{
    assert(configuration_filename.size() > 0);
    configuration_filename_ = configuration_filename;
}

bool Configuration::hasConfigurationFilename() { return configuration_filename_.size() != 0; }
const std::string& Configuration::getConfigurationFilename()
{
    assert(hasConfigurationFilename());
    return configuration_filename_;
}

bool Configuration::hasSubConfiguration(const std::string& class_id, const std::string& instance_id)
{
    std::pair<std::string, std::string> key(class_id, instance_id);
    return sub_configurations_.find(key) != sub_configurations_.end();
}

Configuration& Configuration::addNewSubConfiguration(const std::string& class_id,
                                                     const std::string& instance_id)
{
    std::pair<std::string, std::string> key(class_id, instance_id);
    assert(sub_configurations_.find(key) == sub_configurations_.end());
    sub_configurations_.insert(std::pair<std::pair<std::string, std::string>, Configuration>(
        key, Configuration(class_id, instance_id)));
    return sub_configurations_.at(key);
}

Configuration& Configuration::addNewSubConfiguration(const std::string& class_id)
{
    int instance_number = -1;

    std::map<std::pair<std::string, std::string>, Configuration>::iterator it;

    for (it = sub_configurations_.begin(); it != sub_configurations_.end(); it++)
    {
        if (it->first.first.compare(class_id) == 0)
        {
            int num = String::getAppendedInt(it->first.second);
            if (num > instance_number)
                instance_number = num;
        }
    }
    instance_number++;

    return addNewSubConfiguration(class_id, class_id + std::to_string(instance_number));
}

Configuration& Configuration::addNewSubConfiguration(Configuration& configuration)
{
    std::pair<std::string, std::string> key(configuration.getClassId(),
                                            configuration.getInstanceId());
    assert(sub_configurations_.find(key) == sub_configurations_.end());
    sub_configurations_.insert(
        std::pair<std::pair<std::string, std::string>, Configuration>(key, configuration));
    return sub_configurations_.at(key);
}

Configuration& Configuration::getSubConfiguration(const std::string& class_id,
                                                  const std::string& instance_id)
{
    std::pair<std::string, std::string> key(class_id, instance_id);

    if (sub_configurations_.find(key) == sub_configurations_.end())
    {
        logdbg << "Configuration instance " << instance_id_
               << ": getSubConfiguration: creating new (empty) configuration for class " << class_id
               << " instance " << instance_id;
        addNewSubConfiguration(class_id, instance_id);
    }

    assert(sub_configurations_.find(key) != sub_configurations_.end());
    return sub_configurations_.at(key);
}

void Configuration::removeSubConfiguration(const std::string& class_id,
                                           const std::string& instance_id)
{
    logdbg << "Configuration: removeSubConfiguration: me " << class_id_ << " " << instance_id_
           << " you " << class_id << " " << instance_id;

    std::pair<std::string, std::string> key(class_id, instance_id);

    if (sub_configurations_.find(key) == sub_configurations_.end())
    {
        logerr << "Configuration: removeSubConfiguration: class_id_ " << class_id_
               << " instance_id_ " << instance_id_ << ": sub class_id " << class_id
               << " instance_id " << instance_id << " not found";
        return;
    }

    assert(sub_configurations_.find(key) != sub_configurations_.end());
    sub_configurations_.erase(sub_configurations_.find(key));
}

// void Configuration::setTemplate (bool template_flag, const std::string& template_name)
//{
//    template_flag_ = template_flag;
//    template_name_ = template_name;

//    loginf << "Configuration: setTemplate: " << class_id_ << " instance " << instance_id_ << "
//    flag " << template_flag
//           << " name " << template_name;

//}

// bool Configuration::getSubTemplateNameFree (const std::string& template_name)
//{
//    return configuration_templates_.find (template_name) == configuration_templates_.end();
//}

// void Configuration::addSubTemplate (Configuration* configuration, const std::string&
// template_name)
//{
//    assert (getSubTemplateNameFree(template_name));
//    configuration_templates_.insert (std::pair<std::string, Configuration> (template_name,*
//    configuration)); configuration_templates_.at(template_name).setTemplate(true, template_name);
//    delete configuration;
//}
