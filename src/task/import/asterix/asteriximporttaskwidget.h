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

#ifndef ASTERIXIMPORTTASKWIDGET_H
#define ASTERIXIMPORTTASKWIDGET_H

#include <jasterix/jasterix.h>

#include <QWidget>

#include <memory>

class ASTERIXImportTask;
class ASTERIXConfigWidget;
class ASTERIXOverrideWidget;

class QHBoxLayout;
class QPushButton;
class QListWidget;
class QComboBox;
class QStackedWidget;
class QCheckBox;
class QTabWidget;
class QLabel;
class QGridLayout;

class ASTERIXImportTaskWidget : public QWidget
{
    Q_OBJECT

  public slots:
    void addParserSlot();
    void removeObjectParserSlot();
    void selectedObjectParserSlot(const QString& text);

    void fileLineIDEditSlot(const QString& text);
    void dateChangedSlot(QDate date);

    void debugChangedSlot();
    void testImportSlot();

  public:
    ASTERIXImportTaskWidget(ASTERIXImportTask& task, QWidget* parent=0, Qt::WindowFlags f=0);
    virtual ~ASTERIXImportTaskWidget();

//    void updateSourceLabel();
    void updateSourcesGrid();

    ASTERIXOverrideWidget* overrideWidget() const;

protected:
    ASTERIXImportTask& task_;

    QHBoxLayout* main_layout_{nullptr};

    QTabWidget* tab_widget_{nullptr};

    //QLabel* source_label_{nullptr};

    QGridLayout* sources_grid_{nullptr}; // network or files

    QComboBox* object_parser_box_{nullptr};
    QPushButton* add_object_parser_button_{nullptr};
    QPushButton* delete_object_parser_button_{nullptr};

    QStackedWidget* object_parser_widget_{nullptr};

    ASTERIXConfigWidget* config_widget_{nullptr};
    ASTERIXOverrideWidget* override_widget_{nullptr};

    QCheckBox* debug_check_{nullptr};
    QCheckBox* limit_ram_check_{nullptr};

    void addMainTab();
    void addDecoderTab();
    void addOverrideTab();
    void addMappingsTab();

    void updateParserBox();
};

#endif  // ASTERIXIMPORTTASKWIDGET_H
