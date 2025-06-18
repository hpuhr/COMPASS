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

#include <string>

#include "json_fwd.hpp"

/**
 * Metadata for plot contents, to be displayed by a view.
*/
struct PlotMetadata
{
    PlotMetadata();
    PlotMetadata(const std::string& title,
                 const std::string& subtitle,
                 const std::string& x_axis_label,
                 const std::string& y_axis_label,
                 const std::string& plot_group = "");
    virtual ~PlotMetadata() = default;

    std::string fullTitle() const;
    std::string xAxisLabel() const;
    std::string yAxisLabel() const;

    nlohmann::json toJSON() const;
    bool fromJSON(const nlohmann::json& obj);

    static const std::string TagPlotGroup;
    static const std::string TagTitle;
    static const std::string TagSubtitle;
    static const std::string TagXAxisLabel;
    static const std::string TagYAxisLabel;

    static const std::string DefaultPlotTitle;
    static const std::string DefaultXAxisLabel;
    static const std::string DefaultYAxisLabel;

    std::string plot_group_;    // optional plot group name
    std::string title_;         // plot title
    std::string subtitle_;      // plot subtitle
    std::string x_axis_label_;  // optional x-axis label
    std::string y_axis_label_;  // optional y-axis label
};
