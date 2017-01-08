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

/*
 * DBOVariableOrderedSet.h
 *
 *  Created on: May 22, 2012
 *      Author: sk
 */

#ifndef DBOVARIABLEORDEREDSET_H_
#define DBOVARIABLEORDEREDSET_H_

#include "Configurable.h"
#include "PropertyList.h"

#include "DBOVariable.h"

class DBOVariableSet;
class DBOVariableOrderedSet;

/**
 * @brief Extends DBOVariableDefinition with an ordering index
 */
class DBOVariableOrderDefinition : public DBOVariableDefinition
{
public:
  DBOVariableOrderDefinition(std::string class_id, std::string instance_id, Configurable *parent)
  : DBOVariableDefinition (class_id, instance_id, parent)
  {
    registerParameter ("index", &index_, 0);

  }
  virtual ~DBOVariableOrderDefinition() {}

  unsigned int getIndex () { return index_; }
  void setIndex (unsigned index) { index_=index; }

protected:
  unsigned int index_;
};

/**
 * @brief Ordered set of DBOVariables
 *
 * Set with DBOVariables, which can only be added once (set), and have a specific order.
 */
class DBOVariableOrderedSet : public Configurable
{
public:
  /// @brief Constructor
  DBOVariableOrderedSet(const std::string &class_id, const std::string &instance_id, Configurable *parent);
  /// @brief Destructor
  virtual ~DBOVariableOrderedSet();

  virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

  /// @brief Returns if a change occurred
  bool getChanged () const { return changed_; }
  /// @brief Sets if a change occurred
  void setChanged (bool changed) { changed_=changed; }

  /// @brief Adds a variable to the set at last index
  void add (DBOVariable &var);
  /// @brief Adds a variable set
  void add (const DBOVariableOrderedSet &set);
  /// @brief Adds a variable set for a given DBO type (DBOVariable::getFor)
  //void addOnly (DBOVariableOrderedSet &set, const std::string &dbo_type);
  /// @brief Removes a variable at a given index
  void removeVariableAt (unsigned int index);

  /// @brief Decreases index of a variable at a given index
  void moveVariableUp (unsigned int index);
  /// @brief Increases index of a variable at a given index
  void moveVariableDown (unsigned int index);

  /// @brief Returns if variable is in set
  bool hasVariable (const DBOVariable &variable) const;
  /// @brief Returns if variable is in set
  bool hasVariable (const std::string &dbo_type, const std::string &name) const;

  /// @brief Returns a copied new variable set, with all variables for a given DBO type
  //DBOVariableSet *getFor (const std::string &dbo_type);
  //DBOVariableSet getUnorderedSet () const;

  /// @brief Returns container with all variables
  const std::map <unsigned int, DBOVariable &> &getSet () const { return set_;}
  /// @brief Returns a variable at a given index
  DBOVariable &getVariable (unsigned int index) const;

  /// @brief Returns set as properties for given DBO type
  //PropertyList getPropertyList (const std::string &dbo_type);

  /// @brief Prints information for debugging
  void print () const;
  /// @brief Returns number of variables in set
  unsigned int getSize () const { return variable_definitions_.size(); }

protected:
  /// Container with ordered variable definitions (index -> definition pointer)
  std::map <unsigned int, DBOVariableOrderDefinition *> variable_definitions_;
  /// Container with all variables
  std::map <unsigned int, DBOVariable &> set_;
  //std::vector <DBOVariable &> set_;

  /// Change occurred flag
  bool changed_;

  virtual void checkSubConfigurables ();

  /// Updates set according to variable_definitions_
  void updateDBOVariableSet ();
};

#endif /* DBOVariableOrderedSet_H_ */
