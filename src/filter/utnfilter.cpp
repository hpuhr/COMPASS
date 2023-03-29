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

#include "utnfilter.h"
#include "compass.h"
#include "utnfilterwidget.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/metavariable.h"
#include "logger.h"
#include "stringconv.h"

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace dbContent;

UTNFilter::UTNFilter(const std::string& class_id, const std::string& instance_id,
                     Configurable* parent)
    : DBFilter(class_id, instance_id, parent, false)
{
    registerParameter("utns_str", &utns_str_, "");
    updateUTNSFromStr(utns_str_);

    name_ = "UTNs";

//    if (!COMPASS::instance().dbContentManager().hasAssociations())
//    {
//        loginf << "UTNFilter: constructor: disabled since no associations";

//        active_ = false;
//        visible_ = false;
//    }

    createSubConfigurables();
}

UTNFilter::~UTNFilter() {}

bool UTNFilter::filters(const std::string& dbo_type)
{
    if (!COMPASS::instance().dbContentManager().hasAssociations())
        return false;

    return true;
}

std::string UTNFilter::getConditionString(const std::string& dbcontent_name, bool& first)
{
    logdbg << "UTNFilter: getConditionString: dbo " << dbcontent_name << " active " << active_;

    if (!COMPASS::instance().dbContentManager().hasAssociations())
        return "";

    stringstream ss;

    if (active_)
    {
        DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
        assert (dbcontent_man.existsDBContent(dbcontent_name));

        assert (dbcontent_man.metaVariable(DBContent::meta_var_associations_.name()).existsIn(dbcontent_name));
        //Variable& assoc_var = dbcontent_man.metaVariable(DBContent::meta_var_associations_.name()).getFor(dbcontent_name);

        //extra_from_parts.push_back("json_each("+dbcontent_man.dbContent(dbcontent_name).dbTableName()+".associations)");

        if (!first)
        {
            ss << " AND";
        }

        // SELECT x FROM data_cat062, json_each(data_cat062.associations) WHERE json_each.value == 0;

        ss << " json_each.value IN (" << utns_str_ << ")"; // rest done in SQLGenerator::getSelectCommand

        first = false;
    }

    logdbg << "UTNFilter: getConditionString: here '" << ss.str() << "'";

    return ss.str();
}

void UTNFilter::generateSubConfigurable(const std::string& class_id,
                                        const std::string& instance_id)
{
    logdbg << "UTNFilter: generateSubConfigurable: class_id " << class_id;

    throw std::runtime_error("UTNFilter: generateSubConfigurable: unknown class_id " + class_id);
}

void UTNFilter::checkSubConfigurables()
{
    logdbg << "UTNFilter: checkSubConfigurables";

}

DBFilterWidget* UTNFilter::createWidget()
{
    return new UTNFilterWidget(*this);
}


void UTNFilter::reset()
{
    if (widget_)
        widget_->update();
}

void UTNFilter::saveViewPointConditions (nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (!filters.contains(name_));
    filters[name_] = json::object();
    json& filter = filters.at(name_);

    filter["utns"] = utns_str_;
}

void UTNFilter::loadViewPointConditions (const nlohmann::json& filters)
{
    assert (conditions_.size() == 0);

    assert (filters.contains(name_));
    const json& filter = filters.at(name_);

    assert (filter.contains("utns"));
    utns_str_ = filter.at("utns");

    updateUTNSFromStr(utns_str_);

    if (widget())
        widget()->update();
}

std::string UTNFilter::utns() const
{
    return utns_str_;
}

void UTNFilter::utns(const std::string& utns)
{
    if (!updateUTNSFromStr(utns)) // false on failure
    {
        if (widget_)
            widget_->update();

        return;
    }

    utns_str_ = utns;
}


bool UTNFilter::updateUTNSFromStr(const std::string& utns)
{
    vector<unsigned int> utns_tmp;
    vector<string> utns_tmp_str = String::split(utns, ',');

    bool ok = true;

    for (auto& utn_str : utns_tmp_str)
    {
        unsigned int utn_tmp = QString(utn_str.c_str()).toInt(&ok);

        if (!ok)
        {
            logerr << "UTNFilter: updateUTNSFromStr: utn '" << utn_str << "' not valid";
            break;
        }

        utns_tmp.push_back(utn_tmp);
    }

    if (!ok)
        return false;

    utns_ = utns_tmp;

    return true;
}
