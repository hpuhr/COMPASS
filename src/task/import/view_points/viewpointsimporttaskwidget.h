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

#ifndef VIEWPOINTSIMPORTTASKWIDGET_H
#define VIEWPOINTSIMPORTTASKWIDGET_H

#include <taskwidget.h>

class ViewPointsImportTask;

class QPushButton;
class QListWidget;
class QTextEdit;

class ViewPointsImportTaskWidget : public TaskWidget
{
    Q_OBJECT

public slots:
    void expertModeChangedSlot();

    void addFileSlot();
    void deleteFileSlot();
    void deleteAllFilesSlot();
    void selectedFileSlot();
    void updateFileListSlot();

    void importSlot();

signals:

public:
    ViewPointsImportTaskWidget(ViewPointsImportTask& task, QWidget* parent = nullptr);
    virtual ~ViewPointsImportTaskWidget();

    void addFile(const std::string& filename);
    void selectFile(const std::string& filename);

    void updateContext ();

protected:
    ViewPointsImportTask& task_;

    QListWidget* file_list_{nullptr};
    QPushButton* add_file_button_{nullptr};
    QPushButton* delete_file_button_{nullptr};
    QPushButton* delete_all_files_button_{nullptr};

    QTextEdit* context_edit_ {nullptr};

    QPushButton* import_button_ {nullptr};
};

#endif // VIEWPOINTSIMPORTTASKWIDGET_H
