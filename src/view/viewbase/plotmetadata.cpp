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

#include "plotmetadata.h"

const std::string PlotMetadata::TagPlotGroup  = "plot_group";
const std::string PlotMetadata::TagTitle      = "title";
const std::string PlotMetadata::TagSubtitle   = "subtitle";
const std::string PlotMetadata::TagXAxisLabel = "x_axis_label";
const std::string PlotMetadata::TagYAxisLabel = "y_axis_label";

const std::string PlotMetadata::DefaultPlotTitle = "Unnamed Plot";
const std::string PlotMetadata::DefaultXAxisLabel = "X";
const std::string PlotMetadata::DefaultYAxisLabel = "Y";

/**
*/
PlotMetadata::PlotMetadata()
:   title_(DefaultPlotTitle)
{
}

/**
*/
PlotMetadata::PlotMetadata(const std::string& title,
                           const std::string& subtitle,
                           const std::string& x_axis_label,
                           const std::string& y_axis_label,
                           const std::string& plot_group)
:   plot_group_  (plot_group  )
,   title_       (title       )
,   subtitle_    (subtitle    )
,   x_axis_label_(x_axis_label) 
,   y_axis_label_(y_axis_label)
{
}

/**
*/
std::string PlotMetadata::fullTitle() const
{
    std::string t = title_.empty() ? DefaultPlotTitle : title_;

    if (!subtitle_.empty())
        t += " - " + subtitle_;

    return t;
}

/**
*/
std::string PlotMetadata::xAxisLabel() const
{
    return x_axis_label_.empty() ? DefaultXAxisLabel : x_axis_label_;
}

/**
*/
std::string PlotMetadata::yAxisLabel() const
{
    return y_axis_label_.empty() ? DefaultYAxisLabel : y_axis_label_;
}

/**
*/
nlohmann::json PlotMetadata::toJSON() const
{
    nlohmann::json obj;

    obj[ TagPlotGroup  ] = plot_group_;
    obj[ TagTitle      ] = title_;
    obj[ TagSubtitle   ] = subtitle_;
    obj[ TagXAxisLabel ] = x_axis_label_;
    obj[ TagYAxisLabel ] = y_axis_label_;

    return obj;
}

/**
*/
bool PlotMetadata::fromJSON(const nlohmann::json& obj)
{
    *this = {};

    if (!obj.is_object())
        return false;

    if (obj.contains(TagTitle))
        title_ = obj[ TagTitle ];

    if (obj.contains(TagSubtitle))
        subtitle_ = obj[ TagSubtitle ];

    if (obj.contains(TagXAxisLabel))
        x_axis_label_ = obj[ TagXAxisLabel ];

    if (obj.contains(TagYAxisLabel))
        y_axis_label_ = obj[ TagYAxisLabel ];
    
    if (obj.contains(TagPlotGroup))
        plot_group_ = obj[ TagPlotGroup ];

    return true;
}
