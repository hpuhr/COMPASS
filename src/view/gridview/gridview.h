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

#include "variableview.h"
#include "grid2d_defs.h"

class GridViewWidget;
class GridViewDataSource;
class GridViewDataWidget;

/**
*/
class GridView : public VariableView
{
public:
    struct Settings
    {
        Settings();

        int               value_type;

        unsigned int      grid_resolution;

        unsigned int      render_pixels_per_cell;
        std::string       render_color_min;
        std::string       render_color_max;
        unsigned int      render_color_num_steps;
    };

    enum class Variable
    {
        DataVarX = 0,
        DataVarY,
        DataVarZ
    };

    GridView(const std::string& class_id, 
             const std::string& instance_id, 
             ViewContainer* w,
             ViewManager& view_manager);
    virtual ~GridView() override;

    void setValueType(grid2d::ValueType type, bool notify_changes);
    void setGridResolution(unsigned int n, bool notify_changes);
    void setPixelsPerCell(unsigned int n, bool notify_changes);
    void setColorSteps(unsigned int n, bool notify_changes);
    void setColorMin(const QColor& color, bool notify_changes);
    void setColorMax(const QColor& color, bool notify_changes);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    virtual void accept(LatexVisitor& v) override;

    const Settings& settings() const { return settings_; }

protected:
    friend class LatexVisitor;

    virtual void checkSubConfigurables() override;
    virtual void updateSelection() override;

    virtual bool init_impl() override;
    virtual void viewInfoJSON_impl(nlohmann::json& info) const override;

    virtual dbContent::VariableSet getBaseSet(const std::string& dbcontent_name) override;

    GridViewDataWidget* getDataWidget();

    GridViewWidget* widget_ = nullptr;

    Settings settings_;
};