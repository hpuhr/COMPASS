#include "viewpoint.h"
#include "viewmanager.h"
#include "logger.h"

ViewPoint::ViewPoint(unsigned int id, ViewManager& view_manager)
    : id_(id), view_manager_(view_manager)
{
    data_["id"] = id_;

//    "id":0,
//    "type":"any string",
//    "text":"any string",
//    "position_latitude":49.5,
//    "position_longitude":12.2,
//    "time":666.0,
//    "position_window_interest_latitude":0.05,
//    "position_window_interest_longitude":0.02,
//    "time_window_interest":4.0,
//    "position_window_context_latitude":0.5,
//    "position_window_context_longitude":0.2,
//    "time_window_context":4.0,
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

ViewPoint::ViewPoint(unsigned int id, nlohmann::json& data, ViewManager& view_manager)
    : id_(id), data_(data), view_manager_(view_manager)
{
    assert (data_.contains("id"));
    assert (data_.at("id") == id_);
}

unsigned int ViewPoint::id() const { return id_; }

nlohmann::json& ViewPoint::data() { return data_; }

void ViewPoint::print()
{
    loginf << "ViewPoint id " << id_ <<": print: dirty " << dirty_ << " json '" << data_.dump(4) << "'";
}

bool ViewPoint::dirty() const
{
    return dirty_;
}

void ViewPoint::dirty(bool value)
{
    dirty_ = value;
}
