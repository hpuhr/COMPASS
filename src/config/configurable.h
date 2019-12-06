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

#ifndef CONFIGURABLE_H_
#define CONFIGURABLE_H_

#include <tinyxml2.h>
#include <vector>
#include <map>
#include <assert.h>

#include "configuration.h"

/**
 * @brief Configuration storage and retrieval functionality with sub-class generation
 *
 * @details A Configurable is a super-class that is used for configuration storage and retrieval for any number of
 * parameters.
 * Also can have sub-configurables as children, which are automatically generated at startup.
 *
 * A second constructor exists which should only be used if NO configuration will be saved or used. The idea behind
 * this is that, based on the configuration, sub-configurables are generated, which can be cloned. Since the clones
 * are not to be saved, they can use the simple constructor Configurable (), which disables all Configurable
 * functionality.
 *
 * Holds and uses a Configuration for managing the configuration parameters.
 *
 * If a class is derived from Configurable, it should override the following functions (if required)
 * @code
 * public:
 *   virtual void generateSubConfigurable (std::string class_id, std::string instance_id);
 * protected:
 *   virtual void checkSubConfigurables ();
 * @endcode
 * and call createSubConfigurables() in its constructor. Also, its constructor has to be adapted to supply the required
 * arguments for the Configurable constructor.
 *
 */
class Configurable
{
public:
    /// @brief Constructor
    Configurable (const std::string& class_id, const std::string& instance_id, Configurable *parent=nullptr,
                  const std::string& root_configuration_filename="");
    /// @brief Default constructor, for STL containers
    Configurable () = default;
    Configurable(const Configurable&) = delete;

    Configurable& operator=(const Configurable& other) = delete;
    /// @brief Move constructor
    Configurable& operator=(Configurable&& other);
    /// @brief Destructor
    virtual ~Configurable();

    /// @brief Reset parameters to their reset values
    virtual void resetToDefault ();

    /// @brief Adds a new sub-configuration based on class id and instance id
    Configuration &addNewSubConfiguration (const std::string& class_id, const std::string& instance_id);
    /// @brief Adds a new sub-configuration based on class id, instance id is generated
    Configuration &addNewSubConfiguration (const std::string& class_id);
    /// @brief Adds a new sub-configuration by reference and copy constructor
    Configuration &addNewSubConfiguration (Configuration &configuration);
    /// @brief Creates sub-configurables according to configuration
    void createSubConfigurables ();
    /// @brief Override for creation of sub-configurables
    virtual void generateSubConfigurable (const std::string& class_id, const std::string& instance_id);
    /// @brief Returns if a specified sub-configurable exists
    bool hasSubConfigurable(const std::string& class_id, const std::string& instance_id);

    Configurable& parent() { assert (parent_); return *parent_; }
    void parent(Configurable& parent) { parent_=&parent; }
    /// @brief Returns configuration for this class
    Configuration& configuration () { assert (configuration_); return *configuration_; }
    /// @brief Saves the current configuration as template at its parent
    //void saveConfigurationAsTemplate (const std::string& template_name);

    /// @brief Returns instance identifier
    const std::string& instanceId () const { return instance_id_; }
    /// @brief Returns class identifier
    const std::string& classId () const { return class_id_; }
    /// @brief Returns key identifier (class_id + instance_id)
    const std::string& keyId () const { return key_id_; }

private:
    /// Class identifier
    std::string class_id_;
    /// Instance identifier
    std::string instance_id_;
    /// Key identifier
    std::string key_id_;
    /// Parent pointer, null if Singleton or empty
    Configurable* parent_ {nullptr};
    /// Configuration
    Configuration* configuration_ {nullptr};
    bool is_root_ {false};

    /// Container for all sub-configurables (class id + instance id -> Configurable)
    std::map <std::string, Configurable&> children_;

protected:
    /// @brief Registers a bool parameter
    void registerParameter (const std::string& parameter_id, bool* pointer, bool default_value);
    /// @brief Registers a int parameter
    void registerParameter (const std::string& parameter_id, int* pointer, int default_value);
    /// @brief Registers a unsigned int parameter
    void registerParameter (const std::string& parameter_id, unsigned int* pointer, unsigned int default_value);
    /// @brief Registers a float parameter
    void registerParameter (const std::string& parameter_id, float* pointer, float default_value);
    /// @brief Registers a double parameter
    void registerParameter (const std::string& parameter_id, double* pointer, double default_value);
    /// @brief Registers a string parameter
    void registerParameter (const std::string& parameter_id, std::string* pointer, const std::string& default_value);

    /// @brief Override to check if required sub-configurables exist
    virtual void checkSubConfigurables ();
    /// @brief Saves the specified child's configuration as template
    //void saveTemplateConfiguration (Configurable *child, const std::string& template_name);

    /// @brief Adds a configurable as a child
    Configuration& registerSubConfigurable (Configurable& child, bool config_must_exist=false);
    /// @brief Removes a child configurable
    void removeChildConfigurable (Configurable& child, bool remove_config=true);

};

#endif /* CONFIGURABLE_H_ */
