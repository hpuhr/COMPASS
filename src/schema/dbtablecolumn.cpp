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

#include "dbtable.h"
#include "dbtablecolumn.h"
#include "logger.h"
#include "unitselectionwidget.h"

DBTableColumn::DBTableColumn(const std::string &class_id, const std::string &instance_id, DBTable *table)
 : Configurable (class_id, instance_id, table), table_(*table), widget_(nullptr)
{
  registerParameter ("name", &name_, "");
  registerParameter ("type", &type_, "");
  registerParameter ("is_key", &is_key_, false);
  registerParameter ("comment", &comment_, "");
  registerParameter ("quanitiy", &quanitiy_, "");
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


UnitSelectionWidget *DBTableColumn::unitWidget ()
{
    if (!widget_)
    {
        widget_ = new UnitSelectionWidget (quanitiy_, unit_);
        assert (widget_);
    }

    return widget_;
}
