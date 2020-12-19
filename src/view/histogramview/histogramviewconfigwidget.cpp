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

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>

#include "dbobjectmanager.h"
#include "dbovariableselectionwidget.h"
#include "histogramview.h"
#include "histogramviewdatasource.h"
#include "logger.h"
#include "stringconv.h"

using namespace Utils;

HistogramViewConfigWidget::HistogramViewConfigWidget(HistogramView* view, QWidget* parent)
    : QWidget(parent), view_(view)
{
    QVBoxLayout* vlayout = new QVBoxLayout;

    assert(view_);

    select_var_ = new DBOVariableSelectionWidget();
    select_var_->showMetaVariables(true);
    if (view_->hasDataVar())
    {
        if (view_->isDataVarMeta())
            select_var_->selectedMetaVariable(view_->metaDataVar());
        else
            select_var_->selectedVariable(view_->dataVar());
    }
    connect(select_var_, &DBOVariableSelectionWidget::selectionChanged, this,
            &HistogramViewConfigWidget::selectedVariableChangedSlot);
    vlayout->addWidget(select_var_);

//    only_selected_check_ = new QCheckBox("Show Only Selected");
//    only_selected_check_->setChecked(view_->showOnlySelected());
//    connect(only_selected_check_, &QCheckBox::clicked, this,
//            &HistogramViewConfigWidget::toggleShowOnlySeletedSlot);
//    vlayout->addWidget(only_selected_check_);

    export_button_ = new QPushButton("Export");
    connect(export_button_, SIGNAL(clicked(bool)), this, SLOT(exportSlot()));
    vlayout->addWidget(export_button_);

    vlayout->addStretch();

    update_button_ = new QPushButton("Reload");
    connect(update_button_, &QPushButton::clicked, this,
            &HistogramViewConfigWidget::reloadRequestedSlot);
    update_button_->setDisabled(true);
    vlayout->addWidget(update_button_);

    setLayout(vlayout);
}

HistogramViewConfigWidget::~HistogramViewConfigWidget() {}

//void HistogramViewConfigWidget::toggleShowOnlySeletedSlot()
//{
//    assert(only_selected_check_);
//    bool checked = only_selected_check_->checkState() == Qt::Checked;
//    loginf << "HistogramViewConfigWidget: toggleShowOnlySeletedSlot: setting to " << checked;
//    view_->showOnlySelected(checked);
//}

//void HistogramViewConfigWidget::toggleUseOverwrite()
//{
//    assert(overwrite_check_);
//    bool checked = overwrite_check_->checkState() == Qt::Checked;
//    logdbg << "HistogramViewConfigWidget: toggleUseOverwrite: setting overwrite to " << checked;
//    view_->overwriteCSV(checked);
//}

void HistogramViewConfigWidget::selectedVariableChangedSlot()
{

}

void HistogramViewConfigWidget::exportSlot()
{
    logdbg << "HistogramViewConfigWidget: exportSlot";
    //assert(overwrite_check_);
    assert(export_button_);

    export_button_->setDisabled(true);
    //emit exportSignal(overwrite_check_->checkState() == Qt::Checked);
}

void HistogramViewConfigWidget::exportDoneSlot(bool cancelled)
{
    assert(export_button_);

    export_button_->setDisabled(false);

    if (!cancelled)
    {
        QMessageBox msgBox;
        msgBox.setText("Export complete.");
        msgBox.exec();
    }
}

void HistogramViewConfigWidget::reloadWantedSlot()
{
    reload_needed_ = true;
    updateUpdateButton();
}

void HistogramViewConfigWidget::reloadRequestedSlot()
{
    assert(reload_needed_);
    emit reloadRequestedSignal();
    reload_needed_ = false;

    updateUpdateButton();
}

void HistogramViewConfigWidget::updateUpdateButton()
{
    assert(update_button_);
    update_button_->setEnabled(reload_needed_);
}

void HistogramViewConfigWidget::loadingStartedSlot()
{
    reload_needed_ = false;

    updateUpdateButton();
}
