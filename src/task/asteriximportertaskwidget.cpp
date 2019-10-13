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

ASTERIXImporterTaskWidget::ASTERIXImporterTaskWidget(ASTERIXImporterTask& task, QWidget* parent, Qt::WindowFlags f)
    : QWidget (parent, f), task_(task)
{
    setWindowTitle ("Import ASTERIX Data");
    setMinimumSize(QSize(600, 800));

    QFont font_bold;
    font_bold.setBold(true);

    QFont font_big;
    font_big.setPointSize(18);

    int frame_width_small = 1;

    main_layout_ = new QHBoxLayout ();

    QVBoxLayout* left_layout = new QVBoxLayout ();

    QFrame *left_frame = new QFrame ();
    left_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    left_frame->setLineWidth(frame_width_small);
    left_frame->setLayout(left_layout);

    QSizePolicy sp_left(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp_left.setHorizontalStretch(1);
    left_frame->setSizePolicy(sp_left);

//    QLabel *main_label = new QLabel ("Import ASTERIX data");
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

    {
        config_widget_ = new ASTERIXConfigWidget (task_, this);
        left_layout->addWidget(config_widget_);
        //config_widget_->show();
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

    // final stuff
    {
        debug_ = new QCheckBox ("Debug in Console");
        debug_->setChecked(task_.debug());
        connect(debug_, &QCheckBox::clicked, this, &ASTERIXImporterTaskWidget::debugChangedSlot);
        left_layout->addWidget (debug_);

        create_mapping_stubs_button_ = new QPushButton ("Create Mapping Stubs");
        connect(create_mapping_stubs_button_, &QPushButton::clicked,
                this, &ASTERIXImporterTaskWidget::createMappingsSlot);
        left_layout->addWidget (create_mapping_stubs_button_);

        test_button_ = new QPushButton ("Test Import");
        connect(test_button_, &QPushButton::clicked, this, &ASTERIXImporterTaskWidget::testImportSlot);
        left_layout->addWidget (test_button_);

        import_button_ = new QPushButton ("Import");
        connect(import_button_, &QPushButton::clicked, this, &ASTERIXImporterTaskWidget::importSlot);
        left_layout->addWidget (import_button_);
    }

    //main_layout_->addLayout(left_layout);
    main_layout_->addWidget(left_frame);

    setLayout (main_layout_);

    show();
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
        updateParserList();
    }
}
void ASTERIXImporterTaskWidget::removeObjectParserSlot ()
{
    loginf << "ASTERIXImporterTaskWidget: removeObjectParserSlot";

    if (object_parser_list_->currentItem())
    {
        std::string name = object_parser_list_->currentItem()->text().toStdString();

        assert (task_.schema() != nullptr);
        std::shared_ptr<JSONParsingSchema> current = task_.schema();

        assert (current->hasObjectParser(name));
        current->removeParser(name);

        updateParserList();
        selectedObjectParserSlot();
    }
}


void ASTERIXImporterTaskWidget::selectedObjectParserSlot ()
{
    loginf << "ASTERIXImporterTaskWidget: selectedObjectParserSlot";

    if (object_parser_widget_)
        while (object_parser_widget_->count() > 0)
            object_parser_widget_->removeWidget(object_parser_widget_->widget(0));

    assert (object_parser_list_);

    if (object_parser_list_->currentItem())
    {
        std::string name = object_parser_list_->currentItem()->text().toStdString();

        assert (task_.schema() != nullptr);
        assert (task_.schema()->hasObjectParser(name));

        if (!object_parser_widget_)
            createObjectParserWidget();

        object_parser_widget_->addWidget(task_.schema()->parser(name).widget());
    }
}

void ASTERIXImporterTaskWidget::createObjectParserWidget()
{
    assert (!object_parser_widget_);
    assert (main_layout_);
    setMinimumSize(QSize(1800, 800));

    int frame_width_small = 1;

    QFrame *right_frame = new QFrame ();
    right_frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
    right_frame->setLineWidth(frame_width_small);

    QSizePolicy sp_right(QSizePolicy::Preferred, QSizePolicy::Preferred);
    sp_right.setHorizontalStretch(2);
    right_frame->setSizePolicy(sp_right);

    object_parser_widget_ = new QStackedWidget ();

    object_parser_widget_->setMinimumWidth(800);

    QVBoxLayout* tmp = new QVBoxLayout ();
    tmp->addWidget(object_parser_widget_);

    right_frame->setLayout(tmp);

    main_layout_->addWidget(right_frame);
}

void ASTERIXImporterTaskWidget::updateParserList ()
{
    loginf << "ASTERIXImporterTaskWidget: updateParserList";

    assert (object_parser_list_);
    object_parser_list_->clear();

    if (task_.schema() != nullptr)
    {
        for (auto& parser_it : *task_.schema()) // over all object parsers
        {
            QListWidgetItem* item = new QListWidgetItem(tr(parser_it.first.c_str()), object_parser_list_);
            assert (item);
        }
    }
}

void ASTERIXImporterTaskWidget::debugChangedSlot ()
{
    QCheckBox* box = dynamic_cast<QCheckBox*> (sender());
    assert (box);

    task_.debug(box->checkState() == Qt::Checked);
}

void ASTERIXImporterTaskWidget::createMappingsSlot()
{
    loginf << "ASTERIXImporterTaskWidget: createMappingsSlot";

    if (!file_list_->currentItem())
    {
        QMessageBox m_warning (QMessageBox::Warning, "ASTERIX File Create Mapping Stubs Failed",
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
            QMessageBox m_warning (QMessageBox::Warning, "ASTERIX File Create Mapping Stubs Failed",
                                   "File does not exist.",
                                   QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        task_.test(false);
        task_.createMappingStubs(true);
        task_.importFile(filename.toStdString());

        create_mapping_stubs_button_->setDisabled(true);
        test_button_->setDisabled(true);
        import_button_->setDisabled(true);
    }
}

void ASTERIXImporterTaskWidget::testImportSlot()
{
    loginf << "ASTERIXImporterTaskWidget: testImportSlot";

    if (!file_list_->currentItem())
    {
        QMessageBox m_warning (QMessageBox::Warning, "ASTERIX File Test Import Failed",
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
            QMessageBox m_warning (QMessageBox::Warning, "ASTERIX File Test Import Failed",
                                   "File does not exist.",
                                   QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        task_.test(true);
        task_.createMappingStubs(false);
        task_.importFile(filename.toStdString());

        create_mapping_stubs_button_->setDisabled(true);
        test_button_->setDisabled(true);
        import_button_->setDisabled(true);
    }

}
void ASTERIXImporterTaskWidget::importSlot()
{
    loginf << "ASTERIXImporterTaskWidget: importSlot";

    if (!file_list_->currentItem())
    {
        QMessageBox m_warning (QMessageBox::Warning, "ASTERIX File Import Failed",
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
            QMessageBox m_warning (QMessageBox::Warning, "ASTERIX File Import Failed",
                                   "File does not exist.",
                                   QMessageBox::Ok);
            m_warning.exec();
            return;
        }

        task_.test(false);
        task_.createMappingStubs(false);
        task_.importFile(filename.toStdString());

        create_mapping_stubs_button_->setDisabled(true);
        test_button_->setDisabled(true);
        import_button_->setDisabled(true);
    }
}
