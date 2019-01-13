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

#ifndef DBOEDITDATASOURCEACTIONOPTIONSWIDGET_H
#define DBOEDITDATASOURCEACTIONOPTIONSWIDGET_H

#include "dboeditdatasourceactionoptions.h"


#include <QWidget>


class DBOEditDataSourceActionOptions;
class QCheckBox;
class QComboBox;

class DBOEditDataSourceActionOptionsWidget : public QWidget
{
    Q_OBJECT

public slots:
    void changedPerformSlot ();
    void changedActionSlot (int index);

public:
    DBOEditDataSourceActionOptionsWidget(DBOEditDataSourceActionOptions& options,
                                         QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~DBOEditDataSourceActionOptionsWidget() {}

    void update ();

private:
    DBOEditDataSourceActionOptions* options_;

    QCheckBox* perform_check_ {nullptr};
    QComboBox* action_box_ {nullptr};
};

#endif // DBOEDITDATASOURCEACTIONOPTIONSWIDGET_H
