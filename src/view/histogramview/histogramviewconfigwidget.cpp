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
#include "histogramviewdatawidget.h"
#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableselectionwidget.h"
#include "histogramview.h"
#include "histogramviewdatasource.h"
#include "logger.h"
#include "stringconv.h"
#include "groupbox.h"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QTabWidget>
#include <QRadioButton>

using namespace Utils;

/**
 */
HistogramViewConfigWidget::HistogramViewConfigWidget(HistogramView* view, QWidget* parent)
    : ViewConfigWidget(parent), view_(view)
{
    assert(view_);

    setMinimumWidth(400);

    QFont font_bold;
    font_bold.setBold(true);

    QVBoxLayout* vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(0, 0, 0, 0);

    QTabWidget* tab_widget = new QTabWidget(this);
    tab_widget->setStyleSheet("QTabBar::tab { height: 42px; }");

    bool show_result = view_->showResults() && view_->hasResultID();

    // config
    {
        QWidget*     cfg_widget = new QWidget;
        QVBoxLayout* cfg_layout = new QVBoxLayout;

        // data variable
        {
            selected_var_widget_ = new QWidget;
            selected_var_check_  = new QRadioButton("Show Variable Data");
            selected_var_check_->setChecked(!show_result);

            connect(selected_var_check_, &QRadioButton::toggled, this,
                    &HistogramViewConfigWidget::dataSourceToggled);

            QVBoxLayout* selected_var_layout = new QVBoxLayout;
            selected_var_layout->setMargin(10);
            selected_var_widget_->setLayout(selected_var_layout);

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

            selected_var_layout->addWidget(select_var_);

            cfg_layout->addWidget(selected_var_check_);
            cfg_layout->addWidget(selected_var_widget_);
        }

        // eval
        {
            eval_results_widget_ = new QWidget;
            eval_results_check_  = new QRadioButton("Show Evaluation Result Data");
            eval_results_check_->setChecked(show_result);

            connect(eval_results_check_, &QRadioButton::toggled, this,
                    &HistogramViewConfigWidget::dataSourceToggled);
            
            QVBoxLayout* eval_results_layout = new QVBoxLayout;
            eval_results_layout->setMargin(10);
            eval_results_widget_->setLayout(eval_results_layout);

            QFormLayout* eval_form_layout = new QFormLayout;
            eval_form_layout->setFormAlignment(Qt::AlignRight | Qt::AlignTop);

            eval_results_grpreq_label_ = new QLabel();
            eval_results_grpreq_label_->setWordWrap(true);

            eval_form_layout->addRow(tr("Requirement"), eval_results_grpreq_label_);

            eval_results_id_label_ = new QLabel();
            eval_results_id_label_->setWordWrap(true);

            eval_form_layout->addRow(tr("Result"), eval_results_id_label_);

            eval_results_layout->addLayout(eval_form_layout);

            cfg_layout->addWidget(eval_results_check_);
            cfg_layout->addWidget(eval_results_widget_);
        }

        //general
        {
            log_check_ = new QCheckBox("Logarithmic Y Scale");
            log_check_->setChecked(view_->useLogScale());

            connect(log_check_, &QCheckBox::clicked, this,
                    &HistogramViewConfigWidget::toggleLogScale);

            cfg_layout->addWidget(log_check_);
        }

        cfg_layout->addStretch();

        updateConfig();
        
        cfg_widget->setLayout(cfg_layout);

        tab_widget->addTab(cfg_widget, "Config");
    }

    vlayout->addWidget(tab_widget);

    //info widget
    {
        info_widget_ = new GroupBox("Histogram Range");

        info_range_min_label_ = new QLabel("-");
        info_range_max_label_ = new QLabel("-");
        info_oor_label_       = new QLabel("None");
        info_zoom_label       = new QLabel("Zoom active");

        QFont f = info_zoom_label->font();
        f.setItalic(true);

        info_zoom_label->setFont(f);
        info_zoom_label->setVisible(false);

        QGridLayout* layout = new QGridLayout;
        info_widget_->setLayout(layout);

        QLabel* min_label = new QLabel("Minimum:");
        QLabel* max_label = new QLabel("Maximum:");
        QLabel* oor_label = new QLabel("Out of range values:");
        min_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        max_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        oor_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

        layout->addWidget(min_label, 0, 0);
        layout->addWidget(max_label, 1, 0);
        layout->addWidget(oor_label, 2, 0);
        layout->addWidget(info_range_min_label_, 0, 1);
        layout->addWidget(info_range_max_label_, 1, 1);
        layout->addWidget(info_oor_label_      , 2, 1);
        layout->addWidget(info_zoom_label      , 3, 1);

        vlayout->addWidget(info_widget_);
        vlayout->addSpacerItem(new QSpacerItem(5, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));

        updateInfo();
    }

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

/**
 */
HistogramViewConfigWidget::~HistogramViewConfigWidget() {}

/**
 */
void HistogramViewConfigWidget::selectedVariableChangedSlot()
{
    loginf << "HistogramViewConfigWidget: selectedVariableChangedSlot";

    if (select_var_->hasVariable())
        view_->dataVar(select_var_->selectedVariable());
    else if (select_var_->hasMetaVariable())
        view_->metaDataVar(select_var_->selectedMetaVariable());
}

/**
 */
//void HistogramViewConfigWidget::exportSlot()
//{
//    logdbg << "HistogramViewConfigWidget: exportSlot";
//    //assert(overwrite_check_);
//    assert(export_button_);

//    export_button_->setDisabled(true);
//    //emit exportSignal(overwrite_check_->checkState() == Qt::Checked);
//}

/**
 */
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

/**
 */
void HistogramViewConfigWidget::toggleLogScale()
{
    assert(log_check_);
    bool checked = log_check_->checkState() == Qt::Checked;
    logdbg << "HistogramViewConfigWidget: toggleLogScale: setting overwrite to " << checked;
    view_->useLogScale(checked);
}

/**
 */
void HistogramViewConfigWidget::dataSourceToggled()
{
    //modify state in view based on selected radio button
    bool show_results  = eval_results_check_->isChecked();
    view_->showResults(show_results);

    //then update config
    updateConfig();
}

/**
 */
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
}

/**
 */
void HistogramViewConfigWidget::updateConfig()
{
    bool show_results  = view_->showResults();
    bool has_result_id = view_->evalResultGrpReq().size() && view_->evalResultsID().size();

    eval_results_check_->blockSignals(true);
    eval_results_check_->setChecked(show_results);
    eval_results_check_->blockSignals(false);

    selected_var_check_->blockSignals(true);
    selected_var_check_->setChecked(!show_results);
    selected_var_check_->blockSignals(false);

    updateEvalConfig();

    eval_results_check_->setEnabled(has_result_id);

    eval_results_widget_->setEnabled(show_results);
    selected_var_widget_->setEnabled(!show_results);
}

/**
 */
void HistogramViewConfigWidget::setStatus (const QString& text, bool visible, const QColor& color)
{
    assert (status_label_);

    status_label_->setText(text);
    //status_label_->setStyleSheet("QLabel { color : "+color.name()+"; }");

    QPalette palette = status_label_->palette();
    palette.setColor(status_label_->foregroundRole(), color);

    status_label_->setPalette(palette);
    status_label_->setVisible(visible);
}

/**
 */
void HistogramViewConfigWidget::appModeSwitch (AppMode app_mode)
{
    assert (reload_button_);
    reload_button_->setHidden(app_mode == AppMode::LiveRunning);
    assert (status_label_);
    status_label_->setHidden(app_mode == AppMode::LiveRunning);
}

/**
 */
void HistogramViewConfigWidget::reloadRequestedSlot()
{
    COMPASS::instance().dbContentManager().load();
}

/**
 */
void HistogramViewConfigWidget::loadingStartedSlot()
{
    setDisabled(true); // reenabled in view

    setStatus("Loading Data", true);
}

/**
 */
void HistogramViewConfigWidget::onDisplayChange_impl() 
{
    updateInfo();
}

/**
 */
void HistogramViewConfigWidget::updateInfo()
{
    HistogramViewDataWidget::ViewInfo info;

    if (view_ && view_->hasDataWidget())
        info = view_->getDataWidget()->getViewInfo();

    auto setItalic = [ = ] (QLabel* label, bool ok) 
    {
        QFont f = label->font();
        f.setItalic(ok);
        label->setFont(f);
    };

    auto setText = [ = ] (QLabel* label, const QString& txt, bool italic)
    {
        label->setText(txt);
        setItalic(label, italic);
    };

    if (!info.has_result)
    {
        setText(info_range_min_label_, "-", false);
        setText(info_range_max_label_, "-", false);
        setText(info_oor_label_      , "-", false);

        info_zoom_label->setVisible(false);
    }
    else 
    {
        setText(info_range_min_label_, info.min.isEmpty()     ? "Not available" : info.min, info.min.isEmpty());
        setText(info_range_max_label_, info.max.isEmpty()     ? "Not available" : info.max, info.max.isEmpty());
        setText(info_oor_label_      , QString::number(info.out_of_range), false);

        info_zoom_label->setVisible(info.zoom_active);
    }
}
