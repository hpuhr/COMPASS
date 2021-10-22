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

#ifndef DBOVARIABLEORDEREDSET_H_
#define DBOVARIABLEORDEREDSET_H_

#include "configurable.h"
#include "dbovariable.h"
#include "dbovariabledefinition.h"
#include "propertylist.h"

class MetaDBOVariable;
class DBOVariableSet;
class DBOVariableOrderedSet;
class DBOVariableOrderedSetWidget;

/**
 * @brief Extends DBOVariableDefinition with an ordering index
 */
class DBOVariableOrderDefinition : public DBOVariableDefinition
{
  public:
    DBOVariableOrderDefinition(std::string class_id, std::string instance_id, Configurable* parent)
        : DBOVariableDefinition(class_id, instance_id, parent)
    {
        registerParameter("index", &index_, 0);
    }
    virtual ~DBOVariableOrderDefinition() {}

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
class DBOVariableOrderedSet : public QObject, public Configurable
{
    Q_OBJECT

  signals:
    void setChangedSignal();
    void variableAddedChangedSignal();

  public:
    /// @brief Constructor
    DBOVariableOrderedSet(const std::string& class_id, const std::string& instance_id,
                          Configurable* parent);
    /// @brief Destructor
    virtual ~DBOVariableOrderedSet();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    /// @brief Adds a variable to the set at last index
    void add(DBOVariable& var);
    void add(MetaDBOVariable& var);
    void add (const std::string& dbo_name, const std::string var_name);

    /// @brief Adds a variable set
    // void add (const DBOVariableOrderedSet &set);
    /// @brief Adds a variable set for a given DBO type (DBOVariable::getFor)
    // void addOnly (DBOVariableOrderedSet &set, const std::string &dbo_type);
    /// @brief Removes a variable at a given index
    void removeVariableAt(unsigned int index);
    void removeVariable(const DBOVariable& variable);
    void removeMetaVariable(const MetaDBOVariable& variable);

    /// @brief Decreases index of a variable at a given index
    void moveVariableUp(unsigned int index);
    /// @brief Increases index of a variable at a given index
    void moveVariableDown(unsigned int index);

    /// @brief Returns if variable is in set
    bool hasVariable(const DBOVariable& variable) const;
    bool hasMetaVariable(const MetaDBOVariable& variable) const;
    /// @brief Returns if variable is in set
    bool hasVariable(const std::string& dbo_type, const std::string& name) const;

    /// @brief Returns a copied new variable set, with all variables for a given DBO type
    DBOVariableSet getFor(const std::string& dbo_name);
    DBOVariableSet getExistingInDBFor(const std::string& dbo_name);
    // DBOVariableSet getUnorderedSet () const;

    /// @brief Returns a variable at a given index
    DBOVariableOrderDefinition& variableDefinition(unsigned int index) const;
    const std::map<unsigned int, DBOVariableOrderDefinition*>& definitions()
    {
        return variable_definitions_;
    }

    /// @brief Returns set as properties for given DBO type
    // PropertyList getPropertyList (const std::string &dbo_type);

    /// @brief Prints information for debugging
    // void print () const;
    /// @brief Returns number of variables in set
    unsigned int getSize() const { return variable_definitions_.size(); }

    DBOVariableOrderedSetWidget* widget();

  protected:
    /// Container with ordered variable definitions (index -> definition pointer)
    std::map<unsigned int, DBOVariableOrderDefinition*> variable_definitions_;

    DBOVariableOrderedSetWidget* widget_;

    virtual void checkSubConfigurables();

    void reorderVariables ();
};

#endif /* DBOVariableOrderedSet_H_ */
