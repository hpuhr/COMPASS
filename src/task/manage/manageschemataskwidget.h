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

#ifndef MANAGESCHEMATASKWIDGET_H
#define MANAGESCHEMATASKWIDGET_H

#include <taskwidget.h>

class ManageSchemaTask;

class ManageSchemaTaskWidget : public TaskWidget
{
    Q_OBJECT

  public slots:
    void schemaLockedSlot();
    void expertModeChangedSlot();

  public:
    ManageSchemaTaskWidget(ManageSchemaTask& task, QWidget* parent = nullptr);

  protected:
    ManageSchemaTask& task_;
};

#endif  // MANAGESCHEMATASKWIDGET_H
