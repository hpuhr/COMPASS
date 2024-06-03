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

#pragma once

#include "view.h"
#include "property.h"
#include "viewevaldata.h"

#include <memory>
#include <vector>

class ViewVariable;
class VariableViewDataWidget;
class VariableViewConfigWidget;
class ViewableDataConfig;

/**
*/
class VariableView : public View
{
public:
    VariableView(const std::string& class_id, 
                 const std::string& instance_id,
                 ViewContainer* w,
                 ViewManager& view_manager);
    virtual ~VariableView();

    virtual void loadingDone() override final;

    ViewVariable& variable(int idx);
    size_t numVariables() const;

    virtual dbContent::VariableSet getSet(const std::string& dbcontent_name) override final;

    virtual bool canShowResults() const { return false; }

    bool showResults() const;
    void showResults(bool value);

    bool hasResultID() const;
    const ViewEvalDataID& resultID() const;
    void resultID(const ViewEvalDataID& id);

    bool hasViewPoint () { return current_view_point_ != nullptr; }
    const ViewableDataConfig& viewPoint() { assert (hasViewPoint()); return *current_view_point_; }

public slots:
    virtual void unshowViewPointSlot (const ViewableDataConfig* vp) override final;
    virtual void showViewPointSlot (const ViewableDataConfig* vp) override final;

protected:
    ViewVariable& addVariable(const std::string& id,
                              const std::string& display_name,
                              const std::string& var_name,
                              const std::string& default_dbo,
                              const std::string& default_name,
                              bool show_meta_vars,
                              bool show_empty_vars,
                              const std::vector<PropertyDataType>& valid_data_types);
    void addVariablesToSet(dbContent::VariableSet& set, 
                           const std::string& dbcontent_name);

    virtual dbContent::VariableSet getBaseSet(const std::string& dbcontent_name) = 0;

    virtual void unshowViewPoint(const ViewableDataConfig* vp) {}
    virtual void showViewPoint(const ViewableDataConfig* vp) {}

    virtual bool refreshScreenOnNeededReload() const override { return true; }

    virtual void viewInfoJSON_impl(nlohmann::json& info) const override;

private:
    void resultsChanged();
    void onShowResultsChanged(bool update_config_widget);

    VariableViewDataWidget* getDataWidget();
    VariableViewConfigWidget* getConfigWidget();

    std::vector<std::unique_ptr<ViewVariable>> variables_;

    ViewEvalDataID eval_result_id_;
    bool           show_results_ = false;

    const ViewableDataConfig* current_view_point_ {nullptr};
};