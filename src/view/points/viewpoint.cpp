#include "viewpoint.h"
#include "viewmanager.h"
#include "logger.h"
#include "json.hpp"

using namespace nlohmann;

ViewPoint::ViewPoint(unsigned int id, ViewManager& view_manager)
    : id_(id), view_manager_(view_manager)
{
    data_["id"] = id_;
    data_["status"] = "open";

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

ViewPoint::ViewPoint(unsigned int id, const std::string& json_str, ViewManager& view_manager)
    : id_(id), view_manager_(view_manager)
{
    data_ = json::parse(json_str);
    assert (data_.contains("id"));
    assert (data_.at("id") == id_);

    if (!data_.contains("status"))
    {
        data_["status"] = "open";
        dirty_ = true;
    }
}

unsigned int ViewPoint::id() const { return id_; }

nlohmann::json& ViewPoint::data() { return data_; }

const nlohmann::json& ViewPoint::data() const { return data_; }

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
