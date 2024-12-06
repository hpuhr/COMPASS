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

#include "variableview.h"
#include "viewvariable.h"
#include "viewwidget.h"
#include "variableviewdatawidget.h"
#include "variableviewconfigwidget.h"
#include "viewpointgenerator.h"

#include "compass.h"
#include "evaluationmanager.h"

#include "logger.h"

/**
*/
VariableView::VariableView(const std::string& class_id, 
                           const std::string& instance_id,
                           ViewContainer* w, 
                           ViewManager& view_manager)
:   View(class_id, instance_id, w, view_manager)
{
    // eval
    connect(&COMPASS::instance().evaluationManager(), &EvaluationManager::resultsChangedSignal, this, &VariableView::onEvalResultsChanged);
}

/**
*/
VariableView::~VariableView()
{
}

/**
 */
VariableViewDataWidget* VariableView::getDataWidget()
{
    assert (widget_);

    auto data_widget = dynamic_cast<VariableViewDataWidget*>(widget_->getViewDataWidget());
    assert(data_widget);

    return data_widget;
}

/**
 */
VariableViewConfigWidget* VariableView::getConfigWidget()
{
    assert (widget_);

    auto config_wigdet = dynamic_cast<VariableViewConfigWidget*>(widget_->getViewConfigWidget());
    assert(config_wigdet);

    return config_wigdet;
}

/**
*/
ViewVariable& VariableView::variable(int idx)
{
    auto var = variables_.at(idx).get();
    assert(var);

    return *var;
}

/**
*/
const ViewVariable& VariableView::variable(int idx) const
{
    auto var = variables_.at(idx).get();
    assert(var);

    return *var;
}

/**
*/
size_t VariableView::numVariables() const
{
    return variables_.size();
}

/**
*/
ViewVariable& VariableView::addVariable(const std::string& id,
                                        const std::string& display_name,
                                        const std::string& var_name,
                                        const std::string& default_dbo,
                                        const std::string& default_name,
                                        bool show_meta_vars,
                                        bool show_empty_vars,
                                        const std::vector<PropertyDataType>& valid_data_types)
{
    variables_.emplace_back(new ViewVariable(id, this));

    ViewVariable& var = *variables_.back();

    var.settings().display_name     = display_name;
    var.settings().var_name         = var_name;

    var.settings().show_meta_vars   = show_meta_vars;
    var.settings().show_empty_vars  = show_empty_vars;
    var.settings().valid_data_types = std::set<PropertyDataType>(valid_data_types.begin(), valid_data_types.end());

    registerParameter(var.regParamDBO() , &var.settings().data_var_dbo , default_dbo );
    registerParameter(var.regParamName(), &var.settings().data_var_name, default_name);

    return var;
}

/**
*/
dbContent::VariableSet VariableView::getSet(const std::string& dbcontent_name)
{
    logdbg << "VariableView: getSet";

    dbContent::VariableSet set = getBaseSet(dbcontent_name);

    addVariablesToSet(set, dbcontent_name);

    return set;
}

/**
*/
void VariableView::addVariablesToSet(dbContent::VariableSet& set, 
                                     const std::string& dbcontent_name)
{
    for (const auto& v : variables_)
        v->addToSet(set, dbcontent_name);
}

/**
 */
void VariableView::viewInfoJSON_impl(nlohmann::json& info) const
{
    //variable related
    for (const auto& v : variables_)
    {
        info[ v->regParamDBO()  ] = v->settings().data_var_dbo;
        info[ v->regParamName() ] = v->settings().data_var_name;
    }

    //result related
    info[ "show_annotation"        ] = show_annotation_;
    info[ "current_annotation_idx" ] = current_annotation_idx_;
    info[ "num_annotations"        ] = annotations_.size();
}

/**
 */
void VariableView::showVariables(bool force, bool update_config)
{
    if (!force && show_annotation_ == false)
        return;

    show_annotation_ = false;

    onShowAnnotationChanged(update_config);
}

/**
 */
void VariableView::showVariables()
{
    showVariables(false, false);
}

/**
 */
bool VariableView::showsVariables() const
{
    return !show_annotation_;
}

/**
*/
void VariableView::showAnnotation(bool force, bool update_config)
{
    if (!canShowAnnotations() || !hasAnnotations())
        return;

    if (!force && show_annotation_ == true)
        return;

    show_annotation_ = true;
    
    onShowAnnotationChanged(update_config);
}

/**
 */
void VariableView::showAnnotation()
{
    showAnnotation(false, false);
}

/**
 */
void VariableView::setCurrentAnnotation(int group_idx, int annotation_idx)
{
    if (!canShowAnnotations())
        return;

    if (current_annotation_group_idx_ == group_idx &&
        current_annotation_idx_ == annotation_idx)
        return;

    current_annotation_group_idx_ = group_idx;
    current_annotation_idx_       = annotation_idx;

    onShowAnnotationChanged(false);
}

/**
 */
bool VariableView::showsAnnotation() const
{
    return show_annotation_;
}

/**
 */
bool VariableView::hasAnnotations() const
{
    return !annotations_.empty();
}

/**
 */
const std::vector<VariableView::AnnotationGroup>& VariableView::annotations() const
{
    return annotations_;
}

/**
 */
int VariableView::currentAnnotationGroupIdx() const
{
    return current_annotation_group_idx_;
}

/**
 */
int VariableView::currentAnnotationIdx() const
{
    return current_annotation_idx_;
}

/**
 */
const VariableView::Annotation& VariableView::currentAnnotation() const
{
    assert(hasCurrentAnnotation());
    return annotations_.at(current_annotation_group_idx_).annotations.at(current_annotation_idx_);
}

/**
 */
bool VariableView::hasCurrentAnnotation() const
{
    return (current_annotation_group_idx_ >= 0 && 
            current_annotation_group_idx_ < (int)annotations_.size() &&
            current_annotation_idx_ >= 0 &&
            current_annotation_idx_ < (int)annotations_.at(current_annotation_group_idx_).annotations.size());
}

/**
 */
void VariableView::onShowAnnotationChanged(bool update_config_widget)
{
    if (!canShowAnnotations())
        return;

    if (update_config_widget)
        getConfigWidget()->updateConfig();
    
    widget_->getViewDataWidget()->redrawData(true);
    widget_->updateComponents();
}

/**
 */
void VariableView::onEvalResultsChanged()
{
    //nothing to do, handled via view points
}

/**
 */
void VariableView::unshowViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "VariableView: unshowViewPoint";

    unshowViewPoint(vp);

    current_view_point_ = nullptr;

    if (canShowAnnotations())
    {
        clearAnnotations();

        //switch back to variables
        showVariables(true, true);
    }
}

/**
 */
void VariableView::showViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "VariableView: showViewPoint";

    showViewPoint(vp);

    current_view_point_ = vp;

    if (canShowAnnotations())
    {
        //scan annotations contained in view point
        scanViewPointForAnnotations();

        //switch to annotation if available, else switch back to variables
        if (hasAnnotations() && hasCurrentAnnotation())
            showAnnotation(true, true);
        else
            showVariables(true, true);
    }
}

/**
 */
void VariableView::clearAnnotations()
{
    annotations_.clear();

    current_annotation_group_idx_ = -1;
    current_annotation_idx_       = -1;
}

/**
 */
void VariableView::scanViewPointForAnnotations()
{
    clearAnnotations();

    if (!current_view_point_)
        return;

    const auto& j = current_view_point_->data();

    //scan view point for annotation features
    auto features = ViewPointGenVP::scanForFeatures(j, acceptedAnnotationFeatureTypes());
    if (features.empty())
        return;

    std::map<std::string, AnnotationGroup> groups;

    for (const auto& feat : features)
    {
        PlotMetadata metadata;
        
        if (feat.feature_json.contains(ViewPointGenFeature::FeatureFieldNamePlotMetadata))
            metadata.fromJSON(feat.feature_json[ ViewPointGenFeature::FeatureFieldNamePlotMetadata ]);

        auto& group = groups[ metadata.plot_group_ ];
        group.name = metadata.plot_group_;

        Annotation anno;
        anno.feature_json = feat.feature_json;
        anno.metadata     = metadata;

        group.annotations.push_back(anno);
    }

    for (const auto& g : groups)
        annotations_.push_back(g.second);

    if (!annotations_.empty())
    {
        current_annotation_group_idx_ = 0;
        current_annotation_idx_       = 0;
    }
}

/**
 */
void VariableView::loadingDone()
{
    //call base
    View::loadingDone();
}
