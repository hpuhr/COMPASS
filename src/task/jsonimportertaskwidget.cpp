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

#include "jsonimportertaskwidget.h"
#include "jsonimportertask.h"
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

JSONImporterTaskWidget::JSONImporterTaskWidget(JSONImporterTask& task, QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f), task_(task)
{
    setWindowTitle ("Import JSON Data");
    setMinimumSize(QSize(800, 600));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    int frame_width_small = 1;

    main_layout_ = new QHBoxLayout ();

    QVBoxLayout* left_layout = new QVBoxLayout ();

//    QLabel *main_label = new QLabel ("Import JSON data");
//    main_label->setFont (font_big);
//    left_layout->addWidget (main_label);

    // file stuff
    {
        QFrame *file_frame = new QFrame ();
        file_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        file_frame->setLineWidth(frame_width_small);

        QVBoxLayout* files_layout = new QVBoxLayout();

        QLabel *files_label = new QLabel ("File Selection");
        files_label->setFont(font_bold);
        files_layout->addWidget(files_label);

        file_list_ = new QListWidget ();
        file_list_->setWordWrap(true);
        file_list_->setTextElideMode (Qt::ElideNone);
        file_list_->setSelectionBehavior( QAbstractItemView::SelectItems );
        file_list_->setSelectionMode( QAbstractItemView::SingleSelection );
        connect (file_list_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectedFileSlot()));

        updateFileListSlot ();
        files_layout->addWidget(file_list_);

        QHBoxLayout* button_layout = new QHBoxLayout();

        add_file_button_ = new QPushButton ("Add");
        connect (add_file_button_, SIGNAL(clicked()), this, SLOT(addFileSlot()));
        button_layout->addWidget(add_file_button_);

        delete_file_button_ = new QPushButton ("Remove");
        connect (delete_file_button_, SIGNAL(clicked()), this, SLOT(deleteFileSlot()));
        button_layout->addWidget(delete_file_button_);

        files_layout->addLayout(button_layout);

        file_frame->setLayout (files_layout);

        left_layout->addWidget (file_frame, 1);
    }

    // schema selection
    {
        //

        QFrame *schemas_frame = new QFrame ();
        schemas_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        schemas_frame->setLineWidth(frame_width_small);

        QVBoxLayout* schemas_layout = new QVBoxLayout();

        QLabel *schemas_label = new QLabel ("JSON Parsing Schema");
        schemas_label->setFont(font_bold);
        schemas_layout->addWidget(schemas_label);

        schema_box_ = new QComboBox ();
        connect (schema_box_, SIGNAL(activated(const QString&)), this, SLOT(selectedSchemaChangedSlot(const QString&)));
        updateSchemasBox();
        schemas_layout->addWidget(schema_box_);

        QHBoxLayout* button_layout = new QHBoxLayout();

        add_schema_button_ = new QPushButton ("Add");
        connect (add_schema_button_, SIGNAL(clicked()), this, SLOT(addSchemaSlot()));
        button_layout->addWidget(add_schema_button_);

        delete_schema_button_ = new QPushButton ("Remove");
        connect (delete_schema_button_, SIGNAL(clicked()), this, SLOT(removeSchemaSlot()));
        button_layout->addWidget(delete_schema_button_);

        schemas_layout->addLayout(button_layout);

        schemas_frame->setLayout (schemas_layout);

        left_layout->addWidget (schemas_frame);
    }

    // json object parser stuff
    {
        QFrame *parsers_frame = new QFrame ();
        parsers_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
        parsers_frame->setLineWidth(frame_width_small);

        QVBoxLayout* parsers_layout = new QVBoxLayout();

        QLabel *parser_label = new QLabel ("JSON Object Parsers");
        parser_label->setFont(font_bold);
        parsers_layout->addWidget(parser_label);

        object_parser_list_ = new QListWidget ();
        //file_list_->setTextElideMode (Qt::ElideNone);
        object_parser_list_->setSelectionBehavior( QAbstractItemView::SelectItems );
        object_parser_list_->setSelectionMode( QAbstractItemView::SingleSelection );
        connect (object_parser_list_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectedObjectParserSlot()));

        updateParserList();
        parsers_layout->addWidget(object_parser_list_);

        QHBoxLayout* button_layout = new QHBoxLayout();

        add_object_parser_button_ = new QPushButton ("Add");
        connect (add_object_parser_button_, SIGNAL(clicked()), this, SLOT(addObjectParserSlot()));
        button_layout->addWidget(add_object_parser_button_);

        delete_object_parser_button_ = new QPushButton ("Remove");
        connect (delete_object_parser_button_, SIGNAL(clicked()), this, SLOT(removeObjectParserSlot()));
        button_layout->addWidget(delete_object_parser_button_);

        parsers_layout->addLayout(button_layout);

        parsers_frame->setLayout (parsers_layout);

        left_layout->addWidget (parsers_frame, 1);
    }

    //    QFormLayout *stuff_layout = new QFormLayout;
    //    stuff_layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);
    //    stuff_layout->addLayout(left_layout);

    //    main_layout->addLayout(stuff_layout);


    test_button_ = new QPushButton ("Test Import");
    connect(test_button_, &QPushButton::clicked, this, &JSONImporterTaskWidget::testImportSlot);
    left_layout->addWidget(test_button_);

    import_button_ = new QPushButton ("Import");
    connect(import_button_, &QPushButton::clicked, this, &JSONImporterTaskWidget::importSlot);
    left_layout->addWidget(import_button_);

    main_layout_->addLayout(left_layout);

    setLayout (main_layout_);

    update();

    show();
}

JSONImporterTaskWidget::~JSONImporterTaskWidget()
{

}

void JSONImporterTaskWidget::addFileSlot ()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Add JSON File"));

    if (filename.size() > 0)
    {
        if (!task_.hasFile(filename.toStdString()))
            task_.addFile(filename.toStdString());
    }
}

void JSONImporterTaskWidget::deleteFileSlot ()
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

void JSONImporterTaskWidget::selectedFileSlot ()
{
    loginf << "JSONImporterTaskWidget: selectedFileSlot";
    assert (file_list_->currentItem());

    QString filename = file_list_->currentItem()->text();
    assert (task_.hasFile(filename.toStdString()));
    task_.currentFilename (filename.toStdString());
}

void JSONImporterTaskWidget::updateFileListSlot ()
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

void JSONImporterTaskWidget::addSchemaSlot()
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

void JSONImporterTaskWidget::removeSchemaSlot()
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
    updateParserList();
    selectedObjectParserSlot ();
}

void JSONImporterTaskWidget::selectedSchemaChangedSlot(const QString& text)
{
    loginf << "JSONImporterTaskWidget: selectedSchemaChangedSlot: text " << text.toStdString();

    assert (task_.hasSchema(text.toStdString()));
    task_.currentSchemaName(text.toStdString());

    updateParserList();
    selectedObjectParserSlot ();
}

void JSONImporterTaskWidget::updateSchemasBox()
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

void JSONImporterTaskWidget::addObjectParserSlot ()
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
        updateParserList();
    }
}
void JSONImporterTaskWidget::removeObjectParserSlot ()
{
    loginf << "JSONImporterTaskWidget: removeObjectParserSlot";

    if (object_parser_list_->currentItem())
    {
        std::string name = object_parser_list_->currentItem()->text().toStdString();

        assert (task_.hasCurrentSchema());
        JSONParsingSchema& current = task_.currentSchema();

        assert (current.hasObjectParser(name));
        current.removeParser(name);

        updateParserList();
        selectedObjectParserSlot();
    }
}

void JSONImporterTaskWidget::selectedObjectParserSlot ()
{
    loginf << "JSONImporterTaskWidget: selectedObjectParserSlot";

    if (object_parser_widget_)
        while (object_parser_widget_->count() > 0)
            object_parser_widget_->removeWidget(object_parser_widget_->widget(0));

    assert (object_parser_list_);

    if (object_parser_list_->currentItem())
    {
        std::string name = object_parser_list_->currentItem()->text().toStdString();

        assert (task_.hasCurrentSchema());
        assert (task_.currentSchema().hasObjectParser(name));

        if (!object_parser_widget_)
            createObjectParserWidget();

        object_parser_widget_->addWidget(task_.currentSchema().parser(name).widget());
        //object_parser_widgets_->show();
    }
    //    else
    //        object_parser_widgets_->hide();
}

void JSONImporterTaskWidget::createObjectParserWidget()
{
    assert (!object_parser_widget_);
    assert (main_layout_);
    setMinimumSize(QSize(1200, 600));

    int frame_width_small = 1;

    QFrame *right_frame = new QFrame ();
    right_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    right_frame->setLineWidth(frame_width_small);

    object_parser_widget_ = new QStackedWidget ();

    object_parser_widget_->setMinimumWidth(800);

    QVBoxLayout* tmp = new QVBoxLayout ();
    tmp->addWidget(object_parser_widget_);

    right_frame->setLayout(tmp);

    main_layout_->addWidget(right_frame);
}

void JSONImporterTaskWidget::update ()
{
    loginf << "JSONImporterTaskWidget: update";
}

void JSONImporterTaskWidget::testImportSlot ()
{
    loginf << "JSONImporterTaskWidget: testImportSlot";

    if (!file_list_->currentItem())
    {
        QMessageBox m_warning (QMessageBox::Warning, "JSON File Test Import Failed",
                               "Please select a file in the list.",
                               QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    QString filename = file_list_->currentItem()->text();
    if (filename.size() > 0)
    {
        assert (task_.hasFile(filename.toStdString()));

        if (!task_.canImportFile(filename.toStdString()))
        {
            QMessageBox m_warning (QMessageBox::Warning, "JSON File Test Import Failed",
                                   "File does not exist.",
                                   QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        if (filename.endsWith(".zip") || filename.endsWith(".gz") || filename.endsWith(".tgz"))
            task_.importFileArchive(filename.toStdString(), true);
        else
            task_.importFile(filename.toStdString(), true);

        test_button_->setDisabled(true);
        import_button_->setDisabled(true);
    }
}

void JSONImporterTaskWidget::importSlot ()
{
    loginf << "JSONImporterTaskWidget: importSlot";

    if (!file_list_->currentItem())
    {
        QMessageBox m_warning (QMessageBox::Warning, "JSON File Import Failed",
                               "Please select a file in the list.",
                               QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    for (auto& object_it : ATSDB::instance().objectManager())
    {
        if (object_it.second->hasData())
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "Really Import?", "The database already contains data. "
                                                                  "A data import might not be successful. Continue?",
                                          QMessageBox::Yes|QMessageBox::No);
            if (reply == QMessageBox::Yes)
            {
                loginf << "JSONImporterTaskWidget: importSlot: importing although data in database";
                break;
            }
            else
            {
                loginf << "JSONImporterTaskWidget: importSlot: quit importing since data in database";
                return;
            }
        }
    }

    QString filename = file_list_->currentItem()->text();
    if (filename.size() > 0)
    {
        assert (task_.hasFile(filename.toStdString()));

        if (filename.endsWith(".zip") || filename.endsWith(".gz") || filename.endsWith(".tgz"))
            task_.importFileArchive(filename.toStdString(), false);
        else
            task_.importFile(filename.toStdString(), false);

        test_button_->setDisabled(true);
        import_button_->setDisabled(true);
    }
}

void JSONImporterTaskWidget::importDoneSlot (bool test)
{
    loginf << "JSONImporterTaskWidget: importDoneSlot: test " << test;

    test_button_->setDisabled(false);
    import_button_->setDisabled(false);
}

void JSONImporterTaskWidget::updateParserList ()
{
    loginf << "JSONImporterTaskWidget: updateParserList";

    assert (object_parser_list_);
    object_parser_list_->clear();

    if (task_.hasCurrentSchema())
    {
        for (auto& parser_it : task_.currentSchema())
        {
            QListWidgetItem* item = new QListWidgetItem(tr(parser_it.first.c_str()), object_parser_list_);
            assert (item);
        }
    }
}
