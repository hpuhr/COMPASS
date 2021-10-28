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

#include "dbobject.h"

#include "compass.h"
#include "buffer.h"
#include "dbinterface.h"
#include "dbobjectinfowidget.h"
#include "dbobjectmanager.h"
#include "dbobjectwidget.h"
#include "dbolabeldefinition.h"
#include "dbolabeldefinitionwidget.h"
#include "dboreadassociationsjob.h"
#include "dboreaddbjob.h"
#include "dbovariable.h"
#include "dbtableinfo.h"
#include "filtermanager.h"
#include "finalizedboreadjob.h"
#include "insertbufferdbjob.h"
#include "jobmanager.h"
#include "propertylist.h"
#include "stringconv.h"
#include "taskmanager.h"
#include "updatebufferdbjob.h"

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <memory>

using namespace Utils;

DBObject::DBObject(COMPASS& compass, const std::string& class_id, const std::string& instance_id,
                   DBObjectManager* manager)
    : Configurable(class_id, instance_id, manager,
                   "db_object_" + boost::algorithm::to_lower_copy(instance_id) + ".json"),
      compass_(compass),
      manager_(*manager)
{
    registerParameter("name", &name_, "Undefined");
    registerParameter("info", &info_, "");
    registerParameter("db_table_name", &db_table_name_, "");

    assert (db_table_name_.size());

    createSubConfigurables();

    qRegisterMetaType<std::shared_ptr<Buffer>>("std::shared_ptr<Buffer>");

    logdbg << "DBObject: constructor: created with instance_id " << instanceId() << " name "
           << name_;
}

DBObject::~DBObject()
{
    logdbg << "DBObject: dtor: " << name_;
}

void DBObject::generateSubConfigurable(const std::string& class_id, const std::string& instance_id)
{
    logdbg << "DBObject: generateSubConfigurable: generating variable " << instance_id;
    if (class_id == "DBOVariable")
    {
        std::string var_name = configuration()
                                   .getSubConfiguration(class_id, instance_id)
                                   .getParameterConfigValueString("name");

        assert(variables_.find(var_name) == variables_.end());

        logdbg << "DBObject: generateSubConfigurable: generating variable " << instance_id
               << " with name " << var_name;

        variables_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(var_name),                      // args for key
            std::forward_as_tuple(class_id, instance_id, this));  // args for mapped value
    }
    else if (class_id == "DBOLabelDefinition")
    {
        assert(!label_definition_);
        label_definition_.reset(new DBOLabelDefinition(class_id, instance_id, this));
    }
    else
        throw std::runtime_error("DBObject: generateSubConfigurable: unknown class_id " + class_id);
}

void DBObject::checkSubConfigurables()
{
    // nothing to see here

    if (!label_definition_)
    {
        generateSubConfigurable("DBOLabelDefinition", "DBOLabelDefinition0");
        assert(label_definition_);
    }
}

bool DBObject::hasVariable(const std::string& name) const
{
    return variables_.find(name) != variables_.end();
}

DBOVariable& DBObject::variable(const std::string& name)
{
    assert(hasVariable(name));
    return variables_.at(name);
}

void DBObject::renameVariable(const std::string& name, const std::string& new_name)
{
    loginf << "DBObject: renameVariable: name " << name << " new_name " << new_name;

    assert(hasVariable(name));
    assert(!hasVariable(new_name));

    variables_[new_name] = std::move(variables_.at(name));
    variables_.erase(name);

    assert(hasVariable(new_name));
    variables_.at(new_name).name(new_name);
}

void DBObject::deleteVariable(const std::string& name)
{
    assert(hasVariable(name));
    variables_.erase(name);
    assert(!hasVariable(name));
}

std::string DBObject::associationsTableName()
{
    assert (db_table_name_.size());
    return db_table_name_ + "_assoc";
}


bool DBObject::hasKeyVariable()
{
    for (auto& var_it : variables_)
        if (var_it.second.isKey())
            return true;

    return false;
}

DBOVariable& DBObject::getKeyVariable()
{
    assert(hasKeyVariable());

    for (auto& var_it : variables_)  // search in any
        if (var_it.second.isKey())
        {
            loginf << "DBObject " << name() << ": getKeyVariable: returning first found var "
                   << var_it.second.name();
            return var_it.second;
        }

    throw std::runtime_error("DBObject: getKeyVariable: no key variable found");
}

std::string DBObject::status()
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
    else if (finalize_jobs_.size() > 0)
        return "Post-processing";
    else
        return "Idle";
}

DBObjectWidget* DBObject::widget()
{
    if (!widget_)
    {
        widget_.reset(new DBObjectWidget(this));
        assert(widget_);
    }

    return widget_.get();  // needed for qt integration, not pretty
}

void DBObject::closeWidget() { widget_ = nullptr; }

DBObjectInfoWidget* DBObject::infoWidget()
{
    if (!info_widget_)
    {
        info_widget_.reset(new DBObjectInfoWidget(*this));
        assert(info_widget_);
    }

    return info_widget_.get();  // needed for qt integration, not pretty
}

DBOLabelDefinitionWidget* DBObject::labelDefinitionWidget()
{
    assert(label_definition_);
    return label_definition_->widget();
}

//void DBObject::schemaChangedSlot()
//{
//    loginf << "DBObject: schemaChangedSlot";

//    if (compass_.schemaManager().hasCurrentSchema())
//    {
//        DBSchema& schema = compass_.schemaManager().getCurrentSchema();

//        if (!hasMetaTable(schema.name()))
//        {
//            logwrn << "DBObject: schemaChangedSlot: object " << name_
//                   << " has not main meta table for current schema";
//            current_meta_table_ = nullptr;
//            associations_table_name_ = "";

//            return;
//        }

//        std::string meta_table_name = meta_table_definitions_.at(schema.name()).metaTable();
//        assert(schema.hasMetaTable(meta_table_name));
//        current_meta_table_ = &schema.metaTable(meta_table_name);

//        associations_table_name_ = current_meta_table_->mainTableName() + "_assoc";
//    }
//    else
//    {
//        current_meta_table_ = nullptr;
//        associations_table_name_ = "";
//    }

//    updateToDatabaseContent();
//}

void DBObject::loadingWanted(bool wanted)
{
    if (loading_wanted_ != wanted)
    {
        loading_wanted_ = wanted;

        if (info_widget_)
            info_widget_->updateSlot();
    }
}

void DBObject::load(DBOVariableSet& read_set, bool use_filters, bool use_order,
                    DBOVariable* order_variable, bool use_order_ascending,
                    const std::string& limit_str)
{
    std::string custom_filter_clause;
    std::vector<DBOVariable*> filtered_variables;

    if (use_filters)
    {
        custom_filter_clause =
            COMPASS::instance().filterManager().getSQLCondition(name_, filtered_variables);
    }

    load(read_set, custom_filter_clause, filtered_variables, use_order, order_variable,
         use_order_ascending, limit_str);
}

void DBObject::load(DBOVariableSet& read_set, std::string custom_filter_clause,
                    std::vector<DBOVariable*> filtered_variables, bool use_order,
                    DBOVariable* order_variable, bool use_order_ascending,
                    const std::string& limit_str)
{
    logdbg << "DBObject: load: name " << name_ << " loadable " << is_loadable_;

    assert(is_loadable_);
    assert(existsInDB());

    // do not load associations, should be done in DBObjectManager::load

    if (read_job_)
    {
        JobManager::instance().cancelJob(read_job_);
        read_job_ = nullptr;
    }
    read_job_data_.clear();

    for (auto job_it : finalize_jobs_)
        JobManager::instance().cancelJob(job_it);
    finalize_jobs_.clear();

    clearData();

    //    DBInterface &db_interface, DBObject &dbobject, DBOVariableSet read_list, std::string
    //    custom_filter_clause, DBOVariable *order, const std::string &limit_str

    read_job_ = std::shared_ptr<DBOReadDBJob>(new DBOReadDBJob(
        COMPASS::instance().interface(), *this, read_set, custom_filter_clause, filtered_variables,
        use_order, order_variable, use_order_ascending, limit_str));

    connect(read_job_.get(), SIGNAL(intermediateSignal(std::shared_ptr<Buffer>)), this,
            SLOT(readJobIntermediateSlot(std::shared_ptr<Buffer>)), Qt::QueuedConnection);
    connect(read_job_.get(), SIGNAL(obsoleteSignal()), this, SLOT(readJobObsoleteSlot()),
            Qt::QueuedConnection);
    connect(read_job_.get(), SIGNAL(doneSignal()), this, SLOT(readJobDoneSlot()),
            Qt::QueuedConnection);

    if (info_widget_)
        info_widget_->updateSlot();

    JobManager::instance().addDBJob(read_job_);
}

void DBObject::quitLoading()
{
    if (read_job_)
    {
        read_job_->setObsolete();
    }
}

void DBObject::clearData()
{
    logdbg << "DBObject " << name_ << ": clearData";

    if (data_)
    {
        data_ = nullptr;

        if (info_widget_)
            info_widget_->updateSlot();
    }
}

void DBObject::insertData(DBOVariableSet& list, std::shared_ptr<Buffer> buffer, bool emit_change)
{
    logdbg << "DBObject " << name_ << ": insertData: list " << list.getSize()
           << " buffer " << buffer->size();

    assert(!insert_job_);

    insert_job_ = std::make_shared<InsertBufferDBJob>(COMPASS::instance().interface(), *this, buffer,
                                                      emit_change);

    connect(insert_job_.get(), &InsertBufferDBJob::doneSignal, this, &DBObject::insertDoneSlot,
            Qt::QueuedConnection);
    connect(insert_job_.get(), &InsertBufferDBJob::insertProgressSignal, this,
            &DBObject::insertProgressSlot, Qt::QueuedConnection);

    JobManager::instance().addDBJob(insert_job_);

    logdbg << "DBObject: insertData: end";
}

void DBObject::insertProgressSlot(float percent) { emit insertProgressSignal(percent); }

void DBObject::insertDoneSlot()
{
    assert(insert_job_);

    insert_job_ = nullptr;

    emit insertDoneSignal(*this);
}

void DBObject::updateData(DBOVariable& key_var, DBOVariableSet& list,
                          std::shared_ptr<Buffer> buffer)
{
    assert(!update_job_);

    assert(existsInDB());

    update_job_ =
        std::make_shared<UpdateBufferDBJob>(COMPASS::instance().interface(), *this, key_var, buffer);

    connect(update_job_.get(), &UpdateBufferDBJob::doneSignal, this, &DBObject::updateDoneSlot,
            Qt::QueuedConnection);
    connect(update_job_.get(), &UpdateBufferDBJob::updateProgressSignal, this,
            &DBObject::updateProgressSlot, Qt::QueuedConnection);

    JobManager::instance().addDBJob(update_job_);
}

void DBObject::updateProgressSlot(float percent) { emit updateProgressSignal(percent); }

void DBObject::updateDoneSlot()
{
    update_job_ = nullptr;

    emit updateDoneSignal(*this);
}

std::map<int, std::string> DBObject::loadLabelData(std::vector<int> rec_nums, int break_item_cnt)
{
    assert(is_loadable_);
    assert(existsInDB());

    std::string custom_filter_clause;
    bool first = true;

    // TODO rework to key variable
    assert(hasVariable("rec_num"));

    custom_filter_clause = variable("rec_num").dbColumnIdentifier() + " in (";
    for (auto& rec_num : rec_nums)
    {
        if (first)
            first = false;
        else
            custom_filter_clause += ",";

        custom_filter_clause += std::to_string(rec_num);
    }
    custom_filter_clause += ")";

    DBOVariableSet read_list = label_definition_->readList();

    if (!read_list.hasVariable(variable("rec_num")))
        read_list.add(variable("rec_num"));

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    DBInterface& db_interface = COMPASS::instance().interface();

    db_interface.prepareRead(*this, read_list, custom_filter_clause, {}, false, nullptr, false, "");
    std::shared_ptr<Buffer> buffer = db_interface.readDataChunk(*this);
    db_interface.finalizeReadStatement(*this);

    if (buffer->size() != rec_nums.size())
        throw std::runtime_error("DBObject " + name_ +
                                 ": loadLabelData: failed to load label for " +
                                 custom_filter_clause);

    assert(buffer->size() == rec_nums.size());

    std::map<int, std::string> labels =
        label_definition_->generateLabels(rec_nums, buffer, break_item_cnt);

    boost::posix_time::ptime stop_time = boost::posix_time::microsec_clock::local_time();
    boost::posix_time::time_duration diff = stop_time - start_time;

    logdbg << "DBObject: loadLabelData: done after " << diff.total_milliseconds() << " ms";

    return labels;
}

void DBObject::readJobIntermediateSlot(std::shared_ptr<Buffer> buffer)
{
    assert(buffer);
    logdbg << "DBObject: " << name_ << " readJobIntermediateSlot: buffer size " << buffer->size();

    DBOReadDBJob* sender = dynamic_cast<DBOReadDBJob*>(QObject::sender());

    if (!sender)
    {
        logwrn << "DBObject: readJobIntermediateSlot: null sender, event on the loose";
        return;
    }
    assert(sender == read_job_.get());

    std::vector<DBOVariable*>& variables = sender->readList().getSet();
    const PropertyList& properties = buffer->properties();

    for (auto var_it : variables)
    {
        assert(properties.hasProperty(var_it->dbColumnName()));
        const Property& property = properties.get(var_it->dbColumnName());
        assert(property.dataType() == var_it->dataType());
    }

    logdbg << "DBObject: " << name_ << " readJobIntermediateSlot: got buffer with size "
           << buffer->size();

    read_job_data_.push_back(buffer);

    FinalizeDBOReadJob* job = new FinalizeDBOReadJob(*this, sender->readList(), buffer);

    std::shared_ptr<FinalizeDBOReadJob> job_ptr = std::shared_ptr<FinalizeDBOReadJob>(job);
    connect(job, SIGNAL(doneSignal()), this, SLOT(finalizeReadJobDoneSlot()), Qt::QueuedConnection);
    finalize_jobs_.push_back(job_ptr);

    JobManager::instance().addBlockingJob(job_ptr);

    if (info_widget_)
        info_widget_->updateSlot();
}

void DBObject::readJobObsoleteSlot()
{
    logdbg << "DBObject: " << name_ << " readJobObsoleteSlot";
    read_job_ = nullptr;
    read_job_data_.clear();

    if (info_widget_)
        info_widget_->updateSlot();

    emit loadingDoneSignal(*this);
}

void DBObject::readJobDoneSlot()
{
    logdbg << "DBObject: " << name_ << " readJobDoneSlot";
    read_job_ = nullptr;

    if (info_widget_)
        info_widget_->updateSlot();

    if (!isLoading())
    {
        loginf << "DBObject: " << name_ << " readJobDoneSlot: done";
        emit loadingDoneSignal(*this);
    }
}

void DBObject::finalizeReadJobDoneSlot()
{
    logdbg << "DBObject: " << name_ << " finalizeReadJobDoneSlot";

    FinalizeDBOReadJob* sender = dynamic_cast<FinalizeDBOReadJob*>(QObject::sender());

    if (!sender)
    {
        logwrn << "DBObject: finalizeReadJobDoneSlot: null sender, event on the loose";
        return;
    }

    std::shared_ptr<Buffer> buffer = sender->buffer();

    bool found = false;
    for (auto final_it : finalize_jobs_)
    {
        if (final_it.get() == sender)
        {
            finalize_jobs_.erase(std::find(finalize_jobs_.begin(), finalize_jobs_.end(), final_it));
            found = true;
            break;
        }
    }
    assert(found);

    if (!data_)
        data_ = buffer;
    else
        data_->seizeBuffer(*buffer.get());

    logdbg << "DBObject: " << name_ << " finalizeReadJobDoneSlot: got buffer with size "
           << data_->size();

    if (info_widget_)
        info_widget_->updateSlot();

    if (!isLoading())  // should be last one
    {
        emit newDataSignal(*this);
        loginf << "DBObject: " << name_ << " finalizeReadJobDoneSlot: loading done";
        emit loadingDoneSignal(*this);
        return;
    }

    // read job or finalize jobs exist

    // check if other is still loading
    if (manager_.isOtherDBObjectPostProcessing(*this))
    {
        logdbg << "DBObject: " << name_
               << " finalizeReadJobDoneSlot: delaying new data since other is loading";
        return;
    }

    // check if more data can immediately loaded from read job
    if (read_job_ && data_->size() < read_job_->rowCount())
    {
        logdbg << "DBObject: " << name_
               << " finalizeReadJobDoneSlot: delaying new data since more data can be read";
        return;
    }

    // exact data from read job or finalize jobs still active
    emit newDataSignal(*this);
    return;
}

void DBObject::updateToDatabaseContent()
{
    loginf << "DBObject " << name_ << ": updateToDatabaseContent";

    std::string associations_table_name = associationsTableName();

    is_loadable_ = existsInDB();

    if (is_loadable_)
        count_ = COMPASS::instance().interface().count(db_table_name_);

    logdbg << "DBObject: " << name_ << " updateToDatabaseContent: table " << db_table_name_
           << " count " << count_;

    if (info_widget_)
        info_widget_->updateSlot();

    loginf << "DBObject: " << name_ << " updateToDatabaseContent: done, loadable " << is_loadable_
           << " count " << count_;
}

std::string DBObject::dbTableName() const
{
    return db_table_name_;
}

bool DBObject::associationsLoaded() const
{
    return associations_loaded_;
}

bool DBObject::isLoading() { return read_job_ || finalize_jobs_.size(); }

bool DBObject::isPostProcessing() { return finalize_jobs_.size(); }

bool DBObject::hasData() { return count_ > 0; }

size_t DBObject::count() { return count_; }

size_t DBObject::loadedCount()
{
    if (data_)
        return data_->size();
    else
        return 0;
}

bool DBObject::existsInDB() const
{
    return COMPASS::instance().interface().tableInfo().count(db_table_name_) > 0;
}

void DBObject::loadAssociationsIfRequired()
{
    if (manager_.hasAssociations() && !associations_loaded_)
    {
        std::shared_ptr<DBOReadAssociationsJob> read_job =
            std::make_shared<DBOReadAssociationsJob>(*this);
        JobManager::instance().addDBJob(read_job);  // fire and forget
    }
}

void DBObject::loadAssociations()
{
    loginf << "DBObject " << name_ << ": loadAssociations";

    associations_.clear();

    boost::posix_time::ptime loading_start_time;
    boost::posix_time::ptime loading_stop_time;

    loading_start_time = boost::posix_time::microsec_clock::local_time();

    DBInterface& db_interface = COMPASS::instance().interface();

    std::string associations_table_name = associationsTableName();

    if (db_interface.existsTable(associations_table_name))
        associations_ = db_interface.getAssociations(associations_table_name);

    associations_loaded_ = true;

    loading_stop_time = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = loading_stop_time - loading_start_time;
    load_time = diff.total_milliseconds() / 1000.0;

    loginf << "DBObject " << name_ << ": loadAssociations: " << associations_.size()
           << " associactions done (" << String::doubleToStringPrecision(load_time, 2) << " s).";
}

bool DBObject::hasAssociations() { return associations_.size() > 0; }

void DBObject::addAssociation(unsigned int rec_num, unsigned int utn, bool has_src, unsigned int src_rec_num)
{
    associations_.add(rec_num, DBOAssociationEntry(utn, has_src, src_rec_num));
    associations_changed_ = true;
    associations_loaded_ = true;
}

void DBObject::clearAssociations()
{
    associations_.clear();
    associations_changed_ = true;
    associations_loaded_ = false;
}

void DBObject::saveAssociations()
{
    loginf << "DBObject " << name_ << ": saveAssociations";

    DBInterface& db_interface = COMPASS::instance().interface();

    std::string associations_table_name = associationsTableName();
    assert(associations_table_name.size());

    if (db_interface.existsTable(associations_table_name))
        db_interface.clearTableContent(associations_table_name);
    else
        db_interface.createAssociationsTable(associations_table_name);

    if (!hasAssociations())
        return;

    assert(db_interface.existsTable(associations_table_name));

    // assoc_id INT, rec_num INT, utn INT

    PropertyList list;
    list.addProperty("rec_num", PropertyDataType::INT);
    list.addProperty("utn", PropertyDataType::INT);
    list.addProperty("src_rec_num", PropertyDataType::INT);

    std::shared_ptr<Buffer> buffer_ptr = std::shared_ptr<Buffer>(new Buffer(list, name_));

    NullableVector<int>& rec_nums = buffer_ptr->get<int>("rec_num");
    NullableVector<int>& utns = buffer_ptr->get<int>("utn");
    NullableVector<int>& src_rec_nums = buffer_ptr->get<int>("src_rec_num");

    size_t cnt = 0;
    for (auto& assoc_it : associations_)
    {
        rec_nums.set(cnt, assoc_it.first);
        utns.set(cnt, assoc_it.second.utn_);

        if (assoc_it.second.has_src_)
            src_rec_nums.set(cnt, assoc_it.second.src_rec_num_);

        ++cnt;
    }

    db_interface.insertBuffer(associations_table_name, buffer_ptr);

    associations_changed_ = false;

    loginf << "DBObject " << name_ << ": saveAssociations: done";
}
