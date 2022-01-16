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

#include "histogramviewconfigwidget.h"
#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableselectionwidget.h"
#include "histogramview.h"
#include "histogramviewdatasource.h"
#include "logger.h"
#include "stringconv.h"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTabWidget>


using namespace Utils;

HistogramViewConfigWidget::HistogramViewConfigWidget(HistogramView* view, QWidget* parent)
    : QWidget(parent), view_(view)
{
    assert(view_);

    setMinimumWidth(400);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(0, 0, 0, 0);

    QTabWidget* tab_widget = new QTabWidget(this);
    tab_widget->setStyleSheet("QTabBar::tab { height: 42px; }");

    // config
    {
        QWidget* cfg_widget = new QWidget();
        QVBoxLayout* cfg_layout = new QVBoxLayout();


        // data variable
        selected_var_check_ = new QCheckBox("Show Variable Data");
        selected_var_check_->setChecked(!view_->showResults());
        connect(selected_var_check_, &QCheckBox::clicked, this,
                &HistogramViewConfigWidget::showSelectedVariableDataSlot);
        cfg_layout->addWidget(selected_var_check_);

        select_var_ = new dbContent::VariableSelectionWidget();
        select_var_->showMetaVariables(true);
        select_var_->showDataTypesOnly({PropertyDataType::BOOL,
                                        PropertyDataType::CHAR,
                                        PropertyDataType::UCHAR,
                                        PropertyDataType::INT,
                                        PropertyDataType::UINT,
                                        PropertyDataType::LONGINT,
                                        PropertyDataType::ULONGINT,
                                        PropertyDataType::FLOAT,
                                        PropertyDataType::DOUBLE});
        if (view_->hasDataVar())
        {
            if (view_->isDataVarMeta())
                select_var_->selectedMetaVariable(view_->metaDataVar());
            else
                select_var_->selectedVariable(view_->dataVar());
        }
        connect(select_var_, &dbContent::VariableSelectionWidget::selectionChanged, this,
                &HistogramViewConfigWidget::selectedVariableChangedSlot);
        cfg_layout->addWidget(select_var_);

        // eval
        {
            eval_results_check_ = new QCheckBox("Show Evaluation Result Data");
            eval_results_check_->setChecked(!view_->showResults());
            connect(eval_results_check_, &QCheckBox::clicked, this,
                    &HistogramViewConfigWidget::showEvaluationResultDataSlot);
            cfg_layout->addWidget(eval_results_check_);


            QVBoxLayout* eval_layout = new QVBoxLayout();

            QFormLayout* eval_form_layout = new QFormLayout;
            eval_form_layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);

            eval_results_grpreq_label_ = new QLabel();
            eval_results_grpreq_label_->setWordWrap(true);

            eval_form_layout->addRow(tr("Requirement"), eval_results_grpreq_label_);

            eval_results_id_label_ = new QLabel();
            eval_results_id_label_->setWordWrap(true);

            eval_form_layout->addRow(tr("Result"), eval_results_id_label_);

            eval_layout->addLayout(eval_form_layout);

            cfg_layout->addLayout(eval_layout);

            updateEvalConfig();
        }


        log_check_ = new QCheckBox("Logarithmic Y Scale");
        log_check_->setChecked(view_->useLogScale());
        connect(log_check_, &QCheckBox::clicked, this,
                &HistogramViewConfigWidget::toggleLogScale);
        cfg_layout->addWidget(log_check_);

        cfg_layout->addStretch();

        cfg_widget->setLayout(cfg_layout);

        tab_widget->addTab(cfg_widget, "Config");
    }

    vlayout->addWidget(tab_widget);

    QFont font_status;
    font_status.setItalic(true);

    status_label_ = new QLabel();
    status_label_->setFont(font_status);
    status_label_->setVisible(false);
    vlayout->addWidget(status_label_);

    reload_button_ = new QPushButton("Reload");
    connect(reload_button_, &QPushButton::clicked, this,
            &HistogramViewConfigWidget::reloadRequestedSlot);
    vlayout->addWidget(reload_button_);

    setLayout(vlayout);

    setStatus("No Data Loaded", true);
}

HistogramViewConfigWidget::~HistogramViewConfigWidget() {}

void HistogramViewConfigWidget::showSelectedVariableDataSlot()
{
    loginf << "HistogramViewConfigWidget: showSelectedVariableDataSlot";

    if (selected_var_check_->checkState() == Qt::Unchecked) // only unchecked with other checkbox
        selected_var_check_->setChecked(true);
    else
        view_->showResults(false);
}

void HistogramViewConfigWidget::showEvaluationResultDataSlot()
{
    loginf << "HistogramViewConfigWidget: showEvaluationResultDataSlot";

    if (eval_results_check_->checkState() == Qt::Unchecked) // only unchecked with other checkbox
        eval_results_check_->setChecked(true);
    else
        view_->showResults(true);
}

void HistogramViewConfigWidget::selectedVariableChangedSlot()
{
    loginf << "HistogramViewConfigWidget: selectedVariableChangedSlot";

    if (select_var_->hasVariable())
        view_->dataVar(select_var_->selectedVariable());
    else if (select_var_->hasMetaVariable())
        view_->metaDataVar(select_var_->selectedMetaVariable());

}

//void HistogramViewConfigWidget::exportSlot()
//{
//    logdbg << "HistogramViewConfigWidget: exportSlot";
//    //assert(overwrite_check_);
//    assert(export_button_);

//    export_button_->setDisabled(true);
//    //emit exportSignal(overwrite_check_->checkState() == Qt::Checked);
//}

//void HistogramViewConfigWidget::exportDoneSlot(bool cancelled)
//{
//    assert(export_button_);

//    export_button_->setDisabled(false);

//    if (!cancelled)
//    {
//        QMessageBox msgBox;
//        msgBox.setText("Export complete.");
//        msgBox.exec();
//    }
//}

void HistogramViewConfigWidget::toggleLogScale()
{
    assert(log_check_);
    bool checked = log_check_->checkState() == Qt::Checked;
    logdbg << "HistogramViewConfigWidget: toggleLogScale: setting overwrite to " << checked;
    view_->useLogScale(checked);
}

void HistogramViewConfigWidget::updateEvalConfig()
{
    loginf << "HistogramViewConfigWidget: updateEvalConfig";

    // grp_req
    assert (eval_results_grpreq_label_);
    assert (eval_results_id_label_);

    if (view_->evalResultGrpReq().size())
        eval_results_grpreq_label_->setText(view_->evalResultGrpReq().c_str());
    else
        eval_results_grpreq_label_->setText("None");

    if (view_->evalResultsID().size())
        eval_results_id_label_->setText(view_->evalResultsID().c_str());
    else
        eval_results_id_label_->setText("None");

    eval_results_check_->setEnabled(view_->evalResultGrpReq().size() && view_->evalResultsID().size());
    eval_results_check_->setChecked(view_->showResults());

    selected_var_check_->setChecked(!view_->showResults());
}

void HistogramViewConfigWidget::setStatus (const std::string& status, bool visible, QColor color)
{
    assert (status_label_);
    status_label_->setText(status.c_str());
    //status_label_->setStyleSheet("QLabel { color : "+color.name()+"; }");

    QPalette palette = status_label_->palette();
    palette.setColor(status_label_->foregroundRole(), color);
    status_label_->setPalette(palette);

    status_label_->setVisible(visible);
}

void HistogramViewConfigWidget::reloadRequestedSlot()
{
    COMPASS::instance().objectManager().load();
}

void HistogramViewConfigWidget::loadingStartedSlot()
{
    setDisabled(true); // reenabled in view

    setStatus("Loading Data", true);
}
