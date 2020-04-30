/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "listboxviewdatasource.h"

#include <QMessageBox>

#include "atsdb.h"
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

ListBoxViewDataSource::ListBoxViewDataSource(const std::string& class_id,
                                             const std::string& instance_id, Configurable* parent)
    : QObject(),
      Configurable(class_id, instance_id, parent),
      selection_entries_(ViewSelection::getInstance().getEntries())
{
    // registerParameter ("use_selection", &use_selection_, true);

    connect(&ATSDB::instance().objectManager(), SIGNAL(loadingStartedSignal()), this,
            SLOT(loadingStartedSlot()));

    for (auto& obj_it : ATSDB::instance().objectManager())
    {
        connect(obj_it.second, SIGNAL(newDataSignal(DBObject&)), this,
                SLOT(newDataSlot(DBObject&)));
        connect(obj_it.second, SIGNAL(loadingDoneSignal(DBObject&)), this,
                SLOT(loadingDoneSlot(DBObject&)));
    }

    createSubConfigurables();
}

ListBoxViewDataSource::~ListBoxViewDataSource()
{
    unshowViewPoint(nullptr); // removes tmps TODO not done yet

    if (set_)
    {
        delete set_;
        set_ = nullptr;
    }
}

void ListBoxViewDataSource::generateSubConfigurable(const std::string& class_id,
                                                    const std::string& instance_id)
{
    logdbg << "ListBoxViewDataSource: generateSubConfigurable: class_id " << class_id
           << " instance_id " << instance_id;

    if (class_id.compare("DBOVariableOrderedSet") == 0)
    {
        assert(set_ == 0);
        set_ = new DBOVariableOrderedSet(class_id, instance_id, this);
    }
    else
        throw std::runtime_error(
            "ListBoxViewDataSource: generateSubConfigurable: unknown class_id " + class_id);
}

void ListBoxViewDataSource::checkSubConfigurables()
{
    if (set_ == nullptr)
    {
        generateSubConfigurable("DBOVariableOrderedSet", "DBOVariableOrderedSet0");
        assert(set_);

        DBObjectManager& obj_man = ATSDB::instance().objectManager();

        if (obj_man.existsMetaVariable("rec_num"))
            set_->add(obj_man.metaVariable("rec_num"));

        //        Time of Day
        if (obj_man.existsMetaVariable("tod"))
            set_->add(obj_man.metaVariable("tod"));

        //        Datasource
        if (obj_man.existsMetaVariable("ds_id"))
            set_->add(obj_man.metaVariable("ds_id"));

        //        Lat/Long
        if (obj_man.existsMetaVariable("pos_lat_deg"))
            set_->add(obj_man.metaVariable("pos_lat_deg"));
        if (obj_man.existsMetaVariable("pos_long_deg"))
            set_->add(obj_man.metaVariable("pos_long_deg"));

        //        Mode 3/A code
        if (obj_man.existsMetaVariable("mode3a_code"))
            set_->add(obj_man.metaVariable("mode3a_code"));

        //        Mode S TA
        if (obj_man.existsMetaVariable("target_addr"))
            set_->add(obj_man.metaVariable("target_addr"));

        //        Mode S Callsign
        if (obj_man.existsMetaVariable("callsign"))
            set_->add(obj_man.metaVariable("callsign"));

        //        Mode C
        if (obj_man.existsMetaVariable("modec_code_ft"))
            set_->add(obj_man.metaVariable("modec_code_ft"));

        //        Track Number
        if (obj_man.existsMetaVariable("track_num"))
            set_->add(obj_man.metaVariable("track_num"));
    }
}

void ListBoxViewDataSource::unshowViewPoint (ViewPoint* vp)
{
    for (auto& var_it : temporary_added_variables_)
    {
        loginf << "ListBoxViewDataSource: unshowViewPoint: removing var " << var_it.first << ", " << var_it.second;

        removeTemporaryVariable(var_it.first, var_it.second);
    }

    temporary_added_variables_.clear();
}

void ListBoxViewDataSource::showViewPoint (ViewPoint* vp)
{
    assert (vp);
    json& data = vp->data();

//    "context_variables": {
//        "Tracker": [
//            "mof_long",
//            "mof_trans",
//            "mof_vert"
//        ]
//    }

    if (data.contains("context_variables"))
    {
        json& context_variables = data.at("context_variables");
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
    DBObjectManager& obj_man = ATSDB::instance().objectManager();

    if (dbo_name == META_OBJECT_NAME)
    {
        assert (obj_man.existsMetaVariable(var_name));
        MetaDBOVariable& meta_var = obj_man.metaVariable(var_name);
        if (!set_->hasMetaVariable(meta_var))
        {
            set_->add(meta_var);
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

        if (!set_->hasVariable(var))
        {
            set_->add(var);
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

    DBObjectManager& obj_man = ATSDB::instance().objectManager();

    if (dbo_name == META_OBJECT_NAME)
    {
        assert (obj_man.existsMetaVariable(var_name));
        MetaDBOVariable& meta_var = obj_man.metaVariable(var_name);
        assert (set_->hasMetaVariable(meta_var));
        set_->removeMetaVariable(meta_var);
    }
    else
    {
        assert (obj_man.existsObject(dbo_name));
        DBObject& obj = obj_man.object(dbo_name);

        assert (obj.hasVariable(var_name));
        DBOVariable& var = obj.variable(var_name);
        assert (set_->hasVariable(var));
        set_->removeVariable(var);
    }
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
