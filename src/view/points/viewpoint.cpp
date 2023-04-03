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

#include "viewpoint.h"
#include "viewmanager.h"
#include "logger.h"
#include "json.hpp"
#include "compass.h"
#include "dbinterface.h"
#include "latexvisitor.h"

using namespace nlohmann;

const std::string VP_ID_KEY {"id"};
const std::string VP_NAME_KEY {"name"};
const std::string VP_TYPE_KEY {"type"};
const std::string VP_STATUS_KEY {"status"};
const std::string VP_COMMENT_KEY {"comment"};

const std::string VP_DS_TYPES_KEY {"data_source_types"};
const std::string VP_DS_KEY {"data_sources"};
const std::string VP_FILTERS_KEY {"filters"};

const std::string VP_POS_LAT_KEY {"position_latitude"};
const std::string VP_POS_LON_KEY {"position_longitude"};
const std::string VP_POS_WIN_LAT_KEY {"position_window_latitude"};
const std::string VP_POS_WIN_LON_KEY {"position_window_longitude"};

const std::string VP_TIMESTAMP_KEY {"timestamp"};
const std::string VP_TIME_WIN_KEY {"time_window"};

const std::string VP_ANNOTATION_KEY {"annotations"};

const std::string VP_EVAL_KEY {"evaluation_results"};
const std::string VP_EVAL_SHOW_RES_KEY {"show_results"};
const std::string VP_EVAL_REQGRP_ID_KEY {"req_grp_id"};
const std::string VP_EVAL_RES_ID_KEY {"result_id"};
const std::string VP_EVAL_HIGHDET_KEY {"highlight_details"};

const std::string VP_SHOWSEC_KEY {"show_sectors"};

ViewPoint::ViewPoint(unsigned int id, const nlohmann::json::object_t& data, ViewManager& view_manager, bool needs_save)
    : ViewableDataConfig(data), id_(id), view_manager_(view_manager)
{
    assert (data_.contains(VP_ID_KEY));
    assert (data_.at(VP_ID_KEY) == id_);

    if (!data_.contains(VP_STATUS_KEY))
    {
        data_[VP_STATUS_KEY] = "open";
        needs_save = true;
    }

    if (needs_save)
        save();

//    "id":0,
//    "type":"any string",
//    "text":"any string",
//    "position_latitude":49.5,
//    "position_longitude":12.2,
//    "position_window_latitude":0.05,
//    "position_window_longitude":0.02,
//    "time":666.0,
//    "time_window":4.0,

//    "dbo_data":
//    [
//    {
//    "dbo":"Tracker",
//    "ds_name":"ARTAS",
//    "filters":
//    [
//    {
//    "variable":"track_num",
//    "values":[1234]
//    },
//    ...
//    ]
//    },
//    {
//    "dbo":"Tracker",
//    "ds_name":"ARTAS2",
//    "filters":
//    [
//    {
//    "variable":"track_num",
//    "values":[3234]
//    },
//    ...
//    ]
//    }
//    ],
//    "dbo_context_variables":
//    [
//    {
//    "dbo":"Tracker",
//    "variables":["rocd","barometric_altitude"]
//    },
//    ...
//    ],
//    "deviations":["DEV#1",...]


}

ViewPoint::ViewPoint(unsigned int id, const std::string& json_str, ViewManager& view_manager, bool needs_save)
    : ViewableDataConfig(json_str), id_(id), view_manager_(view_manager)
{
    assert (data_.contains(VP_ID_KEY));
    assert (data_.at(VP_ID_KEY) == id_);

    if (!data_.contains(VP_STATUS_KEY))
    {
        data_[VP_STATUS_KEY] = "open";
        needs_save = true;
    }

    if (needs_save)
        save();
}

unsigned int ViewPoint::id() const { return id_; }

void ViewPoint::setStatus (const std::string& status)
{
    data_[VP_STATUS_KEY] = status;
    save();
}

void ViewPoint::setComment (const std::string& comment)
{
    data_[VP_COMMENT_KEY] = comment;
    save();
}

void ViewPoint::print() const
{
    loginf << "ViewPoint id " << id_ <<": print: data '" << data_.dump(4) << "'";
}

void ViewPoint::accept(LatexVisitor& v) const
{
    logdbg << "ViewPoint: accept";
    v.visit(this);
}

void ViewPoint::save()
{
    loginf << "ViewPoint: save: id " << id_;

    DBInterface& db_interface = COMPASS::instance().interface();
    db_interface.setViewPoint(id_, data_.dump());
}
