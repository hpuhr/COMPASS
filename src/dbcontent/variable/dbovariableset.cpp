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

#include "dbovariableset.h"

#include <algorithm>

#include "dbcontent/dbcontent.h"
#include "dbovariable.h"

namespace dbContent
{

DBContentVariableSet::DBContentVariableSet() { changed_ = false; }

DBContentVariableSet::~DBContentVariableSet() {}

bool DBContentVariableSet::add(DBContentVariable& var)
{
    if (find(set_.begin(), set_.end(), &var) == set_.end())
    {
        set_.push_back(&var);
        changed_ = true;
        return true;
    }
    return false;
}

bool DBContentVariableSet::add(const DBContentVariable& var)
{
    if (find(set_.begin(), set_.end(), &var) == set_.end())
    {
        set_.push_back((DBContentVariable*)(&var));  // TODO this is ugly
        changed_ = true;
        return true;
    }
    return false;
}

bool DBContentVariableSet::add(DBContentVariableSet& set)
{
    std::vector<DBContentVariable*>& setset = set.getSet();
    std::vector<DBContentVariable*>::iterator it;

    bool added = false;

    for (it = setset.begin(); it != setset.end(); it++)
    {
        if (find(set_.begin(), set_.end(), *it) == set_.end())
        {
            set_.push_back(*it);
            changed_ = true;
            added = true;
        }
    }
    return added;
}

// bool DBOVariableSet::addOnly (DBOVariableSet &set, const std::string &dbo_type)
//{
//  logdbg  << "DBOVariableSet: addOnly: type " << dbo_type;
//  std::vector <DBOVariable*> &setset = set.getSet();

//  std::vector <DBOVariable*>::iterator it;
//  bool added=false;

//  for (it=setset.begin(); it != setset.end(); it++)
//  {
//    if (find (set_.begin(), set_.end(), *it) == set_.end())
//    {
//      //loginf  << "DBOVariableSet: addOnly: new var";
//      if ((*it)->existsIn(dbo_type))
//      {
//        logdbg  << "DBOVariableSet: addOnly: pushback";
//        set_.push_back ((*it)->getFor(dbo_type));
//        changed_=true;
//        added=true;
//      }
//    }
//  }

//  return added;
//}

void DBContentVariableSet::removeVariableAt(unsigned int index)
{
    assert(index < set_.size());

    set_.erase(set_.begin() + index);

    changed_ = true;
}

void DBContentVariableSet::removeVariable(const DBContentVariable& var)
{
    set_.erase(std::remove(set_.begin(), set_.end(), &var), set_.end());
}

// DBOVariableSet *DBOVariableSet::getFor (const std::string &dbo_type)
//{
//  logdbg  << "DBOVariableSet: getFor: type " << dbo_type;

//  DBOVariableSet *type_set = new DBOVariableSet ();
//  std::vector <DBOVariable*>::iterator it;

//  for (it=set_.begin(); it != set_.end(); it++)
//  {
//    if ((*it)->existsIn(dbo_type))
//    {
//      logdbg  << "DBOVariableSet: getFor: add";
//      assert (!(*it)->getFor(dbo_type)->isMetaVariable());
//      type_set->add ((*it)->getFor(dbo_type));
//    }
//  }

//  return type_set;
//}

DBContentVariableSet& DBContentVariableSet::operator=(const DBContentVariableSet& source)
{
    logdbg << "DBOVariableSet: copy constructor";

    if (this == &source)  // self assignment
        return *this;

    set_.clear();

    std::vector<DBContentVariable*>::const_iterator it;

    logdbg << "DBOVariableSet: copy constructor: copying " << source.set_.size() << " elements";

    // do the copy
    for (it = source.set_.begin(); it != source.set_.end(); it++)
    {
        add(*(*it));
    }

    return *this;
}

DBContentVariable& DBContentVariableSet::getVariable(unsigned int index) const
{
    assert(index < set_.size());
    return *set_.at(index);
}

bool DBContentVariableSet::intersect(DBContentVariableSet& set)
{
    std::vector<DBContentVariable*> org_set = set_;
    set_.clear();
    bool added = false;

    std::vector<DBContentVariable*>& setset = set.getSet();
    std::vector<DBContentVariable*>::iterator it;
    std::vector<DBContentVariable*>::iterator it2;

    for (it = org_set.begin(); it != org_set.end(); it++)  // traverse original list
    {
        it2 = find(setset.begin(), setset.end(), *it);
        if (it2 == setset.end())  // other set hasn't var
        {
            changed_ = true;
            added = true;
        }
        else
            set_.push_back(*it);
    }

    logdbg << "DBOVariableSet: intersect: size " << set_.size() << " other size "
           << set.getSet().size() << " done";
    return added;
}

void DBContentVariableSet::print()
{
    logdbg << "DBOVariableSet: print: size" << set_.size() << " changed " << changed_;
    std::vector<DBContentVariable*>::iterator it;

    for (it = set_.begin(); it != set_.end(); it++)
    {
        (*it)->print();
    }
}

void DBContentVariableSet::clear()
{
    set_.clear();
    changed_ = true;
}

bool DBContentVariableSet::hasVariable(const DBContentVariable& variable)
{
    return find(set_.begin(), set_.end(), &variable) != set_.end();
}

bool DBContentVariableSet::hasDBColumnName(const std::string& db_column_name)
{
    for (auto var_it : set_)
    {
        if (var_it->dbColumnName() == db_column_name)
            return true;
    }

    return false;
}

unsigned int DBContentVariableSet::getVariableWithDBColumnName(const std::string& db_column_name)
{
    assert(hasDBColumnName(db_column_name));

    unsigned int cnt = 0;
    for (auto var_it : set_)
    {
        if (var_it->dbColumnName() == db_column_name)
            return cnt;

        ++cnt;
    }

    assert(false);
}

}
