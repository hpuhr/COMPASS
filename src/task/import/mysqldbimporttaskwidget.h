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

#ifndef MYSQLDBIMPORTTASKWIDGET_H
#define MYSQLDBIMPORTTASKWIDGET_H

#include <taskwidget.h>

class MySQLDBImportTask;

class QPushButton;
class QListWidget;

class MySQLDBImportTaskWidget : public TaskWidget
{
    Q_OBJECT

public slots:
    void addFileSlot ();
    void deleteFileSlot ();
    void selectedFileSlot ();
    void updateFileListSlot ();

    void expertModeChangedSlot ();

public:
    MySQLDBImportTaskWidget(MySQLDBImportTask& task, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~MySQLDBImportTaskWidget();

protected:
    MySQLDBImportTask& task_;

    QListWidget* file_list_ {nullptr};
    QPushButton* add_file_button_ {nullptr};
    QPushButton* delete_file_button_ {nullptr};
};

#endif // MYSQLDBIMPORTTASKWIDGET_H
