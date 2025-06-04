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

#pragma once

#include "reportdefs.h"

#include "result.h"

#include <QDialog>

#include "json.hpp"

class QToolButton;
class QLineEdit;
class QTextEdit;
class QSpinBox;
class QPushButton;
class QLabel;
class QProgressBar;
class QCheckBox;

class TaskResult;

namespace ResultReport
{

class ReportExport;

/**
 */
class ReportExportDialog : public QDialog
{
public:
    ReportExportDialog(TaskResult& task_result,
                       ReportExport& report_export, 
                       ReportExportMode export_mode,
                       bool no_interaction_mode,
                       QWidget* parent = nullptr, 
                       Qt::WindowFlags f = Qt::WindowFlags());
    virtual ~ReportExportDialog() = default;

    const ResultT<nlohmann::json>& result() const { return export_result_; }

protected:
    void createUI();
    void configureUI();
    void loadSettings();
    void writeSettings();

    void editBaseDir();

    void exportReport();
    void cancel();

    void checkExport();
    void updateProgress();

    void showEvent(QShowEvent *event) override;

    TaskResult&      task_result_;
    ReportExport&    report_export_;
    ReportExportMode export_mode_;
    bool             no_interaction_mode_;

    QString          format_str_;

    QLabel*          base_dir_label_       = nullptr;
    QPushButton*     base_dir_button_      = nullptr;
    QLineEdit*       base_dir_edit_        = nullptr;
    QLabel*          res_dir_name_label_   = nullptr;
    QLineEdit*       res_dir_name_edit_    = nullptr;
    QLabel*          report_name_label_    = nullptr;
    QLineEdit*       report_name_edit_     = nullptr;

    QLineEdit*       author_edit_          = nullptr;
    QTextEdit*       comment_edit_         = nullptr;

    QPushButton*     export_button_        = nullptr;
    QPushButton*     cancel_button_        = nullptr;

    QLabel*          elapsed_label_        = nullptr;
    QLabel*          remaining_label_      = nullptr;
    QLabel*          status_label_         = nullptr;
    QProgressBar*    progress_bar_         = nullptr;

    QLabel*          open_file_label_      = nullptr;
    QCheckBox*       open_file_box_        = nullptr;
    
    QLabel*          latex_max_rows_label_ = nullptr;
    QSpinBox*        latex_max_rows_edit_  = nullptr;
    QLabel*          latex_max_colw_label_ = nullptr;
    QSpinBox*        latex_max_colw_edit_  = nullptr;

    ResultT<nlohmann::json> export_result_;
};

}
