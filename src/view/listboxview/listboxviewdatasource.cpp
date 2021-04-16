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

#include "listboxviewdatasource.h"

#include <QMessageBox>

#include "compass.h"
#include "configuration.h"
#include "configurationmanager.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "job.h"
#include "logger.h"
#include "viewpoint.h"

#include <algorithm>
#include <cassert>

#include "json.hpp"

using namespace std;
using namespace nlohmann;

const string DEFAULT_SET_NAME {"Default"};

ListBoxViewDataSource::ListBoxViewDataSource(const std::string& class_id,
                                             const std::string& instance_id, Configurable* parent)
    : QObject(),
      Configurable(class_id, instance_id, parent),
      selection_entries_(ViewSelection::getInstance().getEntries())
{
    registerParameter ("current_set_name", &current_set_name_, DEFAULT_SET_NAME);

    connect(&COMPASS::instance().objectManager(), SIGNAL(loadingStartedSignal()), this, SLOT(loadingStartedSlot()));

    for (auto& obj_it : COMPASS::instance().objectManager())
    {
        connect(obj_it.second, SIGNAL(newDataSignal(DBObject&)), this, SLOT(newDataSlot(DBObject&)));
        connect(obj_it.second, SIGNAL(loadingDoneSignal(DBObject&)), this, SLOT(loadingDoneSlot(DBObject&)));
    }

    createSubConfigurables();

    if (hasCurrentSet())
        current_set_name_ = DEFAULT_SET_NAME;

    assert (hasCurrentSet());
}

ListBoxViewDataSource::~ListBoxViewDataSource()
{
    unshowViewPoint(nullptr); // removes tmps TODO not done yet
}

void ListBoxViewDataSource::generateSubConfigurable(const std::string& class_id,
                                                    const std::string& instance_id)
{
    logdbg << "ListBoxViewDataSource: generateSubConfigurable: class_id " << class_id
           << " instance_id " << instance_id;

    if (class_id.compare("DBOVariableOrderedSet") == 0)
    {
        assert (!sets_.count(instance_id));

        std::unique_ptr<DBOVariableOrderedSet> set;
        set.reset(new DBOVariableOrderedSet(class_id, instance_id, this));

        sets_[instance_id] = move(set);
    }
    else
        throw std::runtime_error(
            "ListBoxViewDataSource: generateSubConfigurable: unknown class_id " + class_id);
}

void ListBoxViewDataSource::checkSubConfigurables()
{
    if (!hasSet(DEFAULT_SET_NAME))
    {
        generateSubConfigurable("DBOVariableOrderedSet", DEFAULT_SET_NAME);
        assert(hasSet(DEFAULT_SET_NAME));
        addDefaultVariables(*sets_.at(DEFAULT_SET_NAME).get());
    }
}

bool ListBoxViewDataSource::hasCurrentSet()
{
    assert (current_set_name_.size());
    return hasSet(current_set_name_);
}

bool ListBoxViewDataSource::hasSet (const std::string& name)
{
    return sets_.count(name) && sets_.at(name) != nullptr;
}

void ListBoxViewDataSource::addSet (const std::string& name)
{
    assert (name.size());
    assert (!hasSet(name));

    generateSubConfigurable("DBOVariableOrderedSet", name);
    assert(hasSet(name));
    addDefaultVariables(*sets_.at(name).get());
}

DBOVariableOrderedSet* ListBoxViewDataSource::getSet()
{
    assert (hasCurrentSet());
    return sets_.at(current_set_name_).get();
}

const std::map<std::string, std::unique_ptr<DBOVariableOrderedSet>>& ListBoxViewDataSource::getSets()
{
    return sets_;
}

void ListBoxViewDataSource::unshowViewPoint (const ViewableDataConfig* vp)
{
    for (auto& var_it : temporary_added_variables_)
    {
        loginf << "ListBoxViewDataSource: unshowViewPoint: removing var " << var_it.first << ", " << var_it.second;

        removeTemporaryVariable(var_it.first, var_it.second);
    }

    temporary_added_variables_.clear();
}

void ListBoxViewDataSource::showViewPoint (const ViewableDataConfig* vp)
{
    assert (vp);
    const json& data = vp->data();

//    "context_variables": {
//        "Tracker": [
//            "mof_long",
//            "mof_trans",
//            "mof_vert"
//        ]
//    }

    if (data.contains("context_variables"))
    {
        const json& context_variables = data.at("context_variables");
        assert (context_variables.is_object());

        for (auto& obj_it : context_variables.get<json::object_t>())
        {
            string dbo_name = obj_it.first;
            json& variable_names = obj_it.second;

            assert (variable_names.is_array());

            for (auto& var_it : variable_names.get<json::array_t>())
            {
                string var_name = var_it;

                if (addTemporaryVariable(dbo_name, var_name))
                {
                    loginf << "ListBoxViewDataSource: showViewPoint: added var " << dbo_name << ", " << var_name;
                    temporary_added_variables_.push_back({dbo_name, var_name});
                }
            }
        }
    }
}

bool ListBoxViewDataSource::addTemporaryVariable (const std::string& dbo_name, const std::string& var_name)
{
    DBObjectManager& obj_man = COMPASS::instance().objectManager();

    assert (hasCurrentSet());
    if (dbo_name == META_OBJECT_NAME)
    {
        assert (obj_man.existsMetaVariable(var_name));
        MetaDBOVariable& meta_var = obj_man.metaVariable(var_name);
        if (!getSet()->hasMetaVariable(meta_var))
        {
            getSet()->add(meta_var);
            return true;
        }
        else
            return false;
    }
    else
    {
        assert (obj_man.existsObject(dbo_name));
        DBObject& obj = obj_man.object(dbo_name);

        assert (obj.hasVariable(var_name));
        DBOVariable& var = obj.variable(var_name);

        if (!getSet()->hasVariable(var))
        {
            getSet()->add(var);
            return true;
        }
        else
            return false;
    }
}

void ListBoxViewDataSource::removeTemporaryVariable (const std::string& dbo_name, const std::string& var_name)
{
//    auto el = find(temporary_added_variables_.begin(), temporary_added_variables_.end(),
//                   pair<string, string>{dbo_name, var_name});
//    assert (el != temporary_added_variables_.end());
//    temporary_added_variables_.erase(el);

    DBObjectManager& obj_man = COMPASS::instance().objectManager();

    if (dbo_name == META_OBJECT_NAME)
    {
        assert (obj_man.existsMetaVariable(var_name));
        MetaDBOVariable& meta_var = obj_man.metaVariable(var_name);
        assert (getSet()->hasMetaVariable(meta_var));
        getSet()->removeMetaVariable(meta_var);
    }
    else
    {
        assert (obj_man.existsObject(dbo_name));
        DBObject& obj = obj_man.object(dbo_name);

        assert (obj.hasVariable(var_name));
        DBOVariable& var = obj.variable(var_name);
        assert (getSet()->hasVariable(var));
        getSet()->removeVariable(var);
    }
}

void ListBoxViewDataSource::addDefaultVariables (DBOVariableOrderedSet& set)
{
    DBObjectManager& obj_man = COMPASS::instance().objectManager();

    if (obj_man.existsMetaVariable("rec_num"))
        set.add(obj_man.metaVariable("rec_num"));

    //        Time of Day
    if (obj_man.existsMetaVariable("tod"))
        set.add(obj_man.metaVariable("tod"));

    //        Datasource
    if (obj_man.existsMetaVariable("ds_id"))
        set.add(obj_man.metaVariable("ds_id"));

    //        Lat/Long
//        if (obj_man.existsMetaVariable("pos_lat_deg"))
//            set.add(obj_man.metaVariable("pos_lat_deg"));
//        if (obj_man.existsMetaVariable("pos_long_deg"))
//            set.add(obj_man.metaVariable("pos_long_deg"));

    //        Mode 3/A code
    if (obj_man.existsMetaVariable("mode3a_code"))
        set.add(obj_man.metaVariable("mode3a_code"));

    //        Mode S TA
    if (obj_man.existsMetaVariable("target_addr"))
        set.add(obj_man.metaVariable("target_addr"));

    //        Mode S Callsign
    if (obj_man.existsMetaVariable("callsign"))
        set.add(obj_man.metaVariable("callsign"));

    //        Mode C
    if (obj_man.existsMetaVariable("modec_code_ft"))
        set.add(obj_man.metaVariable("modec_code_ft"));

    //        Track Number
    if (obj_man.existsMetaVariable("track_num"))
        set.add(obj_man.metaVariable("track_num"));
}

void ListBoxViewDataSource::loadingStartedSlot()
{
    logdbg << "ListBoxViewDataSource: loadingStartedSlot";
    emit loadingStartedSignal();
}

void ListBoxViewDataSource::newDataSlot(DBObject& object)
{
    logdbg << "ListBoxViewDataSource: newDataSlot: object " << object.name();

    std::shared_ptr<Buffer> buffer = object.data();
    assert(buffer);

    emit updateDataSignal(object, buffer);
}

void ListBoxViewDataSource::loadingDoneSlot(DBObject& object)
{
    logdbg << "ListBoxViewDataSource: loadingDoneSlot: object " << object.name();
}
