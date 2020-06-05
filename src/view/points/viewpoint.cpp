#include "viewpoint.h"
#include "viewmanager.h"
#include "logger.h"
#include "json.hpp"
#include "atsdb.h"
#include "dbinterface.h"
#include "latexvisitor.h"

using namespace nlohmann;

ViewPoint::ViewPoint(unsigned int id, const nlohmann::json::object_t& data, ViewManager& view_manager, bool needs_save)
    : id_(id), view_manager_(view_manager)
{
    data_ = data;

    assert (data_.contains("id"));
    assert (data_.at("id") == id_);

    if (!data_.contains("status"))
    {
        data_["status"] = "open";
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
    : id_(id), view_manager_(view_manager)
{
    data_ = json::parse(json_str);

    assert (data_.contains("id"));
    assert (data_.at("id") == id_);

    if (!data_.contains("status"))
    {
        data_["status"] = "open";
        needs_save = true;
    }

    if (needs_save)
        save();
}

unsigned int ViewPoint::id() const { return id_; }

const nlohmann::json& ViewPoint::data() const { return data_; }

void ViewPoint::setStatus (const std::string& status)
{
    data_["status"] = status;
    save();
}

void ViewPoint::setComment (const std::string& comment)
{
    data_["comment"] = comment;
    save();
}

void ViewPoint::print()
{
    loginf << "ViewPoint id " << id_ <<": print: data '" << data_.dump(4) << "'";
}

void ViewPoint::accept(LatexVisitor& v)
{
    loginf << "ViewPoint: accept";
    v.visit(this);
}

void ViewPoint::save()
{
    loginf << "ViewPoint: save: id " << id_;

    DBInterface& db_interface = ATSDB::instance().interface();
    db_interface.setViewPoint(id_, data_.dump());
}
