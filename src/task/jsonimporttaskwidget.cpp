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

#include "jsonimporttaskwidget.h"
#include "jsonimporttask.h"
#include "dbobjectcombobox.h"
#include "dbovariable.h"
#include "dbovariableselectionwidget.h"
#include "logger.h"
#include "atsdb.h"
#include "stringconv.h"
#include "dbobjectmanager.h"
#include "dbobject.h"
#include "selectdbobjectdialog.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QFrame>
#include <QInputDialog>
#include <QStackedWidget>

using namespace Utils;

JSONImportTaskWidget::JSONImportTaskWidget(JSONImportTask& task, QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f), task_(task)
{
    setContentsMargins(0, 0, 0, 0);

    main_layout_ = new QHBoxLayout ();

    tab_widget_ = new QTabWidget ();
    main_layout_->addWidget(tab_widget_);

    addMainTab();
    addMappingsTab();

    setLayout (main_layout_);
}

void JSONImportTaskWidget::addMainTab ()
{
    assert (tab_widget_);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_tab_layout = new QVBoxLayout ();

    // file stuff
    {
        QLabel *files_label = new QLabel ("File Selection");
        files_label->setFont(font_bold);
        main_tab_layout->addWidget(files_label);

        file_list_ = new QListWidget ();
        file_list_->setWordWrap(true);
        file_list_->setTextElideMode (Qt::ElideNone);
        file_list_->setSelectionBehavior( QAbstractItemView::SelectItems );
        file_list_->setSelectionMode( QAbstractItemView::SingleSelection );
        connect (file_list_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectedFileSlot()));

        updateFileListSlot ();
        main_tab_layout->addWidget(file_list_);
    }

    // file button stuff
    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        add_file_button_ = new QPushButton ("Add");
        connect (add_file_button_, SIGNAL(clicked()), this, SLOT(addFileSlot()));
        button_layout->addWidget(add_file_button_);

        delete_file_button_ = new QPushButton ("Remove");
        connect (delete_file_button_, SIGNAL(clicked()), this, SLOT(deleteFileSlot()));
        button_layout->addWidget(delete_file_button_);

        main_tab_layout->addLayout(button_layout);
    }

    main_tab_layout->addStretch();

    // button stuff
    {
        test_button_ = new QPushButton ("Test Import");
        connect(test_button_, &QPushButton::clicked, this, &JSONImportTaskWidget::testImportSlot);
        main_tab_layout->addWidget(test_button_);
    }

    QWidget* main_tab_widget = new QWidget();
    main_tab_widget->setContentsMargins(0, 0, 0, 0);
    main_tab_widget->setLayout(main_tab_layout);
    tab_widget_->addTab(main_tab_widget, "Main");
}

void JSONImportTaskWidget::addMappingsTab ()
{
    QVBoxLayout* mappings_layout = new QVBoxLayout();

    QFont font_bold;
    font_bold.setBold(true);

    // schema selection
    {
        QHBoxLayout* schema_layout = new QHBoxLayout();

        QLabel *schemas_label = new QLabel ("Schema");
        schemas_label->setFont(font_bold);
        schema_layout->addWidget(schemas_label);

        schema_box_ = new QComboBox ();
        connect (schema_box_, SIGNAL(activated(const QString&)), this, SLOT(selectedSchemaChangedSlot(const QString&)));
        updateSchemasBox();
        schema_layout->addWidget(schema_box_);

        add_schema_button_ = new QPushButton ("Add");
        connect (add_schema_button_, SIGNAL(clicked()), this, SLOT(addSchemaSlot()));
        schema_layout->addWidget(add_schema_button_);

        delete_schema_button_ = new QPushButton ("Remove");
        connect (delete_schema_button_, SIGNAL(clicked()), this, SLOT(removeSchemaSlot()));
        schema_layout->addWidget(delete_schema_button_);

        mappings_layout->addLayout(schema_layout);

        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        mappings_layout->addWidget(line);
    }

    // json object parser stuff
    {
        QVBoxLayout* parsers_layout = new QVBoxLayout();

        QLabel *parser_label = new QLabel ("JSON Object Parsers");
        parser_label->setFont(font_bold);
        parsers_layout->addWidget(parser_label);

        object_parser_box_ = new QComboBox ();
        connect (object_parser_box_, SIGNAL(currentIndexChanged(const QString&)),
                 this, SLOT(selectedObjectParserSlot(const QString&)));

        parsers_layout->addWidget(object_parser_box_);

        QHBoxLayout* button_layout = new QHBoxLayout();

        add_object_parser_button_ = new QPushButton ("Add");
        connect (add_object_parser_button_, SIGNAL(clicked()), this, SLOT(addObjectParserSlot()));
        button_layout->addWidget(add_object_parser_button_);

        delete_object_parser_button_ = new QPushButton ("Remove");
        connect (delete_object_parser_button_, SIGNAL(clicked()), this, SLOT(removeObjectParserSlot()));
        button_layout->addWidget(delete_object_parser_button_);

        parsers_layout->addLayout(button_layout);

        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        parsers_layout->addWidget(line);

        object_parser_widget_ = new QStackedWidget ();
        parsers_layout->addWidget(object_parser_widget_);

        updateParserBox();

        mappings_layout->addLayout(parsers_layout);
    }

    QWidget* mappings_tab_widget = new QWidget();
    mappings_tab_widget->setContentsMargins(0, 0, 0, 0);
    mappings_tab_widget->setLayout(mappings_layout);
    tab_widget_->addTab(mappings_tab_widget, "Mappings");
}

JSONImportTaskWidget::~JSONImportTaskWidget()
{

}

void JSONImportTaskWidget::addFileSlot ()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Add JSON File"));

    if (filename.size() > 0)
    {
        if (!task_.hasFile(filename.toStdString()))
            task_.addFile(filename.toStdString());
    }
}

void JSONImportTaskWidget::deleteFileSlot ()
{
    loginf << "JSONImporterTaskWidget: deleteFileSlot";

    if (!file_list_->currentItem() || !task_.currentFilename().size())
    {
        QMessageBox m_warning (QMessageBox::Warning, "JSON File Deletion Failed",
                               "Please select a file in the list.",
                               QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    assert (task_.currentFilename().size());
    assert (task_.hasFile(task_.currentFilename()));
    task_.removeCurrentFilename();
}

void JSONImportTaskWidget::selectedFileSlot ()
{
    loginf << "JSONImporterTaskWidget: selectedFileSlot";
    assert (file_list_->currentItem());

    QString filename = file_list_->currentItem()->text();
    assert (task_.hasFile(filename.toStdString()));

    bool archive = (filename.endsWith(".zip") || filename.endsWith(".gz") || filename.endsWith(".tgz"));
    task_.currentFilename (filename.toStdString(), archive);
}

void JSONImportTaskWidget::updateFileListSlot ()
{
    assert (file_list_);

    file_list_->clear();

    for (auto it : task_.fileList())
    {
        QListWidgetItem *item = new QListWidgetItem(tr(it.first.c_str()), file_list_);
        if (it.first == task_.currentFilename())
            file_list_->setCurrentItem(item);
    }
}

void JSONImportTaskWidget::addSchemaSlot()
{
    loginf << "JSONImporterTaskWidget: addSchemaSlot";

    bool ok;
    QString text = QInputDialog::getText(this, tr("Input Name of New Schema"),
                                         tr("JSON Parsing Schema Name:"), QLineEdit::Normal,
                                         "New", &ok);

    if (ok)
    {
        std::string name = text.toStdString();
        loginf << "JSONImporterTaskWidget: addSchemaSlot: name '" << name << "'";

        if (!name.size())
        {
            QMessageBox m_warning (QMessageBox::Warning, "JSON Parsing Schema Adding Failed",
                                   "A schema name must be set.",
                                   QMessageBox::Ok);

            m_warning.exec();
            return;
        }

        if (task_.hasSchema(name))
        {
            QMessageBox m_warning (QMessageBox::Warning, "JSON Parsing Schema Adding Failed",
                                   "Schema with this name is already defined.",
                                   QMessageBox::Ok);

            m_warning.exec();
            return;
        }

        std::string instance = "JSONParsingSchema"+name+"0";

        Configuration &config = task_.addNewSubConfiguration ("JSONParsingSchema", instance);
        config.addParameterString ("name", name);

        task_.generateSubConfigurable("JSONParsingSchema", instance);
        updateSchemasBox();
    }
}

void JSONImportTaskWidget::removeSchemaSlot()
{
    loginf << "JSONImporterTaskWidget: removeSchemaSlot";

    if (!task_.currentSchemaName().size())
    {
        QMessageBox m_warning (QMessageBox::Warning, "JSON File Deletion Failed",
                               "Please select a file in the list.",
                               QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    task_.removeCurrentSchema();
    updateSchemasBox();
    updateParserBox();
    //selectedObjectParserSlot ();
}

void JSONImportTaskWidget::selectedSchemaChangedSlot(const QString& text)
{
    loginf << "JSONImporterTaskWidget: selectedSchemaChangedSlot: text " << text.toStdString();

    assert (task_.hasSchema(text.toStdString()));
    task_.currentSchemaName(text.toStdString());

    updateParserBox();
    //selectedObjectParserSlot ();
}

void JSONImportTaskWidget::updateSchemasBox()
{
    loginf << "JSONImporterTaskWidget: updateSchemasBox";

    schema_box_->clear();

    for (auto& schema_it : task_)
    {
        schema_box_->addItem(schema_it.first.c_str());
    }

    if (task_.currentSchemaName().size())
    {
        schema_box_->setCurrentText(task_.currentSchemaName().c_str());
    }
    else if (schema_box_->currentText().size())
    {
        task_.currentSchemaName(schema_box_->currentText().toStdString());
    }
}

void JSONImportTaskWidget::addObjectParserSlot ()
{
    if (!task_.hasCurrentSchema())
    {
        QMessageBox m_warning (QMessageBox::Warning, "JSON Object Parser Adding Failed",
                               "No current JSON Parsing Schema is selected.",
                               QMessageBox::Ok);

        m_warning.exec();
        return;
    }

    SelectDBObjectDialog dialog;

    int ret = dialog.exec();

    if (ret == QDialog::Accepted)
    {
        std::string name = dialog.name();
        std::string dbo_name = dialog.selectedObject();
        loginf << "JSONImporterTaskWidget: addObjectParserSlot: name " << name << " obj " << dbo_name;

        JSONParsingSchema& current = task_.currentSchema();

        if (!name.size() || current.hasObjectParser(name))
        {
            QMessageBox m_warning (QMessageBox::Warning, "JSON Object Parser Adding Failed",
                                   "Object parser name empty or already defined.",
                                   QMessageBox::Ok);

            m_warning.exec();
            return;
        }

        std::string instance = "JSONObjectParser"+name+dbo_name+"0";

        Configuration &config = current.addNewSubConfiguration ("JSONObjectParser", instance);
        config.addParameterString ("name", name);
        config.addParameterString ("db_object_name", dbo_name);

        current.generateSubConfigurable("JSONObjectParser", instance);
        updateParserBox();
    }
}
void JSONImportTaskWidget::removeObjectParserSlot ()
{
    loginf << "JSONImporterTaskWidget: removeObjectParserSlot";

    if (object_parser_box_->currentIndex() >= 0)
    {
        std::string name = object_parser_box_->currentText().toStdString();

        assert (task_.hasCurrentSchema());
        JSONParsingSchema& current = task_.currentSchema();

        assert (current.hasObjectParser(name));
        current.removeParser(name);

        updateParserBox();
        //selectedObjectParserSlot();
    }
}

void JSONImportTaskWidget::selectedObjectParserSlot (const QString& text)
{
    loginf << "JSONImporterTaskWidget: selectedObjectParserSlot: text " << text.toStdString();

    if (object_parser_widget_)
        while (object_parser_widget_->count() > 0)
            object_parser_widget_->removeWidget(object_parser_widget_->widget(0));

    assert (object_parser_box_);

    if (object_parser_box_->currentIndex() >= 0)
    {
        std::string name = object_parser_box_->currentText().toStdString();

        assert (task_.hasCurrentSchema());
        assert (task_.currentSchema().hasObjectParser(name));
        assert (object_parser_widget_);

        if (object_parser_widget_->indexOf(task_.currentSchema().parser(name).widget()) < 0)
            object_parser_widget_->addWidget(task_.currentSchema().parser(name).widget());

        object_parser_widget_->setCurrentWidget(task_.currentSchema().parser(name).widget());
    }
}

//void JSONImporterTaskWidget::createObjectParserWidget()
//{
//    assert (!object_parser_widget_);
//    assert (main_layout_);

//    int frame_width_small = 1;

//    QFrame *right_frame = new QFrame ();
//    right_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
//    right_frame->setLineWidth(frame_width_small);

//    object_parser_widget_ = new QStackedWidget ();

//    QVBoxLayout* tmp = new QVBoxLayout ();
//    tmp->addWidget(object_parser_widget_);

//    right_frame->setLayout(tmp);

//    main_layout_->addWidget(right_frame);
//}

//void JSONImporterTaskWidget::update ()
//{
//    loginf << "JSONImporterTaskWidget: update";
//}

void JSONImportTaskWidget::testImportSlot ()
{
    loginf << "JSONImporterTaskWidget: testImportSlot";

    if (!task_.canImportFile())
    {
        QMessageBox m_warning (QMessageBox::Warning, "JSON File Test Import Failed",
                               "Please select a file in the list.",
                               QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    task_.test(true);
    task_.run();
}

//void JSONImporterTaskWidget::importSlot ()
//{
//    loginf << "JSONImporterTaskWidget: importSlot";

//    if (!file_list_->currentItem())
//    {
//        QMessageBox m_warning (QMessageBox::Warning, "JSON File Import Failed",
//                               "Please select a file in the list.",
//                               QMessageBox::Ok);
//        m_warning.exec();
//        return;
//    }

//    for (auto& object_it : ATSDB::instance().objectManager())
//    {
//        if (object_it.second->hasData())
//        {
//            QMessageBox::StandardButton reply;
//            reply = QMessageBox::question(this, "Really Import?", "The database already contains data. "
//                                                                  "A data import might not be successful. Continue?",
//                                          QMessageBox::Yes|QMessageBox::No);
//            if (reply == QMessageBox::Yes)
//            {
//                loginf << "JSONImporterTaskWidget: importSlot: importing although data in database";
//                break;
//            }
//            else
//            {
//                loginf << "JSONImporterTaskWidget: importSlot: quit importing since data in database";
//                return;
//            }
//        }
//    }

//    QString filename = file_list_->currentItem()->text();
//    if (filename.size() > 0)
//    {
//        assert (task_.hasFile(filename.toStdString()));

//        if (!task_.canImportFile(filename.toStdString()))
//        {
//            QMessageBox m_warning (QMessageBox::Warning, "JSON File Import Failed",
//                                   "File does not exist.",
//                                   QMessageBox::Ok);
//            m_warning.exec();
//            return;
//        }

//        if (filename.endsWith(".zip") || filename.endsWith(".gz") || filename.endsWith(".tgz"))
//            task_.importFileArchive(filename.toStdString(), false);
//        else
//            task_.importFile(filename.toStdString(), false);

//        test_button_->setDisabled(true);
//        import_button_->setDisabled(true);
//    }
//}

void JSONImportTaskWidget::runStarted ()
{
    loginf << "JSONImporterTaskWidget: runStarted";

    test_button_->setDisabled(true);
}

void JSONImportTaskWidget::runDone ()
{
    loginf << "JSONImporterTaskWidget: runDone";

    test_button_->setDisabled(false);
}

void JSONImportTaskWidget::updateParserBox ()
{
    loginf << "JSONImporterTaskWidget: updateParserBox";

    assert (object_parser_box_);
    object_parser_box_->clear();

    if (task_.hasCurrentSchema())
    {
        for (auto& parser_it : task_.currentSchema())
        {
            object_parser_box_->addItem(parser_it.first.c_str());
//            QListWidgetItem* item = new QListWidgetItem(tr(parser_it.first.c_str()), object_parser_box_);
//            assert (item);
        }
    }
}
