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
#include "dbcontent/labeldefinition.h"
#include "dbcontent/labeldefinitionwidget.h"
#include "datasourcemanager.h"
#include "dboreaddbjob.h"
#include "dbcontent/variable/variable.h"
#include "dbtableinfo.h"
#include "filtermanager.h"
#include "finalizedboreadjob.h"
#include "insertbufferdbjob.h"
#include "jobmanager.h"
#include "propertylist.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "updatebufferdbjob.h"
#include "viewmanager.h"
#include "util/number.h"
#include "dbcontent/variable/metavariable.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <memory>

using namespace std;
using namespace Utils;
using namespace dbContent;

const Property DBContent::meta_var_rec_num_ {"Record Number", PropertyDataType::UINT};
const Property DBContent::meta_var_datasource_id_ {"DS ID", PropertyDataType::UINT};
const Property DBContent::meta_var_sac_id_ {"SAC", PropertyDataType::UCHAR};
const Property DBContent::meta_var_sic_id_ {"SIC", PropertyDataType::UCHAR};
const Property DBContent::meta_var_line_id_ {"Line ID", PropertyDataType::UINT};
const Property DBContent::meta_var_tod_ {"Time of Day", PropertyDataType::FLOAT};
const Property DBContent::meta_var_m3a_ {"Mode 3/A Code", PropertyDataType::UINT};
const Property DBContent::meta_var_m3a_g_ {"Mode 3/A Garbled", PropertyDataType::BOOL};
const Property DBContent::meta_var_m3a_v_ {"Mode 3/A Valid", PropertyDataType::BOOL};
const Property DBContent::meta_var_ta_ {"Aircraft Address", PropertyDataType::UINT};
const Property DBContent::meta_var_ti_ {"Aircraft Identification", PropertyDataType::STRING};
const Property DBContent::meta_var_mc_ {"Mode C Code", PropertyDataType::FLOAT};
const Property DBContent::meta_var_mc_g_ {"Mode C Garbled", PropertyDataType::BOOL};
const Property DBContent::meta_var_mc_v_ {"Mode C Valid", PropertyDataType::BOOL};
const Property DBContent::meta_var_ground_bit_ {"Ground Bit", PropertyDataType::BOOL};
const Property DBContent::meta_var_track_num_ {"Track Number", PropertyDataType::UINT};
const Property DBContent::meta_var_track_end_ {"Track End", PropertyDataType::BOOL};

const Property DBContent::meta_var_latitude_ {"Latitude", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_longitude_ {"Longitude", PropertyDataType::DOUBLE};

const Property DBContent::meta_var_detection_type_ {"Type", PropertyDataType::UCHAR};
const Property DBContent::meta_var_artas_hash_ {"ARTAS Hash", PropertyDataType::UINT};
const Property DBContent::meta_var_associations_ {"Associations", PropertyDataType::JSON};

const Property DBContent::meta_var_vx_ {"Vx", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_vy_ {"Vy", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_ground_speed_ {"Track Groundspeed", PropertyDataType::DOUBLE};
const Property DBContent::meta_var_track_angle_ {"Track Angle", PropertyDataType::DOUBLE};

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

const Property DBContent::var_cat062_tris_ {"Target Report Identifiers", PropertyDataType::STRING};
const Property DBContent::var_cat062_track_begin_ {"Track Begin", PropertyDataType::BOOL};
const Property DBContent::var_cat062_coasting_ {"Coasting", PropertyDataType::BOOL};
const Property DBContent::var_cat062_track_end_ {"Track End", PropertyDataType::BOOL};
const Property DBContent::var_cat062_baro_alt_ {"Barometric Altitude Calculated", PropertyDataType::FLOAT};

const Property DBContent::var_cat062_wtc_ {"Wake Turbulence Category FPL", PropertyDataType::STRING};

const Property DBContent::selected_var {"selected", PropertyDataType::BOOL};

DBContent::DBContent(COMPASS& compass, const string& class_id, const string& instance_id,
                     DBContentManager* manager)
    : Configurable(class_id, instance_id, manager,
                   "db_content_" + boost::algorithm::to_lower_copy(instance_id) + ".json"),
      compass_(compass),
      dbo_manager_(*manager)
{
    registerParameter("name", &name_, "Undefined");
    registerParameter("info", &info_, "");
    registerParameter("db_table_name", &db_table_name_, "");

    assert (db_table_name_.size());

    constructor_active_ = true;

    createSubConfigurables();

    constructor_active_ = false;

    sortContent();

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
        string var_name = configuration()
                .getSubConfiguration(class_id, instance_id)
                .getParameterConfigValueString("name");

        if (hasVariable(var_name))
            logerr << "DBContent: generateSubConfigurable: duplicate variable " << instance_id
                   << " with name '" << var_name << "'";

        assert(!hasVariable(var_name));

        logdbg << "DBContent: generateSubConfigurable: generating variable " << instance_id
               << " with name " << var_name;

        variables_.emplace_back(new Variable(class_id, instance_id, this));
    }
    else if (class_id == "LabelDefinition")
    {
        assert(!label_definition_);
        label_definition_.reset(new dbContent::LabelDefinition(class_id, instance_id, this, dbo_manager_));
    }
    else
        throw runtime_error("DBContent: generateSubConfigurable: unknown class_id " + class_id);

    if (!constructor_active_)
        sortContent();
}

void DBContent::checkSubConfigurables()
{
    // nothing to see here

    if (!label_definition_)
    {
        generateSubConfigurable("LabelDefinition", "LabelDefinition0");
        assert(label_definition_);
    }
}

bool DBContent::hasVariable(const string& name) const
{
    auto iter = find_if(variables_.begin(), variables_.end(),
                        [name](const unique_ptr<Variable>& var) { return var->name() == name;});

    return iter != variables_.end();

    //return variables_.find(name) != variables_.end();
}

Variable& DBContent::variable(const string& name)
{
    assert(hasVariable(name));

    auto iter = find_if(variables_.begin(), variables_.end(),
                        [name](const unique_ptr<Variable>& var) { return var->name() == name;});

    assert (iter != variables_.end());
    assert (iter->get());

    return *iter->get();
}

void DBContent::renameVariable(const string& name, const string& new_name)
{
    loginf << "DBContent: renameVariable: name " << name << " new_name " << new_name;

    string old_name = name; // since passed by reference, which will be changed

    assert(hasVariable(old_name));
    assert(!hasVariable(new_name));

    variable(old_name).name(new_name);

    loginf << "DBContent: renameVariable: has old var '" << old_name << "' " << hasVariable(old_name);
    assert(!hasVariable(old_name));
    loginf << "DBContent: renameVariable: has var '" << new_name << "' " << hasVariable(new_name);
    assert(hasVariable(new_name));

}

void DBContent::deleteVariable(const string& name)
{
    assert(hasVariable(name));

    auto iter = find_if(variables_.begin(), variables_.end(),
                        [name](const unique_ptr<Variable>& var) { return var->name() == name;});
    assert (iter != variables_.end());

    variables_.erase(iter);
    assert(!hasVariable(name));
}

bool DBContent::hasVariableDBColumnName(const std::string& name) const
{
    auto iter = find_if(variables_.begin(), variables_.end(),
                        [name](const unique_ptr<Variable>& var) { return var->dbColumnName() == name;});

    logdbg << "DBContent: hasVariableDBColumnName: name '" << name << "' " << (iter != variables_.end());

    return iter != variables_.end();
}

//string DBContent::associationsTableName()
//{
//    assert (db_table_name_.size());
//    return db_table_name_ + "_assoc";
//}


bool DBContent::hasKeyVariable()
{
    for (auto& var_it : variables_)
        if (var_it->isKey())
            return true;

    return false;
}

Variable& DBContent::getKeyVariable()
{
    assert(hasKeyVariable());

    for (auto& var_it : variables_)  // search in any
        if (var_it->isKey())
        {
            loginf << "DBContent " << name() << ": getKeyVariable: returning first found var "
                   << var_it->name();
            return *var_it.get();
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

dbContent::LabelDefinitionWidget* DBContent::labelDefinitionWidget()
{
    assert(label_definition_);
    return label_definition_->widget();
}

void DBContent::load(VariableSet& read_set, bool use_filters, bool use_order,
                     Variable* order_variable, bool use_order_ascending,
                     const string& limit_str)
{
    assert(is_loadable_);
    assert(existsInDB());

    string custom_filter_clause;
    std::vector<std::string> extra_from_parts;
    vector<Variable*> filtered_variables;

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    if (ds_man.hasDSFilter(name_))
    {
        vector<unsigned int> ds_ids_to_load = ds_man.unfilteredDS(name_);
        assert (ds_ids_to_load.size());

        assert (hasVariable(DBContent::meta_var_datasource_id_.name()));

        Variable& datasource_var = variable(DBContent::meta_var_datasource_id_.name());
        assert (datasource_var.dataType() == PropertyDataType::UINT);

        if (ds_man.lineSpecificLoadingRequired(name_)) // ds specific line loading
        {
            assert (hasVariable(DBContent::meta_var_line_id_.name()));

            Variable& line_var = variable(DBContent::meta_var_line_id_.name());
            assert (line_var.dataType() == PropertyDataType::UINT);

            // add to filtered vars
            filtered_variables.push_back(&datasource_var);
            filtered_variables.push_back(&line_var);

            for (auto ds_id_it : ds_ids_to_load)
            {
                assert (ds_man.hasDBDataSource(ds_id_it));

                DBDataSource& src = ds_man.dbDataSource(ds_id_it);

                if (!src.anyLinesLoadingWanted()) // check if any lines should be loaded
                    continue;

                if (custom_filter_clause.size())
                    custom_filter_clause += " OR";
                else
                    custom_filter_clause += " (";

                custom_filter_clause += "(" + datasource_var.dbColumnName() + " = " + to_string(ds_id_it);
                custom_filter_clause += " AND " + line_var.dbColumnName() + " IN (";

                bool first = true;
                for (auto line_it : src.getLoadingWantedLines())
                {
                    if (!first)
                        custom_filter_clause += ",";

                    custom_filter_clause += to_string(line_it);

                    first = false;
                }

                custom_filter_clause += "))";
            }

            if (ds_ids_to_load.size())
                custom_filter_clause += ")";
        }
        else // simple line id in statement
        {

            // add to filtered vars
            filtered_variables.push_back(&datasource_var);

            custom_filter_clause = datasource_var.dbColumnName() + " IN (";

            for (auto ds_id_it = ds_ids_to_load.begin(); ds_id_it != ds_ids_to_load.end(); ++ds_id_it)
            {
                if (ds_id_it != ds_ids_to_load.begin())
                    custom_filter_clause += ",";

                custom_filter_clause += to_string(*ds_id_it);
            }

            custom_filter_clause += ")";
        }
    }

    if (use_filters)
    {
        if (custom_filter_clause.size())
            custom_filter_clause += " AND ";

        custom_filter_clause +=
                COMPASS::instance().filterManager().getSQLCondition(name_, extra_from_parts, filtered_variables);
    }

    loginf << "DBContent: load: filter '" << custom_filter_clause << "'";

    loadFiltered(read_set, extra_from_parts, custom_filter_clause, filtered_variables, use_order, order_variable,
                 use_order_ascending, limit_str);
}

void DBContent::loadFiltered(VariableSet& read_set, const std::vector<std::string>& extra_from_parts,
                             string custom_filter_clause, vector<Variable*> filtered_variables,
                             bool use_order, Variable* order_variable, bool use_order_ascending,
                             const string& limit_str)
{
    logdbg << "DBContent: loadFiltered: name " << name_ << " loadable " << is_loadable_;

    assert(is_loadable_);
    assert(existsInDB());

    // do not load associations, should be done in DBContentManager::load

    if (read_job_)
    {
        JobManager::instance().cancelJob(read_job_);
        read_job_ = nullptr;
    }
    //read_job_data_.clear();

    //    for (auto job_it : finalize_jobs_)
    //        JobManager::instance().cancelJob(job_it);
    //    finalize_jobs_.clear();

    //    DBInterface &db_interface, DBContent &dbobject, VariableSet read_list, string
    //    custom_filter_clause, Variable *order, const string &limit_str

    read_job_ = shared_ptr<DBOReadDBJob>(
                new DBOReadDBJob(
                    COMPASS::instance().interface(), *this, read_set, extra_from_parts,
                    custom_filter_clause, filtered_variables,
                    use_order, order_variable, use_order_ascending, limit_str));

    connect(read_job_.get(), &DBOReadDBJob::intermediateSignal,
            this, &DBContent::readJobIntermediateSlot, Qt::QueuedConnection);
    connect(read_job_.get(),  &DBOReadDBJob::obsoleteSignal,
            this, &DBContent::readJobObsoleteSlot, Qt::QueuedConnection);
    connect(read_job_.get(), &DBOReadDBJob::doneSignal,
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

    Variable& datasource_var = variable(DBContent::meta_var_datasource_id_.name());
    assert (datasource_var.dataType() == PropertyDataType::UINT);

    string datasource_col_str = datasource_var.dbColumnName();
    assert (buffer->has<unsigned int>(datasource_col_str));

    Variable& line_var = variable(DBContent::meta_var_line_id_.name());
    assert (line_var.dataType() == PropertyDataType::UINT);

    string line_col_str = line_var.dbColumnName();
    assert (buffer->has<unsigned int>(line_col_str));

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    NullableVector<unsigned int>& datasource_vec = buffer->get<unsigned int>(datasource_col_str);
    NullableVector<unsigned int>& line_vec = buffer->get<unsigned int>(line_col_str);

    map<unsigned int, map<unsigned int, unsigned int>> line_counts; // ds_id -> line -> cnt

    unsigned int buffer_size = buffer->size();

    assert (datasource_vec.isNeverNull());
    assert (line_vec.isNeverNull());

    for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
    {
        line_counts[datasource_vec.get(cnt)][line_vec.get(cnt)]++;
    }

    for (auto& ds_id_it : line_counts) // ds_id -> line -> cnt
    {
        if (!ds_man.hasDBDataSource(ds_id_it.first))
            ds_man.addNewDataSource(ds_id_it.first);

        assert (ds_man.hasDBDataSource(ds_id_it.first));

        for (auto& line_cnt_it : ds_id_it.second) // line -> cnt
        {

            ds_man.dbDataSource(ds_id_it.first).addNumInserted(name_, line_cnt_it.first, line_cnt_it.second);
            ds_man.dbDataSource(ds_id_it.first).addNumLoaded(name_, line_cnt_it.first, line_cnt_it.second);
            // because propagated after
        }
    }
}


void DBContent::insertDoneSlot()
{
    logdbg << "DBContent " << name_ << ": insertDoneSlot";

    assert(insert_job_);

    insert_job_ = nullptr;
    insert_active_ = false;

    dbo_manager_.insertDone(*this);

    is_loadable_ = true;

    //dbo_manager_.databaseContentChangedSlot();

    assert (existsInDB()); // check
}

void DBContent::updateData(Variable& key_var, shared_ptr<Buffer> buffer)
{
    assert(!update_job_);

    assert(existsInDB());

    update_job_ =
            make_shared<UpdateBufferDBJob>(COMPASS::instance().interface(), *this, key_var, buffer);

    connect(update_job_.get(), &UpdateBufferDBJob::doneSignal, this, &DBContent::updateDoneSlot,
            Qt::QueuedConnection);
    connect(update_job_.get(), &UpdateBufferDBJob::updateProgressSignal, this,
            &DBContent::updateProgressSlot, Qt::QueuedConnection);

    JobManager::instance().addDBJob(update_job_);
}

void DBContent::updateProgressSlot(float percent) { emit updateProgressSignal(percent); }

void DBContent::updateDoneSlot()
{
    update_job_ = nullptr;

    emit updateDoneSignal(*this);
}

map<unsigned int, string> DBContent::loadLabelData(vector<unsigned int> rec_nums, int break_item_cnt)
{
    assert(is_loadable_);
    assert(existsInDB());

    string custom_filter_clause;
    bool first = true;

    assert (dbo_manager_.existsMetaVariable(DBContent::meta_var_rec_num_.name()));
    assert (dbo_manager_.metaVariable(DBContent::meta_var_rec_num_.name()).existsIn(name_));

    Variable& rec_num_var = dbo_manager_.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(name_);

    custom_filter_clause = rec_num_var.dbColumnName() + " in (";
    for (auto& rec_num : rec_nums)
    {
        if (first)
            first = false;
        else
            custom_filter_clause += ",";

        custom_filter_clause += to_string(rec_num);
    }
    custom_filter_clause += ")";

    VariableSet read_list = label_definition_->readList();

    if (!read_list.hasVariable(rec_num_var))
        read_list.add(rec_num_var);

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    DBInterface& db_interface = COMPASS::instance().interface();

    db_interface.prepareRead(*this, read_list, {}, custom_filter_clause, {}, false, nullptr, false, "");
    shared_ptr<Buffer> buffer = db_interface.readDataChunk(*this);
    db_interface.finalizeReadStatement(*this);

    if (buffer->size() != rec_nums.size())
        throw runtime_error("DBContent " + name_ +
                            ": loadLabelData: failed to load label for " +
                            custom_filter_clause);

    assert(buffer->size() == rec_nums.size());

    map<unsigned int, string> labels =
            label_definition_->generateLabels(rec_nums, buffer, break_item_cnt);

    boost::posix_time::ptime stop_time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = stop_time - start_time;

    logdbg << "DBContent: loadLabelData: done after " << diff.total_milliseconds() << " ms";

    return labels;
}

void DBContent::readJobIntermediateSlot(shared_ptr<Buffer> buffer)
{
    assert(buffer);
    loginf << "DBContent: " << name_ << " readJobIntermediateSlot: buffer size " << buffer->size();

    DBOReadDBJob* sender = dynamic_cast<DBOReadDBJob*>(QObject::sender());

    assert (sender);
    assert(sender == read_job_.get());

    // check variables
    vector<Variable*>& variables = sender->readList().getSet();
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
    dbo_manager_.addLoadedData({{name_, buffer}});

    if (!isLoading())  // is last one
    {
        loginf << "DBContent: " << name_ << " finalizeReadJobDoneSlot: loading done";
        dbo_manager_.loadingDone(*this);
    }

    //    read_job_data_.push_back(buffer);

    //    FinalizeDBOReadJob* job = new FinalizeDBOReadJob(*this, sender->readList(), buffer);

    //    shared_ptr<FinalizeDBOReadJob> job_ptr = shared_ptr<FinalizeDBOReadJob>(job);
    //    connect(job, SIGNAL(doneSignal()), this, SLOT(finalizeReadJobDoneSlot()), Qt::QueuedConnection);
    //    finalize_jobs_.push_back(job_ptr);

    //    JobManager::instance().addBlockingJob(job_ptr);

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
        dbo_manager_.loadingDone(*this);
    }
}

//void DBContent::finalizeReadJobDoneSlot()
//{
//    logdbg << "DBContent: " << name_ << " finalizeReadJobDoneSlot";

//    FinalizeDBOReadJob* sender = dynamic_cast<FinalizeDBOReadJob*>(QObject::sender());

//    if (!sender)
//    {
//        logwrn << "DBContent: finalizeReadJobDoneSlot: null sender, event on the loose";
//        return;
//    }

//    shared_ptr<Buffer> buffer = sender->buffer();
//    assert (buffer);

//    bool found = false;
//    for (auto final_it : finalize_jobs_)
//    {
//        if (final_it.get() == sender)
//        {
//            finalize_jobs_.erase(find(finalize_jobs_.begin(), finalize_jobs_.end(), final_it));
//            found = true;
//            break;
//        }
//    }
//    assert(found);

//    // add loaded data
//    dbo_manager_.addLoadedData({{name_, buffer}});

//    if (!isLoading())  // is last one
//    {
//        loginf << "DBContent: " << name_ << " finalizeReadJobDoneSlot: loading done";
//        dbo_manager_.loadingDone(*this);
//    }

//    return;
//}

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

void DBContent::checkLabelDefinitions()
{
    assert (label_definition_);
    label_definition_->checkLabelDefinitions();
}

//bool DBContent::associationsLoaded() const
//{
//    return associations_loaded_;
//}

bool DBContent::isLoading() { return read_job_ != nullptr; }

bool DBContent::isInserting() { return insert_active_; }

//bool DBContent::isPostProcessing() { return finalize_jobs_.size(); }

bool DBContent::hasData() { return count_ > 0; }

size_t DBContent::count() { return count_; }

size_t DBContent::loadedCount()
{
    if (dbo_manager_.data().count(name_))
        return dbo_manager_.data().at(name_)->size();
    else
        return 0;
}

bool DBContent::existsInDB() const
{
    return COMPASS::instance().interface().existsTable(db_table_name_);
}

//void DBContent::loadAssociationsIfRequired()
//{
//    if (dbo_manager_.hasAssociations() && !associations_loaded_)
//    {
//        shared_ptr<DBOReadAssociationsJob> read_job =
//                make_shared<DBOReadAssociationsJob>(*this);
//        JobManager::instance().addDBJob(read_job);  // fire and forget
//    }
//}

//void DBContent::loadAssociations()
//{
//    loginf << "DBContent " << name_ << ": loadAssociations";

//    associations_.clear();

//    boost::posix_time::ptime loading_start_time;
//    boost::posix_time::ptime loading_stop_time;

//    loading_start_time = boost::posix_time::microsec_clock::local_time();

//    DBInterface& db_interface = COMPASS::instance().interface();

//    string associations_table_name = associationsTableName();

//    if (db_interface.existsTable(associations_table_name))
//        associations_ = db_interface.getAssociations(associations_table_name);

//    associations_loaded_ = true;

//    loading_stop_time = boost::posix_time::microsec_clock::local_time();

//    double load_time;
//    boost::posix_time::time_duration diff = loading_stop_time - loading_start_time;
//    load_time = diff.total_milliseconds() / 1000.0;

//    loginf << "DBContent " << name_ << ": loadAssociations: " << associations_.size()
//           << " associactions done (" << String::doubleToStringPrecision(load_time, 2) << " s).";
//}

//bool DBContent::hasAssociations() { return associations_.size() > 0; }

//void DBContent::addAssociation(unsigned int rec_num, unsigned int utn, bool has_src, unsigned int src_rec_num)
//{
//    associations_.add(rec_num, DBOAssociationEntry(utn, has_src, src_rec_num));
//    associations_changed_ = true;
//    associations_loaded_ = true;
//}

//void DBContent::clearAssociations()
//{
//    associations_.clear();
//    associations_changed_ = true;
//    associations_loaded_ = false;
//}

//void DBContent::saveAssociations()
//{
//    loginf << "DBContent " << name_ << ": saveAssociations";

//    DBInterface& db_interface = COMPASS::instance().interface();

//    string associations_table_name = associationsTableName();
//    assert(associations_table_name.size());

//    if (db_interface.existsTable(associations_table_name))
//        db_interface.clearTableContent(associations_table_name);
//    else
//        db_interface.createAssociationsTable(associations_table_name);

//    if (!hasAssociations())
//        return;

//    assert(db_interface.existsTable(associations_table_name));

//    // assoc_id INT, rec_num INT, utn INT

//    PropertyList list;
//    list.addProperty("rec_num", PropertyDataType::INT);
//    list.addProperty("utn", PropertyDataType::INT);
//    list.addProperty("src_rec_num", PropertyDataType::INT);

//    shared_ptr<Buffer> buffer_ptr = shared_ptr<Buffer>(new Buffer(list, name_));

//    NullableVector<int>& rec_nums = buffer_ptr->get<int>("rec_num");
//    NullableVector<int>& utns = buffer_ptr->get<int>("utn");
//    NullableVector<int>& src_rec_nums = buffer_ptr->get<int>("src_rec_num");

//    size_t cnt = 0;
//    for (auto& assoc_it : associations_)
//    {
//        rec_nums.set(cnt, assoc_it.first);
//        utns.set(cnt, assoc_it.second.utn_);

//        if (assoc_it.second.has_src_)
//            src_rec_nums.set(cnt, assoc_it.second.src_rec_num_);

//        ++cnt;
//    }

//    db_interface.insertBuffer(associations_table_name, buffer_ptr);

//    associations_changed_ = false;

//    loginf << "DBContent " << name_ << ": saveAssociations: done";
//}

void DBContent::sortContent()
{
    sort(variables_.begin(), variables_.end(),
         [](const std::unique_ptr<Variable>& a, const std::unique_ptr<Variable>& b) -> bool
    {
        return a->name() < b->name();
    });
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

