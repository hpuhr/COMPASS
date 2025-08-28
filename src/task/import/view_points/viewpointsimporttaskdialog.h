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

#pragma once

#include <QDialog>

class ViewPointsImportTask;
class ViewPointsImportTaskWidget;

class QPushButton;

class ViewPointsImportTaskDialog : public QDialog
{
    Q_OBJECT

signals:
    void importSignal();
    void cancelSignal();

public slots:
    void importClickedSlot();
    void cancelClickedSlot();

public:
    ViewPointsImportTaskDialog(ViewPointsImportTask& task);

    void updateText ();
    void updateButtons();

protected:
    ViewPointsImportTask& task_;

    ViewPointsImportTaskWidget* task_widget_ {nullptr}; // owned here

    QPushButton* cancel_button_{nullptr};
    QPushButton* import_button_{nullptr};
};
