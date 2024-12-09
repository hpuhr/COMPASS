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

#include "histogramview.h"

#include <QApplication>

#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/metavariable.h"
#include "histogramviewconfigwidget.h"
#include "histogramviewdatasource.h"
#include "histogramviewdatawidget.h"
#include "histogramviewwidget.h"
#include "logger.h"
#include "latexvisitor.h"
#include "evaluationmanager.h"
#include "viewpointgenerator.h"

using namespace dbContent;

const std::string HistogramView::ParamUseLogScale = "use_log_scale";

/**
 */
HistogramView::Settings::Settings()
:   use_log_scale(false)
{
}

/**
 */
HistogramView::HistogramView(const std::string& 
                             class_id, 
                             const std::string& instance_id,
                             ViewContainer* w, 
                             ViewManager& view_manager)
:   VariableView(class_id, instance_id, w, view_manager)
{
    registerParameter(ParamUseLogScale, &settings_.use_log_scale, Settings().use_log_scale);

    const std::vector<PropertyDataType> valid_types = { PropertyDataType::BOOL,
                                                        PropertyDataType::CHAR,
                                                        PropertyDataType::UCHAR,
                                                        PropertyDataType::INT,
                                                        PropertyDataType::UINT,
                                                        PropertyDataType::LONGINT,
                                                        PropertyDataType::ULONGINT,
                                                        PropertyDataType::FLOAT,
                                                        PropertyDataType::DOUBLE };

    addVariable("data_var", "", "data_var", META_OBJECT_NAME, DBContent::meta_var_timestamp_.name(), true, true, false, valid_types);

    // create sub done in init
}

/**
 */
HistogramView::~HistogramView()
{
    if (data_source_)
    {
        delete data_source_;
        data_source_ = nullptr;
    }

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

/**
 */
bool HistogramView::init_impl()
{
    createSubConfigurables();

    assert(data_source_);

//    connect(data_source_, &HistogramViewDataSource::loadingStartedSignal, widget_->getDataWidget(),
//            &HistogramViewDataWidget::loadingStartedSlot);
//    connect(data_source_, &HistogramViewDataSource::updateDataSignal, widget_->getDataWidget(),
//            &HistogramViewDataWidget::updateDataSlot);

//    connect(widget_->configWidget(), &HistogramViewConfigWidget::exportSignal,
//            widget_->getDataWidget(), &HistogramViewDataWidget::exportDataSlot);
//    connect(widget_->getDataWidget(), &HistogramViewDataWidget::exportDoneSignal,
//            widget_->configWidget(), &HistogramViewConfigWidget::exportDoneSlot);

//    connect(data_source_, &HistogramViewDataSource::loadingStartedSignal, widget_->configWidget(),
//            &HistogramViewConfigWidget::loadingStartedSlot);

    //    connect(this, &HistogramView::showOnlySelectedSignal, widget_->getDataWidget(),
    //            &HistogramViewDataWidget::showOnlySelectedSlot);
    //    connect(this, &HistogramView::usePresentationSignal, widget_->getDataWidget(),
    //            &HistogramViewDataWidget::usePresentationSlot);
    //    connect(this, &HistogramView::showAssociationsSignal, widget_->getDataWidget(),
    //            &HistogramViewDataWidget::showAssociationsSlot);

    //    widget_->getDataWidget()->showOnlySelectedSlot(show_only_selected_);
    //    widget_->getDataWidget()->usePresentationSlot(use_presentation_);
    //    widget_->getDataWidget()->showAssociationsSlot(show_associations_);


    return true;
}

/**
 */
void HistogramView::generateSubConfigurable(const std::string& class_id,
                                            const std::string& instance_id)
{
    logdbg << "HistogramView: generateSubConfigurable: class_id " << class_id << " instance_id "
           << instance_id;
    if (class_id == "HistogramViewDataSource")
    {
        assert(!data_source_);
        data_source_ = new HistogramViewDataSource(class_id, instance_id, this);
    }
    else if (class_id == "HistogramViewWidget")
    {
        widget_ = new HistogramViewWidget(class_id, instance_id, this, this, central_widget_);
        setWidget(widget_);
    }
    else
        throw std::runtime_error("HistogramView: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

/**
 */
void HistogramView::checkSubConfigurables()
{
    if (!data_source_)
    {
        generateSubConfigurable("HistogramViewDataSource", "HistogramViewDataSource0");
    }

    if (!widget_)
    {
        generateSubConfigurable("HistogramViewWidget", "HistogramViewWidget0");
    }
}

/**
 */
HistogramViewDataWidget* HistogramView::getDataWidget()
{
    assert (widget_);
    return widget_->getViewDataWidget();
}

/**
 */
VariableSet HistogramView::getBaseSet(const std::string& dbcontent_name)
{
    assert(data_source_);
    return data_source_->getSet()->getFor(dbcontent_name);
}

/**
 */
void HistogramView::accept(LatexVisitor& v)
{
    v.visit(this);
}

/**
 */
std::set<std::string> HistogramView::acceptedAnnotationFeatureTypes() const
{
    std::set<std::string> types;
    types.insert(ViewPointGenFeatureHistogram::FeatureName);

    return types;
}

/**
 */
bool HistogramView::useLogScale() const
{
    return settings_.use_log_scale;
}

/**
 */
void HistogramView::useLogScale(bool use_log_scale, bool notify_changes)
{
    setParameter(settings_.use_log_scale, use_log_scale);

    HistogramViewDataWidget* data_widget = dynamic_cast<HistogramViewDataWidget*>(getDataWidget());
    assert (data_widget);

    if (notify_changes)
    {
        updateView(VU_Redraw);
    }
}

/**
 */
void HistogramView::updateSelection()
{
    loginf << "HistogramView: updateSelection";
    assert(widget_);
    
    widget_->getViewDataWidget()->redrawData(true);

    //    if (show_only_selected_)
    //        widget_->getDataWidget()->updateToSelection();
    //    else
    //        widget_->getDataWidget()->resetModels();  // just updates the checkboxes
}

/**
 */
void HistogramView::viewInfoJSON_impl(nlohmann::json& info) const
{
    //!call base!
    VariableView::viewInfoJSON_impl(info);

    info[ "use_log_scale"] = settings_.use_log_scale;
}
