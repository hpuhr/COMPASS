#include "trackertracknumberfilter.h"
#include "trackertracknumberfilterwidget.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "datasourcemanager.h"
#include "util/timeconv.h"

#include <sstream>

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace dbContent;

TrackerTrackNumberFilter::TrackerTrackNumberFilter(const std::string& class_id, const std::string& instance_id,
                                                   Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    registerParameter("tracker_track_nums", &tracker_track_nums_, json::object());

    name_ = "Tracker Track Number";

    createSubConfigurables();
}

TrackerTrackNumberFilter::~TrackerTrackNumberFilter() {}

bool TrackerTrackNumberFilter::filters(const std::string& dbcontent_name)
{
    return dbcontent_name == "CAT062";
}

std::string TrackerTrackNumberFilter::getConditionString(const std::string& dbcontent_name, bool& first,
                                                         std::vector<std::string>& extra_from_parts,
                                                         std::vector<dbContent::Variable*>& filtered_variables)
{
    logdbg << "TrackerTrackNumberFilter: getConditionString: dbo " << dbcontent_name << " active " << active_;

    if (dbcontent_name != "CAT062")
        return "";

    stringstream ss;

    assert (COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_datasource_id_.name()).existsIn(dbcontent_name));

    assert (COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_track_num_.name()).existsIn(dbcontent_name));

    if (active_ && getActiveTrackerTrackNums().size())
    {
        dbContent::Variable& ds_id_var = COMPASS::instance().dbContentManager().metaVariable(
                    DBContent::meta_var_datasource_id_.name()).getFor(dbcontent_name);
        filtered_variables.push_back(&ds_id_var);

        dbContent::Variable& tn_var = COMPASS::instance().dbContentManager().metaVariable(
                    DBContent::meta_var_track_num_.name()).getFor(dbcontent_name);
        filtered_variables.push_back(&tn_var);

        for (auto& val_it : getActiveTrackerTrackNums())
        {

            if (!first)
            {
                ss << " AND";
            }

            ss << " (" + ds_id_var.dbColumnName() << " = " << val_it.first;
            ss << " AND " << tn_var.dbColumnName() << " IN (" << val_it.second << ")";

            ss << ")";

            first = false;
        }
    }

    loginf << "TrackerTrackNumberFilter: getConditionString: here '" << ss.str() << "'";

    return ss.str();
}


void TrackerTrackNumberFilter::generateSubConfigurable(const std::string& class_id, const std::string& instance_id)
{
    logdbg << "TrackerTrackNumberFilter: generateSubConfigurable: class_id " << class_id;

    throw std::runtime_error("TrackerTrackNumberFilter: generateSubConfigurable: unknown class_id " + class_id);
}

DBFilterWidget* TrackerTrackNumberFilter::createWidget()
{
    return new TrackerTrackNumberFilterWidget(*this);
}


void TrackerTrackNumberFilter::checkSubConfigurables()
{
    logdbg << "TrackerTrackNumberFilter: checkSubConfigurables";
}


void TrackerTrackNumberFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (!filters.contains(name_));
    filters[name_] = json::object();

    json& filter = filters.at(name_);

    assert (!filter.contains("values"));
    filter["values"] = getActiveTrackerTrackNums();
}

void TrackerTrackNumberFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    logdbg << "TrackerTrackNumberFilter: loadViewPointConditions: filter '" << filters.dump(4) << "'";

    assert (conditions_.size() == 0);

    assert (filters.contains(name_));
    const json& filter = filters.at(name_);

    assert (filter.contains("values"));

    std::map<std::string, std::string> vp_values = filter.at("values").get<std::map<std::string, std::string>>();

    for (auto& val_it : vp_values)
    {
        tracker_track_nums_[val_it.first] = val_it.second;
    }

    if (widget())
        widget()->update();
}

void TrackerTrackNumberFilter::setTrackerTrackNum(unsigned int ds_id, const std::string& value)
{
    loginf << "TrackerTrackNumberFilter: setTrackerTrackNum: ds_id " << ds_id << " value '" << value << "'";

    tracker_track_nums_[to_string(ds_id)] = value;
}

std::map<unsigned int, std::string> TrackerTrackNumberFilter::getActiveTrackerTrackNums ()
{
    std::map<std::string, std::string> saved_values = tracker_track_nums_.get<std::map<std::string, std::string>>();

    std::map<unsigned int, std::string> active_values;

    for (auto& ds_it : COMPASS::instance().dataSourceManager().dbDataSources())
    {
        if (ds_it->dsType() != "Tracker")
            continue;

        if (!ds_it->hasNumInserted())
            continue;

        string ds_id_str = to_string(ds_it->id());

        if (saved_values.count(ds_id_str))
            active_values[ds_it->id()] = saved_values.at(ds_id_str);
        else
            active_values[ds_it->id()] = "";
    }

    return active_values;
}

void TrackerTrackNumberFilter::updateDataSourcesSlot()
{
    loginf << "TrackerTrackNumberFilter: updateDataSourcesSlot";

    if (widget_)
        widget_->update();
}
