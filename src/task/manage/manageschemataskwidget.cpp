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

#include "manageschemataskwidget.h"
#include "manageschematask.h"
#include "atsdb.h"
#include "dbschemamanager.h"
#include "dbschemamanagerwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>

ManageSchemaTaskWidget::ManageSchemaTaskWidget(ManageSchemaTask& task, QWidget* parent)
    : TaskWidget(parent), task_(task)
{
    QVBoxLayout* main_layout = new QVBoxLayout ();

    DBSchemaManagerWidget* dbschema_manager_widget = ATSDB::instance().schemaManager().widget();
    connect(dbschema_manager_widget, &DBSchemaManagerWidget::schemaLockedSignal,
            this, &ManageSchemaTaskWidget::schemaLockedSlot);
    main_layout->addWidget(dbschema_manager_widget);

    expertModeChangedSlot();

    setLayout (main_layout);
}


void ManageSchemaTaskWidget::schemaLockedSlot ()
{
    loginf << "ManageSchemaTaskWidget: schemaLockedSlot";
    emit task_.statusChangedSignal(task_.name());
}

void ManageSchemaTaskWidget::expertModeChangedSlot ()
{

}
