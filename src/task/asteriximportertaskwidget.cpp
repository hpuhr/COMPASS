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

#include "asteriximportertaskwidget.h"
#include "asteriximportertask.h"
#include "asterixconfigwidget.h"
#include "logger.h"
#include "selectdbobjectdialog.h"
#include "system.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidget>
#include <QFrame>
#include <QInputDialog>
#include <QStackedWidget>
#include <QCheckBox>
#include <QComboBox>

using namespace Utils;

const float ram_threshold = 4.0;

ASTERIXImporterTaskWidget::ASTERIXImporterTaskWidget(ASTERIXImporterTask& task, QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f), task_(task)
{
    setContentsMargins(0, 0, 0, 0);

    main_layout_ = new QHBoxLayout ();

    tab_widget_ = new QTabWidget ();

    main_layout_->addWidget(tab_widget_);

    addMainTab();
    addASTERIXConfigTab();
    addMappingsTab();

    setLayout (main_layout_);
}

void ASTERIXImporterTaskWidget::addMainTab ()
{
    assert (tab_widget_);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_tab_layout = new QVBoxLayout ();

    // file stuff
    {
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

        main_tab_layout->addLayout(files_layout);
    }

    // final stuff
    {
        debug_check_ = new QCheckBox ("Debug in Console");
        debug_check_->setChecked(task_.debug());
        connect(debug_check_, &QCheckBox::clicked, this, &ASTERIXImporterTaskWidget::debugChangedSlot);
        main_tab_layout->addWidget (debug_check_);

        limit_ram_check_ = new QCheckBox ("Limit RAM Usage");
        limit_ram_check_->setChecked(task_.limitRAM());
        connect(limit_ram_check_, &QCheckBox::clicked, this, &ASTERIXImporterTaskWidget::limitRAMChangedSlot);
        main_tab_layout->addWidget (limit_ram_check_);

        create_mapping_stubs_button_ = new QPushButton ("Create Mapping Stubs");
        connect(create_mapping_stubs_button_, &QPushButton::clicked,
                this, &ASTERIXImporterTaskWidget::createMappingsSlot);
        main_tab_layout->addWidget (create_mapping_stubs_button_);

        test_button_ = new QPushButton ("Test Import");
        connect(test_button_, &QPushButton::clicked, this, &ASTERIXImporterTaskWidget::testImportSlot);
        main_tab_layout->addWidget (test_button_);
    }

    QWidget* main_tab_widget = new QWidget();
    main_tab_widget->setContentsMargins(0, 0, 0, 0);
    main_tab_widget->setLayout(main_tab_layout);
    tab_widget_->addTab(main_tab_widget, "Main");
}
void ASTERIXImporterTaskWidget::addASTERIXConfigTab()
{
    assert (tab_widget_);

    config_widget_ = new ASTERIXConfigWidget (task_, this);
    tab_widget_->addTab(config_widget_, "Decoder");
}

void ASTERIXImporterTaskWidget::addMappingsTab()
{
    QVBoxLayout* parsers_layout = new QVBoxLayout();

    QHBoxLayout* parser_manage_layout = new QHBoxLayout();

    object_parser_box_ = new QComboBox ();
    connect (object_parser_box_, SIGNAL(currentIndexChanged(const QString&)),
             this, SLOT(selectedObjectParserSlot(const QString&)));

    parser_manage_layout->addWidget(object_parser_box_);

    add_object_parser_button_ = new QPushButton ("Add");
    connect (add_object_parser_button_, SIGNAL(clicked()), this, SLOT(addObjectParserSlot()));
    parser_manage_layout->addWidget(add_object_parser_button_);

    delete_object_parser_button_ = new QPushButton ("Remove");
    connect (delete_object_parser_button_, SIGNAL(clicked()), this, SLOT(removeObjectParserSlot()));
    parser_manage_layout->addWidget(delete_object_parser_button_);

    parsers_layout->addLayout(parser_manage_layout);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    parsers_layout->addWidget(line);

    object_parser_widget_ = new QStackedWidget ();
    parsers_layout->addWidget(object_parser_widget_);

    updateParserBox();

    QWidget* mappings_tab_widget = new QWidget();
    mappings_tab_widget->setContentsMargins(0, 0, 0, 0);
    mappings_tab_widget->setLayout(parsers_layout);
    tab_widget_->addTab(mappings_tab_widget, "Mappings");
}

ASTERIXImporterTaskWidget::~ASTERIXImporterTaskWidget()
{
    config_widget_ = nullptr;
}

void ASTERIXImporterTaskWidget::addFileSlot ()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Add ASTERIX File"));

    if (filename.size() > 0)
    {
        if (!task_.hasFile(filename.toStdString()))
            task_.addFile(filename.toStdString());
    }
}

void ASTERIXImporterTaskWidget::deleteFileSlot ()
{
    loginf << "ASTERIXImporterTaskWidget: deleteFileSlot";

    if (!file_list_->currentItem() || !task_.currentFilename().size())
    {
        QMessageBox m_warning (QMessageBox::Warning, "ASTERIX File Deletion Failed",
                               "Please select a file in the list.",
                               QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    assert (task_.currentFilename().size());
    assert (task_.hasFile(task_.currentFilename()));
    task_.removeCurrentFilename();
}

void ASTERIXImporterTaskWidget::selectedFileSlot ()
{
    loginf << "ASTERIXImporterTaskWidget: selectedFileSlot";
    assert (file_list_->currentItem());

    QString filename = file_list_->currentItem()->text();
    assert (task_.hasFile(filename.toStdString()));
    task_.currentFilename (filename.toStdString());
}

void ASTERIXImporterTaskWidget::updateFileListSlot ()
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

void ASTERIXImporterTaskWidget::addObjectParserSlot ()
{
    if (task_.schema() == nullptr)
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
        loginf << "ASTERIXImporterTaskWidget: addObjectParserSlot: name " << name << " obj " << dbo_name;

        std::shared_ptr<JSONParsingSchema> current = task_.schema();

        if (!name.size() || current->hasObjectParser(name))
        {
            QMessageBox m_warning (QMessageBox::Warning, "JSON Object Parser Adding Failed",
                                   "Object parser name empty or already defined.",
                                   QMessageBox::Ok);

            m_warning.exec();
            return;
        }

        std::string instance = "JSONObjectParser"+name+dbo_name+"0";

        Configuration &config = current->addNewSubConfiguration ("JSONObjectParser", instance);
        config.addParameterString ("name", name);
        config.addParameterString ("db_object_name", dbo_name);

        current->generateSubConfigurable("JSONObjectParser", instance);
        updateParserBox();
    }
}
void ASTERIXImporterTaskWidget::removeObjectParserSlot ()
{
    loginf << "ASTERIXImporterTaskWidget: removeObjectParserSlot";

    assert (object_parser_box_);

    if (object_parser_box_->currentIndex() >= 0)
    {
        std::string name = object_parser_box_->currentText().toStdString();

        assert (task_.schema() != nullptr);
        std::shared_ptr<JSONParsingSchema> current = task_.schema();

        assert (current->hasObjectParser(name));
        current->removeParser(name);

        updateParserBox();
        selectedObjectParserSlot(object_parser_box_->currentText());
    }
}


void ASTERIXImporterTaskWidget::selectedObjectParserSlot (const QString& text)
{
    loginf << "ASTERIXImporterTaskWidget: selectedObjectParserSlot: text '" << text.toStdString() << "'";

    assert (object_parser_widget_);

    if (!text.size())
    {
        while (object_parser_widget_->count() > 0) // remove all widgets
            object_parser_widget_->removeWidget(object_parser_widget_->widget(0));
        return;
    }

    assert (text.size());

    assert (object_parser_box_);
    std::string name = text.toStdString();

    assert (task_.schema() != nullptr);
    assert (task_.schema()->hasObjectParser(name));

    if (object_parser_widget_->indexOf(task_.schema()->parser(name).widget()) < 0)
        object_parser_widget_->addWidget(task_.schema()->parser(name).widget());

    object_parser_widget_->setCurrentWidget(task_.schema()->parser(name).widget());
}

void ASTERIXImporterTaskWidget::updateParserBox ()
{
    loginf << "ASTERIXImporterTaskWidget: updateParserList";

    assert (object_parser_box_);
    object_parser_box_->clear();

    if (task_.schema() != nullptr)
    {
        for (auto& parser_it : *task_.schema()) // over all object parsers
        {
            object_parser_box_->addItem(parser_it.first.c_str());
            //            QListWidgetItem* item = new QListWidgetItem(tr(parser_it.first.c_str()), object_parser_list_);
            //            assert (item);
        }
    }
}

void ASTERIXImporterTaskWidget::debugChangedSlot ()
{
    QCheckBox* box = dynamic_cast<QCheckBox*> (sender());
    assert (box);

    task_.debug(box->checkState() == Qt::Checked);
}

void ASTERIXImporterTaskWidget::limitRAMChangedSlot ()
{
    QCheckBox* box = dynamic_cast<QCheckBox*> (sender());
    assert (box);

    task_.limitRAM(box->checkState() == Qt::Checked);
}

void ASTERIXImporterTaskWidget::createMappingsSlot()
{
    loginf << "ASTERIXImporterTaskWidget: createMappingsSlot";

    if (!task_.canImportFile())
    {
        QMessageBox m_warning (QMessageBox::Warning, "ASTERIX File Create Mapping Stubs Failed",
                               "Please select a file in the list.",
                               QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    task_.test(false);
    task_.createMappingStubs(true);
    task_.run();

    //    create_mapping_stubs_button_->setDisabled(true);
    //    test_button_->setDisabled(true);
}

void ASTERIXImporterTaskWidget::testImportSlot()
{
    loginf << "ASTERIXImporterTaskWidget: testImportSlot";

    if (!task_.canImportFile())
    {
        QMessageBox m_warning (QMessageBox::Warning, "ASTERIX File Test Import Failed",
                               "Please select a file in the list.",
                               QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    task_.test(true);
    task_.createMappingStubs(false);
    task_.run();

    //    create_mapping_stubs_button_->setDisabled(true);
    //    test_button_->setDisabled(true);
}

//void ASTERIXImporterTaskWidget::importSlot()
//{
//    loginf << "ASTERIXImporterTaskWidget: importSlot";

//    if (!file_list_->currentItem())
//    {
//        QMessageBox m_warning (QMessageBox::Warning, "ASTERIX File Import Failed",
//                               "Please select a file in the list.",
//                               QMessageBox::Ok);
//        m_warning.exec();
//        return;
//    }

//    float free_ram = System::getFreeRAMinGB();

//    if (free_ram < ram_threshold && !task_.limitRAM())
//    {
//        loginf << "ASTERIXImporterTaskWidget: importSlot: only " << free_ram << " GB free ram, recommending limiting";

//        QMessageBox::StandardButton reply;
//        reply = QMessageBox::question(this, "RAM Limiting",
//                                      "There is only "+QString::number(free_ram)+" GB free RAM available.\n"
//                                      +"This will result in decreased decoding performance.\n\n"
//                                      +"Do you agree to limiting RAM usage?",
//                                      QMessageBox::Yes|QMessageBox::No);
//        if (reply == QMessageBox::Yes)
//        {
//            task_.limitRAM(true);
//            limit_ram_check_->setChecked(true);
//        }
//    }
//    else if (free_ram >= ram_threshold && task_.limitRAM())
//    {
//        loginf << "ASTERIXImporterTaskWidget: importSlot: " << free_ram << " GB free ram, recommending not limiting";

//        QMessageBox::StandardButton reply;
//        reply = QMessageBox::question(this, "RAM Limiting",
//                                      "There is "+QString::number(free_ram)+" GB free RAM available.\n"
//                                      +"This will result in increased decoding performance.\n\n"
//                                      +"Do you agree to increased RAM usage?",
//                                      QMessageBox::Yes|QMessageBox::No);
//        if (reply == QMessageBox::Yes)
//        {
//            task_.limitRAM(false);
//            limit_ram_check_->setChecked(false);
//        }
//    }

//    QString filename = file_list_->currentItem()->text();
//    if (filename.size() > 0)
//    {
//        assert (task_.hasFile(filename.toStdString()));

//        if (!task_.canImportFile(filename.toStdString()))
//        {
//            QMessageBox m_warning (QMessageBox::Warning, "ASTERIX File Import Failed",
//                                   "File does not exist.",
//                                   QMessageBox::Ok);
//            m_warning.exec();
//            return;
//        }

//        task_.test(false);
//        task_.createMappingStubs(false);
//        task_.importFile(filename.toStdString());

//        create_mapping_stubs_button_->setDisabled(true);
//        test_button_->setDisabled(true);
//        import_button_->setDisabled(true);
//    }
//}

void ASTERIXImporterTaskWidget::runStarted ()
{
    loginf << "ASTERIXImporterTaskWidget: runStarted";

    create_mapping_stubs_button_->setDisabled(true);
    test_button_->setDisabled(true);
}

void ASTERIXImporterTaskWidget::runDone ()
{
    loginf << "ASTERIXImporterTaskWidget: runDone";

    create_mapping_stubs_button_->setDisabled(false);
    test_button_->setDisabled(false);
    //import_button_->setDisabled(false);
}
