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

#ifndef DBCONTENT_VARIABLEORDEREDSET_H_
#define DBCONTENT_VARIABLEORDEREDSET_H_

#include "configurable.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/variabledefinition.h"
#include "propertylist.h"

namespace dbContent
{

class MetaVariable;
class VariableSet;
class VariableOrderedSet;
class VariableOrderedSetWidget;

/**
 * @brief Extends DBOVariableDefinition with an ordering index
 */
class VariableOrderDefinition : public VariableDefinition
{
  public:
    VariableOrderDefinition(std::string class_id, std::string instance_id, Configurable* parent)
        : VariableDefinition(class_id, instance_id, parent)
    {
        registerParameter("index", &index_, 0);
    }
    virtual ~VariableOrderDefinition() {}

    unsigned int getIndex() { return index_; }
    void setIndex(unsigned index) { index_ = index; }

  protected:
    unsigned int index_;
};

/**
 * @brief Ordered set of DBOVariables
 *
 * Set with DBOVariables, which can only be added once (set), and have a specific order.
 */
class VariableOrderedSet : public QObject, public Configurable
{
    Q_OBJECT

  signals:
    void setChangedSignal();
    void variableAddedChangedSignal();

  public:
    /// @brief Constructor
    VariableOrderedSet(const std::string& class_id, const std::string& instance_id,
                          Configurable* parent);
    /// @brief Destructor
    virtual ~VariableOrderedSet();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    /// @brief Adds a variable to the set at last index
    void add(Variable& var);
    void add(MetaVariable& var);
    void add (const std::string& dbcontent_name, const std::string var_name);

    /// @brief Adds a variable set
    // void add (const DBOVariableOrderedSet &set);
    /// @brief Adds a variable set for a given DBO type (DBOVariable::getFor)
    // void addOnly (DBOVariableOrderedSet &set, const std::string &dbo_type);
    /// @brief Removes a variable at a given index
    void removeVariableAt(unsigned int index);
    void removeVariable(const Variable& variable);
    void removeMetaVariable(const MetaVariable& variable);

    /// @brief Decreases index of a variable at a given index
    void moveVariableUp(unsigned int index);
    /// @brief Increases index of a variable at a given index
    void moveVariableDown(unsigned int index);

    /// @brief Returns if variable is in set
    bool hasVariable(const Variable& variable) const;
    bool hasMetaVariable(const MetaVariable& variable) const;
    /// @brief Returns if variable is in set
    bool hasVariable(const std::string& dbo_type, const std::string& name) const;

    /// @brief Returns a copied new variable set, with all variables for a given DBO type
    VariableSet getFor(const std::string& dbcontent_name);
    VariableSet getExistingInDBFor(const std::string& dbcontent_name);
    // DBOVariableSet getUnorderedSet () const;

    /// @brief Returns a variable at a given index
    VariableOrderDefinition& variableDefinition(unsigned int index) const;
    const std::map<unsigned int, VariableOrderDefinition*>& definitions()
    {
        return variable_definitions_;
    }

    /// @brief Returns set as properties for given DBO type
    // PropertyList getPropertyList (const std::string &dbo_type);

    /// @brief Prints information for debugging
    // void print () const;
    /// @brief Returns number of variables in set
    unsigned int getSize() const { return variable_definitions_.size(); }

    VariableOrderedSetWidget* widget();

  protected:
    /// Container with ordered variable definitions (index -> definition pointer)
    std::map<unsigned int, VariableOrderDefinition*> variable_definitions_;

    VariableOrderedSetWidget* widget_;

    virtual void checkSubConfigurables();

    void reorderVariables ();
};

}

#endif /* DBCONTENT_VARIABLEORDEREDSET_H_ */
