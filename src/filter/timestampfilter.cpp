#include "timestampfilter.h"
#include "timestampfilterwidget.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "util/timeconv.h"

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace dbContent;

TimestampFilter::TimestampFilter(const std::string& class_id, const std::string& instance_id,
                                 Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    registerParameter("min_value", &min_value_str_, std::string());
    registerParameter("max_value", &max_value_str_, std::string());

    if (min_value_str_.size())
        min_value_ = Time::fromString(min_value_str_);

    if (max_value_str_.size())
        max_value_ = Time::fromString(max_value_str_);

    name_ = "Timestamp";

    createSubConfigurables();
}

TimestampFilter::~TimestampFilter() {}

bool TimestampFilter::filters(const std::string& dbo_type)
{
    return COMPASS::instance().dbContentManager().metaVariable(
                DBContent::meta_var_timestamp_.name()).existsIn(dbo_type);
}

std::string TimestampFilter::getConditionString(const std::string& dbcontent_name, bool& first)
{
    logdbg << "TimestampFilter: getConditionString: dbo " << dbcontent_name << " active " << active_;

    auto& dbcont_man = COMPASS::instance().dbContentManager();

    if (!dbcont_man.metaVariable(
                DBContent::meta_var_timestamp_.name()).existsIn(dbcontent_name))
        return "";

    stringstream ss;

    if (active_)
    {
        dbContent::Variable& var = dbcont_man.metaVariable(
                    DBContent::meta_var_timestamp_.name()).getFor(dbcontent_name);

        if (!first)
        {
            ss << " AND";
        }

        ss << " (" << var.dbColumnName() << " >= " << Time::toLong(min_value_)
           << " AND " << var.dbColumnName() << " <= " << Time::toLong(max_value_) << ")";

        loginf << "TimestampFilter: getConditionString: dbo " << dbcontent_name << " active " << active_
               << " min " << Time::toString(min_value_) << " max " << Time::toString(max_value_);

        first = false;
    }

    logdbg << "TimestampFilter: getConditionString: here '" << ss.str() << "'";

    return ss.str();
}


void TimestampFilter::generateSubConfigurable(const std::string& class_id, const std::string& instance_id)
{
    logdbg << "TimestampFilter: generateSubConfigurable: class_id " << class_id;

    throw std::runtime_error("TimestampFilter: generateSubConfigurable: unknown class_id " + class_id);
}

DBFilterWidget* TimestampFilter::createWidget()
{
    return new TimestampFilterWidget(*this);
}


void TimestampFilter::checkSubConfigurables()
{
    logdbg << "TimestampFilter: checkSubConfigurables";
}


void TimestampFilter::reset()
{
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    loginf << "TimestampFilter: reset: has db min/max " << dbcont_man.hasMinMaxTimestamp();

    if (dbcont_man.hasMinMaxTimestamp())
    {
        std::pair<boost::posix_time::ptime , boost::posix_time::ptime> minmax_ts =  dbcont_man.minMaxTimestamp();
        minValue(get<0>(minmax_ts));
        maxValue(get<1>(minmax_ts));
    }

    widget_->update();
}

void TimestampFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (!filters.contains(name_));
    filters[name_] = json::object();
    json& filter = filters.at(name_);

    filter["Timestamp Minimum"] = min_value_str_;
    filter["Timestamp Maximum"] = max_value_str_;
}

void TimestampFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    logdbg << "TimestampFilter: loadViewPointConditions: filter '" << filters.dump(4) << "'";

    assert (conditions_.size() == 0);

    assert (filters.contains(name_));
    const json& filter = filters.at(name_);

    assert (filter.contains("Timestamp Minimum"));
    min_value_str_ = filter.at("Timestamp Minimum");
    assert (min_value_str_.size());
    min_value_ = Time::fromString(min_value_str_);

    assert (filter.contains("Timestamp Maximum"));
    max_value_str_ = filter.at("Timestamp Maximum");
    assert (max_value_str_.size());
    max_value_ = Time::fromString(max_value_str_);

    if (widget())
        widget()->update();
}

boost::posix_time::ptime TimestampFilter::minValue() const
{
    return min_value_;
}

void TimestampFilter::minValue(boost::posix_time::ptime min_value, bool update_widget)
{
    min_value_ = min_value;
    min_value_str_ = Time::toString(min_value_);

    loginf << "TimestampFilter: minValue: " << min_value_str_;

    if (widget_ && update_widget)
        widget_->update();
}

boost::posix_time::ptime TimestampFilter::maxValue() const
{
    return max_value_;
}

void TimestampFilter::maxValue(boost::posix_time::ptime max_value, bool update_widget)
{
    max_value_ = max_value;
    max_value_str_ = Time::toString(max_value_);

    loginf << "TimestampFilter: maxValue: " << max_value_str_;

    if (widget_ && update_widget)
        widget_->update();
}
