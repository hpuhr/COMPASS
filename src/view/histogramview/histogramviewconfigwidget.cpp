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
#include "histogramviewwidget.h"
#include "histogramviewdatawidget.h"
//#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableselectionwidget.h"
#include "histogramview.h"
//#include "histogramviewdatasource.h"
#include "logger.h"
//#include "stringconv.h"
#include "groupbox.h"
#include "ui_test_common.h"
#include "metavariable.h"

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
HistogramViewConfigWidget::HistogramViewConfigWidget(HistogramViewWidget* view_widget, 
                                                     QWidget* parent)
:   VariableViewConfigWidget(view_widget, view_widget->getView(), parent)
{
    view_ = view_widget->getView();
    assert(view_);

    //log scale
    {
        auto config_layout = configLayout();

        log_check_ = new QCheckBox("Logarithmic Y Scale");
        UI_TEST_OBJ_NAME(log_check_, log_check_->text())

        updateLogScale();

        connect(log_check_, &QCheckBox::clicked, this,
                &HistogramViewConfigWidget::toggleLogScale);

        config_layout->addWidget(log_check_);
    }

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

        getMainLayout()->addWidget(info_widget_);
        getMainLayout()->addSpacerItem(new QSpacerItem(5, 10, QSizePolicy::Fixed, QSizePolicy::Fixed));

        updateInfo();
    }
}

/**
 */
HistogramViewConfigWidget::~HistogramViewConfigWidget() = default;

/**
 */
void HistogramViewConfigWidget::toggleLogScale()
{
    assert(log_check_);
    bool checked = log_check_->checkState() == Qt::Checked;
    logdbg << "HistogramViewConfigWidget: toggleLogScale: setting overwrite to " << checked;
    view_->useLogScale(checked, true);
}

/**
 */
void HistogramViewConfigWidget::onDisplayChange_impl()
{
    updateInfo();
    updateLogScale();
}

/**
 */
void HistogramViewConfigWidget::updateInfo()
{
    auto data_widget = dynamic_cast<HistogramViewDataWidget*>(getWidget()->getViewDataWidget());
    
    HistogramViewDataWidget::ViewInfo info = data_widget->getViewInfo();

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

/**
 */
void HistogramViewConfigWidget::configChanged_impl()
{
    updateLogScale();
}

/**
 */
void HistogramViewConfigWidget::updateLogScale()
{
    log_check_->setChecked(view_->useLogScale());
}

/**
 */
void HistogramViewConfigWidget::viewInfoJSON_impl(nlohmann::json& info) const
{
    //!call base!
    VariableViewConfigWidget::viewInfoJSON_impl(info);

    info[ "log_enabled" ] = log_check_->isChecked();

    info[ "info_zoom_active" ] = info_zoom_label->isVisible();
    info[ "info_range_min"   ] = info_range_min_label_->text().toStdString();
    info[ "info_range_max"   ] = info_range_max_label_->text().toStdString();
    info[ "info_oor_count"   ] = info_oor_label_->text().toStdString();
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