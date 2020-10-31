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

#ifndef DBSCHEMASELECTIONCOMBOBOX_H
#define DBSCHEMASELECTIONCOMBOBOX_H

#include <QComboBox>

#include "compass.h"
#include "dbschema.h"
#include "dbschemamanager.h"

class DBSchemaSelectionComboBox : public QComboBox
{
  public:
    DBSchemaSelectionComboBox() { update(); }

    DBSchemaSelectionComboBox(const std::string& selection)
    {
        update();
        select(selection);
    }

    void update()
    {
        logdbg << "DBSchemaSelectionComboBox: update";

        std::string selection;

        if (count() > 0)
            selection = currentText().toStdString();

        while (count() > 0)
            removeItem(0);

        for (auto& schema_it : COMPASS::instance().schemaManager().getSchemas())
            addItem(schema_it.first.c_str());

        if (selection.size())
            select(selection);
    }

    void select(const std::string& selection)
    {
        int index = findText(selection.c_str());

        if (index >= 0)
            setCurrentIndex(index);
    }

  protected:
};

#endif  // DBSCHEMASELECTIONCOMBOBOX_H
