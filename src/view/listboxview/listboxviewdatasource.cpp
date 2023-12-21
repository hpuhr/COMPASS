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
#include "compass.h"
//#include "configuration.h"
//#include "configurationmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
//#include "job.h"
#include "logger.h"
//#include "viewpoint.h"
#include "global.h"

#include <QMessageBox>

#include <algorithm>
#include <cassert>

#include "json.hpp"

using namespace std;
using namespace nlohmann;
using namespace dbContent;

const string DEFAULT_SET_NAME {"Default"};

ListBoxViewDataSource::ListBoxViewDataSource(const std::string& current_set_name,
                                             const std::string& class_id,
                                             const std::string& instance_id, 
                                             Configurable* parent)
:   QObject()
,   Configurable(class_id, instance_id, parent)
,   current_set_name_(current_set_name)
{
    createSubConfigurables();

    if (!hasCurrentSet())
        current_set_name_ = DEFAULT_SET_NAME;

    assert (hasCurrentSet());

    currentSetName(current_set_name_, false);
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

    if (class_id.compare("VariableOrderedSet") == 0)
    {
        assert (!sets_.count(instance_id));

        std::unique_ptr<VariableOrderedSet> set;
        set.reset(new VariableOrderedSet(class_id, instance_id, this));

        sets_[instance_id] = move(set);
    }
    else
    {
        throw std::runtime_error(
            "ListBoxViewDataSource: generateSubConfigurable: unknown class_id " + class_id);
    }
}

void ListBoxViewDataSource::checkSubConfigurables()
{
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    if (!hasSet(DEFAULT_SET_NAME))
    {
        generateSubConfigurable("VariableOrderedSet", DEFAULT_SET_NAME);
        assert(hasSet(DEFAULT_SET_NAME));
        addDefaultVariables(*sets_.at(DEFAULT_SET_NAME).get());
    }

    if (!hasSet("ADS-B Quality"))
    {
        DBContent& cat021_cont = dbcont_man.dbContent("CAT021");

        generateSubConfigurable("VariableOrderedSet", "ADS-B Quality");
        assert(hasSet("ADS-B Quality"));

        std::unique_ptr<dbContent::VariableOrderedSet>& set = sets_.at("ADS-B Quality");

        addDefaultVariables(*set.get());

        assert (cat021_cont.hasVariable(DBContent::var_cat021_mops_version_.name()));
        set->add(cat021_cont.variable(DBContent::var_cat021_mops_version_.name()));

        assert (cat021_cont.hasVariable(DBContent::var_cat021_nacp_.name()));
        set->add(cat021_cont.variable(DBContent::var_cat021_nacp_.name()));

        assert (cat021_cont.hasVariable(DBContent::var_cat021_sil_.name()));
        set->add(cat021_cont.variable(DBContent::var_cat021_sil_.name()));

        assert (cat021_cont.hasVariable(DBContent::var_cat021_nucp_nic_.name()));
        set->add(cat021_cont.variable(DBContent::var_cat021_nucp_nic_.name()));
    }

    if (!hasSet("Horizontal Movement"))
    {
        generateSubConfigurable("VariableOrderedSet", "Horizontal Movement");
        assert(hasSet("Horizontal Movement"));

        std::unique_ptr<dbContent::VariableOrderedSet>& set = sets_.at("Horizontal Movement");

        addDefaultVariables(*set.get());

        if (dbcont_man.existsMetaVariable(DBContent::meta_var_track_angle_.name()))
            set->add(dbcont_man.metaVariable(DBContent::meta_var_track_angle_.name()));

        if (dbcont_man.existsMetaVariable(DBContent::meta_var_ground_speed_.name()))
            set->add(dbcont_man.metaVariable(DBContent::meta_var_ground_speed_.name()));

        if (dbcont_man.existsMetaVariable(DBContent::meta_var_horizontal_man_.name()))
            set->add(dbcont_man.metaVariable(DBContent::meta_var_horizontal_man_.name()));
    }

    if (!hasSet("Mode A/C Info"))
    {
        generateSubConfigurable("VariableOrderedSet", "Mode A/C Info");
        assert(hasSet("Mode A/C Info"));

        std::unique_ptr<dbContent::VariableOrderedSet>& set = sets_.at("Mode A/C Info");

        addDefaultVariables(*set.get());

        if (dbcont_man.existsMetaVariable(DBContent::meta_var_m3a_g_.name()))
            set->add(dbcont_man.metaVariable(DBContent::meta_var_m3a_g_.name()));

        if (dbcont_man.existsMetaVariable(DBContent::meta_var_m3a_v_.name()))
            set->add(dbcont_man.metaVariable(DBContent::meta_var_m3a_v_.name()));

        if (dbcont_man.existsMetaVariable(DBContent::meta_var_m3a_smoothed_.name()))
            set->add(dbcont_man.metaVariable(DBContent::meta_var_m3a_smoothed_.name()));

        if (dbcont_man.existsMetaVariable(DBContent::meta_var_mc_g_.name()))
            set->add(dbcont_man.metaVariable(DBContent::meta_var_mc_g_.name()));

        if (dbcont_man.existsMetaVariable(DBContent::meta_var_mc_v_.name()))
            set->add(dbcont_man.metaVariable(DBContent::meta_var_mc_v_.name()));
    }

    if (!hasSet("Track Lifetime"))
    {
        generateSubConfigurable("VariableOrderedSet", "Track Lifetime");
        assert(hasSet("Track Lifetime"));

        std::unique_ptr<dbContent::VariableOrderedSet>& set = sets_.at("Track Lifetime");

        addDefaultVariables(*set.get());

        if (dbcont_man.existsMetaVariable(DBContent::meta_var_track_begin_.name()))
            set->add(dbcont_man.metaVariable(DBContent::meta_var_track_begin_.name()));

        if (dbcont_man.existsMetaVariable(DBContent::meta_var_track_confirmed_.name()))
            set->add(dbcont_man.metaVariable(DBContent::meta_var_track_confirmed_.name()));

        if (dbcont_man.existsMetaVariable(DBContent::meta_var_track_coasting_.name()))
            set->add(dbcont_man.metaVariable(DBContent::meta_var_track_coasting_.name()));

        if (dbcont_man.existsMetaVariable(DBContent::meta_var_track_end_.name()))
            set->add(dbcont_man.metaVariable(DBContent::meta_var_track_end_.name()));
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
    loginf << "ListBoxViewDataSource: addSet: name '" << name << "'";

    assert (name.size());
    assert (!hasSet(name));

    generateSubConfigurable("VariableOrderedSet", name);
    assert(hasSet(name));
    addDefaultVariables(*sets_.at(name).get());
}

void ListBoxViewDataSource::copySet (const std::string& name, const std::string& new_name)
{
    loginf << "ListBoxViewDataSource: copySet: name '" << name << "' new name '" << new_name << "'";

    assert (name.size());
    assert (hasSet(name));

    assert (new_name.size());
    assert (!hasSet(new_name));

    generateSubConfigurable("VariableOrderedSet", new_name);
    assert(hasSet(new_name));

    for (const auto& var_def_it : getSet()->definitions())
        sets_.at(new_name)->add(var_def_it.first, var_def_it.second);
}

void ListBoxViewDataSource::removeSet (const std::string& name)
{
    loginf << "ListBoxViewDataSource: removeSet: name '" << name << "'";

    assert (hasSet(name));
    assert (name != DEFAULT_SET_NAME);

    sets_.erase(name);

    if (current_set_name_ == name)
        currentSetName(DEFAULT_SET_NAME);
}

VariableOrderedSet* ListBoxViewDataSource::getSet()
{
    assert (hasCurrentSet());
    return sets_.at(current_set_name_).get();
}

const std::map<std::string, std::unique_ptr<VariableOrderedSet>>& ListBoxViewDataSource::getSets()
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
            string dbcontent_name = obj_it.first;
            json& variable_names = obj_it.second;

            assert (variable_names.is_array());

            for (auto& var_it : variable_names.get<json::array_t>())
            {
                string var_name = var_it;

                if (addTemporaryVariable(dbcontent_name, var_name))
                {
                    loginf << "ListBoxViewDataSource: showViewPoint: added var " << dbcontent_name << ", " << var_name;
                    temporary_added_variables_.push_back({dbcontent_name, var_name});
                }
            }
        }
    }
}

std::string ListBoxViewDataSource::currentSetName() const
{
    return current_set_name_;
}

void ListBoxViewDataSource::currentSetName(const std::string& current_set_name, bool signal_reload)
{
    loginf << "ListBoxViewDataSource: currentSetName: name '" << current_set_name << "'";

    assert (hasSet(current_set_name));
    current_set_name_ = current_set_name;

    connect(getSet(), &VariableOrderedSet::setChangedSignal, this,
            &ListBoxViewDataSource::setChangedSignal, Qt::UniqueConnection);
    connect(getSet(), &VariableOrderedSet::setChangedSignal, this,
            &ListBoxViewDataSource::reloadNeeded, Qt::UniqueConnection);

    emit currentSetChangedSignal();

    if (signal_reload)
        emit reloadNeeded();
}

bool ListBoxViewDataSource::addTemporaryVariable (const std::string& dbcontent_name, const std::string& var_name)
{
    DBContentManager& obj_man = COMPASS::instance().dbContentManager();
    
    assert (hasCurrentSet());
    if (dbcontent_name == META_OBJECT_NAME)
    {
        assert (obj_man.existsMetaVariable(var_name));
        MetaVariable& meta_var = obj_man.metaVariable(var_name);
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
        assert (obj_man.existsDBContent(dbcontent_name));
        DBContent& obj = obj_man.dbContent(dbcontent_name);

        assert (obj.hasVariable(var_name));
        Variable& var = obj.variable(var_name);

        if (!getSet()->hasVariable(var))
        {
            getSet()->add(var);
            return true;
        }
        else
            return false;
    }
}

void ListBoxViewDataSource::removeTemporaryVariable (const std::string& dbcontent_name, const std::string& var_name)
{
//    auto el = find(temporary_added_variables_.begin(), temporary_added_variables_.end(),
//                   pair<string, string>{dbcontent_name, var_name});
//    assert (el != temporary_added_variables_.end());
//    temporary_added_variables_.erase(el);

    DBContentManager& obj_man = COMPASS::instance().dbContentManager();

    if (dbcontent_name == META_OBJECT_NAME)
    {
        assert (obj_man.existsMetaVariable(var_name));
        MetaVariable& meta_var = obj_man.metaVariable(var_name);
        assert (getSet()->hasMetaVariable(meta_var));
        getSet()->removeMetaVariable(meta_var);
    }
    else
    {
        assert (obj_man.existsDBContent(dbcontent_name));
        DBContent& obj = obj_man.dbContent(dbcontent_name);

        assert (obj.hasVariable(var_name));
        Variable& var = obj.variable(var_name);
        assert (getSet()->hasVariable(var));
        getSet()->removeVariable(var);
    }
}

void ListBoxViewDataSource::addDefaultVariables (VariableOrderedSet& set)
{
    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    if (dbcont_man.existsMetaVariable(DBContent::meta_var_rec_num_.name()))
        set.add(dbcont_man.metaVariable(DBContent::meta_var_rec_num_.name()));

    // Timestamp
    if (dbcont_man.existsMetaVariable(DBContent::meta_var_timestamp_.name()))
        set.add(dbcont_man.metaVariable(DBContent::meta_var_timestamp_.name()));

    // Datasource
    if (dbcont_man.existsMetaVariable(DBContent::meta_var_datasource_id_.name()))
        set.add(dbcont_man.metaVariable(DBContent::meta_var_datasource_id_.name()));

    // Mode 3/A code
    if (dbcont_man.existsMetaVariable(DBContent::meta_var_m3a_.name()))
        set.add(dbcont_man.metaVariable(DBContent::meta_var_m3a_.name()));

    // Mode S TA
    if (dbcont_man.existsMetaVariable(DBContent::meta_var_ta_.name()))
        set.add(dbcont_man.metaVariable(DBContent::meta_var_ta_.name()));

    // Mode S Callsign
    if (dbcont_man.existsMetaVariable(DBContent::meta_var_ti_.name()))
        set.add(dbcont_man.metaVariable(DBContent::meta_var_ti_.name()));

    // Mode C
    if (dbcont_man.existsMetaVariable(DBContent::meta_var_mc_.name()))
        set.add(dbcont_man.metaVariable(DBContent::meta_var_mc_.name()));

    // Track Number
    if (dbcont_man.existsMetaVariable(DBContent::meta_var_track_num_.name()))
        set.add(dbcont_man.metaVariable(DBContent::meta_var_track_num_.name()));
}
