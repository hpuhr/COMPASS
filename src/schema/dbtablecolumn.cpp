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
 * DBTableColumn.cpp
 *
 *  Created on: Aug 13, 2012
 *      Author: sk
 */

#include <boost/assign/list_of.hpp>

#include "dbtable.h"
#include "dbtablecolumn.h"
#include "logger.h"
#include "unitselectionwidget.h"

// TODO watch out for unsigned
std::map<std::string, PropertyDataType> DBTableColumn::db_types_2_data_types_ = boost::assign::map_list_of
        ("bool", PropertyDataType::BOOL)
        ("tinyint", PropertyDataType::CHAR)
        //("UCHAR", PropertyDataType::UCHAR)
        ("smallint", PropertyDataType::INT)
        ("mediumint", PropertyDataType::INT)
        ("int", PropertyDataType::INT)
        //("UINT", PropertyDataType::UCHAR)
        ("bigint", PropertyDataType::LONGINT)
        //("ULONGINT", PropertyDataType::ULONGINT)
        ("float", PropertyDataType::FLOAT)
        ("double", PropertyDataType::DOUBLE)
        ("enum", PropertyDataType::STRING)
        ("tinyblob", PropertyDataType::STRING)
        ("char", PropertyDataType::STRING)
        ("blob", PropertyDataType::STRING)
        ("mediumblob", PropertyDataType::STRING)
        ("longblob", PropertyDataType::STRING)
        ("varchar", PropertyDataType::STRING);

DBTableColumn::DBTableColumn(const std::string &class_id, const std::string &instance_id, DBTable *table)
 : Configurable (class_id, instance_id, table), table_(*table), widget_(nullptr)
{
  registerParameter ("name", &name_, "");
  registerParameter ("type", &type_, "");
  registerParameter ("is_key", &is_key_, false);
  registerParameter ("comment", &comment_, "");
  registerParameter ("dimension", &dimension_, "");
  registerParameter ("unit", &unit_, "");
  registerParameter ("special_null", &special_null_, "");

  createSubConfigurables();
}

DBTableColumn::~DBTableColumn()
{
    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }

}

PropertyDataType DBTableColumn::propertyType () const
{
//        BOOL, CHAR, UCHAR, INT, UINT, LONGINT, ULONGINT, FLOAT, DOUBLE, STRING
    if (db_types_2_data_types_.count(type_) == 0)
        return Property::asDataType(type_);
    else
        return db_types_2_data_types_.at(type_);
}


UnitSelectionWidget *DBTableColumn::unitWidget ()
{
    if (!widget_)
    {
        widget_ = new UnitSelectionWidget (dimension_, unit_);
        assert (widget_);
    }

    return widget_;
}
