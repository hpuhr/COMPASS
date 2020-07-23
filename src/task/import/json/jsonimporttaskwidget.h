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

#ifndef JSONIMPORTERTASKWIDGET_H
#define JSONIMPORTERTASKWIDGET_H

#include <taskwidget.h>

class JSONImportTask;
class RadarPlotPositionCalculatorTask;
class DBObjectComboBox;
class DBOVariableSelectionWidget;

class QPushButton;
class QListWidget;
class QCheckBox;
class QLineEdit;
class QComboBox;
class QHBoxLayout;
class QStackedWidget;
class QTabWidget;
class QLabel;

class JSONImportTaskWidget : public TaskWidget
{
    Q_OBJECT

  public slots:
    void testImportSlot();

    void addFileSlot();
    void deleteFileSlot();
    void deleteAllFilesSlot();
    void selectedFileSlot();
    void updateFileListSlot();

    void addSchemaSlot();
    void removeSchemaSlot();
    void selectedSchemaChangedSlot(const QString& text);

    void addObjectParserSlot();
    void removeObjectParserSlot();
    void selectedObjectParserSlot(const QString& text);

    void expertModeChangedSlot();

  public:
    JSONImportTaskWidget(JSONImportTask& task, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~JSONImportTaskWidget();

    void addFile(const std::string& filename);
    void selectFile(const std::string& filename);
    void selectSchema(const std::string& schema_name);

    void runStarted();
    void runDone();

  protected:
    JSONImportTask& task_;

    QHBoxLayout* main_layout_{nullptr};

    QTabWidget* tab_widget_{nullptr};

    QListWidget* file_list_{nullptr};
    QPushButton* add_file_button_{nullptr};
    QPushButton* delete_file_button_{nullptr};
    QPushButton* delete_all_files_button_{nullptr};

    QComboBox* schema_box_{nullptr};
    QPushButton* add_schema_button_{nullptr};
    QPushButton* delete_schema_button_{nullptr};

    QLabel* parser_label_{nullptr};
    QComboBox* object_parser_box_{nullptr};
    QPushButton* add_object_parser_button_{nullptr};
    QPushButton* delete_object_parser_button_{nullptr};

    QStackedWidget* object_parser_widget_{nullptr};

    QPushButton* test_button_{nullptr};

    void addMainTab();
    void addMappingsTab();

    void updateSchemasBox();
    void updateToCurrentSchema();
    void updateParserBox();
};

#endif  // JSONIMPORTERTASKWIDGET_H
