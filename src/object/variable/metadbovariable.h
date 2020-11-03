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

#ifndef METADBOVARIABLE_H
#define METADBOVARIABLE_H

#include "configurable.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "stringconv.h"

class MetaDBOVariableWidget;

class MetaDBOVariable : public Configurable
{
  public:
    MetaDBOVariable(const std::string& class_id, const std::string& instance_id,
                    DBObjectManager* object_manager);
    virtual ~MetaDBOVariable();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    bool hasVariables() const { return variables_.size() > 0; }
    PropertyDataType dataType() const;
    const std::string& dataTypeString() const;
    DBOVariable::Representation representation();

    /// @brief Return if variable exist in DBO of type
    bool existsIn(const std::string& dbo_name);
    /// @brief Returns variable existing in DBO of type
    DBOVariable& getFor(const std::string& dbo_name);
    /// @brief Return variable identifier in DBO of type
    std::string getNameFor(const std::string& dbo_name);

    void removeVariable(const std::string& dbo_name);
    /// @brief Sets sub-variable name for DBO of type
    void addVariable(const std::string& dbo_name, const std::string& dbovariable_name);

    const std::map<std::string, DBOVariable&>& variables() { return variables_; }
    bool uses(const DBOVariable& variable);

    std::string name() const;
    void name(const std::string& name);

    std::string description() const;
    void description(const std::string& description);

    std::string getMinString() const;
    std::string getMaxString() const;
    std::string getMinStringRepresentation() const;
    std::string getMaxStringRepresentation() const;

    MetaDBOVariableWidget* widget();

    void unlock();
    void lock();

    bool existsInDB() const;

    void removeOutdatedVariables();

  protected:
    std::string name_;
    std::string description_;

    DBObjectManager& object_manager_;

    MetaDBOVariableWidget* widget_;

    bool locked_{false};

    std::map<std::string, DBOVariableDefinition*> definitions_;
    std::map<std::string, DBOVariable&> variables_;

    virtual void checkSubConfigurables();
};

#endif  // METADBOVARIABLE_H
