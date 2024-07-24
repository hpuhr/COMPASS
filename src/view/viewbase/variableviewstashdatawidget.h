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

#include "variableviewdatawidget.h"
#include "variableviewstash.h"
#include "nullablevector.h"
#include "util/timeconv.h"

#include <limits>

#include <boost/optional.hpp>

class Buffer;

/**
 * Base view data class for variable-based views which stash their buffer data on a per-variable basis.
 * This stash also does a lot of counting and can be processed later on to generate the final view data.
 *
 */
class VariableViewStashDataWidget : public VariableViewDataWidget
{
public:
    VariableViewStashDataWidget(ViewWidget* view_widget,
                                VariableView* view,
                                QWidget* parent = nullptr, 
                                Qt::WindowFlags f = 0);
    virtual ~VariableViewStashDataWidget();

    unsigned int nullValueCount() const;

    QRectF getPlanarBounds(int var_x, int var_y) const;
    boost::optional<std::pair<double, double>> getBounds(int var) const;

protected:
    virtual void resetVariableData() override final;
    virtual void resetIntermediateVariableData() override final;
    virtual void preUpdateVariableDataEvent() override final;
    virtual void postUpdateVariableDataEvent() override final;
    virtual void updateVariableData(const std::string& dbcontent_name,
                                    Buffer& buffer) override final;

    /// derived behavior during postUpdateVariableDataEvent()
    virtual void processStash(const VariableViewStash<double>& stash) = 0;
    /// clear data computed in processStash() 
    virtual void resetStashDependentData() = 0;

    void viewInfoJSON_impl(nlohmann::json& info) const override;

    void selectData (double x_min, 
                     double x_max, 
                     double y_min, 
                     double y_max,
                     int var_x,
                     int var_y);

    const VariableViewStash<double>& getStash() const { return stash_; }

private:
    void resetStash();
    void updateStash();

    void updateVariableData(size_t var_idx, 
                            std::string dbcontent_name, 
                            unsigned int current_size);

    template<typename T>
    void appendData(NullableVector<T>& data, 
                    std::vector<double>& target, 
                    unsigned int last_size,
                    unsigned int current_size)
    {
        for (unsigned int cnt=last_size; cnt < current_size; ++cnt)
        {
            if (data.isNull(cnt))
                target.push_back(std::numeric_limits<double>::signaling_NaN());
            else
                target.push_back(data.get(cnt));
        }
    }

    VariableViewStash<double> stash_;
};

/**
*/
template<>
inline void VariableViewStashDataWidget::appendData<boost::posix_time::ptime>(NullableVector<boost::posix_time::ptime>& data, 
                                                                              std::vector<double>& target, 
                                                                              unsigned int last_size,
                                                                              unsigned int current_size)
{
    for (unsigned int cnt=last_size; cnt < current_size; ++cnt)
    {
        if (data.isNull(cnt))
        {
            target.push_back(std::numeric_limits<double>::signaling_NaN());
            continue;
        }

        //to utc msecs since epoch
        long t = Utils::Time::toLong(data.get(cnt));

        target.push_back(t);
    }
}

/**
*/
template<>
inline void VariableViewStashDataWidget::appendData<std::string>(NullableVector<std::string>& data, 
                                                                 std::vector<double>& target, 
                                                                 unsigned int last_size,
                                                                 unsigned int current_size)
{
    throw std::runtime_error("VariableViewStashDataWidget: appendData: string not supported");
}
