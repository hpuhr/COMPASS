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

#include "filtermanager.h"

#include "compass.h"
#include "configurationmanager.h"
#include "sqliteconnection.h"
#include "dbfilter.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "filtermanagerwidget.h"
#include "logger.h"
#include "viewpoint.h"
#include "dbospecificvaluesdbfilter.h"
#include "utnfilter.h"
#include "adsbqualityfilter.h"

#include "json.hpp"

using namespace std;
using namespace nlohmann;

FilterManager::FilterManager(const std::string& class_id, const std::string& instance_id,
                             COMPASS* compass)
    : Configurable(class_id, instance_id, compass, "filter.json")
{
    logdbg << "FilterManager: constructor";

    registerParameter("use_filters", &use_filters_, false);
    registerParameter("db_id", &db_id_, "");

    createSubConfigurables();
}

FilterManager::~FilterManager()
{
    filters_.clear();

    if (widget_)
    {
        delete widget_;
        widget_ = nullptr;
    }
}

bool FilterManager::useFilters() const
{
    return use_filters_;
}

void FilterManager::useFilters(bool use_filters)
{
    use_filters_ = use_filters;
    loginf << "FilterManager: useFilters: " << use_filters_;

    if (widget_)
        widget_->updateUseFilters();
}

void FilterManager::generateSubConfigurable(const std::string& class_id,
                                            const std::string& instance_id)
{
    if (hasSubConfigurable(class_id, instance_id))
    {
        logerr << "FilterManager: generateSubConfigurable: filter " << instance_id
               << " already present";
        return;
    }

    logdbg << "FilterManager: generateSubConfigurable: filter class_id " << class_id << " instance_id " << instance_id;

    if (class_id == "DBFilter")
    {
        DBFilter* filter = new DBFilter(class_id, instance_id, this);
        filters_.emplace_back(filter);
    }
    else if (class_id == "DBOSpecificValuesDBFilter")
    {
        if (hasSubConfigurable(class_id, instance_id))
        {
            logerr << "FilterManager: generateSubConfigurable: dbo specific filter "
                   << instance_id << " already present";
            return;
        }
        std::string dbo_name = configuration()
                .getSubConfiguration(class_id, instance_id)
                .getParameterConfigValueString("dbo_name");

        if (!checkDBObject(dbo_name))
        {
            loginf << "FilterManager: generateSubConfigurable: disabling dbo specific filter "
                   << instance_id << " for failed check dbobject '" << dbo_name << "'";
            return;
        }

        DBOSpecificValuesDBFilter* filter = new DBOSpecificValuesDBFilter(class_id, instance_id, this);

        if (filter->disabled())
        {
            loginf << "FilterManager: generateSubConfigurable: deleting disabled dbo specific filter"
                   << filter->instanceId();
            delete filter;
        }
        else
            filters_.emplace_back(filter);
    }
    else if (class_id == "UTNFilter")
    {
        try
        {
            if (hasSubConfigurable(class_id, instance_id))
            {
                logerr << "FilterManager: generateSubConfigurable: utn filter "
                       << instance_id << " already present";
                return;
            }

            UTNFilter* filter = new UTNFilter(class_id, instance_id, this);

            filters_.emplace_back(filter);
        }
        catch (const std::exception& e)
        {
            loginf << "FilterManager: generateSubConfigurable: data source filter exception '"
                   << e.what() << "', deleting";
            configuration().removeSubConfiguration(class_id, instance_id);
        }
    }
    else if (class_id == "ADSBQualityFilter")
    {
        try
        {
            if (hasSubConfigurable(class_id, instance_id))
            {
                logerr << "FilterManager: generateSubConfigurable: adsb quality filter "
                       << instance_id << " already present";
                return;
            }

            ADSBQualityFilter* filter = new ADSBQualityFilter(class_id, instance_id, this);

            filters_.emplace_back(filter);
        }
        catch (const std::exception& e)
        {
            loginf << "FilterManager: generateSubConfigurable: data source filter exception '"
                   << e.what() << "', deleting";
            configuration().removeSubConfiguration(class_id, instance_id);
        }
    }
    else
        throw std::runtime_error("FilterManager: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

bool FilterManager::checkDBObject (const std::string& dbo_name)
{
    if (!COMPASS::instance().dbContentManager().existsDBContent(dbo_name))
    {
        loginf << "FilterManager: checkDBObject: failed because of non-existing dbobject '" << dbo_name << "'";
        return false;
    }

    DBContent& object = COMPASS::instance().dbContentManager().dbContent(dbo_name);

    if (!object.existsInDB())
    {
        loginf << "FilterManager: checkDBObject: failed because of empty dbobject '" << dbo_name << "'";
        return false;
    }

    return true;
}

void FilterManager::checkSubConfigurables()
{
    // check for UTN filter

    string classid = "UTNFilter";

    if (std::find_if(filters_.begin(), filters_.end(),
                     [&classid](const unique_ptr<DBFilter>& x) { return x->classId() == classid;}) == filters_.end())
    { // not UTN filter
        addNewSubConfiguration(classid, classid+"0");
        generateSubConfigurable(classid, classid+"0");
    }

//    classid = "ADSBQualityFilter";

//    if (std::find_if(filters_.begin(), filters_.end(),
//                     [&classid](const DBFilter* x) { return x->classId() == classid;}) == filters_.end())
//    { // not UTN filter
//        addNewSubConfiguration(classid, classid+"0");
//        generateSubConfigurable(classid, classid+"0");
//    }
}

std::string FilterManager::getSQLCondition(const std::string& dbo_name,
                                           std::vector<std::string>& extra_from_parts,
                                           std::vector<dbContent::Variable*>& filtered_variables)
{
    assert(COMPASS::instance().dbContentManager().dbContent(dbo_name).loadable());

    std::stringstream ss;

    bool first = true;

    for (auto& filter : filters_)
    {
        loginf << "FilterManager: getSQLCondition: filter " << filter->instanceId() << " active "
               << filter->getActive() << " filters " << dbo_name << " "
               << filter->filters(dbo_name);

        if (filter->getActive() && filter->filters(dbo_name))
        {
            ss << filter->getConditionString(dbo_name, first, extra_from_parts, filtered_variables);
        }
    }

    logdbg << "FilterManager: getSQLCondition: name " << dbo_name << " '" << ss.str() << "'";
    return ss.str();
}

unsigned int FilterManager::getNumFilters() { return filters_.size(); }

DBFilter* FilterManager::getFilter(unsigned int index)
{
    assert(index < filters_.size());

    return filters_.at(index).get();
}

bool FilterManager::hasFilter (const std::string& name)
{
    auto it = find_if(filters_.begin(), filters_.end(), [name] (const unique_ptr<DBFilter>& f)
    { return f->getName() == name; } );

    return it != filters_.end();
}

DBFilter* FilterManager::getFilter (const std::string& name)
{
    auto it = find_if(filters_.begin(), filters_.end(), [name] (const unique_ptr<DBFilter>& f)
    { return f->getName() == name; } );

    assert (it != filters_.end());

    return it->get();
}


void FilterManager::reset()
{
    for (unsigned int cnt = 0; cnt < filters_.size(); cnt++)
    {
        if (!filters_.at(cnt)->disabled())
            filters_.at(cnt)->reset();
    }
}

//void FilterManager::deleteFilterSlot(DBFilter* filter)
//{
//    auto it = find(filters_.begin(), filters_.end(), filter);
//    if (it == filters_.end())
//        throw std::runtime_error("FilterManager: deleteFilter: called with unknown filter");
//    else
//    {
//        filters_.erase(it);
//    }

//    emit changedFiltersSignal();
//}

void FilterManager::unshowViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "FilterManager: unshowViewPointSlot";
    assert (vp);
}

void FilterManager::showViewPointSlot (const ViewableDataConfig* vp)
{
    loginf << "FilterManager: showViewPointSlot";
    assert (vp);

    const json& data = vp->data();

    DBContentManager& obj_man = COMPASS::instance().dbContentManager();

    TODO_ASSERT

    // add all db objects that need loading
//    if (data.contains("db_objects")) // the listed ones should be loaded
//    {
//        const json& db_objects  = data.at("db_objects");
//        for (auto& obj_it : obj_man)
//            obj_it.second->loadingWanted(
//                        std::find(db_objects.begin(), db_objects.end(), obj_it.first) != db_objects.end());
//    }
//    else // all should be loaded
//    {
//        for (auto& obj_it : obj_man)
//            obj_it.second->loadingWanted(true);
//    }

    // add filters
    use_filters_ = data.contains("filters");

    if (data.contains("filters"))
    {
        const json& filters = data.at("filters");

        logdbg << "FilterManager: showViewPointSlot: filter data '" << filters.dump(4) << "'";

        assert (filters.is_object());

        disableAllFilters();

        for (auto& fil_it : filters.get<json::object_t>())
        {
            std::string filter_name = fil_it.first;

            auto it = find_if(filters_.begin(), filters_.end(),
                              [filter_name] (const unique_ptr<DBFilter>& f) { return f->getName() == filter_name; } );

            if (it == filters_.end())
            {
                logerr << "FilterManager: showViewPointSlot: filter '" << filter_name << "' not found";
                continue;
            }

            (*it)->setActive(true);
            (*it)->loadViewPointConditions(filters);
        }
    }
}

void FilterManager::setConfigInViewPoint (nlohmann::json& data)
{
    loginf << "FilterManager: setConfigInViewPoint";

    DBContentManager& obj_man = COMPASS::instance().dbContentManager();

    data["db_objects"] = json::array();
    json& db_objects = data["db_objects"];

    TODO_ASSERT

    // add all db objects that need loading
    unsigned int cnt=0;
//    for (auto& obj_it : obj_man)
//    {
//        if (obj_it.second->loadable() && obj_it.second->loadingWanted())
//        {
//            db_objects[cnt] = obj_it.first;
//            ++cnt;
//        }
//    }

    // add filters
    if (use_filters_)
    {
        data["filters"] = json::object();
        json& filters = data.at("filters");

        for (auto& fil_it : filters_)
        {
            if (fil_it->getActive())
                fil_it->saveViewPointConditions(filters);
        }

        loginf << "FilterManager: setConfigInViewPoint: filters: '" << filters.dump(4) << "'";
    }
}

FilterManagerWidget* FilterManager::widget()
{
    if (!widget_)
    {
        widget_ = new FilterManagerWidget(*this);
        connect(this, SIGNAL(changedFiltersSignal()), widget_, SLOT(updateFiltersSlot()));
    }

    assert(widget_);
    return widget_;
}

void FilterManager::databaseOpenedSlot()
{
    loginf << "FilterManager: databaseOpenedSlot";

    if (widget_)
        widget_->setDisabled(false);
}

void FilterManager::databaseClosedSlot()
{
    loginf << "FilterManager: databaseClosedSlot";

    if (widget_)
        widget_->setDisabled(true);
}

//void FilterManager::startedSlot()
//{
//    loginf << "FilterManager: startedSlot";
//    createSubConfigurables();

//    std::string tmpstr = COMPASS::instance().interface().connection().identifier();
//    replace(tmpstr.begin(), tmpstr.end(), ' ', '_');

//    if (db_id_.compare(tmpstr) != 0)
//    {
//        loginf << "FilterManager: startedSlot: different db id, resetting filters";
//        reset();
//        db_id_ = tmpstr;
//    }

//    emit changedFiltersSignal();

//    if (widget_)
//        widget_->databaseOpenedSlot();
//}

void FilterManager::disableAllFilters ()
{
    for (auto& fil_it : filters_)
        if (!fil_it->disabled())
            fil_it->setActive(false);
}

