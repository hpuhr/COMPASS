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

#ifndef ASTERIXIMPORTERTASKWIDGET_H
#define ASTERIXIMPORTERTASKWIDGET_H

#include <QWidget>
#include <memory>

#include <jasterix/jasterix.h>

class ASTERIXImporterTask;
class ASTERIXConfigWidget;

class QHBoxLayout;
class QPushButton;
class QListWidget;
class QComboBox;
class QStackedWidget;
class QCheckBox;
class QTabWidget;

class ASTERIXImporterTaskWidget : public QWidget
{
    Q_OBJECT

public slots:
    void addFileSlot ();
    void deleteFileSlot ();
    void selectedFileSlot ();
    void updateFileListSlot ();

    void addObjectParserSlot ();
    void removeObjectParserSlot ();
    void selectedObjectParserSlot (const QString& text);

    void debugChangedSlot ();
    void limitRAMChangedSlot ();
    void createMappingsSlot();
    void testImportSlot();

public:
    ASTERIXImporterTaskWidget(ASTERIXImporterTask& task, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~ASTERIXImporterTaskWidget();

    void runStarted ();
    void runDone ();

protected:
    ASTERIXImporterTask& task_;

    QHBoxLayout *main_layout_ {nullptr};

    QTabWidget* tab_widget_ {nullptr};

    QListWidget* file_list_ {nullptr};
    QPushButton* add_file_button_ {nullptr};
    QPushButton* delete_file_button_ {nullptr};

    QComboBox* object_parser_box_ {nullptr};
    QPushButton* add_object_parser_button_ {nullptr};
    QPushButton* delete_object_parser_button_ {nullptr};

    QStackedWidget* object_parser_widget_ {nullptr};

    ASTERIXConfigWidget* config_widget_ {nullptr};

    QCheckBox* debug_check_ {nullptr};
    QCheckBox* limit_ram_check_ {nullptr};
    QPushButton* create_mapping_stubs_button_ {nullptr};
    QPushButton* test_button_ {nullptr};

    void addMainTab ();
    void addASTERIXConfigTab();
    void addMappingsTab();

    void updateParserBox ();
};

#endif // ASTERIXIMPORTERTASKWIDGET_H
