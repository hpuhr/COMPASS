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
 * DBOVariable.cpp
 *
 *  Created on: Apr 25, 2012
 *      Author: sk
 */

#include <algorithm>

#include "configuration.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbovariable.h"
#include "atsdb.h"
#include "dbobjectmanager.h"
#include "dbtablecolumn.h"
#include "dbtable.h"
#include "metadbtable.h"
#include "dbschema.h"
#include "unit.h"
#include "unitmanager.h"
#include "dbschemamanager.h"
#include "dbovariablewidget.h"
//#include "DBOVariableMinMaxObserver.h"

#include <boost/algorithm/string.hpp>

#include "stringconv.h"
#include "stringrepresentation.h"

using namespace Utils;

DBOVariable::DBOVariable(const std::string &class_id, const std::string &instance_id, DBObject *parent)
    : Property (), Configurable (class_id, instance_id, parent), dbo_parent_(*parent), widget_(nullptr)
{
    registerParameter ("name", &name_, "");
    registerParameter ("description", &description_, "");
    registerParameter ("data_type_str", &data_type_str_, "");
    registerParameter ("representation_str", &representation_str_, "");
    registerParameter ("dimension", &dimension_, "");
    registerParameter ("unit", &unit_, "");

    assert (name_.size() > 0);
    assert (data_type_str_.size() > 0);
    data_type_ = Property::asDataType(data_type_str_);

    if (representation_str_.size() == 0)
    {
        representation_str_ = representation_2_string.at(StringRepresentation::STANDARD);
    }

    assert (string_2_representation.count(representation_str_) == 1);
    representation_ = string_2_representation.at(representation_str_);

    //loginf  << "DBOVariable: constructor: name " << id_ << " unitdim '" << unit_dimension_ << "' unitunit '" << unit_unit_ << "'";

    createSubConfigurables ();
}

DBOVariable::~DBOVariable()
{
    for (auto it : schema_variables_)
        delete it.second;
    schema_variables_.clear();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

void DBOVariable::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id.compare("DBOSchemaVariableDefinition") == 0)
    {
        DBOSchemaVariableDefinition *definition = new DBOSchemaVariableDefinition (class_id, instance_id, this);
        assert (schema_variables_.find (definition->getSchema()) == schema_variables_.end());
        schema_variables_[definition->getSchema()] = definition;
    }
    else
        throw std::runtime_error ("DBOVariable: generateSubConfigurable: unknown class_id "+class_id);
}

bool DBOVariable::operator==(const DBOVariable &var)
{
    if (dboName() != var.dboName())
        return false;
    if (data_type_ != var.data_type_)
        return false;
    if (name_.compare (var.name_) != 0)
        return false;

    return true;
}

void DBOVariable::print ()
{
    loginf  << "DBOVariable: print: dbo " << parent_->getInstanceId() << " id " << name_ << " data type " << data_type_str_;
}


//std::string DBOVariable::getValueFromRepresentation (std::string representation_string, bool transform, bool* ok)
//{

//    std::stringstream ss;
//    bool ssok = true;

//    if (representation_int_ == R_STANDARD)
//    {
//        ss << representation_string;
//    }
//    else if (representation_int_ == R_TIME_SECONDS)
//    {
//        ss << timeFromString (representation_string, &ssok);
//    }
//    else if (representation_int_ == R_OCTAL)
//    {
//        ss << intFromOctalString (representation_string, &ssok);
//    }
//    else if (representation_int_ ==R_FLIGHT_LEVEL)
//    {
//        std::stringstream ss2;
//        ss2 << representation_string;
//        double value;
//        ssok = ( ss2 >> value );
//        if( !ssok )
//            logerr << "DBOVariable: getValueFromRepresentation: Could not convert to flight level";
//        ss << value*100.0;
//    }
//    else if (representation_int_ == R_SENSOR_NAME)
//    {
//        throw std::runtime_error ("DBOVariable: getRepresentationFromValue: unknown for sensor name");
//    }
//    else if (representation_int_ == R_HEX)
//    {
//        ss << intFromHexString (representation_string, &ssok);
//    }
//    else
//    {
//        throw std::runtime_error ("DBOVariable: getRepresentationFromValue: unknown representation");
//    }

//    if( ok )
//        *ok = ssok;

//    if (transform)
//    {
//        logdbg  << "DBOVariable: getValueFromRepresentation: var " << id_ << " representation " << representation_string;

//        DBOVariable *variable;

//        if (isMetaVariable())
//            variable = getFirst();
//        else
//            variable = this;

//        std::string meta_tablename = variable->getCurrentMetaTable ();
//        std::string table_varname = variable->getCurrentVariableName ();

//        DBTableColumn *table_column = DBSchemaManager::getInstance().getCurrentSchema ()->getMetaTable(meta_tablename)->getTableColumn(table_varname);

//        if (hasUnit () || table_column->hasUnit())
//        {
//            //loginf  << "var type " << variable->getDBOType() << " dim '" << variable->getUnitDimension() << "'";
//            if (variable->hasUnit () != table_column->hasUnit())
//            {
//                logerr << "DBOVariable: getValueFromRepresentation: unit transformation inconsistent: var " << variable->getName ()
//                                            << " has unit " << hasUnit () << " table column " << table_column->getName() << " has unit "
//                                            << table_column->hasUnit();
//                throw std::runtime_error ("DBOVariable: getValueFromRepresentation: tranformation error 1");
//            }

//            if (variable->getUnitDimension().compare(table_column->getUnitDimension()) != 0)
//            {
//                logerr << "DBOVariable: getValueFromRepresentation: unit transformation inconsistent: var "
//                        << variable->getName () << " has dimension " << getUnitDimension () << " table column "
//                        << table_column->getName() << " has dimension " << table_column->getUnitDimension();
//                throw std::runtime_error ("DBOVariable: getValueFromRepresentation: tranformation error 2");
//            }

//            Unit *unit = UnitManager::getInstance().getUnit (variable->getUnitDimension());
//            double factor = unit->getFactor (variable->getUnitUnit(), table_column->getUnitUnit());
//            logdbg  << "DBOVariable: getValueFromRepresentation: correct unit transformation with factor " << factor;

//            double var = doubleFromString(ss.str());
//            var *= factor;
//            std::string transformed = doubleToString (var);

//            logdbg  << "DBOVariable: getValueFromRepresentation: var " << id_ << " transformed representation " << transformed;
//            return transformed;
//        }
//        else
//            return ss.str();
//    }
//    else
//        return ss.str();
//}
//std::string DBOVariable::getRepresentationFromValue (std::string value_string)
//{
//    std::stringstream ss;

//    if (representation_int_ == R_STANDARD)
//    {
//        ss << value_string;
//    }
//    else if (representation_int_ == R_TIME_SECONDS)
//    {
//        std::stringstream ss2;
//        ss2 << value_string;
//        double value;
//        ss2 >> value;
//        ss << timeStringFromDouble (value);
//    }
//    else if (representation_int_ == R_OCTAL)
//    {
//        std::stringstream ss2;
//        ss2 << value_string;
//        unsigned int value;
//        ss2 >> value;
//        ss << std::oct << std::setfill ('0') << std::setw (4) << value;
//    }
//    else if (representation_int_ ==R_FLIGHT_LEVEL)
//    {
//        std::stringstream ss2;
//        ss2 << value_string;
//        int value;
//        ss2 >> value;
//        ss << ((double)value)/100.0;
//    }
//    else if (representation_int_ == R_HEX)
//    {
//        std::stringstream ss2;
//        ss2 << value_string;
//        unsigned int value;
//        ss2 >> value;
//        ss << std::uppercase << std::hex << value;
//    }
//    else if (representation_int_ == R_SENSOR_NAME)
//    {
//        std::stringstream ss2;
//        ss2 << value_string;
//        unsigned int value;
//        ss2 >> value;
//        ss << "TODO"; // TODO HACK
//    }
//    else
//    {
//        throw std::runtime_error ("DBOVariable: getRepresentationFromValue: unknown representation");
//    }

//    return ss.str();
//}

//void DBOVariable::setStringRepresentation (STRING_REPRESENTATION representation)
//{
//    logdbg  << " DBOVariable: setStringRepresentation " << representation;
//    representation_int_=representation;

//    for (unsigned int cnt=0; cnt < sub_variable_definitions_.size(); cnt++)
//    {
//        DBOVariable *variable = DBObjectManager::getInstance().getDBOVariable (sub_variable_definitions_.at(cnt)->getDBOType(), sub_variable_definitions_.at(cnt)->getId());
//        variable->setStringRepresentation (representation);
//    }
//}

void DBOVariable::checkSubConfigurables ()
{
    // nothing to do here
}

const std::string &DBOVariable::dboName () const
{
    return dbo_parent_.name();
}


bool DBOVariable::hasSchema (const std::string &schema)
{
    return schema_variables_.find (schema) != schema_variables_.end();
}
const std::string &DBOVariable::metaTable (const std::string &schema)
{
    assert (hasSchema(schema));
    return schema_variables_.at(schema)->getMetaTable();
}
const std::string &DBOVariable::variableName (const std::string &schema)
{
    assert (hasSchema(schema));
    return schema_variables_.at(schema)->getVariable();
}

bool DBOVariable::hasCurrentDBColumn ()
{
    std::string meta_tablename = currentMetaTableString ();
    std::string table_varname = currentVariableName ();

    return ATSDB::instance().schemaManager().getCurrentSchema().metaTable(meta_tablename).hasColumn(table_varname);
}

const DBTableColumn &DBOVariable::currentDBColumn ()
{
    assert (hasCurrentDBColumn());

    std::string meta_tablename = currentMetaTableString ();
    std::string table_varname = currentVariableName ();

    return ATSDB::instance().schemaManager().getCurrentSchema().metaTable(meta_tablename).column(table_varname);
}

bool DBOVariable::hasCurrentSchema ()
{
    return hasSchema(ATSDB::instance().schemaManager().getCurrentSchemaName());
}

const std::string &DBOVariable::currentMetaTableString ()
{
    assert (hasCurrentSchema());
    std::string schema = ATSDB::instance().schemaManager().getCurrentSchemaName();
    return schema_variables_.at(schema)->getMetaTable();

}

const MetaDBTable &DBOVariable::currentMetaTable ()
{
    assert (hasCurrentSchema());
    std::string schema = ATSDB::instance().schemaManager().getCurrentSchemaName();
    std::string meta_table = schema_variables_.at(schema)->getMetaTable();
    assert (ATSDB::instance().schemaManager().getCurrentSchema().hasMetaTable(meta_table));
    return ATSDB::instance().schemaManager().getCurrentSchema().metaTable(meta_table);
}

const std::string &DBOVariable::currentVariableName ()
{
    assert (hasCurrentSchema());
    std::string schema = ATSDB::instance().schemaManager().getCurrentSchemaName();
    return schema_variables_.at(schema)->getVariable();
}

//bool DBOVariable::hasMinMaxInfo ()
//{
//    if (isMetaVariable())
//    {
//        std::map <DB_OBJECT_TYPE, std::string>::iterator it;
//        bool info_present=true;
//        for (it = sub_variables_.begin(); it != sub_variables_.end(); it++)
//            info_present &= DBObjectManager::getInstance().getDBOVariable(it->first, it->second)->hasMinMaxInfo();
//        return info_present;
//    }
//    else
//        return !(min_.size() == 0 && max_.size() == 0);
//}

//void DBOVariable::buildMinMaxInfo ()
//{
//    assert (!hasMinMaxInfo());

//    if (!isMetaVariable())
//        ATSDB::getInstance().buildMinMaxInfo(this);
//    else
//    {
//        std::map <DB_OBJECT_TYPE, std::string>::iterator it;

//        for (it = sub_variables_.begin(); it != sub_variables_.end(); it++)
//        {
//            assert (DBObjectManager::getInstance().existsDBOVariable(it->first, it->second));
//            DBOVariable *var = DBObjectManager::getInstance().getDBOVariable(it->first, it->second);

//            if (!var->hasMinMaxInfo())
//                ATSDB::getInstance().buildMinMaxInfo(var);
//        }
//    }
//}

//void DBOVariable::setMinMax (std::string min, std::string max)
//{
//    if (isMetaVariable())
//        throw std::runtime_error ("DBOVariable: setMinString: "+id_+" not possible when meta");

//    min_=min;
//    max_=max;

//    logdbg << "DBOVariable: setMinMax: min " << min_ << " max " << max_;

//    assert (hasMinMaxInfo());
//    notifyMinMaxObservers ();

//    if (parent_variables_.size() > 0)
//    {
//        logdbg << "DBOVariable: setMinMax: " << id_ << " updating parents";
//        std::vector <DBOVariable*>::iterator it;
//        for (it = parent_variables_.begin(); it != parent_variables_.end(); it++)
//            (*it)->subVariableHasMinMaxInfo();
//    }
//}

//std::string DBOVariable::getMinString ()
//{
//    std::string min;
//    if (isMetaVariable())
//    {
//        std::map <DB_OBJECT_TYPE, std::string>::iterator it;
//        for (it = sub_variables_.begin(); it != sub_variables_.end(); it++)
//        {
//            DBOVariable *var = DBObjectManager::getInstance().getDBOVariable(it->first, it->second);
//            if (it == sub_variables_.begin())
//                min = var->getMinString();
//            else if (!isLargerAs(min, var->getMinString(), (PROPERTY_DATA_TYPE) data_type_int_))
//            {
//                logdbg << "DBOVariable: getMinString: new min " << var->getMinString() << " old " << min;
//                min = var->getMinString();
//            }
//        }
//    }
//    else
//        min = min_;

//    DBOVariable *tmpvar = getFirst();

//    std::string meta_tablename = tmpvar->getCurrentMetaTable ();
//    std::string table_varname = tmpvar->getCurrentVariableName ();

//    DBTableColumn *table_column = DBSchemaManager::getInstance().getCurrentSchema ()->getMetaTable(meta_tablename)->getTableColumn(table_varname);

//    if (!isMetaVariable() && (tmpvar->hasUnit () || table_column->hasUnit()))
//    {
//        if (tmpvar->hasUnit () != table_column->hasUnit())
//        {
//            logerr << "DBOVariable: getMinString: unit transformation inconsistent: var " << tmpvar->getName () << " has unit " << tmpvar->hasUnit ()
//                                                                      << " table column " << table_column->getName() << " has unit " << table_column->hasUnit();
//            throw std::runtime_error ("DBOVariable: getMinString: unit transformation error 1");
//        }

//        if (tmpvar->getUnitDimension().compare(table_column->getUnitDimension()) != 0)
//        {
//            logerr << "DBOVariable: getMinString: unit transformation inconsistent: var " << tmpvar->getName () << " has dimension " << tmpvar->getUnitDimension ()
//                                                                      << " table column " << table_column->getName() << " has dimension " << table_column->getUnitDimension();
//            throw std::runtime_error ("DBOVariable: getMinString: unit transformation error 2");
//        }

//        Unit *unit = UnitManager::getInstance().getUnit (tmpvar->getUnitDimension());
//        double factor = unit->getFactor (table_column->getUnitUnit(), tmpvar->getUnitUnit());
//        logdbg  << "DBOVariable: getMinString: adapting " << tmpvar->getName () << " unit transformation with factor " << factor;

//        multiplyString (min, (PROPERTY_DATA_TYPE) tmpvar->data_type_int_, factor);
//    }

//    logdbg << "DBOVariable: getMinString: type " << dbo_type_int_ << " name " << id_ << " returning " << min;
//    return min;
//}

//std::string DBOVariable::getMaxString ()
//{
//    std::string max;
//    if (isMetaVariable())
//    {
//        std::map <DB_OBJECT_TYPE, std::string>::iterator it;
//        for (it = sub_variables_.begin(); it != sub_variables_.end(); it++)
//        {
//            DBOVariable *var = DBObjectManager::getInstance().getDBOVariable(it->first, it->second);
//            if (it == sub_variables_.begin())
//                max = var->getMaxString();
//            else if (isLargerAs(max, var->getMaxString(), (PROPERTY_DATA_TYPE) data_type_int_))
//            {
//                logdbg << "DBOVariable: getMaxString: new max " << var->getMaxString() << " old " << max;
//                max = var->getMaxString();
//            }
//        }
//    }
//    else
//        max = max_;

//    DBOVariable *tmpvar = getFirst();

//    std::string meta_tablename = tmpvar->getCurrentMetaTable ();
//    std::string table_varname = tmpvar->getCurrentVariableName ();

//    DBTableColumn *table_column = DBSchemaManager::getInstance().getCurrentSchema ()->getMetaTable(meta_tablename)->getTableColumn(table_varname);

//    if (!isMetaVariable() && (tmpvar->hasUnit () || table_column->hasUnit()))
//    {
//        if (tmpvar->hasUnit () != table_column->hasUnit())
//        {
//            logerr << "DBOVariable: getMinString: unit transformation inconsistent: var " << tmpvar->getName () << " has unit " << tmpvar->hasUnit ()
//                                                                      << " table column " << table_column->getName() << " has unit " << table_column->hasUnit();
//            throw std::runtime_error ("DBOVariable: getMinString: unit transformation error 1");
//        }

//        if (tmpvar->getUnitDimension().compare(table_column->getUnitDimension()) != 0)
//        {
//            logerr << "DBOVariable: getMinString: unit transformation inconsistent: var " << tmpvar->getName () << " has dimension " << tmpvar->getUnitDimension ()
//                                                                      << " table column " << table_column->getName() << " has dimension " << table_column->getUnitDimension();
//            throw std::runtime_error ("DBOVariable: getMinString: unit transformation error 2");
//        }

//        Unit *unit = UnitManager::getInstance().getUnit (tmpvar->getUnitDimension());
//        double factor = unit->getFactor (table_column->getUnitUnit(), tmpvar->getUnitUnit());
//        logdbg  << "DBOVariable: getMinString: adapting " << tmpvar->getName () << " unit transformation with factor " << factor;

//        multiplyString (max, (PROPERTY_DATA_TYPE) tmpvar->data_type_int_, factor);
//    }

//    logdbg << "DBOVariable: getMaxString: type " << dbo_type_int_ << " name " << id_ << " returning " << max;
//    return max;
//}

//void DBOVariable::addMinMaxObserver (DBOVariableMinMaxObserver *observer)
//{
//    assert (find (min_max_observers_.begin(), min_max_observers_.end(), observer) ==
//            min_max_observers_.end());
//    min_max_observers_.push_back (observer);
//}
//void DBOVariable::removeMinMaxObserver (DBOVariableMinMaxObserver *observer)
//{
//    assert (find (min_max_observers_.begin(), min_max_observers_.end(), observer) !=
//            min_max_observers_.end());
//    min_max_observers_.erase (find (min_max_observers_.begin(), min_max_observers_.end(), observer));
//}

//void DBOVariable::notifyMinMaxObservers ()
//{
//    std::vector <DBOVariableMinMaxObserver *>::iterator it;
//    for (it=min_max_observers_.begin(); it != min_max_observers_.end(); it++)
//        (*it)->notifyMinMax (this);
//}

//void DBOVariable::registerParentVariable (DBOVariable *parent)
//{
//    assert (find (parent_variables_.begin(), parent_variables_.end(), parent) == parent_variables_.end());
//    parent_variables_.push_back(parent);
//}
//void DBOVariable::unregisterParentVariable (DBOVariable *parent)
//{
//    assert (find (parent_variables_.begin(), parent_variables_.end(), parent) != parent_variables_.end());
//    parent_variables_.erase(find (parent_variables_.begin(), parent_variables_.end(), parent));
//}

//void DBOVariable::subVariableHasMinMaxInfo ()
//{
//    assert (isMetaVariable());

//    std::map <std::string, std::string>::iterator it;
//    bool min_max_valid = true;
//    for (it = sub_variables_.begin(); it != sub_variables_.end(); it++)
//    {
//        DBOVariable *var = DBObjectManager::getInstance().getDBOVariable(it->first, it->second);
//        min_max_valid &= var->hasMinMaxInfo();
//    }

//    if (min_max_valid)
//    {
//        logdbg << "DBOVariable: subVariableHasMinMaxInfo: " << id_ << " has now valid info";
//        notifyMinMaxObservers();
//    }
//    else
//        logdbg << "DBOVariable: subVariableHasMinMaxInfo: " << id_ << " has incomplete info";
//}

DBOVariableWidget *DBOVariable::widget ()
{
    if (!widget_)
    {
        widget_ = new DBOVariableWidget (*this);
    }

    assert (widget_);
    return widget_;
}

StringRepresentation DBOVariable::representation() const
{
    return representation_;
}

void DBOVariable::representation(const StringRepresentation &representation)
{
    assert (representation_2_string.count(representation) == 1);

    representation_ = representation;
    representation_str_ = representation_2_string.at(representation_);
}
