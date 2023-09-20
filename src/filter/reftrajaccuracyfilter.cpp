#include "reftrajaccuracyfilter.h"
#include "compass.h"
#include "reftrajaccuracyfilterwidget.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "logger.h"

#include <iostream>
#include <string>

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace dbContent;

RefTrajAccuracyFilter::RefTrajAccuracyFilter(const std::string& class_id, const std::string& instance_id,
                       Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    registerParameter("min_value", &min_value_, 30.0);

    name_ = "RefTraj Accuracy";

    createSubConfigurables();
}

RefTrajAccuracyFilter::~RefTrajAccuracyFilter() {}

bool RefTrajAccuracyFilter::filters(const std::string& dbcontent_name)
{
    return dbcontent_name == "RefTraj";
}

std::string RefTrajAccuracyFilter::getConditionString(const std::string& dbcontent_name, bool& first)
{
    logdbg << "RefTrajAccuracyFilter: getConditionString: " << dbcontent_name << " active " << active_;

    if (!COMPASS::instance().dbContentManager().metaVariable(DBContent::meta_var_mc_.name()).existsIn(dbcontent_name))
        return "";

    stringstream ss;

    if (active_)
    {
        dbContent::Variable& x_stddev_var = COMPASS::instance().dbContentManager().metaVariable(
                    DBContent::meta_var_x_stddev_.name()).getFor(dbcontent_name);

        dbContent::Variable& y_stddev_var = COMPASS::instance().dbContentManager().metaVariable(
                    DBContent::meta_var_y_stddev_.name()).getFor(dbcontent_name);

        if (!first)
        {
            ss << " AND";
        }

            ss << " sqrt(pow(" << x_stddev_var.dbColumnName() << ",2) + (pow("
               << y_stddev_var.dbColumnName() << ",2))) <= " << min_value_ << "";

        first = false;
    }

    loginf << "RefTrajAccuracyFilter: getConditionString: here '" << ss.str() << "'";

    return ss.str();
}

void RefTrajAccuracyFilter::generateSubConfigurable(const std::string& class_id, const std::string& instance_id)
{
    logdbg << "RefTrajAccuracyFilter: generateSubConfigurable: class_id " << class_id;

    throw std::runtime_error("RefTrajAccuracyFilter: generateSubConfigurable: unknown class_id " + class_id);
}

DBFilterWidget* RefTrajAccuracyFilter::createWidget()
{
    return new RefTrajAccuracyFilterWidget(*this);
}


void RefTrajAccuracyFilter::checkSubConfigurables()
{
    logdbg << "RefTrajAccuracyFilter: checkSubConfigurables";
}


void RefTrajAccuracyFilter::reset()
{
    widget_->update();
}

void RefTrajAccuracyFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (!filters.contains(name_));
    filters[name_] = json::object();
    json& filter = filters.at(name_);

    filter["Accuracy Minimum"] = min_value_;
}

void RefTrajAccuracyFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (filters.contains(name_));
    const json& filter = filters.at(name_);

    assert (filter.contains("Accuracy Minimum"));
    string value = filter.at("Accuracy Minimum");
    min_value_ = std::stod(value);

    if (widget())
        widget()->update();
}

float RefTrajAccuracyFilter::minValue() const
{
    return min_value_;
}

void RefTrajAccuracyFilter::minValue(float min_value)
{
    min_value_ = min_value;
}


