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
#ifndef MANAGEMENTWIDGET_H_
#define MANAGEMENTWIDGET_H_

#include <QWidget>

// class FilterConfigWidget;
// class DBInfoWidget;
// class ResultSetWidget;
// class WorkerThreadWidget;
// class ViewsWidget;

class ManagementWidget : public QWidget
{
    Q_OBJECT

  public slots:

  public:
    ManagementWidget();
    virtual ~ManagementWidget();

  protected:
    //  DBInfoWidget *db_info_;
    //  ResultSetWidget *result_;
    //  FilterConfigWidget *filter_config_gui_;
    //  WorkerThreadWidget *worker_widget_;
    //  ViewsWidget *views_;

    virtual void resizeEvent(QResizeEvent* event);
};

#endif /* MANAGMENTWIDGET_H_ */
