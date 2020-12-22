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

    log_check_ = new QCheckBox("Logarithmic Y Scale");
    log_check_->setChecked(view_->useLogScale());
    connect(log_check_, &QCheckBox::clicked, this,
            &HistogramViewConfigWidget::toggleLogScale);
    vlayout->addWidget(log_check_);

//    export_button_ = new QPushButton("Export");
//    connect(export_button_, SIGNAL(clicked(bool)), this, SLOT(exportSlot()));
//    vlayout->addWidget(export_button_);

    vlayout->addStretch();

    reload_button_ = new QPushButton("Reload");
    connect(reload_button_, &QPushButton::clicked, this,
            &HistogramViewConfigWidget::reloadRequestedSlot);
    vlayout->addWidget(reload_button_);

    setLayout(vlayout);
}

HistogramViewConfigWidget::~HistogramViewConfigWidget() {}

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

void HistogramViewConfigWidget::reloadRequestedSlot()
{
    emit reloadRequestedSignal();
}

void HistogramViewConfigWidget::loadingStartedSlot()
{
    setDisabled(true); // reenabled in view
}
