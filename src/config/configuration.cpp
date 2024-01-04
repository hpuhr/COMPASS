/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "configuration.h"
#include "configurable.h"
#include "files.h"
#include "logger.h"
#include "stringconv.h"

#include <fstream>
#include <typeinfo>

using namespace Utils;

using namespace nlohmann;

const std::string Configuration::ParameterSection     = "parameters";
const std::string Configuration::SubConfigSection     = "sub_configs";
const std::string Configuration::SubConfigFileSection = "sub_config_files";
const std::string Configuration::InstanceID           = "instance_id";
const std::string Configuration::ClassID              = "class_id";
const std::string Configuration::SubConfigFilePath    = "path";

/*
 *  Initializes members
 *
 *  \param configuration_id configuration identifier
 *  \param configuration_filename special filename, default ""
 */
Configuration::Configuration(const std::string& class_id, 
                             const std::string& instance_id,
                             const std::string& configuration_filename)
:   class_id_(class_id),
    instance_id_(instance_id),
    configuration_filename_(configuration_filename)
{
    logdbg << "Configuration: constructor: class " << class_id_ << " instance " << instance_id_;
    assert(class_id_.size() != 0);
    assert(instance_id.size() != 0);
}

/**
 * Private constructor with class id only.
 * Note: Should only be used when creating a configuration via Configuration::create(class_id).
 */
Configuration::Configuration(const std::string& class_id)
:   class_id_            (class_id)
,   create_instance_name_(true    )
{
    logdbg << "Configuration: constructor: class " << class_id_;
    assert(class_id_.size() != 0);
}

/**
*/
Configuration::~Configuration()
{
    parameters_.clear();
}

/**
*/
std::string Configuration::newInstanceID(const std::string& class_id) const
{
    //find unique instance id
    int instance_number = -1;

    for (auto it = sub_configurations_.begin(); it != sub_configurations_.end(); it++)
    {
        if (it->first.first.compare(class_id) == 0)
        {
            int num = String::getAppendedInt(it->first.second);
            if (num > instance_number)
                instance_number = num;
        }
    }
    instance_number++;

    return class_id + std::to_string(instance_number);
}

/**
 * Creates an empty configuration of the given class and instance id.
 */
Configuration::Ptr Configuration::create(const std::string& class_id, 
                                         const std::string& instance_id)
{
    return std::unique_ptr<Configuration>(new Configuration(class_id, instance_id));
}

/**
 * Creates an empty configuration of the given class id and with empty instance id.
 * A unique instance id will be created automatically upon adding it to another
 * configuration as its sub-configuration (see addNewSubConfiguration(std::unique_ptr<Configuration>&&)).
 */
Configuration::Ptr Configuration::create(const std::string& class_id)
{
    return std::unique_ptr<Configuration>(new Configuration(class_id));
}

/**
 *  Resets all registered parameters to their default values.
 */
void Configuration::resetToDefault()
{
    logdbg << "Configuration: resetToDefault: " << instance_id_;

    for (auto it = parameters_.begin(); it != parameters_.end(); it++)
        it->second->resetToDefault();
}

/**
 * Checks if the given parameter is currently registered.
 */
bool Configuration::hasParameter(const std::string& parameter_id) const
{
    return (parameters_.count(parameter_id) > 0);
}

/**
 * Checks if a parameter of the given type exists.
 */
template <typename T>
bool Configuration::hasParameterOfType(const std::string& parameter_id) const
{
    auto it = parameters_.find(parameter_id);
    if (it == parameters_.end())
        return false;

    return it->second->isType<T>();
}

/**
 * Checks if a config value for the given parameter_id exists either in the json config or in a registered parameter.
 */
bool Configuration::hasParameterConfigValue(const std::string& parameter_id) const
{
    bool has_param        = hasParameter(parameter_id);
    bool has_param_config = parameterInConfig(parameter_id);

    // exists in org json config and not yet stored in parameters?
    if (has_param_config && !has_param)
        return true;

    return has_param;
}

/**
 * Checks if the given parameter exists in the json config.
 */
bool Configuration::parameterInConfig(const std::string& parameter_id) const
{
    return org_config_parameters_.contains(parameter_id);
}

/**
 * Return a parameter value from the json config.
 */
template <typename T>
T Configuration::parameterValueFromConfig(const std::string& parameter_id) const
{
    //check parameterInConfig(parameter_id) beforehand in order to prevent this
    assert(parameterInConfig(parameter_id));

    return ConfigurableParameterT<T>::valueFromJSON(org_config_parameters_.at(parameter_id));
}

/**
 * @brief Registers a parameter of the given type.
 */
template <typename T>
void Configuration::registerParameter(const std::string& parameter_id, T* pointer, const T& default_value)
{
    logdbg << "Configuration " << instance_id_ << ": registerParameter: " << parameter_id;

    //pointer should be valid
    assert(pointer);

    //parameter not existing yet?
    if (!hasParameter(parameter_id))
    {
        bool in_config = parameterInConfig(parameter_id);

        //register new parameter
        //if no config value is available use the default value
        auto ptr = new ConfigurableParameterT<T>(parameter_id, 
                                                 nullptr,     
                                                 in_config ? parameterValueFromConfig<T>(parameter_id) : default_value, 
                                                 default_value);
        parameters_.insert(std::make_pair(parameter_id, std::unique_ptr<ConfigurableParameter>(ptr)));
    }

    //get existing parameter as type
    ConfigurableParameterT<T>* param = parameters_.at(parameter_id)->as<T>();
    assert(param);

    //update existing parameter
    param->update(pointer, default_value, true);

    used_ = true;

    logdbg << "Configuration " << instance_id_ << ": registerParameter " << parameter_id << ": " << param->getParameterType() << " value is " << *param->getStoredPointer();
}

/**
 * @brief Adds the parameter of the given type.
 */
template <typename T>
void Configuration::addParameter(const std::string& parameter_id, const T& default_value)
{
    logdbg << "Configuration: addParameter: parameter " << parameter_id << " default " << default_value;
        
    //parameter already existing?
    if (hasParameter(parameter_id))
    {
        //@TODO: maybe assert on this?
        logwrn << "Configuration " << instance_id_ << ": addParameter: " << parameter_id << " already exists";
        return;
    }

    //create new parameter using default value
    auto ptr = new ConfigurableParameterT<T>(parameter_id, nullptr, default_value, default_value);

    parameters_.insert(std::make_pair(parameter_id, std::unique_ptr<ConfigurableParameter>(ptr)));
}

/**
 * @brief Updates the parameter's value pointer.
 */
template <typename T>
void Configuration::updateParameterPointer(const std::string& parameter_id, T* pointer)
{
    logdbg << "Configuration " << instance_id_ << ": updateParameterPointer: " << parameter_id;

    assert(pointer);
    assert(hasParameter(parameter_id));

    //get parameter as type
    auto param = parameters_.at(parameter_id)->as<T>();
    assert(param);

    //update pointer
    param->update(pointer);

    used_ = true;
}

/**
 * @brief Retrieves the value stored in the parameters pointer.
 */
template <typename T>
void Configuration::getParameter(const std::string& parameter_id, T& value) const
{
    //check if parameter exists
    if (!hasParameter(parameter_id))
        throw std::runtime_error("Configuration: getParameter: unknown parameter id " + parameter_id);

    //get parameter as type
    auto param = parameters_.at(parameter_id)->as<T>();
    assert(param);
    
    //different to ConfigurationParameterT::getParameterValue() we really retrieve the pointer value here,
    //so make sure it is set and throw if this is not the case.
    if (!param->hasStoredPointer())
        throw std::runtime_error("Configuration: getParameter: " + parameter_id + " not in use");

    value = *param->getStoredPointer();
}

/**
 * Retrieves the config value for the given parameter_id, either from the 
 * json config or from a stored parameter.
 */
template <typename T>
T Configuration::getParameterConfigValue(const std::string& parameter_id) const
{
    bool has_param        = hasParameter(parameter_id);
    bool has_param_config = parameterInConfig(parameter_id);

    if (has_param_config && !has_param)
    {
        // only exists in org json config and not yet stored in parameters => obtain value from config
        return parameterValueFromConfig<T>(parameter_id);
    }

    assert(has_param);
    
    //get stored parameter as type
    auto param = parameters_.at(parameter_id)->as<T>();
    assert(param);

    //retrieve config value from registered parameter
    return param->getConfigValue();
}

/**
 * Parses the stored configuration file.
 */
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
        //parse json file
        json config = json::parse(config_file);

        //parse json config
        parseJSONConfig(config);
    }
    catch (json::exception& e)
    {
        logerr << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
               << ": parseJSONConfigFile: could not load file '" << file_path << "'";
        throw e;
    }
}

/**
 * Parses a given json config struct.
 */
void Configuration::parseJSONConfig(const nlohmann::json& config)
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_ << ": parseJSONConfig";

    auto cb_params          = [ this ] (const nlohmann::json& config) { this->parseJSONParameters(config); };
    auto cb_subconfigs      = [ this ] (const nlohmann::json& config) { this->parseJSONSubConfigs(config); };
    auto cb_subconfig_files = [ this ] (const std::string& class_id,
                                        const std::string& instance_id,
                                        const std::string& path) { this->parseJSONSubConfigFile(class_id, instance_id, path); };

    parseJSONConfig(config, cb_params, cb_subconfigs, cb_subconfig_files);
}

/**
 * Parses a given json config struct and invoke the section-specific callbacks.
 */
void Configuration::parseJSONConfig(const nlohmann::json& config,
                                    const std::function<void(const nlohmann::json&)>& parse_parameters_cb,
                                    const std::function<void(const nlohmann::json&)>& parse_sub_configs_cb,
                                    const std::function<void(const std::string&, const std::string&, const std::string&)>& parse_sub_config_files_cb)
{
    assert(config.is_object());

    for (auto& it : config.items())
    {
        if (it.value() == nullptr)  // empty
            continue;

        if (it.key() == ParameterSection)
        {
            assert(it.value().is_object());

            //parse parameters
            parse_parameters_cb(it.value());
        }
        else if (it.key() == SubConfigFileSection)
        {
            std::string class_id;
            std::string instance_id;
            std::string path;

            assert(it.value().is_array());

            for (auto& file_cfg_it : it.value().get<json::array_t>())
            {
                assert(file_cfg_it.contains(ClassID));
                assert(file_cfg_it.contains(InstanceID));
                assert(file_cfg_it.contains(SubConfigFilePath));

                class_id    = file_cfg_it.at(ClassID);
                instance_id = file_cfg_it.at(InstanceID);
                path        = file_cfg_it.at(SubConfigFilePath);

                assert(class_id.size() && instance_id.size() && path.size());

                //parse subconfig file
                parse_sub_config_files_cb(class_id, instance_id, path);
            }
        }
        else if (it.key() == SubConfigSection)
        {
            assert(it.value().is_object());

            //parse subconfigs
            parse_sub_configs_cb(it.value());
        }
        else
        {
            throw std::runtime_error("Configuration class_id " + class_id_ + " instance_id " +
                                     instance_id_ + ": parseJSONConfig: unknown key '" + it.key() +
                                     "'");
        }
    }
}

/**
 * Parses a subconfig file using the provided path. 
 */
void Configuration::parseJSONSubConfigFile(const std::string& class_id,
                                           const std::string& instance_id, 
                                           const std::string& path)
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": parseJSONSubConfigFile: class_id " << class_id << " instance_id " << instance_id
           << " path '" << path << "'";

    SubConfigKey key(class_id, instance_id);
    assert(sub_configurations_.find(key) == sub_configurations_.end());  // should not exist

    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": parseJSONConfigurationFile: creating new configuration for class " << class_id
           << " instance " << instance_id;
    
    //create new configuration for subconfig
    auto ptr = new Configuration(class_id, instance_id);
    sub_configurations_.insert(std::make_pair(key, std::unique_ptr<Configuration>(ptr)));

    //parse subconfig from given path
    sub_configurations_.at(key)->setConfigurationFilename(path);
    sub_configurations_.at(key)->parseJSONConfigFile();
}

/**
 * Overwrites certain parameter keys of the stored json configuration.
 */
void Configuration::overrideJSONParameters(nlohmann::json& parameters_config)
{
    loginf << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": overrideJSONParameters";

    // is object
    assert(parameters_config.is_object());

    // store parameters in local json config
    for (auto& it : parameters_config.items())
    {
        loginf << "Configuration: overrideJSONParameters: overriding '" << it.key()
               << "' with '" << it.value().dump(0) << "'";

        //overwrite key with new value
        org_config_parameters_[it.key()] = it.value();
    }
}

/**
 * Parses config parameters from the provided json struct.
 */
void Configuration::parseJSONParameters(const nlohmann::json& parameters_config)
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": parseJSONParameters";

    auto cb = [ & ] (const std::string& key, const nlohmann::json& value)
    {
        //assert(value.is_primitive());
        assert(!org_config_parameters_.contains(key));

        // logdbg << "param key " << key << " value '" << value << "'";

        org_config_parameters_[key] = value;
    };

    parseJSONParameters(parameters_config, cb);
}

/**
*/
void Configuration::parseJSONParameters(const nlohmann::json& parameters_config,
                                        const std::function<void(const std::string&, const nlohmann::json&)>& parse_param_cb)
{
    // is object
    assert(parameters_config.is_object());

    // store parameters in local json config
    for (auto& it : parameters_config.items())
        parse_param_cb(it.key(), it.value());
}

/**
 * Parses sub configurations from the provided json struct.
*/
void Configuration::parseJSONSubConfigs(const nlohmann::json& sub_configs_config)
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
           << ": parseJSONSubConfigs";

    auto cb = [ & ] (const SubConfigKey& key, const nlohmann::json& value)
    {
        //        assert (!org_config_sub_configs_.contains(it.key()));
        //        loginf << "sub-config key " << it.key();
        //        org_config_sub_configs_[it.key()] = std::move(it.value()); // move out, might
        //        be big

        assert(sub_configurations_.find(key) == sub_configurations_.end());  // should not exist

        logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
               << ": parseJSONSubConfigs: creating new configuration for class " << key.first
               << " instance " << key.second;

        //create new configuration for sub config
        auto ptr = new Configuration(key.first, key.second);
        sub_configurations_.insert(std::make_pair(key, std::unique_ptr<Configuration>(ptr)));

        //parse sub config from json struct
        sub_configurations_.at(key)->parseJSONConfig(value);
    };

    parseJSONSubConfigs(sub_configs_config, cb);
}

/**
*/
void Configuration::parseJSONSubConfigs(const nlohmann::json& sub_configs_config,
                                        const std::function<void(const SubConfigKey&, const nlohmann::json&)>& parse_subconfig_cb)
{
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

            SubConfigKey key(class_id, instance_id);

            parse_subconfig_cb(key, sub_cfg_instance_it.value());
        }
    }
}

/**
 * Writes full json config or sub-file to parent.
 */
void Configuration::writeJSON(nlohmann::json& parent_json, 
                              JSONExportType export_type) const
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_ << ": writeJSON";

    assert(instance_id_.size() != 0);

    json config;  // my config
    generateJSON(config, export_type);

    if (configuration_filename_.size() > 0)  // if we had custom filename
    {
        std::string file_path = CURRENT_CONF_DIRECTORY + configuration_filename_;

        logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_
               << ": writeJSON: saving sub-configuration file '" << file_path << "'";
        // Files::verifyFileExists(file_path);

        // save file
        std::ofstream file(file_path);
        file << config.dump(4);

        if (!parent_json.contains(SubConfigFileSection))
            parent_json[SubConfigFileSection] = json::array();

        assert(parent_json[SubConfigFileSection].is_array());

        json sub_file_json = json::object();
        sub_file_json[ClassID] = class_id_;
        sub_file_json[InstanceID] = instance_id_;
        sub_file_json[SubConfigFilePath] = configuration_filename_;

        parent_json[SubConfigFileSection][parent_json[SubConfigFileSection].size()] = sub_file_json;
    }
    else  // add full config to parent
    {
        parent_json[SubConfigSection][class_id_][instance_id_] = std::move(config);
    }
}

/**
 * Generates the full json config.
 */
void Configuration::generateJSON(nlohmann::json& target, 
                                 JSONExportType export_type) const
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_ << ": generateJSON: writing into '" << target.dump(4) << "'";

    json& param_config = target[ParameterSection];

    // original parameters, in case config was not used

    for (auto& par_it : org_config_parameters_.items())
    {
        param_config[par_it.key()] = par_it.value();
    }

    // overwrite new parameter values
    for (auto& par_it : parameters_)
    {
        logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_ << ": generateJSON: writing '" << par_it.second->getParameterId() << "'";
        // assert (!param_config.contains(par_it.second.getParameterId()));

        par_it.second->toJSON(param_config);
    }

    auto export_filters = jsonExportFilters(export_type);

    for (auto& config_it : sub_configurations_)
    {
        //apply class id based filter and skip sub-configurable?
        if (export_filters && !export_filters->empty() && export_filters->count(config_it.second->getClassId()) > 0)
            continue;

        config_it.second->writeJSON(target, export_type);
    }
}

/**
 * Sets the configuration filename the configuration can be parsed from.
 */
void Configuration::setConfigurationFilename(const std::string& configuration_filename)
{
    assert(!configuration_filename.empty());

    configuration_filename_ = configuration_filename;
}

/**
 * Checks if a configuration filename is set.
 */
bool Configuration::hasConfigurationFilename() const
{ 
    return !configuration_filename_.empty(); 
}

/**
 * Returns the stored configuration filename.
 */
const std::string& Configuration::getConfigurationFilename() const
{
    assert(hasConfigurationFilename());
    return configuration_filename_;
}

/**
 * Checks if the configuration obtains the given subconfiguration.
 */
bool Configuration::hasSubConfiguration(const std::string& class_id, const std::string& instance_id) const
{
    std::pair<std::string, std::string> key(class_id, instance_id);
    return (sub_configurations_.find(key) != sub_configurations_.end());
}

/**
 * Checks if the configuration obtains the given subconfiguration.
 */
bool Configuration::hasSubConfiguration(const SubConfigKey& key) const
{
    return (sub_configurations_.find(key) != sub_configurations_.end());
}

/**
 * Adds a new (empty) subconfiguration.
 */
Configuration& Configuration::addNewSubConfiguration(const std::string& class_id,
                                                     const std::string& instance_id)
{
    assert(instance_id.size() != 0);

    SubConfigKey key(class_id, instance_id);
    assert(!hasSubConfiguration(key));

    auto ptr = new Configuration(class_id, instance_id);
    sub_configurations_.insert(std::make_pair(key, std::unique_ptr<Configuration>(ptr)));

    return *sub_configurations_.at(key);
}

/**
 * Adds a new (empty) subconfiguration and creates a suitable unique instance id.
 */
Configuration& Configuration::addNewSubConfiguration(const std::string& class_id)
{
    auto instance_id = newInstanceID(class_id);
    assert(instance_id.size() != 0);

    //add instance
    return addNewSubConfiguration(class_id, instance_id);
}

/**
 * Adds the passed configuration as sub-configuration and takes ownership.
 */
Configuration& Configuration::addNewSubConfiguration(std::unique_ptr<Configuration>&& configuration)
{
    assert(configuration);
    assert(configuration->create_instance_name_ || !instance_id_.empty());

    //create a new unique instance name for the given configuration?
    if (configuration->create_instance_name_)
    {
        configuration->instance_id_          = newInstanceID(configuration->getClassId());
        configuration->create_instance_name_ = false;

        logdbg << "Configuration: addNewSubConfiguration: created instance " << configuration->instance_id_;
    }

    //logdbg << "Configuration::addNewSubConfiguration: Adding group of class " 
    //       << configuration->getClassId() << " instance " << configuration->getInstanceId();

    //key unique?
    SubConfigKey key(configuration->getClassId(), configuration->getInstanceId());
    assert(!hasSubConfiguration(key));

    //add to sub-configurations
    sub_configurations_.insert(std::make_pair(key, std::move(configuration)));

    return *sub_configurations_.at(key);
}

/**
 * Returns the given subconfiguration.
 */
Configuration& Configuration::getSubConfiguration(const std::string& class_id,
                                                  const std::string& instance_id) const
{
    SubConfigKey key(class_id, instance_id);
    assert(hasSubConfiguration(key));

    return *sub_configurations_.at(key);
}

/**
 * Optionally creates a new subconfiguration and returns it.
 */
Configuration& Configuration::assertSubConfiguration(const std::string& class_id,
                                                     const std::string& instance_id)
{
    SubConfigKey key(class_id, instance_id);

    //non-existent => create new one
    if (!hasSubConfiguration(key))
    {
        logdbg << "Configuration instance " << instance_id_
               << ": getSubConfiguration: creating new (empty) configuration for class " << class_id
               << " instance " << instance_id;
        
        addNewSubConfiguration(class_id, instance_id);
    }

    assert(hasSubConfiguration(key));

    return *sub_configurations_.at(key);
}

/**
 * Removes the given subconfiguration.
 */
void Configuration::removeSubConfiguration(const std::string& class_id,
                                           const std::string& instance_id)
{
    logdbg << "Configuration: removeSubConfiguration: me " << class_id_ << " " << instance_id_ << " you " << class_id << " " << instance_id;

    SubConfigKey key(class_id, instance_id);

    if (!hasSubConfiguration(key))
    {
        logerr << "Configuration: removeSubConfiguration: class_id_ " << class_id_
               << " instance_id_ " << instance_id_ << ": sub class_id " << class_id
               << " instance_id " << instance_id << " not found";
        return;
    }

    sub_configurations_.erase(key);
}

/**
 * Removes all subconfigurations of the given class id.
 */
void Configuration::removeSubConfigurations(const std::string& class_id)
{
    logdbg << "Configuration: removeSubConfigurations: me " << class_id_;

    std::vector<SubConfigKey> to_be_removed;

    for (auto& sub_it : sub_configurations_)
        if (sub_it.first.first == class_id)
            to_be_removed.push_back(sub_it.first);

    for (auto& sub_key_it : to_be_removed)
        sub_configurations_.erase(sub_key_it);
}

/**
 * Connects a listener for receiving configuration change updates.
*/
boost::signals2::connection Configuration::connectListener(const std::function<void(const ParameterList&)>& changed_cb)
{
    return changed_signal_.connect(changed_cb);
}

/**
 * Reconfigures the configuration's registered parameters and those of its subconfigurations.
 */
std::vector<std::string> Configuration::reconfigure(const nlohmann::json& config, 
                                                    Configurable* configurable,
                                                    std::vector<SubConfigKey>* missing_keys)
{
    logdbg << "Configuration class_id " << class_id_ << " instance_id " << instance_id_ << ": reconfigure";

    std::set<std::string> param_set;

    auto mode = configurable ? configurable->reconfigureSubConfigMode() : ReconfigureSubConfigMode::MustExist;

    auto logError = [ & ] (const SubConfigKey& key)
    {
        logerr << "Configuration: reconfigure: sub-config " << key.first << "." << key.second 
               << " not found in config " << this->class_id_ << "." << this->instance_id_;
    };

    //callbacks used for parsing
    auto cb_param = [ & ] (const std::string& key, const nlohmann::json& value)
    {
        //parameter must exist
        assert(hasParameter(key));

        //set parameter's internal pointer value
        setParameterFromJSON(key, value);

        param_set.insert(key);
    };
    auto cb_params = [ & ] (const nlohmann::json& config)
    {
        //parse parameters
        parseJSONParameters(config, cb_param);
    };
    auto cb_subconfig = [ & ] (const SubConfigKey& key, const nlohmann::json& config)
    { 
        //subconfig missing?
        if (!hasSubConfiguration(key))
        {
            //handle missing keys depending on mode
            if (mode == ReconfigureSubConfigMode::CreateIfMissing)
            {
                assert(configurable);

                //create new configuration for sub config
                auto ptr = new Configuration(key.first, key.second);
                sub_configurations_.insert(std::make_pair(key, std::unique_ptr<Configuration>(ptr)));

                //parse sub config from json struct
                sub_configurations_.at(key)->parseJSONConfig(config);

                //tell configurable to create missing subconfigurable
                configurable->generateSubConfigurable(key.first, key.second);
            } 
            else if (mode == ReconfigureSubConfigMode::WarnIfMissing)
            {
                //collect missing key
                if (missing_keys)
                    missing_keys->push_back(key);

                logError(key);

                //do not descend deeper
                return;
            }
        }

        bool has_subconfig = hasSubConfiguration(key);

        if (!has_subconfig)
            logError(key);

        //subconfig must exist
        assert(has_subconfig);
        
        //get subconfigurable
        Configurable* sub_configurable = nullptr;
        if (configurable)
        {
            assert(configurable->hasSubConfigurable(key.first, key.second));
            sub_configurable = &configurable->getChild(key.first, key.second);
        }

        //reconfigure subconfig
        auto params_subconfig = getSubConfiguration(key.first, key.second).reconfigure(config, sub_configurable, missing_keys);
        for (const auto& p : params_subconfig)
            param_set.insert(key.first + Configurable::ConfigurablePathSeparator + p);
    };
    auto cb_subconfigs = [ & ] (const nlohmann::json& config) 
    { 
        //parse subconfigurations
        parseJSONSubConfigs(config, cb_subconfig);
    };

    //parse config struct using the specified callbacks
    parseJSONConfig(config, cb_params, cb_subconfigs, {});

    ParameterList param_list(param_set.begin(), param_set.end());

    //if my own parameters changed signal changes
    if (!param_list.empty())
        changed_signal_(param_list);

    return param_list;
}

/**
 * Sets the given parameter's value pointer to the given value.
 */
void Configuration::setParameterFromJSON(const std::string& parameter_id, const nlohmann::json& value)
{
    //needs to be registered
    assert(hasParameter(parameter_id));

    parameters_.at(parameter_id)->setValue(value);
}

/**
 * Adds a new filtered class id to the export filter for the given export type.
 */
void Configuration::addJSONExportFilter(JSONExportType export_type, 
                                        const std::string& class_id)
{
    json_export_filters_[ export_type ].insert(class_id);
}

/**
 * Adds new filtered class ids to the export filter for the given export type.
 */
void Configuration::addJSONExportFilter(JSONExportType export_type, 
                                        const std::vector<std::string>& class_ids)
{
    json_export_filters_[ export_type ].insert(class_ids.begin(), class_ids.end());
}

/**
 * Returns the filtered class ids of the export filter for the given export type.
 */
const std::set<std::string>* Configuration::jsonExportFilters(JSONExportType export_type) const
{
    if (json_export_filters_.empty() || json_export_filters_.count(export_type) == 0)
        return nullptr;

    return &json_export_filters_.at(export_type);
}

// void Configuration::setTemplate (bool template_flag, const std::string& template_name)
//{
//    template_flag_ = template_flag;
//    template_name_ = template_name;

//    loginf << "Configuration: setTemplate: " << class_id_ << " instance " << instance_id_ << "
//    flag " << template_flag
//           << " name " << template_name;

//}

// bool Configuration::getSubTemplateNameFree (const std::string& template_name) const
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

template void Configuration::registerParameter<bool>(const std::string& parameter_id, bool* pointer, const bool& default_value);
template void Configuration::registerParameter<int>(const std::string& parameter_id, int* pointer, const int& default_value);
template void Configuration::registerParameter<unsigned int>(const std::string& parameter_id, unsigned int* pointer, const unsigned int& default_value);
template void Configuration::registerParameter<float>(const std::string& parameter_id, float* pointer, const float& default_value);
template void Configuration::registerParameter<double>(const std::string& parameter_id, double* pointer, const double& default_value);
template void Configuration::registerParameter<std::string>(const std::string& parameter_id, std::string* pointer, const std::string& default_value);
template void Configuration::registerParameter<nlohmann::json>(const std::string& parameter_id, nlohmann::json* pointer, const nlohmann::json& default_value);

template void Configuration::addParameter<bool>(const std::string& parameter_id, const bool& default_value);
template void Configuration::addParameter<int>(const std::string& parameter_id, const int& default_value);
template void Configuration::addParameter<unsigned int>(const std::string& parameter_id, const unsigned int& default_value);
template void Configuration::addParameter<float>(const std::string& parameter_id, const float& default_value);
template void Configuration::addParameter<double>(const std::string& parameter_id, const double& default_value);
template void Configuration::addParameter<std::string>(const std::string& parameter_id, const std::string& default_value);
template void Configuration::addParameter<nlohmann::json>(const std::string& parameter_id, const nlohmann::json& default_value);

template void Configuration::updateParameterPointer<bool>(const std::string& parameter_id, bool* pointer);
template void Configuration::updateParameterPointer<int>(const std::string& parameter_id, int* pointer);
template void Configuration::updateParameterPointer<unsigned int>(const std::string& parameter_id, unsigned int* pointer);
template void Configuration::updateParameterPointer<float>(const std::string& parameter_id, float* pointer);
template void Configuration::updateParameterPointer<double>(const std::string& parameter_id, double* pointer);
template void Configuration::updateParameterPointer<std::string>(const std::string& parameter_id, std::string* pointer);
template void Configuration::updateParameterPointer<nlohmann::json>(const std::string& parameter_id, nlohmann::json* pointer);

template void Configuration::getParameter<bool>(const std::string& parameter_id, bool& value) const;
template void Configuration::getParameter<int>(const std::string& parameter_id, int& value) const;
template void Configuration::getParameter<unsigned int>(const std::string& parameter_id, unsigned int& value) const;
template void Configuration::getParameter<float>(const std::string& parameter_id, float& value) const;
template void Configuration::getParameter<double>(const std::string& parameter_id, double& value) const;
template void Configuration::getParameter<std::string>(const std::string& parameter_id, std::string& value) const;
template void Configuration::getParameter<nlohmann::json>(const std::string& parameter_id, nlohmann::json& value) const;

template bool Configuration::hasParameterOfType<bool>(const std::string& parameter_id) const;
template bool Configuration::hasParameterOfType<int>(const std::string& parameter_id) const;
template bool Configuration::hasParameterOfType<unsigned int>(const std::string& parameter_id) const;
template bool Configuration::hasParameterOfType<float>(const std::string& parameter_id) const;
template bool Configuration::hasParameterOfType<double>(const std::string& parameter_id) const;
template bool Configuration::hasParameterOfType<std::string>(const std::string& parameter_id) const;
template bool Configuration::hasParameterOfType<nlohmann::json>(const std::string& parameter_id) const;

template bool Configuration::getParameterConfigValue<bool>(const std::string& parameter_id) const;
template int Configuration::getParameterConfigValue<int>(const std::string& parameter_id) const;
template unsigned int Configuration::getParameterConfigValue<unsigned int>(const std::string& parameter_id) const;
template float Configuration::getParameterConfigValue<float>(const std::string& parameter_id) const;
template double Configuration::getParameterConfigValue<double>(const std::string& parameter_id) const;
template std::string Configuration::getParameterConfigValue<std::string>(const std::string& parameter_id) const;
template nlohmann::json Configuration::getParameterConfigValue<nlohmann::json>(const std::string& parameter_id) const;

template bool Configuration::parameterValueFromConfig<bool>(const std::string& parameter_id) const;
template int Configuration::parameterValueFromConfig<int>(const std::string& parameter_id) const;
template unsigned int Configuration::parameterValueFromConfig<unsigned int>(const std::string& parameter_id) const;
template float Configuration::parameterValueFromConfig<float>(const std::string& parameter_id) const;
template double Configuration::parameterValueFromConfig<double>(const std::string& parameter_id) const;
template std::string Configuration::parameterValueFromConfig<std::string>(const std::string& parameter_id) const;
template nlohmann::json Configuration::parameterValueFromConfig<nlohmann::json>(const std::string& parameter_id) const;
