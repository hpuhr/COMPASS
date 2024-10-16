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
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableselectionwidget.h"
#include "scatterplotviewwidget.h"
#include "scatterplotview.h"
#include "logger.h"
#include "variable.h"
#include "metavariable.h"
#include "ui_test_common.h"

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTabWidget>

using namespace Utils;
using namespace dbContent;

ScatterPlotViewConfigWidget::ScatterPlotViewConfigWidget(ScatterPlotViewWidget* view_widget, QWidget* parent)
:   TabStyleViewConfigWidget(view_widget, parent)
{
    //QVBoxLayout* vlayout = new QVBoxLayout;

    QFont font_bold;
    font_bold.setBold(true);

    view_ = view_widget->getView();
    assert(view_);

    // config
    {
        QWidget* cfg_widget = new QWidget();
        QVBoxLayout* cfg_layout = new QVBoxLayout();

        cfg_layout->addWidget(new QLabel("X Variable"));

        select_var_x_ = new VariableSelectionWidget();
        select_var_x_->showMetaVariables(true);
        select_var_x_->showEmptyVariable(false);
        select_var_x_->showDataTypesOnly({PropertyDataType::BOOL,
                                          PropertyDataType::CHAR,
                                          PropertyDataType::UCHAR,
                                          PropertyDataType::INT,
                                          PropertyDataType::UINT,
                                          PropertyDataType::LONGINT,
                                          PropertyDataType::ULONGINT,
                                          PropertyDataType::FLOAT,
                                          PropertyDataType::DOUBLE,
                                          PropertyDataType::TIMESTAMP});
        select_var_x_->setObjectName("variable_selection_x");

        updateSelectedVarX();
        connect(select_var_x_, &VariableSelectionWidget::selectionChanged, this,
                &ScatterPlotViewConfigWidget::selectedVariableXChangedSlot);
        cfg_layout->addWidget(select_var_x_);

        cfg_layout->addWidget(new QLabel("Y Variable"));

        select_var_y_ = new VariableSelectionWidget();
        select_var_y_->showMetaVariables(true);
        select_var_y_->showEmptyVariable(false);
        select_var_y_->showDataTypesOnly({PropertyDataType::BOOL,
                                          PropertyDataType::CHAR,
                                          PropertyDataType::UCHAR,
                                          PropertyDataType::INT,
                                          PropertyDataType::UINT,
                                          PropertyDataType::LONGINT,
                                          PropertyDataType::ULONGINT,
                                          PropertyDataType::FLOAT,
                                          PropertyDataType::DOUBLE,
                                          PropertyDataType::TIMESTAMP});
        select_var_y_->setObjectName("variable_selection_y");

        updateSelectedVarY();
        connect(select_var_y_, &VariableSelectionWidget::selectionChanged, this,
                &ScatterPlotViewConfigWidget::selectedVariableYChangedSlot);
        cfg_layout->addWidget(select_var_y_);

        use_connection_lines_ = new QCheckBox("Use Connection Lines");
        use_connection_lines_->setChecked(view_->useConnectionLines());
        UI_TEST_OBJ_NAME(use_connection_lines_, use_connection_lines_->text())

        connect(use_connection_lines_, &QCheckBox::clicked,
                this, &ScatterPlotViewConfigWidget::useConnectionLinesSlot);
        cfg_layout->addWidget(use_connection_lines_);

        cfg_layout->addStretch();

        cfg_widget->setLayout(cfg_layout);

        getTabWidget()->addTab(cfg_widget, "Config");
    }
}

ScatterPlotViewConfigWidget::~ScatterPlotViewConfigWidget() = default;

void ScatterPlotViewConfigWidget::selectedVariableXChangedSlot()
{
    loginf << "ScatterPlotViewConfigWidget: selectedVariableChangedSlot";

    if (select_var_x_->hasVariable())
        view_->dataVarX(select_var_x_->selectedVariable(), true);
    else if (select_var_x_->hasMetaVariable())
        view_->metaDataVarX(select_var_x_->selectedMetaVariable(), true);

}

void ScatterPlotViewConfigWidget::selectedVariableYChangedSlot()
{
    loginf << "ScatterPlotViewConfigWidget: selectedVariableChangedSlot";

    if (select_var_y_->hasVariable())
        view_->dataVarY(select_var_y_->selectedVariable(), true);
    else if (select_var_y_->hasMetaVariable())
        view_->metaDataVarY(select_var_y_->selectedMetaVariable(), true);
}

void ScatterPlotViewConfigWidget::useConnectionLinesSlot()
{
    loginf << "ScatterPlotViewConfigWidget: useConnectionLinesSlot";

    assert (use_connection_lines_);
    view_->useConnectionLines(use_connection_lines_->checkState() == Qt::Checked);
}

void ScatterPlotViewConfigWidget::configChanged()
{
    updateSelectedVarX();
    updateSelectedVarY();
}

void ScatterPlotViewConfigWidget::updateSelectedVarX()
{
    if (view_->hasDataVarX())
    {
        if (view_->isDataVarXMeta())
            select_var_x_->selectedMetaVariable(view_->metaDataVarX());
        else
            select_var_x_->selectedVariable(view_->dataVarX());
    }
}

void ScatterPlotViewConfigWidget::updateSelectedVarY()
{
    if (view_->hasDataVarY())
    {
        if (view_->isDataVarYMeta())
            select_var_y_->selectedMetaVariable(view_->metaDataVarY());
        else
            select_var_y_->selectedVariable(view_->dataVarY());
    }
}

void ScatterPlotViewConfigWidget::onDisplayChange_impl()
{
    assert (use_connection_lines_);
    use_connection_lines_->setChecked(view_->useConnectionLines());
}

void ScatterPlotViewConfigWidget::viewInfoJSON_impl(nlohmann::json& info) const
{
    if (select_var_x_->hasMetaVariable())
        info[ "selected_var_x" ] = "Meta - " + select_var_x_->selectedMetaVariable().name();
    else
        info[ "selected_var_x" ] = select_var_x_->selectedVariable().dbContentName() + " - " + select_var_x_->selectedVariable().name();

    if (select_var_y_->hasMetaVariable())
        info[ "selected_var_y" ] = "Meta - " + select_var_y_->selectedMetaVariable().name();
    else
        info[ "selected_var_y" ] = select_var_y_->selectedVariable().dbContentName() + " - " + select_var_y_->selectedVariable().name();

    info[ "use_connection_lines" ] = use_connection_lines_->isChecked();
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
