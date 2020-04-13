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

#ifndef DATABASEOPENTASKWIDGET_H
#define DATABASEOPENTASKWIDGET_H

#include <taskwidget.h>

#include <QComboBox>

class DBInterface;
class QStackedWidget;

class DatabaseOpenTask;

class DatabaseOpenTaskWidget : public TaskWidget
{
    Q_OBJECT

  public slots:
    void databaseTypeSelectSlot();
    void databaseOpenedSlot();

    void expertModeChangedSlot();

  signals:
    void databaseOpenedSignal();

  public:
    DatabaseOpenTaskWidget(DatabaseOpenTask& task, DBInterface& db_interface,
                           QWidget* parent = nullptr);
    virtual ~DatabaseOpenTaskWidget();

    void updateUsedConnection();

  protected:
    DatabaseOpenTask& task_;

    DBInterface& db_interface_;

    QStackedWidget* connection_stack_{nullptr};
};

#endif  // DATABASEOPENTASKWIDGET_H
