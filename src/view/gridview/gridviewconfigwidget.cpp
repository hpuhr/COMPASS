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
#include "gridviewdatawidget.h"
#include "gridview.h"

#include "viewvariable.h"

#include "logger.h"
#include "ui_test_common.h"

#include "global.h"

#include "compass.h"
#include "viewmanager.h"
#include "viewpointgenerator.h"
#include "geotiff.h"
#include "colormap_defs.h"
#include "metavariable.h"
#include "dbcontent.h"

#include "variableselectionwidget.h"

#if USE_EXPERIMENTAL_SOURCE == true
#include "geographicview.h"
#endif

#include "colorscaleselection.h"
#include "propertyvalueedit.h"

#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QToolButton>
#include <QMessageBox>
#include <QMenu>
#include <QFormLayout>
#include <QFileDialog>

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

    color_selection_ = new ColorScaleSelection;

    connect(color_selection_, &ColorScaleSelection::scaleChanged, this, &GridViewConfigWidget::colorScaleChanged);

    layout->addRow("Color Scale:", color_selection_);

    color_steps_box_ = new QSpinBox;
    color_steps_box_->setMinimum(2);
    color_steps_box_->setMaximum(256);

    connect(color_steps_box_, QOverload<int>::of(&QSpinBox::valueChanged), this, &GridViewConfigWidget::colorStepsChanged);

    layout->addRow("Color Steps:", color_steps_box_);

    color_value_min_box_ = new PropertyValueEdit();
    reset_min_button_    = new QPushButton("Reset");
    reset_min_button_->setVisible(false); //@TODO

    QHBoxLayout* layout_color_min = new QHBoxLayout;
    layout_color_min->setContentsMargins(0, 0, 0, 0);
    layout_color_min->setSpacing(0);

    layout_color_min->addWidget(color_value_min_box_);
    layout_color_min->addWidget(reset_min_button_);

    connect(color_value_min_box_, &PropertyValueEdit::valueEdited, this, &GridViewConfigWidget::minValueChanged);

    layout->addRow("Color Min. Value:", layout_color_min);

    color_value_max_box_ = new PropertyValueEdit();
    reset_max_button_    = new QPushButton("Reset");
    reset_max_button_->setVisible(false); //@TODO

    QHBoxLayout* layout_color_max = new QHBoxLayout;
    layout_color_max->setContentsMargins(0, 0, 0, 0);
    layout_color_max->setSpacing(0);

    layout_color_max->addWidget(color_value_max_box_);
    layout_color_max->addWidget(reset_max_button_);

    connect(color_value_max_box_, &PropertyValueEdit::valueEdited, this, &GridViewConfigWidget::maxValueChanged);

    layout->addRow("Color Max. Value:", layout_color_max);

    export_button_ = new QPushButton("Export");

    layout->addRow("", export_button_);

    attachExportMenu();
    updateConfig();
    updateExport();
}

/**
*/
GridViewConfigWidget::~GridViewConfigWidget() = default;

/**
*/
void GridViewConfigWidget::attachExportMenu()
{
    assert(export_button_);

    //attach export menu
    QMenu* menu = new QMenu(export_button_);

#if USE_EXPERIMENTAL_SOURCE == true
    auto export_geoview_action = menu->addAction("Export to GeographicView");
    connect(export_geoview_action, &QAction::triggered, this, &GridViewConfigWidget::exportToGeographicView);
#endif

    auto export_geotiff_action = menu->addAction("Export to GeoTIFF");
    connect(export_geotiff_action, &QAction::triggered, this, &GridViewConfigWidget::exportToGeoTiff);

    export_button_->setMenu(menu);
}

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
void GridViewConfigWidget::postVariableChangedEvent(int idx)
{
    if (idx == 0 || idx == 1)
        updateExport();
    else if (idx == 2)
        updateVariableDataType();
}

/**
*/
void GridViewConfigWidget::redrawDone()
{
    VariableViewConfigWidget::redrawDone();

    updateExport();
}

/**
*/
void GridViewConfigWidget::loadingDone()
{
    VariableViewConfigWidget::loadingDone();

    updateExport();
}

/**
*/
void GridViewConfigWidget::updateExport()
{
    assert(view_);

    auto const_view = dynamic_cast<const GridView*>(view_);
    assert(const_view);

    if (!const_view->isInit() || !const_view->getDataWidget()->hasValidGrid())
    {
        export_button_->setEnabled(false);
        export_button_->setToolTip("No grid data available");

        return;
    }

    auto var_sel_x = variableSelection(0);
    auto var_sel_y = variableSelection(1);

    assert(var_sel_x && var_sel_y);

    auto& dbc_man = COMPASS::instance().dbContentManager();

    const auto& metavar_lon = dbc_man.metaVariable(DBContent::meta_var_longitude_.name());
    const auto& metavar_lat = dbc_man.metaVariable(DBContent::meta_var_latitude_.name());

    bool has_lon       = var_sel_x->hasMetaVariable() && &var_sel_x->selectedMetaVariable() == &metavar_lon;
    bool has_lat       = var_sel_y->hasMetaVariable() && &var_sel_y->selectedMetaVariable() == &metavar_lat;
    bool enable_export = has_lon && has_lat;

    const std::string tt_deactiv = "<html><head/><body>"
                                   "<p>To reenable export, please select the following variables.</p>"
                                   "<ul>"
                                   "<li><b>X Variable</b>: Meta - Longitude</li>"
                                   "<li><b>Y Variable</b>: Meta - Latitude</li>"
                                   "</ul>"
                                   "<p></p>"
                                   "</body></html>";

    std::string tooltip = enable_export ? "" : tt_deactiv;

    export_button_->setEnabled(enable_export);
    export_button_->setToolTip(QString::fromStdString(tooltip));
}

/**
*/
void GridViewConfigWidget::valueTypeChanged()
{
    view_->setValueType((grid2d::ValueType)value_type_combo_->currentData().toInt(), true);

    updateVariableDataType();
}

/**
*/
void GridViewConfigWidget::gridResolutionChanged()
{
    view_->setGridResolution((unsigned int)grid_resolution_box_->value(), true);
}

/**
*/
void GridViewConfigWidget::colorScaleChanged()
{
    view_->setColorScale(color_selection_->selectedScale(), true);
}

/**
*/
void GridViewConfigWidget::colorStepsChanged()
{
    view_->setColorSteps((unsigned int)color_steps_box_->value(), true);
}

/**
*/
void GridViewConfigWidget::minValueChanged()
{
    auto v_str = color_value_min_box_->isValid() ? color_value_min_box_->valueAsString() : "";

    view_->setMinValue(v_str, true);
}

/**
*/
void GridViewConfigWidget::maxValueChanged()
{
    auto v_str = color_value_max_box_->isValid() ? color_value_max_box_->valueAsString() : "";

    view_->setMaxValue(v_str, true);
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

    color_selection_->blockSignals(true);
    color_selection_->setSelectedScale((colorscale::ColorScale)settings.render_color_scale);
    color_selection_->blockSignals(false);

    color_steps_box_->blockSignals(true);
    color_steps_box_->setValue((int)settings.render_color_num_steps);
    color_steps_box_->blockSignals(false);

    updateVariableDataType();

    color_value_min_box_->blockSignals(true);
    color_value_min_box_->setValue(settings.render_color_value_min);
    color_value_min_box_->blockSignals(false);

    color_value_max_box_->blockSignals(true);
    color_value_max_box_->setValue(settings.render_color_value_max);
    color_value_max_box_->blockSignals(false);
}

/**
*/
void GridViewConfigWidget::updateVariableDataType()
{
    //determine actual datatype depending on selected variable and grid value type
    auto dtype = view_->currentDataType();

    color_value_min_box_->blockSignals(true);
    color_value_min_box_->setPropertyDataType(dtype);
    color_value_min_box_->blockSignals(false);

    color_value_max_box_->blockSignals(true);
    color_value_max_box_->setPropertyDataType(dtype);
    color_value_max_box_->blockSignals(false);
}

#if USE_EXPERIMENTAL_SOURCE == true
namespace
{
    struct ExportGeoViewConfig
    {
        GeographicView* view = nullptr;
        std::string     item_name;
    };

    /**
    */
    boost::optional<ExportGeoViewConfig> getExportGeoViewConfig(QWidget* parent,
                                                                const std::string& default_name)
    {
        auto geo_views = COMPASS::instance().viewManager().viewsOfType<GeographicView>();

        QDialog dlg(parent);
        dlg.setWindowTitle("Export to GeographicView");

        auto layout = new QVBoxLayout;
        dlg.setLayout(layout);

        auto flayout = new QFormLayout;
        layout->addLayout(flayout);

        auto view_sel = new QComboBox;
        for (auto v : geo_views)
            view_sel->addItem(QString::fromStdString(v->getName()));
        if (view_sel->count() > 0)
            view_sel->setCurrentIndex(0);

        flayout->addRow("View:", view_sel);

        auto txt_edit = new QLineEdit;
        txt_edit->setText(QString::fromStdString(default_name));

        flayout->addRow("Grid Name:", txt_edit);

        layout->addStretch(1);

        auto button_layout = new QHBoxLayout;
        layout->addLayout(button_layout);

        auto button_ok     = new QPushButton("Ok");
        auto button_cancel = new QPushButton("Cancel");

        QObject::connect(button_ok    , &QPushButton::pressed, &dlg, &QDialog::accept);
        QObject::connect(button_cancel, &QPushButton::pressed, &dlg, &QDialog::reject);

        button_layout->addStretch(1);
        button_layout->addWidget(button_ok);
        button_layout->addWidget(button_cancel);

        auto okCB = [ = ] ()
        {
            button_ok->setEnabled(!txt_edit->text().isEmpty() && view_sel->currentIndex() >= 0);
        };
        QObject::connect(txt_edit, &QLineEdit::textEdited, okCB);
        QObject::connect(view_sel, QOverload<int>::of(&QComboBox::currentIndexChanged), okCB);

        okCB();
        
        if (dlg.exec() == QDialog::Rejected)
            return {};

        ExportGeoViewConfig config;
        config.item_name = txt_edit->text().toStdString();
        config.view      = geo_views.at(view_sel->currentIndex());

        return config;
    }
}
#endif

/**
*/
std::string GridViewConfigWidget::exportName() const
{
    auto var_sel = variableSelection(2);
    auto var = var_sel->selectionAsString();

    std::string name;
    name += (!var.first.empty() && !var.second.empty()) ? (var.first + " - " + var.second) : "";
    name += (!name.empty() ? " - " : "") + value_type_combo_->currentText().toStdString();

    return name;
}

/**
*/
void GridViewConfigWidget::exportToGeographicView()
{
#if USE_EXPERIMENTAL_SOURCE == true

    const auto& data_widget = dynamic_cast<const GridView*>(view_)->getDataWidget();

    auto geo_image = data_widget->currentGeoImage();
    if (!geo_image.has_value())
    {
        QMessageBox::critical(this, "Error", "No grid available to send");
        return;
    }

    std::string name = exportName();

    auto export_config = getExportGeoViewConfig(this, name);
    if (!export_config.has_value())
        return;

    assert(export_config->view);
    assert(!export_config->item_name.empty());

    ViewPointGenAnnotation anno(export_config->item_name);

    auto geo_image_feat = new ViewPointGenFeatureGeoImage(geo_image->first, geo_image->second);
    anno.addFeature(geo_image_feat);

    nlohmann::json j;
    anno.toJSON(j);

    export_config->view->addInternalAnnotation(j);
#else
    QMessageBox::critical(this, "Error", "Geographic View not part of installation");
#endif
}

/**
*/
void GridViewConfigWidget::exportToGeoTiff()
{
    const auto& data_widget = dynamic_cast<const GridView*>(view_)->getDataWidget();

    auto geo_image = data_widget->currentGeoImage();
    if (!geo_image.has_value())
    {
        QMessageBox::critical(this, "Error", "No grid available to send");
        return;
    }

    std::string fn_default = COMPASS::instance().lastUsedPath() + "/" + exportName() + ".tif";

    auto fn = QFileDialog::getSaveFileName(this, "Export to GeoTIFF", QString::fromStdString(fn_default), "*.tif");
    if (fn.isEmpty())
        return;

    if (!GeoTIFFWriter::writeGeoTIFF(fn.toStdString(), geo_image->first, geo_image->second))
        QMessageBox::critical(this, "Error", "Export to GeoTIFF failed.");
}
