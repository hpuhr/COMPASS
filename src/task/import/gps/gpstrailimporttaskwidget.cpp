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

#include "gpstrailimporttaskwidget.h"
#include "gpstrailimporttask.h"
#include "logger.h"
#include "stringconv.h"

#include "textfielddoublevalidator.h"
#include "textfieldhexvalidator.h"
#include "textfieldoctvalidator.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QCheckBox>

#include <iostream>

using namespace std;
using namespace Utils;

GPSTrailImportTaskWidget::GPSTrailImportTaskWidget(GPSTrailImportTask& task, QWidget* parent, Qt::WindowFlags f)
    : TaskWidget(parent, f), task_(task)
{
    main_layout_ = new QHBoxLayout();

    tab_widget_ = new QTabWidget();

    main_layout_->addWidget(tab_widget_);

    addMainTab();
    addConfigTab();

    updateText();
    updateConfig();

    setLayout(main_layout_);
}

void GPSTrailImportTaskWidget::addMainTab()
{
    assert(tab_widget_);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* tab_layout = new QVBoxLayout();

    // file stuff
    {
        QLabel* files_label = new QLabel("NMEA File Selection");
        files_label->setFont(font_bold);
        tab_layout->addWidget(files_label);

        file_list_ = new QListWidget();
        file_list_->setWordWrap(true);
        file_list_->setTextElideMode(Qt::ElideNone);
        file_list_->setSelectionBehavior(QAbstractItemView::SelectItems);
        file_list_->setSelectionMode(QAbstractItemView::SingleSelection);
        connect(file_list_, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selectedFileSlot()));

        updateFileListSlot();
        tab_layout->addWidget(file_list_);
    }

    // file button stuff
    {
        QHBoxLayout* button_layout = new QHBoxLayout();

        add_file_button_ = new QPushButton("Add");
        connect(add_file_button_, &QPushButton::clicked, this, &GPSTrailImportTaskWidget::addFileSlot);
        button_layout->addWidget(add_file_button_);

        delete_file_button_ = new QPushButton("Remove");
        connect(delete_file_button_, &QPushButton::clicked, this, &GPSTrailImportTaskWidget::deleteFileSlot);
        button_layout->addWidget(delete_file_button_);

        delete_all_files_button_ = new QPushButton("Remove All");
        connect(delete_all_files_button_, &QPushButton::clicked, this, &GPSTrailImportTaskWidget::deleteAllFilesSlot);
        button_layout->addWidget(delete_all_files_button_);

        tab_layout->addLayout(button_layout);
    }

    text_edit_ = new QTextEdit ();
    text_edit_->setReadOnly(true);
    tab_layout->addWidget(text_edit_);

    QWidget* main_tab_widget = new QWidget();
    main_tab_widget->setContentsMargins(0, 0, 0, 0);
    main_tab_widget->setLayout(tab_layout);
    tab_widget_->addTab(main_tab_widget, "Main");
}

void GPSTrailImportTaskWidget::addConfigTab()
{
    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* tab_layout = new QVBoxLayout();

    QGridLayout* grid = new QGridLayout();

    unsigned int row = 0;

    // sac
    grid->addWidget(new QLabel("SAC"), row, 0);

    sac_edit_ = new QLineEdit();
    sac_edit_->setValidator(new TextFieldDoubleValidator(0, 255, 0));
    connect(sac_edit_, &QLineEdit::textEdited, this, &GPSTrailImportTaskWidget::sacEditedSlot);
    grid->addWidget(sac_edit_, row, 1);

    // sic
    ++row;

    grid->addWidget(new QLabel("SIC"), row, 0);

    sic_edit_ = new QLineEdit();
    sic_edit_->setValidator(new TextFieldDoubleValidator(0, 255, 0));
    connect(sic_edit_, &QLineEdit::textEdited, this, &GPSTrailImportTaskWidget::sicEditedSlot);
    grid->addWidget(sic_edit_, row, 1);

    // name

    ++row;
    grid->addWidget(new QLabel("Name"), row, 0);

    name_edit_ = new QLineEdit();
    connect(name_edit_, &QLineEdit::textEdited, this, &GPSTrailImportTaskWidget::nameEditedSlot);
    grid->addWidget(name_edit_, row, 1);

    // tod offset
    ++row;
    grid->addWidget(new QLabel("Time of Day Offset"), row, 0);

    tod_offset_edit_ = new QLineEdit();
    tod_offset_edit_->setValidator(new TextFieldDoubleValidator(-24 * 3600, 24 * 3600, 3));
    connect(tod_offset_edit_, &QLineEdit::textEdited, this,
            &GPSTrailImportTaskWidget::todOffsetEditedSlot);
    grid->addWidget(tod_offset_edit_, row, 1);

    // mode 3a

    ++row;
    set_mode_3a_code_check_ = new QCheckBox("Mode 3/A Code (octal)");
    connect(set_mode_3a_code_check_, &QCheckBox::clicked, this, &GPSTrailImportTaskWidget::mode3ACheckedSlot);
    grid->addWidget(set_mode_3a_code_check_, row, 0);

    mode_3a_code_edit_ = new QLineEdit();
    mode_3a_code_edit_->setValidator(new TextFieldOctValidator(4));
    connect(mode_3a_code_edit_, &QLineEdit::textEdited, this,
            &GPSTrailImportTaskWidget::mode3AEditedSlot);
    grid->addWidget(mode_3a_code_edit_, row, 1);

    // target address

    ++row;
    set_target_address_check_ = new QCheckBox("Target Address (hexadecimal)");
    connect(set_target_address_check_, &QCheckBox::clicked, this, &GPSTrailImportTaskWidget::targetAddressCheckedSlot);
    grid->addWidget(set_target_address_check_, row, 0);

    target_address_edit_ = new QLineEdit();
    target_address_edit_->setValidator(new TextFieldHexValidator(6));
    connect(target_address_edit_, &QLineEdit::textEdited, this,
            &GPSTrailImportTaskWidget::targetAddressEditedSlot);
    grid->addWidget(target_address_edit_, row, 1);

    // callsign

    ++row;
    set_callsign_check_ = new QCheckBox("Callsign");
    connect(set_callsign_check_, &QCheckBox::clicked, this, &GPSTrailImportTaskWidget::callsignCheckedSlot);
    grid->addWidget(set_callsign_check_, row, 0);

    callsign_edit_ = new QLineEdit();
    callsign_edit_->setMaxLength(8);
    connect(callsign_edit_, &QLineEdit::textEdited, this,
            &GPSTrailImportTaskWidget::callsignEditedSlot);
    grid->addWidget(callsign_edit_, row, 1);


    // finish him
    tab_layout->addLayout(grid);

    tab_layout->addStretch();

    QWidget* tab_widget = new QWidget();
    tab_widget->setContentsMargins(0, 0, 0, 0);
    tab_widget->setLayout(tab_layout);

    tab_widget_->addTab(tab_widget, "Config");
}

GPSTrailImportTaskWidget::~GPSTrailImportTaskWidget() {}

void GPSTrailImportTaskWidget::addFile(const std::string& filename)
{
    if (!task_.hasFile(filename))
        task_.addFile(filename);
}

void GPSTrailImportTaskWidget::selectFile(const std::string& filename)
{
    QList<QListWidgetItem*> items = file_list_->findItems(filename.c_str(), Qt::MatchExactly);
    assert (items.size() > 0);

    assert(task_.hasFile(filename));
    task_.currentFilename(filename);

    for (auto item_it : items)
    {
        assert (item_it);
        file_list_->setCurrentItem(item_it);
    }

    updateText();
}

void GPSTrailImportTaskWidget::addFileSlot()
{
    QString filename = QFileDialog::getOpenFileName(this, tr("Add NMEA File"));

    if (filename.size() > 0)
        addFile(filename.toStdString());
}

void GPSTrailImportTaskWidget::deleteFileSlot()
{
    loginf << "JSONImporterTaskWidget: deleteFileSlot";

    if (!file_list_->currentItem() || !task_.currentFilename().size())
    {
        QMessageBox m_warning(QMessageBox::Warning, "JSON File Deletion Failed",
                              "Please select a file in the list.", QMessageBox::Ok);
        m_warning.exec();
        return;
    }

    assert(task_.currentFilename().size());
    assert(task_.hasFile(task_.currentFilename()));
    task_.removeCurrentFilename();
}

void GPSTrailImportTaskWidget::deleteAllFilesSlot()
{
    loginf << "GPSTrailImportTaskWidget: deleteAllFilesSlot";
    task_.removeAllFiles();
}


void GPSTrailImportTaskWidget::selectedFileSlot()
{
    logdbg << "JSONImporterTaskWidget: selectedFileSlot";
    assert(file_list_->currentItem());

    QString filename = file_list_->currentItem()->text();
    assert(task_.hasFile(filename.toStdString()));

    task_.currentFilename(filename.toStdString());

    updateText();
}

void GPSTrailImportTaskWidget::updateFileListSlot()
{
    assert(file_list_);

    file_list_->clear();

    for (auto it : task_.fileList())
    {
        QListWidgetItem* item = new QListWidgetItem(tr(it.first.c_str()), file_list_);
        if (it.first == task_.currentFilename())
            file_list_->setCurrentItem(item);
    }
}

void GPSTrailImportTaskWidget::sacEditedSlot(const QString& value)
{
    assert (sac_edit_);

    TextFieldDoubleValidator::displayValidityAsColor(sac_edit_);

    if (sac_edit_->hasAcceptableInput())
        task_.dsSAC(sac_edit_->text().toUInt());
}

void GPSTrailImportTaskWidget::sicEditedSlot(const QString& value)
{
    assert (sic_edit_);

    TextFieldDoubleValidator::displayValidityAsColor(sic_edit_);

    if (sic_edit_->hasAcceptableInput())
        task_.dsSIC(sic_edit_->text().toUInt());
}

void GPSTrailImportTaskWidget::nameEditedSlot(const QString& value)
{
    assert (name_edit_);
    task_.dsName(name_edit_->text().toStdString());
}

void GPSTrailImportTaskWidget::todOffsetEditedSlot(const QString& value)
{
    assert (tod_offset_edit_);

    TextFieldDoubleValidator::displayValidityAsColor(tod_offset_edit_);

    if (tod_offset_edit_->hasAcceptableInput())
        task_.todOffset(value.toFloat());
}

void GPSTrailImportTaskWidget::mode3ACheckedSlot()
{
    loginf << "GPSTrailImportTaskWidget: mode3ACheckedSlot";

    assert (set_mode_3a_code_check_);
    task_.setMode3aCode(set_mode_3a_code_check_->checkState() == Qt::Checked);
}

void GPSTrailImportTaskWidget::mode3AEditedSlot(const QString& value)
{
    loginf << "GPSTrailImportTaskWidget: mode3AEditedSlot: value " << value.toStdString();

    assert (mode_3a_code_edit_);

    if (mode_3a_code_edit_->hasAcceptableInput())
        task_.mode3aCode(String::intFromOctalString(value.toStdString()));
}

void GPSTrailImportTaskWidget::targetAddressCheckedSlot()
{
    loginf << "GPSTrailImportTaskWidget: targetAddressCheckedSlot";

    assert (set_target_address_check_);
    task_.setTargetAddress(set_target_address_check_->checkState() == Qt::Checked);
}

void GPSTrailImportTaskWidget::targetAddressEditedSlot(const QString& value)
{
    loginf << "GPSTrailImportTaskWidget: targetAddressEditedSlot: value " << value.toStdString();

    assert (target_address_edit_);

    if (target_address_edit_->hasAcceptableInput())
        task_.targetAddress(String::intFromHexString(value.toStdString()));
}

void GPSTrailImportTaskWidget::callsignCheckedSlot()
{
    loginf << "GPSTrailImportTaskWidget: callsignCheckedSlot";

    assert (set_callsign_check_);
    task_.setCallsign(set_callsign_check_->checkState() == Qt::Checked);
}

void GPSTrailImportTaskWidget::callsignEditedSlot(const QString& value)
{
    loginf << "GPSTrailImportTaskWidget: callsignEditedSlot: value '" << value.toStdString() << "'";

    QString upper_value = value.toUpper();

    callsign_edit_->setText(upper_value);

    task_.callsign(upper_value.toStdString());
}

void GPSTrailImportTaskWidget::updateConfig ()
{
    assert (sac_edit_);
    sac_edit_->setText(QString::number(task_.dsSAC()));

    assert (sic_edit_);
    sic_edit_->setText(QString::number(task_.dsSIC()));

    assert (name_edit_);
    name_edit_->setText(task_.dsName().c_str());

    assert (tod_offset_edit_);
    tod_offset_edit_->setText(QString::number(task_.todOffset()));

    assert (set_mode_3a_code_check_);
    set_mode_3a_code_check_->setChecked(task_.setMode3aCode());

    assert (mode_3a_code_edit_);
    mode_3a_code_edit_->setText(String::octStringFromInt(task_.mode3aCode()).c_str());

    assert (set_target_address_check_);
    set_target_address_check_->setChecked(task_.setTargetAddress());
    assert (target_address_edit_);
    target_address_edit_->setText(String::hexStringFromInt(task_.targetAddress()).c_str());

    assert (set_callsign_check_);
    set_callsign_check_->setChecked(task_.setCallsign());
    assert (callsign_edit_);
    callsign_edit_->setText(task_.callsign().c_str());
}

void GPSTrailImportTaskWidget::expertModeChangedSlot() {}

//void GPSTrailImportTaskWidget::runStarted()
//{
//    loginf << "GPSTrailImportTaskWidget: runStarted";

//    //test_button_->setDisabled(true);
//}

//void GPSTrailImportTaskWidget::runDone()
//{
//    loginf << "GPSTrailImportTaskWidget: runDone";

//    //test_button_->setDisabled(false);
//}

void GPSTrailImportTaskWidget::updateText ()
{
    loginf << "ViewPointsImportTaskWidget: updateText";

    assert (text_edit_);

    stringstream ss;

    if (task_.currentError().size())
        ss << "Errors:\n" << task_.currentError() << "\n";

    if (task_.currentText().size())
        ss << task_.currentText();

    text_edit_->setText(ss.str().c_str());

    //    if (task_.currentError().size())
    //    {
    //        context_edit_->setText(QString("Error: ")+task_.currentError().c_str());
    //        import_button_->setDisabled(true);
    //    }
    //    else
    //    {
    //        const nlohmann::json& data = task_.currentData();

    //        assert (data.contains("view_point_context"));

    //        context_edit_->setText(data.at("view_point_context").dump(4).c_str());

    //        import_button_->setDisabled(false);
    //    }
}
