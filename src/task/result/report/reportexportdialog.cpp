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

#include "reportexportdialog.h"
#include "reportexport.h"

#include "taskresult.h"

#include "logger.h"
#include "compass.h"
#include "files.h"

#include <QFormLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QTextEdit>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QTimer>
#include <QApplication>

#include <boost/filesystem.hpp>

namespace ResultReport
{

/**
 */
ReportExportDialog::ReportExportDialog(TaskResult& task_result,
                                       ReportExport& report_export, 
                                       ReportExportMode export_mode,
                                       bool no_interaction_mode,
                                       QWidget* parent, 
                                       Qt::WindowFlags f)
:   QDialog             (parent, f          )
,   task_result_        (task_result        )
,   report_export_      (report_export      )
,   export_mode_        (export_mode        )
,   no_interaction_mode_(no_interaction_mode)
{
    format_str_ = QString::fromStdString(reportExportMode2String(export_mode));

    setWindowTitle("Export Report as " + format_str_);
    setMinimumSize(QSize(800, 600));

    createUI();
    configureUI();

    connect(&report_export, &ReportExport::progressChanged, this, &ReportExportDialog::updateProgress);
}

/**
 */
void ReportExportDialog::showEvent(QShowEvent *event)
{
    if (no_interaction_mode_)
        QTimer::singleShot(10, this, &ReportExportDialog::exportReport);
}

/**
 */
void ReportExportDialog::createUI()
{
    auto main_layout = new QVBoxLayout;
    setLayout(main_layout);

    auto layout = new QFormLayout;
    main_layout->addLayout(layout);
    
    base_dir_button_ = new QPushButton;
    base_dir_button_->setIcon(Utils::Files::getIcon("folder.png"));

    connect(base_dir_button_, &QPushButton::pressed, this, &ReportExportDialog::editBaseDir);

    base_dir_edit_ = new QLineEdit;

    auto base_dir_layout = new QHBoxLayout;
    base_dir_layout->setSpacing(0);
    base_dir_layout->setContentsMargins(0, 0, 0, 0);

    base_dir_layout->addWidget(base_dir_button_);
    base_dir_layout->addWidget(base_dir_edit_);

    layout->addRow("Base Directory", base_dir_layout);
    base_dir_label_ = dynamic_cast<QLabel*>(layout->labelForField(base_dir_layout));
    assert(base_dir_label_);

    res_dir_name_edit_ = new QLineEdit;
    layout->addRow("Report Directory", res_dir_name_edit_);
    res_dir_name_label_ = dynamic_cast<QLabel*>(layout->labelForField(res_dir_name_edit_));
    assert(res_dir_name_label_);

    connect(res_dir_name_edit_, &QLineEdit::textChanged, this, &ReportExportDialog::checkExport);

    report_name_edit_ = new QLineEdit;
    layout->addRow("Report Name", report_name_edit_);
    report_name_label_ = dynamic_cast<QLabel*>(layout->labelForField(report_name_edit_));
    assert(report_name_label_);

    connect(report_name_edit_, &QLineEdit::textChanged, this, &ReportExportDialog::checkExport);

    author_edit_ = new QLineEdit;
    layout->addRow("Author", author_edit_);

    comment_edit_ = new QTextEdit;
    layout->addRow("Comments", comment_edit_);

    latex_max_rows_edit_ = new QSpinBox;
    latex_max_rows_edit_->setMinimum(0);
    latex_max_rows_edit_->setMaximum(std::numeric_limits<int>::max());
    layout->addRow("Maximum Table Rows", latex_max_rows_edit_);
    latex_max_rows_label_ = dynamic_cast<QLabel*>(layout->labelForField(latex_max_rows_edit_));
    assert(latex_max_rows_label_);

    latex_max_colw_edit_ = new QSpinBox;
    latex_max_colw_edit_->setMinimum(0);
    latex_max_colw_edit_->setMaximum(std::numeric_limits<int>::max());
    layout->addRow("Maximum Table Column Width", latex_max_colw_edit_);
    latex_max_colw_label_ = dynamic_cast<QLabel*>(layout->labelForField(latex_max_colw_edit_));
    assert(latex_max_colw_label_);

    open_file_box_ = new QCheckBox;
    layout->addRow("Open Created " + format_str_ + " File", open_file_box_);
    open_file_label_ = dynamic_cast<QLabel*>(layout->labelForField(open_file_box_));
    assert(open_file_label_);

    main_layout->addStretch(1);

    elapsed_label_   = new QLabel;
    remaining_label_ = new QLabel;
    status_label_    = new QLabel;

    progress_bar_ = new QProgressBar;
    progress_bar_->setMinimum(0);
    progress_bar_->setMaximum(100);

    auto progress_layout = new QFormLayout;
    progress_layout->addRow("Progress" , progress_bar_   );
    progress_layout->addRow("Status"   , status_label_   );
    //progress_layout->addRow("Elapsed"  , elapsed_label_  );
    //progress_layout->addRow("Remaining", remaining_label_);

    main_layout->addLayout(progress_layout);

    export_button_  = new QPushButton("Export");
    cancel_button_  = new QPushButton("Cancel");

    connect(export_button_, &QPushButton::pressed, this, &ReportExportDialog::exportReport);
    connect(cancel_button_, &QPushButton::pressed, this, &ReportExportDialog::cancel);

    auto button_layout = new QHBoxLayout;
    button_layout->addWidget(cancel_button_);
    button_layout->addStretch(1);
    button_layout->addWidget(export_button_);

    main_layout->addLayout(button_layout);

    checkExport();
}

/**
 */
void ReportExportDialog::configureUI()
{
    auto db_fn      = COMPASS::instance().lastDbFilename();
    auto db_dir     = Utils::Files::getDirectoryFromPath(db_fn);
    auto base_name  = boost::filesystem::path(db_fn).stem().string();
    auto res_name   = task_result_.name();
    auto exp_folder = reportExportMode2Folder(export_mode_);

    assert(!exp_folder.empty());

    auto report_dir  = Utils::Files::normalizeFilename(base_name + " " + res_name, false) +
                       "/" + exp_folder;
    auto report_name = Utils::Files::normalizeFilename(res_name, false) + 
                       reportExportMode2Extension(export_mode_);
    
    base_dir_edit_->setText(QString::fromStdString(db_dir));
    res_dir_name_edit_->setText(QString::fromStdString(report_dir));
    report_name_edit_->setText(QString::fromStdString(report_name));

    base_dir_label_->setVisible(export_mode_ != ReportExportMode::JSONBlob);
    base_dir_button_->setVisible(export_mode_ != ReportExportMode::JSONBlob);
    base_dir_edit_->setVisible(export_mode_ != ReportExportMode::JSONBlob);
    res_dir_name_label_->setVisible(export_mode_ != ReportExportMode::JSONBlob);
    res_dir_name_edit_->setVisible(export_mode_ != ReportExportMode::JSONBlob);
    report_name_label_->setVisible(export_mode_ != ReportExportMode::JSONBlob);
    report_name_edit_->setVisible(export_mode_ != ReportExportMode::JSONBlob);

    latex_max_rows_label_->setVisible(export_mode_ == ReportExportMode::Latex ||
                                      export_mode_ == ReportExportMode::LatexPDF);
    latex_max_rows_edit_->setVisible(export_mode_ == ReportExportMode::Latex ||
                                     export_mode_ == ReportExportMode::LatexPDF);

    latex_max_colw_label_->setVisible(export_mode_ == ReportExportMode::Latex ||
                                      export_mode_ == ReportExportMode::LatexPDF);
    latex_max_colw_edit_->setVisible(export_mode_ == ReportExportMode::Latex ||
                                     export_mode_ == ReportExportMode::LatexPDF);

    open_file_label_->setVisible(export_mode_ == ReportExportMode::LatexPDF);
    open_file_box_->setVisible(export_mode_ == ReportExportMode::LatexPDF);

    loadSettings();
}

/**
 */
void ReportExportDialog::loadSettings()
{
    const auto& settings = report_export_.settings();

    author_edit_->setText(QString::fromStdString(settings.author));

    open_file_box_->setChecked(settings.open_created_file);

    latex_max_rows_edit_->setValue(settings.latex_table_max_rows);
    latex_max_colw_edit_->setValue(settings.latex_table_max_col_width);
}

/**
 */
void ReportExportDialog::writeSettings()
{
    auto& settings = report_export_.settings();

    settings.author   = author_edit_->text().toStdString();
    settings.abstract = comment_edit_->toPlainText().toStdString();

    settings.open_created_file = open_file_box_->isChecked();

    settings.latex_table_max_rows      = latex_max_rows_edit_->value();
    settings.latex_table_max_col_width = latex_max_colw_edit_->value();
}

/**
 */
void ReportExportDialog::checkExport()
{
    bool can_export = !res_dir_name_edit_->text().isEmpty() &&
                      !base_dir_edit_->text().isEmpty() &&
                      !report_name_edit_->text().isEmpty();

    export_button_->setEnabled(can_export);
}

/**
 */
void ReportExportDialog::exportReport()
{
    writeSettings();

    progress_bar_->setValue(0);
    status_label_->setText("");

    export_button_->setEnabled(false);
    cancel_button_->setEnabled(false);

    auto dir = base_dir_edit_->text().toStdString() + "/" +
               res_dir_name_edit_->text().toStdString();
    auto fn  = report_name_edit_->text().toStdString();

    QApplication::processEvents();

    export_result_ = report_export_.exportReport(task_result_, export_mode_, fn, dir);

    QApplication::processEvents();

    if (!export_result_.ok())
    {
        logerr << "ReportExportDialog: exportReport: Error: " << export_result_.error();

        progress_bar_->setValue(0);
        status_label_->setText("Exporting report failed");
    }

    export_button_->setEnabled(true);
    cancel_button_->setEnabled(true);

    if (export_result_.ok() || no_interaction_mode_)
        accept();
}

/**
 */
void ReportExportDialog::cancel()
{
    writeSettings();
    reject();
}

/**
 */
void ReportExportDialog::updateProgress()
{
    status_label_->setText(QString::fromStdString(report_export_.status()));
    progress_bar_->setValue(progress_bar_->maximum() * report_export_.progress());
}

/**
 */
void ReportExportDialog::editBaseDir()
{
    auto dir = QFileDialog::getExistingDirectory(this, "Select Base Directory", base_dir_edit_->text());
    if(dir.isEmpty())
        return;

    base_dir_edit_->setText(dir);

    checkExport();
}

}
