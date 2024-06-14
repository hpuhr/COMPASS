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
    typedef std::map<std::string, std::vector<nlohmann::json>> AnnotationMap;

    VariableView(const std::string& class_id, 
                 const std::string& instance_id,
                 ViewContainer* w,
                 ViewManager& view_manager);
    virtual ~VariableView();

    virtual void loadingDone() override final;

    ViewVariable& variable(int idx);
    size_t numVariables() const;

    virtual dbContent::VariableSet getSet(const std::string& dbcontent_name) override final;

    virtual bool canShowAnnotations() const { return false; }

    void showVariables();
    bool showsVariables() const;

    void showAnnotation();
    bool showsAnnotation() const;
    void setCurrentAnnotation(const std::string& id);
    bool hasAnnotations() const;

    const AnnotationMap& annotations() const;
    const std::string& currentAnnotationID() const;
    const std::vector<nlohmann::json>& currentAnnotation() const;
    bool hasCurrentAnnotation() const;

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

    virtual std::set<std::string> acceptedAnnotationFeatureTypes() const { return {}; }

    virtual void unshowViewPoint(const ViewableDataConfig* vp) {}
    virtual void showViewPoint(const ViewableDataConfig* vp) {}

    virtual bool refreshScreenOnNeededReload() const override { return true; }

    virtual void viewInfoJSON_impl(nlohmann::json& info) const override;

private:
    VariableViewDataWidget* getDataWidget();
    VariableViewConfigWidget* getConfigWidget();

    void clearAnnotations();
    void scanViewPointForAnnotations();
    void onShowAnnotationChanged(bool update_config_widget);
    void onEvalResultsChanged();

    void showVariables(bool force, bool update_config);
    void showAnnotation(bool force, bool update_config);

    std::vector<std::unique_ptr<ViewVariable>> variables_;

    AnnotationMap annotations_;
    std::string   current_annotation_id_;
    bool          show_annotation_ = false;

    const ViewableDataConfig* current_view_point_ {nullptr};
};
