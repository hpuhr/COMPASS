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

#ifndef DBOVARIABLESET_H_
#define DBOVARIABLESET_H_

#include "propertylist.h"

class DBOVariable;

/**
 * @brief Set of DBOVariables
 *
 * Set with DBOVariables, which can only be added once (set), and have no specific order.
 *
 * \todo check meta variable in add
 * \todo return bool change on add
 */
class DBOVariableSet
{
  public:
    /// @brief Constructor
    DBOVariableSet();
    /// @brief Destructor
    virtual ~DBOVariableSet();

    /// @brief Returns flag indicating if a change occurred
    bool getChanged() { return changed_; }
    /// @brief Sets the change occurred flag
    void setChanged(bool changed) { changed_ = changed; }

    /// @brief Adds a DBOVariable
    bool add(DBOVariable& var);
    bool add(const DBOVariable& var);
    /// @brief Adds another set of variables
    bool add(DBOVariableSet& set);
    /// @brief Adds variables for a given type from a given set of variables
    // bool addOnly (DBOVariableSet &set, const std::string &dbo_type);
    /// @brief Removes variable at a given index
    void removeVariableAt(unsigned int index);
    void removeVariable(const DBOVariable& var);
    /// @brief Intersects with another set of variables
    bool intersect(DBOVariableSet& set);
    /// @brief Removes all variables
    void clear();
    /// @brief Returns if given variable is in the set
    bool hasVariable(const DBOVariable& variable);

    DBOVariableSet& operator=(const DBOVariableSet& source);
    // DBOVariableSet *clone ();

    /// @brief Returns container with all variables
    std::vector<DBOVariable*>& getSet() { return set_; }
    std::vector<DBOVariable*> getExistsInDBSet();
    /// @brief Returns variable at a given index
    DBOVariable& getVariable(unsigned int index) const;

    /// @brief Prints contents, for debugging purposes
    void print();

    /// @brief Returns number of variables in the set
    unsigned int getSize() const { return set_.size(); }

  protected:
    /// Container with all variables in the set
    std::vector<DBOVariable*> set_;

    /// Change occurred flag
    bool changed_;
};

#endif /* DBOVARIABLESET_H_ */
