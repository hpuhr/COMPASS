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

#include "viewdatawidget.h"

#include <map>
#include <vector>

class ViewWidget;
class VariableView;
class Buffer;

/**
 */
class VariableViewDataWidget : public ViewDataWidget
{
public:
    enum class VariableState
    {
        Ok = 0,
        MissingFromBuffer
    };

    VariableViewDataWidget(ViewWidget* view_widget,
                           VariableView* view,
                           QWidget* parent = nullptr, 
                           Qt::WindowFlags f = 0);
    virtual ~VariableViewDataWidget();

    bool canUpdate(int var_idx, const std::string& dbcontent_name) const;
    bool canUpdate(const std::string& dbcontent_name) const;

    VariableState variableState(int var_idx) const;
    bool variableOk(int var_idx) const;
    bool variablesOk() const;

protected:
    virtual void loadingStarted_impl() override final;
    virtual void loadingDone_impl() override final;
    virtual void updateData_impl(bool requires_reset) override final;
    virtual void clearData_impl() override final;
    virtual bool redrawData_impl(bool recompute) override final;
    virtual void liveReload_impl() override final;

    void viewInfoJSON_impl(nlohmann::json& info) const override;

    void setVariableState(int var_idx, VariableState state);

    //to be implemented in derived classes
    virtual bool postLoadTrigger() { return false; }
    virtual void updateDataEvent(bool requires_reset) {}
    virtual void resetVariableData() = 0;
    virtual void resetVariableDisplay() = 0;
    virtual void preUpdateVariableDataEvent() = 0;
    virtual void postUpdateVariableDataEvent() = 0;
    virtual bool updateVariableDisplay() = 0;
    virtual void updateFromVariables();
    virtual void updateFromResults() {}
    virtual void updateVariableData(const std::string& dbcontent_name, Buffer& buffer) {}

private:
    void resetVariableStates();
    
    VariableView* variable_view_ = nullptr;

    mutable std::vector<VariableState> variable_states_;
};
