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

#include "asteriximporttaskwidget.h"
#include "asterixconfigwidget.h"
#include "asteriximporttask.h"
#include "asterixoverridewidget.h"
#include "logger.h"
#include "dbcontent/selectdialog.h"
#include "util/timeconv.h"

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QInputDialog>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QDateEdit>

using namespace Utils;
using namespace std;

ASTERIXImportTaskWidget::ASTERIXImportTaskWidget(ASTERIXImportTask& task, QWidget* parent,
                                                 Qt::WindowFlags f)
    : QWidget(parent, f), task_(task)
{
    main_layout_ = new QHBoxLayout();

    tab_widget_ = new QTabWidget();

    main_layout_->addWidget(tab_widget_);

    addMainTab();
    addDecoderTab();
    addOverrideTab();
    addMappingsTab();

    setLayout(main_layout_);
}

void ASTERIXImportTaskWidget::addMainTab()
{
    assert(tab_widget_);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* main_tab_layout = new QVBoxLayout();

    // source stuff
    {
        QFormLayout* source_layout = new QFormLayout();

        sources_grid_ = new QGridLayout();
        updateSourcesGrid();

        main_tab_layout->addLayout(sources_grid_);

        main_tab_layout->addStretch();

        if (task_.isImportNetwork())
        {
            loginf << "ASTERIXImportTaskWidget: addMainTab: is network import";
        }
        else
        {
            loginf << "ASTERIXImportTaskWidget: addMainTab: is file import";

            // line
            QComboBox* file_line_box = new QComboBox();
            file_line_box->addItems({"1", "2", "3", "4"});

            connect(file_line_box, &QComboBox::currentTextChanged,
                    this, &ASTERIXImportTaskWidget::fileLineIDEditSlot);
            source_layout->addRow("Line ID", file_line_box);

            // date
            QDateEdit* date_edit = new QDateEdit();
            date_edit->setDisplayFormat("yyyy-MM-dd");

            //loginf << "UGA " << Time::toDateString(task_.date());

            QDate date = QDate::fromString(Time::toDateString(task_.settings().date_).c_str(), "yyyy-MM-dd");
            //loginf << "UGA2 " << date.toString().toStdString();

            date_edit->setDate(date);

            connect(date_edit, &QDateEdit::dateChanged,
                    this, &ASTERIXImportTaskWidget::dateChangedSlot);
            source_layout->addRow("UTC Day", date_edit);
        }

        main_tab_layout->addLayout(source_layout);
    }

    main_tab_layout->addStretch();

    // final stuff
    {
        debug_check_ = new QCheckBox("Debug in Console");
        debug_check_->setChecked(task_.settings().debug_jasterix_);
        connect(debug_check_, &QCheckBox::clicked, this,
                &ASTERIXImportTaskWidget::debugChangedSlot);
        main_tab_layout->addWidget(debug_check_);

    }

    QWidget* main_tab_widget = new QWidget();
    main_tab_widget->setContentsMargins(0, 0, 0, 0);
    main_tab_widget->setLayout(main_tab_layout);
    tab_widget_->addTab(main_tab_widget, "Main");
}

void ASTERIXImportTaskWidget::addDecoderTab()
{
    assert(tab_widget_);

    config_widget_ = new ASTERIXConfigWidget(task_, this);
    tab_widget_->addTab(config_widget_, "Decoder");
}

void ASTERIXImportTaskWidget::addOverrideTab()
{
    assert(tab_widget_);

    override_widget_ = new ASTERIXOverrideWidget(task_, this);
    tab_widget_->addTab(override_widget_, "Override/Filter");
}

void ASTERIXImportTaskWidget::addMappingsTab()
{
    QVBoxLayout* parsers_layout = new QVBoxLayout();

    QHBoxLayout* parser_manage_layout = new QHBoxLayout();

    object_parser_box_ = new QComboBox();
    connect(object_parser_box_, SIGNAL(currentIndexChanged(const QString&)), this,
            SLOT(selectedObjectParserSlot(const QString&)));

    parser_manage_layout->addWidget(object_parser_box_);

    add_object_parser_button_ = new QPushButton("Add");
    connect(add_object_parser_button_, SIGNAL(clicked()), this, SLOT(addParserSlot()));
    add_object_parser_button_->setEnabled(COMPASS::instance().expertMode());
    parser_manage_layout->addWidget(add_object_parser_button_);

    delete_object_parser_button_ = new QPushButton("Remove");
    connect(delete_object_parser_button_, SIGNAL(clicked()), this, SLOT(removeObjectParserSlot()));
    delete_object_parser_button_->setEnabled(COMPASS::instance().expertMode());
    parser_manage_layout->addWidget(delete_object_parser_button_);

    parsers_layout->addLayout(parser_manage_layout);

    QFrame* line = new QFrame();
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    parsers_layout->addWidget(line);

    object_parser_widget_ = new QStackedWidget();
    parsers_layout->addWidget(object_parser_widget_);

    updateParserBox();

    QWidget* mappings_tab_widget = new QWidget();
    mappings_tab_widget->setContentsMargins(0, 0, 0, 0);
    mappings_tab_widget->setLayout(parsers_layout);
    tab_widget_->addTab(mappings_tab_widget, "Mappings");
}

ASTERIXImportTaskWidget::~ASTERIXImportTaskWidget() { config_widget_ = nullptr; }

void ASTERIXImportTaskWidget::addParserSlot()
{
    if (task_.schema() == nullptr)
    {
        QMessageBox m_warning(QMessageBox::Warning, "JSON Object Parser Adding Failed",
                              "No current JSON Parsing Schema is selected.", QMessageBox::Ok);

        m_warning.exec();
        return;
    }

    dbContent::SelectDBContentDialog dialog;

    int ret = dialog.exec();

    if (ret == QDialog::Accepted)
    {
        unsigned int cat = dialog.category();
        std::string dbcontent_name = dialog.selectedObject();
        loginf << "ASTERIXImportTaskWidget: addObjectParserSlot: cat " << cat << " obj "
               << dbcontent_name;

        std::shared_ptr<ASTERIXJSONParsingSchema> current = task_.schema();

        if (current->hasObjectParser(cat))
        {
            QMessageBox m_warning(QMessageBox::Warning, "ASTERIX JSON Parser Adding Failed",
                                  "ASTERIX parser for category already defined.", QMessageBox::Ok);

            m_warning.exec();
            return;
        }

        std::string instance = "ASTERIXJSONParserCAT" + to_string(cat) + "0";

        auto config = Configuration::create("ASTERIXJSONParser", instance);
        config->addParameter<unsigned int>("category", cat);
        config->addParameter<std::string>("dbcontent_name", dbcontent_name);

        current->generateSubConfigurableFromConfig(std::move(config));
        updateParserBox();
    }
}
void ASTERIXImportTaskWidget::removeObjectParserSlot()
{
    loginf << "ASTERIXImportTaskWidget: removeObjectParserSlot";

    assert(object_parser_box_);

    if (object_parser_box_->currentIndex() >= 0)
    {
        unsigned int cat = object_parser_box_->currentText().toUInt();

        assert(task_.schema() != nullptr);
        std::shared_ptr<ASTERIXJSONParsingSchema> current = task_.schema();

        assert(current->hasObjectParser(cat));
        current->removeParser(cat);

        updateParserBox();
        selectedObjectParserSlot(object_parser_box_->currentText());
    }
}

void ASTERIXImportTaskWidget::selectedObjectParserSlot(const QString& text)
{
    loginf << "ASTERIXImportTaskWidget: selectedObjectParserSlot: text '" << text.toStdString()
           << "'";

    assert(object_parser_widget_);

    if (!text.size())
    {
        while (object_parser_widget_->count() > 0)  // remove all widgets
            object_parser_widget_->removeWidget(object_parser_widget_->widget(0));
        return;
    }

    assert(text.size());

    assert(object_parser_box_);
    unsigned int cat = text.toUInt();

    assert(task_.schema() != nullptr);
    assert(task_.schema()->hasObjectParser(cat));

    if (object_parser_widget_->indexOf(task_.schema()->parser(cat).widget()) < 0)
        object_parser_widget_->addWidget(task_.schema()->parser(cat).widget());

    object_parser_widget_->setCurrentWidget(task_.schema()->parser(cat).widget());
}


void ASTERIXImportTaskWidget::fileLineIDEditSlot(const QString& text)
{
    loginf << "ASTERIXImportTaskWidget: fileLineIDEditSlot: value '" << text.toStdString() << "'";

    bool ok;

    unsigned int line_id = text.toUInt(&ok);

    assert (ok);

    assert (line_id > 0 && line_id <= 4);

    assert (false); // TODO

    //task_.settings().file_line_id_ = line_id-1;
}

void ASTERIXImportTaskWidget::dateChangedSlot(QDate date)
{
    string tmp = date.toString("yyyy-MM-dd").toStdString();

    loginf << "ASTERIXImportTaskWidget: dateChangedSlot: " << tmp;

    task_.settings().date_ = Time::fromDateString(tmp);
}

void ASTERIXImportTaskWidget::updateParserBox()
{
    loginf << "ASTERIXImportTaskWidget: updateParserList";

    assert(object_parser_box_);
    object_parser_box_->clear();

    if (task_.schema() != nullptr)
    {
        for (auto& parser_it : *task_.schema())  // over all object parsers
        {
            object_parser_box_->addItem(QString::number(parser_it.first));
        }
    }
}

void ASTERIXImportTaskWidget::debugChangedSlot()
{
    QCheckBox* box = dynamic_cast<QCheckBox*>(sender());
    assert(box);

    task_.settings().debug_jasterix_ = box->checkState() == Qt::Checked;
}

//void ASTERIXImportTaskWidget::runStarted()
//{
//    loginf << "ASTERIXImportTaskWidget: runStarted";

//    test_button_->setDisabled(true);
//}

//void ASTERIXImportTaskWidget::runDone()
//{
//    loginf << "ASTERIXImportTaskWidget: runDone";

//    test_button_->setDisabled(false);
//}

void ASTERIXImportTaskWidget::updateSourcesGrid()
{
    QLayoutItem* child;
    while ((child = sources_grid_->takeAt(0)) != nullptr)
    {
        if (child->widget())
            delete child->widget();
        delete child;
    }

    unsigned int row{0};

    if (task_.isImportNetwork())
        sources_grid_->addWidget(new QLabel("Source: Network"), row, 0);
    else // files
    {
        for (auto& file_info : task_.filesInfo())
        {
            sources_grid_->addWidget(new QLabel(file_info.filename_.c_str()), row, 0);

            if (!file_info.decoding_tried_)
                sources_grid_->addWidget(new QLabel("?"), row, 1);
            else if (file_info.errors_found_)
                sources_grid_->addWidget(new QLabel("Decoding Errors"), row, 1);
            else
                sources_grid_->addWidget(new QLabel("OK"), row, 1);

            ++row;
        }
    }
}

//void ASTERIXImportTaskWidget::updateSourceLabel()
//{
//    assert (source_label_);

//    if (task_.isImportNetwork())
//        source_label_->setText("Source: Network");
//    else // file
//        source_label_->setText(("Source: \n"+task_.importFilenamesStr()).c_str());
//}

ASTERIXOverrideWidget* ASTERIXImportTaskWidget::overrideWidget() const
{
    return override_widget_;
}

void ASTERIXImportTaskWidget::testImportSlot()
{
    loginf << "ASTERIXImportTaskWidget: testImportSlot";

    if (!task_.canImportFiles())
    {
        QMessageBox m_warning(QMessageBox::Warning, "ASTERIX File Test Import Failed",
                              "Please select a file in the list.", QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    task_.run(true);
}
