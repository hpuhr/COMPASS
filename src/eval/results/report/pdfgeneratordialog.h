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

#ifndef EVALUATIONRESULTSREPORTPDFGENERATORDIALOG_H
#define EVALUATIONRESULTSREPORTPDFGENERATORDIALOG_H

#include <QDialog>

class EvaluationManager;

class QPushButton;
class QLabel;
class QProgressBar;
class QLineEdit;
class QCheckBox;

namespace EvaluationResultsReport
{
class PDFGenerator;

class PDFGeneratorDialog : public QDialog
{
    Q_OBJECT

public slots:
    void setPathSlot ();
    void pathEditedSlot ();
    void filenameEditedSlot();

    void authorEditedSlot (const QString& text);
    void abstractEditedSlot(const QString& text);

    void waitOnMapLoadingEditedSlot(bool checked);
    void includeTargetDetailsEditedSlot(bool checked);
    void skipTargetDetailsWOIssuesEditedSlot(bool checked);
    void includeTargetTRDetailsEditedSlot(bool checked);
    void numMaxTableRowsEditedSlot(const QString& text);
    void numMaxTableColWidthEditedSlot(const QString& text);

    void runPDFLatexChangedSlot (bool checked);
    void openPDFChangedSlot (bool checked);

    void runSlot();
    void cancelSlot();

public:
    PDFGeneratorDialog(PDFGenerator& generator, EvaluationManager& eval_man,
                       QWidget* parent = nullptr, Qt::WindowFlags f = 0);

    void updateFileInfo ();

    void setRunning (bool value);

    void setElapsedTime (const std::string& time_str);
    void setProgress (unsigned int min, unsigned int max, unsigned int value);
    void setStatus (const std::string& status);
    void setRemainingTime (const std::string& time_str);

protected:
    PDFGenerator& generator_;
    EvaluationManager& eval_man_;

    QWidget* config_container_ {nullptr};

    QLineEdit* directory_edit_ {nullptr};
    QLineEdit* filename_edit_ {nullptr};

    QLineEdit* author_edit_ {nullptr};
    QLineEdit* abstract_edit_ {nullptr};

    QCheckBox* include_target_details_check_ {nullptr};
    QCheckBox* skip_target_details_wo_issues_check_ {nullptr};
    QCheckBox* include_target_tr_details_check_ {nullptr};

    QLineEdit* num_max_table_rows_edit_ {nullptr};
    QLineEdit* num_max_table_col_width_edit_ {nullptr};

    QCheckBox* wait_on_map_loading_check_ {nullptr};

    QCheckBox* pdflatex_check_ {nullptr};
    QCheckBox* open_pdf_check_ {nullptr};

    QPushButton* run_button_{nullptr};

    QLabel* elapsed_time_label_{nullptr};
    QProgressBar* progress_bar_{nullptr};
    QLabel* status_label_{nullptr};
    QLabel* remaining_time_label_{nullptr};

    QPushButton* quit_button_{nullptr};
};

}

#endif // EVALUATIONRESULTSREPORTPDFGENERATORDIALOG_H
