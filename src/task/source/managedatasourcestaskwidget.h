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

#ifndef MANAGEDATASOURCESTASKWIDGET_H
#define MANAGEDATASOURCESTASKWIDGET_H

#include <taskwidget.h>

class ManageDataSourcesTask;

class QTabWidget;
class DBOEditDataSourcesWidget;

class ManageDataSourcesTaskWidget : public TaskWidget
{
    Q_OBJECT

  public slots:
    void expertModeChangedSlot();

    void exportConfigDataSourcesSlot();
    void clearConfigDataSourcesSlot();
    void importConfigDataSourcesSlot();
    void autoSyncAllConfigDataSourcesToDB();

    void dbItemChangedSlot();

  public:
    ManageDataSourcesTaskWidget(ManageDataSourcesTask& task, QWidget* parent = nullptr);

    void setCurrentWidget(DBOEditDataSourcesWidget* widget);

  protected:
    ManageDataSourcesTask& task_;

    QTabWidget* tab_widget_{nullptr};
};

#endif  // MANAGEDATASOURCESTASKWIDGET_H
