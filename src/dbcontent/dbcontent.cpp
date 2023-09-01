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

#include "dbcontent/dbcontent.h"
#include "compass.h"
#include "buffer.h"
#include "dbinterface.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/dbcontentwidget.h"
#include "datasourcemanager.h"
#include "dbcontentreaddbjob.h"
#include "dbcontent/variable/variable.h"
#include "dbtableinfo.h"
#include "filtermanager.h"
#include "insertbufferdbjob.h"
#include "jobmanager.h"
#include "propertylist.h"
//#include "stringconv.h"
//#include "taskmanager.h"
#include "updatebufferdbjob.h"
//#include "viewmanager.h"
//#include "util/number.h"
//#include "dbcontent/variable/metavariable.h"
#include "dbcontentdeletedbjob.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <memory>

using namespace std;
using namespace Utils;
using namespace dbContent;

const Property DBContent::meta_var_rec_num_ {"Record Number", PropertyDataType::ULONGINT};
const Property DBContent::meta_var_datasource_id_ {"DS ID", PropertyDataType::UINT};
const Property DBContent::meta_var_sac_id_ {"SAC", PropertyDataType::UCHAR};
const Property DBContent::meta_var_sic_id_ {"SIC", PropertyDataType::UCHAR};
const Property DBContent::meta_var_line_id_ {"Line ID", PropertyDataType::UINT};
const Property DBContent::meta_var_time_of_day_ {"Time of Day", PropertyDataType::FLOAT};
const Property DBContent::meta_var_timestamp_ {"Timestamp", PropertyDataType::TIMESTAMP};
const Property DBContent::meta_var_m3a_ {"Mode 3/A Code", PropertyDataType::UINT};
const Property DBContent::meta_var_m3a_g_ {"Mode 3/A Garbled", PropertyDataType::BOOL};
const Property DBContent::meta_var_m3a_v_ {"Mode 3/A Valid", PropertyDataType::BOOL};
const Property DBContent::meta_var_m3a_smoothed_ {"Mode 3/A Smoothed", PropertyDataType::BOOL};
const Property DBContent::meta_var_ta_ {"Aircraft Address", PropertyDataType::UINT};
const Property DBContent::meta_var_ti_ {"Aircraft Identification", PropertyDataType::STRING};
const Property DBContent::meta_var_mc_ {"Mode C Code", PropertyDataType::FLOAT};
const Property DBContent::meta_var_mc_g_ {"Mode C Garbled", PropertyDataType::BOOL};
const Property DBContent::meta_var_mc_v_ {"Mode C Valid", PropertyDataType::BOOL};
const Property DBContent::meta_var_ground_bit_ {"Ground Bit", PropertyDataType::BOOL};
const Property DBContent::meta_var_track_num_ {"Track Number", PropertyDataType::UINT};

const Property DBContent::meta_var_track_begin_ {"Track Begin", PropertyDataType::BOOL};
const Property DBContent::meta_var_track_confirmed_ {"Track Confirmed", PropertyDataType::BOOL};
const Property DBContent::meta_var_track_coasting_ {"Track Coasting", PropertyDataType::UCHAR};
const Property DBContent::meta_var_track_end_ {"Track End", PropertyDataType::BOOL};

const Property DBContent::meta_var_latitude_ {"Latitude", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_longitude_ {"Longitude", PropertyDataType::DOUBLE};

const Property DBContent::meta_var_detection_type_ {"Type", PropertyDataType::UCHAR};
const Property DBContent::meta_var_artas_hash_ {"ARTAS Hash", PropertyDataType::UINT};
const Property DBContent::meta_var_utn_ {"UTN", PropertyDataType::UINT};

const Property DBContent::meta_var_vx_ {"Vx", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_vy_ {"Vy", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_ground_speed_ {"Track Groundspeed", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_track_angle_ {"Track Angle", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_horizontal_man_ {"Track Horizontal Manoeuvre", PropertyDataType::BOOL};

const Property DBContent::meta_var_x_stddev_ {"X StdDev", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_y_stddev_ {"Y StdDev", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_xy_cov_ {"X/Y Covariance", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_latitude_stddev_ {"Latitude StdDev", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_longitude_stddev_ {"Longitude StdDev", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_latlon_cov_ {"Lat/Lon Cov", PropertyDataType::DOUBLE};

const Property DBContent::meta_var_climb_descent_{"Track Climbing/Descending", PropertyDataType::UCHAR};
const Property DBContent::meta_var_spi_{"SPI", PropertyDataType::BOOL};

const Property DBContent::var_radar_range_ {"Range", PropertyDataType::DOUBLE};
const Property DBContent::var_radar_azimuth_ {"Azimuth", PropertyDataType::DOUBLE};
const Property DBContent::var_radar_altitude_ {"Mode C Code", PropertyDataType::FLOAT};

const Property DBContent::var_cat021_mops_version_ {"MOPS Version", PropertyDataType::UCHAR};
const Property DBContent::var_cat021_nacp_ {"NACp", PropertyDataType::UCHAR};
const Property DBContent::var_cat021_nucp_nic_ {"NUCp or NIC", PropertyDataType::UCHAR};
const Property DBContent::var_cat021_nucv_nacv_ {"NUCr or NACv", PropertyDataType::UCHAR};
const Property DBContent::var_cat021_sil_ {"SIL", PropertyDataType::UCHAR};

const Property DBContent::var_cat062_tris_ {"Target Report Identifiers", PropertyDataType::STRING};
const Property DBContent::var_cat062_track_begin_ {"Track Begin", PropertyDataType::BOOL};
const Property DBContent::var_cat062_coasting_ {"Coasting", PropertyDataType::BOOL};
const Property DBContent::var_cat062_track_end_ {"Track End", PropertyDataType::BOOL};
const Property DBContent::var_cat062_mono_sensor_ {"Monosensor", PropertyDataType::BOOL};
const Property DBContent::var_cat062_type_lm_ {"Type LM", PropertyDataType::UCHAR};
const Property DBContent::var_cat062_baro_alt_ {"Barometric Altitude Calculated", PropertyDataType::FLOAT};
const Property DBContent::var_cat062_fl_measured_ {"Flight Level Measured", PropertyDataType::FLOAT};

const Property DBContent::var_cat062_wtc_ {"Wake Turbulence Category FPL", PropertyDataType::STRING};
const Property DBContent::var_cat062_callsign_fpl_ {"Callsign FPL", PropertyDataType::STRING};

const Property DBContent::var_cat062_vx_stddev_ {"Vx StdDev", PropertyDataType::DOUBLE};
const Property DBContent::var_cat062_vy_stddev_ {"Vy StdDev", PropertyDataType::DOUBLE};

const Property DBContent::selected_var {"selected", PropertyDataType::BOOL};

DBContent::DBContent(COMPASS& compass, const string& class_id, const string& instance_id,
                     DBContentManager* manager)
    : Configurable(class_id, instance_id, manager,
                   "db_content_" + boost::algorithm::to_lower_copy(instance_id) + ".json"),
      compass_(compass),
      dbcont_manager_(*manager)
{
    registerParameter("name", &name_, "Undefined");
    registerParameter("id", &id_, 0);
    registerParameter("info", &info_, "");
    registerParameter("db_table_name", &db_table_name_, "");

    assert (db_table_name_.size());

    createSubConfigurables();

    logdbg << "DBContent: constructor: created with instance_id " << instanceId() << " name "
           << name_;

    checkStaticVariable(DBContent::meta_var_datasource_id_);
    checkStaticVariable(DBContent::meta_var_latitude_);
    checkStaticVariable(DBContent::meta_var_longitude_);

    if (name_ == "CAT001" || name_ == "CAT048")
    {
        checkStaticVariable(DBContent::var_radar_range_);
        checkStaticVariable(DBContent::var_radar_azimuth_);
        checkStaticVariable(DBContent::var_radar_altitude_);
    }
}

DBContent::~DBContent()
{
    logdbg << "DBContent: dtor: " << name_;
}

void DBContent::generateSubConfigurable(const string& class_id, const string& instance_id)
{
    logdbg << "DBContent: generateSubConfigurable: generating variable " << instance_id;
    if (class_id == "Variable")
    {
        Variable* var = new Variable(class_id, instance_id, this);

        if (hasVariable(var->name()))
            logerr << "DBContent: generateSubConfigurable: duplicate variable " << instance_id
                   << " with name '" << var->name() << "'";

        assert(!hasVariable(var->name()));

        logdbg << "DBContent: generateSubConfigurable: generating variable " << instance_id
               << " with name " << var->name();

        variables_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(var->name()),   // args for key
                    std::forward_as_tuple(var));  // args for mapped value
    }
    else
        throw runtime_error("DBContent: generateSubConfigurable: unknown class_id " + class_id);
}

void DBContent::checkSubConfigurables()
{
    // nothing to see here
}

bool DBContent::hasVariable(const string& name) const
{
    return variables_.count(name);
}

Variable& DBContent::variable(const string& name) const
{
    assert(hasVariable(name));

    return *(variables_.at(name).get());
}

void DBContent::renameVariable(const string& old_name, const string& new_name)
{
    loginf << "DBContent: renameVariable: name " << old_name << " new_name " << new_name;

    assert(hasVariable(old_name));
    assert(!hasVariable(new_name));

    std::unique_ptr<Variable> var = std::move(variables_.at(old_name));
    variables_.erase(old_name);
    var->name(new_name);
    variables_.emplace(new_name, std::move(var));

    assert(!hasVariable(old_name));
    assert(hasVariable(new_name));

}

void DBContent::deleteVariable(const string& name)
{
    assert(hasVariable(name));

    variables_.erase(name);
    assert(!hasVariable(name));
}

bool DBContent::hasVariableDBColumnName(const std::string& col_name) const
{

    for (const auto& var : variables_)
    {
        if (var.second->dbColumnName() == col_name)
            return true;
    }

    return false;

}

unsigned int DBContent::id()
{
    return id_;
}

bool DBContent::hasKeyVariable()
{
    for (const auto& var_it : variables_)
        if (var_it.second->isKey())
            return true;

    return false;
}

Variable& DBContent::getKeyVariable()
{
    assert(hasKeyVariable());

    for (const auto& var_it : variables_)  // search in any
        if (var_it.second->isKey())
        {
            loginf << "DBContent " << name() << ": getKeyVariable: returning first found var "
                   << var_it.first;
            return *var_it.second.get();
        }

    throw runtime_error("DBContent: getKeyVariable: no key variable found");
}

string DBContent::status()
{
    if (read_job_)
    {
        if (loadedCount())
            return "Loading";
        else
        {
            if (read_job_->started())
                return "Started";
            else
                return "Queued";
        }
    }
    //    else if (finalize_jobs_.size() > 0)
    //        return "Post-processing";
    else
        return "Idle";
}

DBContentWidget* DBContent::widget()
{
    if (!widget_)
    {
        widget_.reset(new DBContentWidget(this));
        assert(widget_);
    }

    return widget_.get();  // needed for qt integration, not pretty
}

void DBContent::closeWidget() { widget_ = nullptr; }

void DBContent::load(dbContent::VariableSet& read_set, bool use_datasrc_filters, bool use_filters,
                     const std::string& custom_filter_clause)
{
    assert(is_loadable_);
    assert(existsInDB());

    string filter_clause;

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    if (use_datasrc_filters && (ds_man.hasDSFilter(name_) || ds_man.lineSpecificLoadingRequired(name_)))
    {
        vector<unsigned int> ds_ids_to_load = ds_man.unfilteredDS(name_);
        assert (ds_ids_to_load.size());

        assert (hasVariable(DBContent::meta_var_datasource_id_.name()));

        Variable& datasource_var = variable(DBContent::meta_var_datasource_id_.name());
        assert (datasource_var.dataType() == PropertyDataType::UINT);

        if (ds_man.lineSpecificLoadingRequired(name_)) // ds specific line loading
        {
            loginf << "DBContent " << name_ << ": load: line specific loading wanted";

            assert (hasVariable(DBContent::meta_var_line_id_.name()));

            Variable& line_var = variable(DBContent::meta_var_line_id_.name());
            assert (line_var.dataType() == PropertyDataType::UINT);

            bool any_added = false;

            for (auto ds_id_it : ds_ids_to_load)
            {
                assert (ds_man.hasDBDataSource(ds_id_it));

                DBDataSource& src = ds_man.dbDataSource(ds_id_it);

                // prefix
                if (filter_clause.size())
                    filter_clause += " OR";
                else
                    filter_clause += " (";

                // add data source specific part
                filter_clause += " (" + datasource_var.dbColumnName() + " = " + to_string(ds_id_it);

                if (!src.anyLinesLoadingWanted()) // check if any lines should be loaded
                {
                    filter_clause += " AND " + line_var.dbColumnName() + " IN ())"; // empty lines to load

                    any_added = true;
                    continue;
                }

                filter_clause += " AND " + line_var.dbColumnName() + " IN (";

                bool first = true;
                for (auto line_it : src.getLoadingWantedLines())
                {
                    if (!first)
                        filter_clause += ",";

                    filter_clause += to_string(line_it);

                    first = false;
                }

                filter_clause += "))";

                any_added = true;
            }

            if (ds_ids_to_load.size() && any_added)
                filter_clause += ")";
        }
        else // simple ds id in statement
        {
            loginf << "DBContent " << name_ << ": load: no line specific loading wanted";

            filter_clause = datasource_var.dbColumnName() + " IN (";

            for (auto ds_id_it = ds_ids_to_load.begin(); ds_id_it != ds_ids_to_load.end(); ++ds_id_it)
            {
                if (ds_id_it != ds_ids_to_load.begin())
                    filter_clause += ",";

                filter_clause += to_string(*ds_id_it);
            }

            filter_clause += ")";
        }
    }

    if (use_filters)
    {
        string filter_sql = COMPASS::instance().filterManager().getSQLCondition(name_);

        if (filter_sql.size())
        {
            if (filter_clause.size())
                filter_clause += " AND ";

            filter_clause += filter_sql;
        }
    }

    if (custom_filter_clause.size())
    {
        if (filter_clause.size())
            filter_clause += " AND ";

        filter_clause += custom_filter_clause;
    }

    loginf << "DBContent: load: filter_clause '" << filter_clause << "'";

    loadFiltered(read_set, filter_clause);
}

void DBContent::loadFiltered(dbContent::VariableSet& read_set, std::string custom_filter_clause)
{
    logdbg << "DBContent: loadFiltered: name " << name_ << " loadable " << is_loadable_;

    assert(is_loadable_);
    assert(existsInDB());

    assert (!read_job_);

    // add required vars for processing
    assert (dbcont_manager_.metaCanGetVariable(name_, DBContent::meta_var_rec_num_));
    read_set.add(dbcont_manager_.metaGetVariable(name_, DBContent::meta_var_rec_num_));

    assert (dbcont_manager_.metaCanGetVariable(name_, DBContent::meta_var_datasource_id_));
    read_set.add(dbcont_manager_.metaGetVariable(name_, DBContent::meta_var_datasource_id_));

    assert (dbcont_manager_.metaCanGetVariable(name_, DBContent::meta_var_line_id_));
    read_set.add(dbcont_manager_.metaGetVariable(name_, DBContent::meta_var_line_id_));

    read_job_ = shared_ptr<DBContentReadDBJob>(
                new DBContentReadDBJob(COMPASS::instance().interface(), *this, read_set, custom_filter_clause));

    connect(read_job_.get(), &DBContentReadDBJob::intermediateSignal,
            this, &DBContent::readJobIntermediateSlot, Qt::QueuedConnection);
    connect(read_job_.get(),  &DBContentReadDBJob::obsoleteSignal,
            this, &DBContent::readJobObsoleteSlot, Qt::QueuedConnection);
    connect(read_job_.get(), &DBContentReadDBJob::doneSignal,
            this, &DBContent::readJobDoneSlot, Qt::QueuedConnection);

    JobManager::instance().addDBJob(read_job_);
}

void DBContent::quitLoading()
{
    if (read_job_)
    {
        read_job_->setObsolete();
    }
}

void DBContent::insertData(shared_ptr<Buffer> buffer)
{
    logdbg << "DBContent " << name_ << ": insertData: buffer " << buffer->size();

    assert (!insert_active_);
    insert_active_ = true;

    VariableSet list;

    for (auto prop_it : buffer->properties().properties())
    {
        assert (hasVariable(prop_it.name()));
        list.add(variable(prop_it.name()));
    }

    assert(!insert_job_);

    // transform variable names from dbovars to dbcolumns
    buffer->transformVariables(list, false);

    doDataSourcesBeforeInsert(buffer);

    insert_job_ = make_shared<InsertBufferDBJob>(COMPASS::instance().interface(), *this, buffer, false);

    connect(insert_job_.get(), &InsertBufferDBJob::doneSignal, this, &DBContent::insertDoneSlot,
            Qt::QueuedConnection);

    JobManager::instance().addDBJob(insert_job_);

    logdbg << "DBContent: insertData: end";
}

void DBContent::doDataSourcesBeforeInsert (shared_ptr<Buffer> buffer)
{
    logdbg << "DBContent " << name_ << ": doDataSourcesBeforeInsert";

    assert (hasVariable(DBContent::meta_var_datasource_id_.name()));

    // ds
    Variable& datasource_var = variable(DBContent::meta_var_datasource_id_.name());
    assert (datasource_var.dataType() == PropertyDataType::UINT);

    string datasource_col_str = datasource_var.dbColumnName();
    assert (buffer->has<unsigned int>(datasource_col_str));

    // line
    Variable& line_var = variable(DBContent::meta_var_line_id_.name());
    assert (line_var.dataType() == PropertyDataType::UINT);

    string line_col_str = line_var.dbColumnName();
    assert (buffer->has<unsigned int>(line_col_str));

    // timestamp
    Variable& timestamp_var = variable(DBContent::meta_var_timestamp_.name());
    assert (timestamp_var.dataType() == PropertyDataType::TIMESTAMP);
    string timestamp_col_str = timestamp_var.dbColumnName();
    assert (buffer->has<boost::posix_time::ptime>(timestamp_col_str));

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    NullableVector<unsigned int>& datasource_vec = buffer->get<unsigned int>(datasource_col_str);
    NullableVector<unsigned int>& line_vec = buffer->get<unsigned int>(line_col_str);
    NullableVector<boost::posix_time::ptime>& timestamp_vec = buffer->get<boost::posix_time::ptime>(timestamp_col_str);

    map<unsigned int, map<unsigned int, unsigned int>> line_counts; // ds_id -> line -> cnt
    map<unsigned int, map<unsigned int, boost::posix_time::ptime>> line_tods; // ds_id -> line-> last timestamp

    unsigned int buffer_size = buffer->size();

    assert (datasource_vec.isNeverNull());
    assert (line_vec.isNeverNull());

    for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
    {
        line_counts[datasource_vec.get(cnt)][line_vec.get(cnt)]++;

        if (!timestamp_vec.isNull(cnt))
            line_tods[datasource_vec.get(cnt)][line_vec.get(cnt)] = timestamp_vec.get(cnt);
    }

    for (auto& ds_id_it : line_counts) // ds_id -> line -> cnt
    {
        // add s
        if (!ds_man.hasDBDataSource(ds_id_it.first))
            ds_man.addNewDataSource(ds_id_it.first);

        assert (ds_man.hasDBDataSource(ds_id_it.first));

        for (auto& line_cnt_it : ds_id_it.second) // line -> cnt
            ds_man.dbDataSource(ds_id_it.first).addNumInserted(name_, line_cnt_it.first, line_cnt_it.second);

        if (line_tods.count(ds_id_it.first))
        {
            for (auto& line_tod_it : line_tods.at(ds_id_it.first))
                ds_man.dbDataSource(ds_id_it.first).maxTimestamp(line_tod_it.first, line_tod_it.second);
        }
    }
}


void DBContent::insertDoneSlot()
{
    logdbg << "DBContent " << name_ << ": insertDoneSlot";

    assert(insert_job_);

    is_loadable_ = true;
    count_ += insert_job_->buffer()->size();

    insert_job_ = nullptr;
    insert_active_ = false;

    dbcont_manager_.insertDone(*this);

    assert (existsInDB()); // check
}

void DBContent::updateData(Variable& key_var, shared_ptr<Buffer> buffer)
{
    assert(!update_job_);

    assert(existsInDB());

    VariableSet list;

    for (auto prop_it : buffer->properties().properties())
    {
        assert (hasVariable(prop_it.name()));
        list.add(variable(prop_it.name()));
    }

    assert(!insert_job_);

    // transform variable names from dbovars to dbcolumns
    buffer->transformVariables(list, false);

    update_job_ =
            make_shared<UpdateBufferDBJob>(COMPASS::instance().interface(), *this, key_var, buffer);

    connect(update_job_.get(), &UpdateBufferDBJob::doneSignal, this, &DBContent::updateDoneSlot,
            Qt::QueuedConnection);
    connect(update_job_.get(), &UpdateBufferDBJob::updateProgressSignal, this,
            &DBContent::updateProgressSlot, Qt::QueuedConnection);

    JobManager::instance().addDBJob(update_job_);
}

void DBContent::deleteDBContentData()
{
    loginf << "DBContent: deleteDBContentData: dbcontent_name '" << name_ << "'";

    if (!existsInDB())
        return;

    assert (!delete_job_);

    delete_job_ = make_shared<DBContentDeleteDBJob>(COMPASS::instance().interface());
    delete_job_->setSpecificDBContent(name_);

    connect(delete_job_.get(), &DBContentDeleteDBJob::doneSignal, this, &DBContent::deleteJobDoneSlot,
            Qt::QueuedConnection);

    JobManager::instance().addDBJob(delete_job_);
}

void DBContent::deleteDBContentData(unsigned int sac, unsigned int sic)
{
    loginf << "DBContent: deleteDBContentData: dbcontent_name '" << name_ << "' sac/sic " << sac << "/" << sic;

    if (!existsInDB())
        return;

    assert (!delete_job_);

    delete_job_ = make_shared<DBContentDeleteDBJob>(COMPASS::instance().interface());
    delete_job_->setSpecificDBContent(name_);

    connect(delete_job_.get(), &DBContentDeleteDBJob::doneSignal, this, &DBContent::deleteJobDoneSlot,
            Qt::QueuedConnection);

    JobManager::instance().addDBJob(delete_job_);
}

void DBContent::deleteDBContentData(unsigned int sac, unsigned int sic, unsigned int line_id)
{
    loginf << "DBContent: deleteDBContentData: dbcontent_name '" << name_ << "' sac/sic " << sac << "/" << sic
           << " line_id " << line_id;

    if (!existsInDB())
        return;

    assert (!delete_job_);

    delete_job_ = make_shared<DBContentDeleteDBJob>(COMPASS::instance().interface());
    delete_job_->setSpecificDBContent(name_);

    connect(delete_job_.get(), &DBContentDeleteDBJob::doneSignal, this, &DBContent::deleteJobDoneSlot,
            Qt::QueuedConnection);

    JobManager::instance().addDBJob(delete_job_);
}

void DBContent::updateProgressSlot(float percent) { emit updateProgressSignal(percent); }

void DBContent::updateDoneSlot()
{
    update_job_ = nullptr;

    emit updateDoneSignal(*this);
}

void DBContent::deleteJobDoneSlot()
{
    loginf << "DBContent: deleteJobDoneSlot";

    assert (delete_job_);

    delete_job_ = nullptr;

    // remove from inserted count
    COMPASS::instance().dataSourceManager().clearInsertedCounts(name_);
    COMPASS::instance().dataSourceManager().saveDBDataSources();

    // remove from targets count
    dbcont_manager_.removeDBContentFromTargets(name_);

    count_ = 0;
}

void DBContent::readJobIntermediateSlot(shared_ptr<Buffer> buffer)
{
    assert(buffer);
    loginf << "DBContent: " << name_ << " readJobIntermediateSlot: buffer size " << buffer->size();

    DBContentReadDBJob* sender = dynamic_cast<DBContentReadDBJob*>(QObject::sender());

    assert (sender);
    assert(sender == read_job_.get());

    // check variables
    const vector<Variable*>& variables = sender->readList().getSet();
    const PropertyList& properties = buffer->properties();

    for (auto var_it : variables)
    {
        assert(properties.hasProperty(var_it->dbColumnName()));
        const Property& property = properties.get(var_it->dbColumnName());
        assert(property.dataType() == var_it->dataType());
    }

    logdbg << "DBContent: " << name_ << " readJobIntermediateSlot: got buffer with size "
           << buffer->size();

    // finalize buffer
    buffer->transformVariables(sender->readList(), true);

    // add boolean to indicate selection
    buffer->addProperty(DBContent::selected_var);

    // add loaded data
    dbcont_manager_.addLoadedData({{name_, buffer}});

    if (!isLoading())  // is last one
    {
        loginf << "DBContent: " << name_ << " finalizeReadJobDoneSlot: loading done";
        dbcont_manager_.loadingDone(*this);
    }


}

void DBContent::readJobObsoleteSlot()
{
    logdbg << "DBContent: " << name_ << " readJobObsoleteSlot";
    read_job_ = nullptr;
    //read_job_data_.clear();
}

void DBContent::readJobDoneSlot()
{
    logdbg << "DBContent: " << name_ << " readJobDoneSlot";
    read_job_ = nullptr;

    if (!isLoading()) // also no more finalize jobs
    {
        loginf << "DBContent: " << name_ << " readJobDoneSlot: done";
        dbcont_manager_.loadingDone(*this);
    }
}

void DBContent::databaseOpenedSlot()
{
    loginf << "DBContent " << name_ << ": databaseOpenedSlot";

    //string associations_table_name = associationsTableName();

    is_loadable_ = existsInDB();

    if (is_loadable_)
        count_ = COMPASS::instance().interface().count(db_table_name_);

    logdbg << "DBContent: " << name_ << " databaseOpenedSlot: table " << db_table_name_
           << " count " << count_;
}

void DBContent::databaseClosedSlot()
{
    loginf << "DBContent: databaseClosedSlot";

    is_loadable_ = false;
    count_ = 0;
}

string DBContent::dbTableName() const
{
    return db_table_name_;
}

bool DBContent::isLoading() { return read_job_ != nullptr; }

bool DBContent::isInserting() { return insert_active_; }

bool DBContent::isDeleting() { return delete_job_ != nullptr; }

bool DBContent::hasData() { return count_ > 0; }

size_t DBContent::count() { return count_; }

size_t DBContent::loadedCount()
{
    if (dbcont_manager_.data().count(name_))
        return dbcont_manager_.data().at(name_)->size();
    else
        return 0;
}

bool DBContent::existsInDB() const
{
    return COMPASS::instance().interface().existsTable(db_table_name_);
}

void DBContent::checkStaticVariable(const Property& property)
{
    if (!hasVariable(property.name()))
        logwrn << "DBContent: checkStaticVariable: " << name_ << " has no variable " << property.name();
    else if (variable(property.name()).dataType() != property.dataType())
        logwrn << "DBContent: checkStaticVariable: " << name_ << " variable " << property.name()
               << " has wrong data type (" << variable(property.name()).dataTypeString()
               << " insteaf of " << property.dataTypeString() << ")";
}

