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

#include "gridviewconfigwidget.h"
#include "gridviewwidget.h"
#include "gridview.h"

#include "logger.h"
#include "ui_test_common.h"

#include <QComboBox>
#include <QSpinBox>
#include <QFormLayout>

using namespace Utils;
using namespace dbContent;

/**
*/
GridViewConfigWidget::GridViewConfigWidget(GridViewWidget* view_widget, 
                                           QWidget* parent)
:   VariableViewConfigWidget(view_widget, view_widget->getView(), parent)
{
    view_ = view_widget->getView();
    assert(view_);

    auto config_layout = configLayout();

    QFormLayout* layout = new QFormLayout;
    config_layout->addLayout(layout);

    value_type_combo_ = new QComboBox;
    for (int i = 0; i < grid2d::NumValueTypes; ++i)
        value_type_combo_->addItem(QString::fromStdString(grid2d::valueTypeToString((grid2d::ValueType)i)), QVariant(i));

    connect(value_type_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GridViewConfigWidget::valueTypeChanged);

    layout->addRow("Value Type:", value_type_combo_);

    grid_resolution_box_ = new QSpinBox;
    grid_resolution_box_->setMinimum(1);
    grid_resolution_box_->setMaximum(1000);

    connect(grid_resolution_box_, QOverload<int>::of(&QSpinBox::valueChanged), this, &GridViewConfigWidget::gridResolutionChanged);

    layout->addRow("Grid Resolution:", grid_resolution_box_);

    color_steps_box_ = new QSpinBox;
    color_steps_box_->setMinimum(2);
    color_steps_box_->setMaximum(256);

    connect(color_steps_box_, QOverload<int>::of(&QSpinBox::valueChanged), this, &GridViewConfigWidget::colorStepsChanged);

    layout->addRow("Color Steps:", color_steps_box_);

    updateConfig();
}

/**
*/
GridViewConfigWidget::~GridViewConfigWidget() = default;

/**
*/
void GridViewConfigWidget::viewInfoJSON_impl(nlohmann::json& info) const
{
    //!call base!
    VariableViewConfigWidget::viewInfoJSON_impl(info);

    //@TODO?
}

/**
*/
void GridViewConfigWidget::valueTypeChanged()
{
    view_->setValueType((grid2d::ValueType)value_type_combo_->currentData().toInt(), true);
}

/**
*/
void GridViewConfigWidget::gridResolutionChanged()
{
    view_->setGridResolution((unsigned int)grid_resolution_box_->value(), true);
}

/**
*/
void GridViewConfigWidget::colorStepsChanged()
{
    view_->setColorSteps((unsigned int)color_steps_box_->value(), true);
}

/**
*/
void GridViewConfigWidget::updateConfig()
{
    const auto& settings = view_->settings();

    value_type_combo_->blockSignals(true);
    value_type_combo_->setCurrentIndex(value_type_combo_->findData(QVariant((int)settings.value_type)));
    value_type_combo_->blockSignals(false);

    grid_resolution_box_->blockSignals(true);
    grid_resolution_box_->setValue((int)settings.grid_resolution);
    grid_resolution_box_->blockSignals(false);

    color_steps_box_->blockSignals(true);
    color_steps_box_->setValue((int)settings.render_color_num_steps);
    color_steps_box_->blockSignals(false);
}
