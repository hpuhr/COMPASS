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
    connect(&COMPASS::instance().evaluationManager(), &EvaluationManager::resultsChangedSignal, this, &VariableView::resultsChanged);
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
    info[ "show_results"     ] = show_results_;
    info[ "eval_results_id_" ] = eval_result_id_.toString();
}

/**
 */
bool VariableView::showResults() const
{
    return (canShowResults() && show_results_);
}

/**
 */
void VariableView::showResults(bool value)
{
    if (!canShowResults())
        return;

    if (show_results_ == value)
        return;

    show_results_ = value;

    onShowResultsChanged(true);
}

/**
 */
const ViewEvalDataID& VariableView::resultID() const
{
    return eval_result_id_;
}

/**
 */
void VariableView::resultID(const ViewEvalDataID& id)
{
    if (!canShowResults())
        return;

    if (eval_result_id_.eval_results_grpreq == id.eval_results_grpreq &&
        eval_result_id_.eval_results_id     == id.eval_results_id)
        return;

    eval_result_id_ = id;

    loginf << "VariableView: evalResultsID: " << eval_result_id_.toString();

    onShowResultsChanged(false);
}

/**
 */
bool VariableView::hasResultID() const
{
    return eval_result_id_.valid();
}

/**
 */
void VariableView::onShowResultsChanged(bool update_config_widget)
{
    if (update_config_widget)
        getConfigWidget()->updateConfig();
    
    widget_->getViewDataWidget()->redrawData(true);
    widget_->updateComponents();
}

/**
 */
void VariableView::resultsChanged()
{
    if (!canShowResults())
        return;

    loginf << "VariableView: resultsChanged";

    EvaluationManager& eval_man = COMPASS::instance().evaluationManager();

    const EvaluationManager::ResultMap& results = eval_man.results();

    // check if result ids are still valid
    if (!results.size() || !eval_result_id_.inResults(eval_man))
    {
        eval_result_id_.reset();
    }

    //reset result visualization if result id is bad
    if (show_results_ && !hasResultID())
        show_results_ = false;

    //update on result change
    onShowResultsChanged(true);
}

/**
 */
void VariableView::unshowViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "HistogramView: unshowViewPoint";

    unshowViewPoint(vp);

    current_view_point_ = nullptr;
}

/**
 */
void VariableView::showViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "HistogramView: showViewPoint";

    showViewPoint(vp);

    current_view_point_ = vp;
}

/**
 */
void VariableView::loadingDone()
{
    //finish loading procedures first by calling base
    View::loadingDone();

    if (!canShowResults())
        return;

    //now update view to any existing results
    if (current_view_point_ && current_view_point_->data().contains("evaluation_results"))
    {
        //infer result visualization from view point
        const nlohmann::json& data = current_view_point_->data();
        assert (data.at("evaluation_results").contains("show_results"));
        assert (data.at("evaluation_results").contains("req_grp_id"));
        assert (data.at("evaluation_results").contains("result_id"));

        show_results_= data.at("evaluation_results").at("show_results");

        eval_result_id_.eval_results_grpreq = data.at("evaluation_results").at("req_grp_id");
        eval_result_id_.eval_results_id     = data.at("evaluation_results").at("result_id");

        //show_results_ = true;

        //eval_highlight_details_.clear();

        //if (data.at("evaluation_results").contains("highlight_details"))
        //{
        //    loginf << "GeographicView: loadingDoneSlot: highlight_details "
        //           << data.at("evaluation_results").at("highlight_details").dump();

        //    vector<unsigned int> highlight_details = data.at("evaluation_results").at("highlight_details");
        //    eval_highlight_details_ = move(highlight_details);
        //}

        resultsChanged();
    }
    else
    {
        //do not show results
        showResults(false);
    }
}
