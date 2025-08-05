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

#pragma once

#include "configurable.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variabledefinition.h"
//#include "propertylist.h"

namespace dbContent
{

class MetaVariable;
class VariableSet;
class VariableOrderedSet;
class VariableOrderedSetWidget;

class VariableOrderedSet : public QObject, public Configurable
{
    Q_OBJECT

  signals:
    void setChangedSignal();
    void variableAddedChangedSignal();
    void variableRemovedSignal();
    void variableMovedSignal();

  public:
    VariableOrderedSet(const std::string& class_id, 
                       const std::string& instance_id,
                       Configurable* parent);
    virtual ~VariableOrderedSet();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    void add(Variable& var);
    void add(MetaVariable& var);
    void add(const std::string& dbcontent_name, const std::string var_name);

    void set(const std::vector<std::pair<std::string,std::string>>& vars);

    void removeVariableAt(unsigned int index);
    void removeVariable(const Variable& variable);
    void removeMetaVariable(const MetaVariable& variable);

    /// @brief Decreases position of a variable at a given index
    void moveVariableUp(unsigned int index);
    /// @brief Increases positions of a variable at a given index
    void moveVariableDown(unsigned int index);

    /// @brief Returns if variable is in set
    bool hasVariable(const Variable& variable) const;
    bool hasMetaVariable(const MetaVariable& variable) const;
    /// @brief Returns if variable is in set
    bool hasVariable(const std::string& dbcont_name, const std::string& name) const;
    unsigned int getIndexFor(const std::string& dbcontent_name, const std::string var_name);

    VariableSet getFor(const std::string& dbcontent_name);
    //VariableSet getExistingInDBFor(const std::string& dbcontent_name);

    std::vector<std::pair<std::string,std::string>> definitions() const;
    std::pair<std::string,std::string> variableDefinition(unsigned int index) const;
    unsigned int getSize() const;

    VariableOrderedSetWidget* createWidget();

  protected:
    nlohmann::json variable_definitions_; // json list of std::string pairs

    virtual void checkSubConfigurables() override;
    virtual void onConfigurationChanged(const std::vector<std::string>& changed_params) override;

    void removeVariableAt(unsigned int index, bool signal_changes);
};

}
