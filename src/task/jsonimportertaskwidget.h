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

#include <QWidget>

class JSONImporterTask;
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

class JSONImporterTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void testImportSlot ();
    //void importSlot ();
    //void importDoneSlot ();

    void addFileSlot ();
    void deleteFileSlot ();
    void selectedFileSlot ();
    void updateFileListSlot ();

    void addSchemaSlot();
    void removeSchemaSlot();
    void selectedSchemaChangedSlot(const QString& text);

    void addObjectParserSlot ();
    void removeObjectParserSlot ();
    void selectedObjectParserSlot ();

public:
    JSONImporterTaskWidget(JSONImporterTask& task, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~JSONImporterTaskWidget();

    //void update ();
    void runStarted ();
    void runDone ();

protected:
    JSONImporterTask& task_;

    QHBoxLayout* main_layout_ {nullptr};

    QListWidget* file_list_ {nullptr};
    QPushButton* add_file_button_ {nullptr};
    QPushButton* delete_file_button_ {nullptr};

    QComboBox* schema_box_ {nullptr};
    QPushButton* add_schema_button_ {nullptr};
    QPushButton* delete_schema_button_ {nullptr};

    QListWidget* object_parser_list_ {nullptr};
    QPushButton* add_object_parser_button_ {nullptr};
    QPushButton* delete_object_parser_button_ {nullptr};

    QStackedWidget* object_parser_widget_ {nullptr};
    //QHBoxLayout* object_parser_layout_ {nullptr};

    QPushButton* test_button_ {nullptr};
    //QPushButton* import_button_ {nullptr};

    void updateSchemasBox();
    void updateParserList ();
    void createObjectParserWidget();
};

#endif // JSONIMPORTERTASKWIDGET_H
