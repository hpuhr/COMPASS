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

#include "scatterplotviewconfigwidget.h"
#include "dbobjectmanager.h"
#include "dbovariableselectionwidget.h"
#include "scatterplotview.h"
#include "scatterplotviewdatasource.h"
#include "logger.h"
#include "stringconv.h"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTabWidget>

using namespace Utils;

ScatterPlotViewConfigWidget::ScatterPlotViewConfigWidget(ScatterPlotView* view, QWidget* parent)
    : QWidget(parent), view_(view)
{
    //QVBoxLayout* vlayout = new QVBoxLayout;

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

        cfg_layout->addWidget(new QLabel("X Variable"));

        select_var_x_ = new DBOVariableSelectionWidget();
        select_var_x_->showMetaVariables(true);
        select_var_x_->showDataTypesOnly({PropertyDataType::BOOL,
                                          PropertyDataType::CHAR,
                                          PropertyDataType::UCHAR,
                                          PropertyDataType::INT,
                                          PropertyDataType::UINT,
                                          PropertyDataType::LONGINT,
                                          PropertyDataType::ULONGINT,
                                          PropertyDataType::FLOAT,
                                          PropertyDataType::DOUBLE});
        if (view_->hasDataVarX())
        {
            if (view_->isDataVarXMeta())
                select_var_x_->selectedMetaVariable(view_->metaDataVarX());
            else
                select_var_x_->selectedVariable(view_->dataVarX());
        }
        connect(select_var_x_, &DBOVariableSelectionWidget::selectionChanged, this,
                &ScatterPlotViewConfigWidget::selectedVariableXChangedSlot);
        cfg_layout->addWidget(select_var_x_);

        cfg_layout->addWidget(new QLabel("Y Variable"));

        select_var_y_ = new DBOVariableSelectionWidget();
        select_var_y_->showMetaVariables(true);
        select_var_y_->showDataTypesOnly({PropertyDataType::BOOL,
                                          PropertyDataType::CHAR,
                                          PropertyDataType::UCHAR,
                                          PropertyDataType::INT,
                                          PropertyDataType::UINT,
                                          PropertyDataType::LONGINT,
                                          PropertyDataType::ULONGINT,
                                          PropertyDataType::FLOAT,
                                          PropertyDataType::DOUBLE});
        if (view_->hasDataVarY())
        {
            if (view_->isDataVarYMeta())
                select_var_y_->selectedMetaVariable(view_->metaDataVarY());
            else
                select_var_y_->selectedVariable(view_->dataVarY());
        }
        connect(select_var_y_, &DBOVariableSelectionWidget::selectionChanged, this,
                &ScatterPlotViewConfigWidget::selectedVariableYChangedSlot);
        cfg_layout->addWidget(select_var_y_);

        cfg_layout->addStretch();

        cfg_widget->setLayout(cfg_layout);

        tab_widget->addTab(cfg_widget, "Config");
    }

    vlayout->addWidget(tab_widget);

    reload_button_ = new QPushButton("Reload");
    connect(reload_button_, &QPushButton::clicked, this,
            &ScatterPlotViewConfigWidget::reloadRequestedSlot);
    vlayout->addWidget(reload_button_);

    setLayout(vlayout);
}

ScatterPlotViewConfigWidget::~ScatterPlotViewConfigWidget() {}

void ScatterPlotViewConfigWidget::selectedVariableXChangedSlot()
{
    loginf << "ScatterPlotViewConfigWidget: selectedVariableChangedSlot";

    if (select_var_x_->hasVariable())
        view_->dataVarX(select_var_x_->selectedVariable());
    else if (select_var_x_->hasMetaVariable())
        view_->metaDataVarX(select_var_x_->selectedMetaVariable());

}

void ScatterPlotViewConfigWidget::selectedVariableYChangedSlot()
{
    loginf << "ScatterPlotViewConfigWidget: selectedVariableChangedSlot";

    if (select_var_y_->hasVariable())
        view_->dataVarY(select_var_y_->selectedVariable());
    else if (select_var_y_->hasMetaVariable())
        view_->metaDataVarY(select_var_y_->selectedMetaVariable());

}

//void ScatterPlotViewConfigWidget::exportSlot()
//{
//    logdbg << "ScatterPlotViewConfigWidget: exportSlot";
//    //assert(overwrite_check_);
//    assert(export_button_);

//    export_button_->setDisabled(true);
//    //emit exportSignal(overwrite_check_->checkState() == Qt::Checked);
//}

//void ScatterPlotViewConfigWidget::exportDoneSlot(bool cancelled)
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

void ScatterPlotViewConfigWidget::reloadRequestedSlot()
{
    emit reloadRequestedSignal();
}

void ScatterPlotViewConfigWidget::loadingStartedSlot()
{
    setDisabled(true); // reenabled in view
}
