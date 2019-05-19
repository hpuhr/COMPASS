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

#include "asteriximportertask.h"
#include "asteriximportertaskwidget.h"
#include "asterixcategoryconfig.h"
#include "taskmanager.h"
#include "configurable.h"
#include "files.h"
#include "stringconv.h"
#include "savedfile.h"
#include "logger.h"
#include "jobmanager.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "dbtable.h"
#include "dbtablecolumn.h"

#include <jasterix/jasterix.h>
#include <jasterix/category.h>

#include <algorithm>
#include <iomanip>

#include <QMessageBox>
#include <QCoreApplication>
#include <QThread>

using namespace Utils;
using namespace nlohmann;
using namespace std;
//using namespace jASTERIX;

ASTERIXImporterTask::ASTERIXImporterTask(const std::string& class_id, const std::string& instance_id,
                                         TaskManager* task_manager)
    : Configurable (class_id, instance_id, task_manager)
{
    qRegisterMetaType<std::shared_ptr<nlohmann::json>>("std::shared_ptr<nlohmann::json>");

    registerParameter("debug_jasterix", &debug_jasterix_, false);
    registerParameter("current_filename", &current_filename_, "");
    registerParameter("current_framing", &current_framing_, "");

    createSubConfigurables();

    std::string jasterix_definition_path = HOME_DATA_DIRECTORY+"/jasterix_definitions";

    loginf << "ASTERIXImporterTask: contructor: jasterix definition path '" << jasterix_definition_path << "'";
    assert (Files::directoryExists(jasterix_definition_path));

    jASTERIX::frame_chunk_size = 5000;
    jASTERIX::record_chunk_size = 5000;

    jasterix_.reset(new jASTERIX::jASTERIX(jasterix_definition_path, false, debug_jasterix_));

    std::vector<std::string> framings = jasterix_->framings();
    if (std::find(framings.begin(), framings.end(), current_framing_) == framings.end())
    {
        loginf << "ASTERIXImporterTask: contructor: resetting to no framing";
        current_framing_ = "";
    }
}


ASTERIXImporterTask::~ASTERIXImporterTask()
{
    //    if (msg_box_)
    //    {
    //        delete msg_box_;
    //        msg_box_ = nullptr;
    //    }

    for (auto it : file_list_)
        delete it.second;

    file_list_.clear();
}

void ASTERIXImporterTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id == "ASTERIXFile")
    {
        SavedFile *file = new SavedFile (class_id, instance_id, this);
        assert (file_list_.count (file->name()) == 0);
        file_list_.insert (std::pair <std::string, SavedFile*> (file->name(), file));
    }
    else if (class_id == "ASTERIXCategoryConfig")
    {
        std::string category = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueString("category");

        assert (category_configs_.find (category) == category_configs_.end());

        logdbg << "ASTERIXImporterTask: generateSubConfigurable: generating asterix config " << instance_id
               << " with cat " << category;

        category_configs_.emplace(std::piecewise_construct,
                                  std::forward_as_tuple(category),  // args for key
                                  std::forward_as_tuple(category, class_id, instance_id, this));  // args for mapped value
    }
    else if (class_id == "JSONParsingSchema")
    {
        std::string name = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueString("name");

        assert (schema_ == nullptr);
        assert (name == "jASTERIX");

        logdbg << "ASTERIXImporterTask: generateSubConfigurable: generating schema " << instance_id
               << " with name " << name;

        schema_.reset(new JSONParsingSchema(class_id, instance_id, this));
    }
    else
        throw std::runtime_error ("ASTERIXImporterTask: generateSubConfigurable: unknown class_id "+class_id );
}

void ASTERIXImporterTask::checkSubConfigurables ()
{
    if (schema_ == nullptr)
    {
        Configuration &config = addNewSubConfiguration ("JSONParsingSchema", "JSONParsingSchemajASTERIX0");
        config.addParameterString("name", "jASTERIX");
        generateSubConfigurable ("JSONParsingSchema", "JSONParsingSchemajASTERIX0");
    }
}

ASTERIXImporterTaskWidget* ASTERIXImporterTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new ASTERIXImporterTaskWidget (*this));
    }

    assert (widget_);
    return widget_.get();
}

void ASTERIXImporterTask::refreshjASTERIX()
{
    std::string jasterix_definition_path = HOME_DATA_DIRECTORY+"/jasterix_definitions";

    loginf << "ASTERIXImporterTask: refreshjASTERIX: jasterix definition path '" << jasterix_definition_path << "'";
    assert (Files::directoryExists(jasterix_definition_path));

    jasterix_.reset(new jASTERIX::jASTERIX(jasterix_definition_path, false, debug_jasterix_));

    std::vector<std::string> framings = jasterix_->framings();
    if (std::find(framings.begin(), framings.end(), current_framing_) == framings.end())
    {
        loginf << "ASTERIXImporterTask: refreshjASTERIX: resetting to no framing";
        current_framing_ = "";
    }
}

void ASTERIXImporterTask::addFile (const std::string &filename)
{
    if (file_list_.count (filename) != 0)
        throw std::invalid_argument ("ASTERIXImporterTask: addFile: name '"+filename+"' already in use");

    std::string instancename = filename;
    instancename.erase (std::remove(instancename.begin(), instancename.end(), '/'), instancename.end());

    Configuration &config = addNewSubConfiguration ("ASTERIXFile", "ASTERIXFile"+instancename);
    config.addParameterString("name", filename);
    generateSubConfigurable ("ASTERIXFile", "ASTERIXFile"+instancename);

    if (widget_)
        widget_->updateFileListSlot();
}

void ASTERIXImporterTask::removeCurrentFilename ()
{
    assert (current_filename_.size());
    assert (hasFile(current_filename_));

    if (file_list_.count (current_filename_) != 1)
        throw std::invalid_argument ("ASTERIXImporterTask: addFile: name '"+current_filename_+"' not in use");

    delete file_list_.at(current_filename_);
    file_list_.erase(current_filename_);

    if (widget_)
        widget_->updateFileListSlot();
}

const std::string& ASTERIXImporterTask::currentFraming() const
{
    return current_framing_;
}

void ASTERIXImporterTask::currentFraming(const std::string &current_framing)
{
    current_framing_ = current_framing;
}

bool ASTERIXImporterTask::hasConfiguratonFor (const std::string& category)
{
    return category_configs_.count(category) > 0;
}

bool ASTERIXImporterTask::decodeCategory (const std::string& category)
{
    assert (hasConfiguratonFor(category));
    return category_configs_.at(category).decode();
}

void ASTERIXImporterTask::decodeCategory (const std::string& category, bool decode)
{
    assert (jasterix_->hasCategory(category));

    loginf << "ASTERIXImporterTask: decodeCategory: cat " << category << " decode " << decode;

    if (!hasConfiguratonFor(category))
    {
        Configuration &new_cfg = configuration().addNewSubConfiguration ("ASTERIXCategoryConfig");
        new_cfg.addParameterString ("category", category);
        new_cfg.addParameterBool ("decode", decode);
        new_cfg.addParameterString ("edition", jasterix_->categories().at(category).defaultEdition());

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert (hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).decode(decode);
}

std::string ASTERIXImporterTask::editionForCategory (const std::string& category)
{
    assert (hasConfiguratonFor(category));

    // check if edition exists, otherwise rest to default
    if (jasterix_->categories().at(category).editions().count(category_configs_.at(category).edition()) == 0)
    {
        loginf << "ASTERIXImporterTask: editionForCategory: cat " << category << " reset to default edition";
        category_configs_.at(category).edition(jasterix_->categories().at(category).defaultEdition());
    }

    return category_configs_.at(category).edition();
}

void ASTERIXImporterTask::editionForCategory (const std::string& category, const std::string& edition)
{
    assert (jasterix_->hasCategory(category));

    loginf << "ASTERIXImporterTask: decodeCategory: cat " << category << " edition " << edition;

    if (!hasConfiguratonFor(category))
    {
        Configuration &new_cfg = configuration().addNewSubConfiguration ("ASTERIXCategoryConfig");
        new_cfg.addParameterString ("category", category);
        new_cfg.addParameterBool ("decode", false);
        new_cfg.addParameterString ("edition", edition);

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert (hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).edition(edition);
}

std::shared_ptr<JSONParsingSchema> ASTERIXImporterTask::schema() const
{
    return schema_;
}

bool ASTERIXImporterTask::canImportFile (const std::string& filename)
{
    if (!Files::fileExists(filename))
    {
        loginf << "ASTERIXImporterTask: canImportFile: not possible since file does not exist";
        return false;
    }

    //    if (!ATSDB::instance().objectManager().existsObject("ADSB"))
    //    {
    //        loginf << "ASTERIXImporterTask: canImportFile: not possible since DBObject does not exist";
    //        return false;
    //    }

    //    if (!current_schema_.size())
    //        return false;

    //    if (!schemas_.count(current_schema_))
    //    {
    //        current_schema_ = "";
    //        return false;
    //    }

    return true;
}

void ASTERIXImporterTask::importFile(const std::string& filename, bool test)
{
    loginf << "ASTERIXImporterTask: importFile: filename " << filename << " test " << test;

    assert (canImportFile(filename));

    filename_ = filename;
    test_ = test;

    num_frames_ = 0;
    num_records_ = 0;

    assert (schema_);

    for (auto& map_it : *schema_)
        if (!map_it.second.initialized())
            map_it.second.initialize();

    start_time_ = boost::posix_time::microsec_clock::local_time();

    // set category configs
    jasterix_->decodeNoCategories();

    for (auto& cat_it : category_configs_)
    {
        if (!jasterix_->hasCategory(cat_it.first))
        {
            logwrn << "ASTERIXImporterTask: importFile: cat " << cat_it.first << " not defined in decoder";
            continue;
        }

        if (!jasterix_->hasEdition(cat_it.first, cat_it.second.edition()))
        {
            logwrn << "ASTERIXImporterTask: importFile: cat " << cat_it.first << " edition "
                   << cat_it.second.edition() << " not defined in decoder";
            continue;
        }

        jasterix_->setDecodeCategory(cat_it.first, cat_it.second.decode());
        jasterix_->setEdition(cat_it.first, cat_it.second.edition());
        loginf << "ASTERIXImporterTask: importFile: set cat " << cat_it.first << " decode " <<  cat_it.second.decode()
               << " edition " << cat_it.second.edition();
        // TODO mapping
    }

    loginf << "ASTERIXImporterTask: importFile: filename " << filename;

    assert (decode_job_ == nullptr);
    decode_job_.reset(new ASTERIXDecodeJob(*this, filename, current_framing_, test));

    connect (decode_job_.get(), SIGNAL(obsoleteSignal()), this, SLOT(decodeASTERIXObsoleteSlot()),
             Qt::QueuedConnection);
    connect (decode_job_.get(), SIGNAL(doneSignal()), this, SLOT(decodeASTERIXDoneSlot()), Qt::QueuedConnection);
    connect (decode_job_.get(), SIGNAL(decodedASTERIXSignal(std::shared_ptr<nlohmann::json>)),
             this, SLOT(addDecodedASTERIXSlot(std::shared_ptr<nlohmann::json>)), Qt::QueuedConnection);

    JobManager::instance().addBlockingJob(decode_job_);

    return;
}

void ASTERIXImporterTask::decodeASTERIXDoneSlot ()
{
    logdbg << "ASTERIXImporterTask: decodeASTERIXDoneSlot";

    assert (decode_job_);

    decode_job_ = nullptr;

    updateMsgBox();
}
void ASTERIXImporterTask::decodeASTERIXObsoleteSlot ()
{
    decode_job_ = nullptr;
}

void ASTERIXImporterTask::addDecodedASTERIXSlot (std::shared_ptr<nlohmann::json> data)
{
    logdbg << "ASTERIXImporterTask: addDecodedASTERIX";

    assert (decode_job_);

    num_frames_ += decode_job_->numFrames();
    num_records_ += decode_job_->numRecords();

    //category_counts_ = decode_job_->categoryCounts();

    //    stop_time_ = boost::posix_time::microsec_clock::local_time();

    //    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    //    std::string time_str = std::to_string(diff.hours())+"h "+std::to_string(diff.minutes())
    //            +"m "+std::to_string(diff.seconds())+"s";

    //    loginf << "ASTERIXImporterTask: addDecodedASTERIX: num frames " << num_frames_ << " records " << num_records_
    //           << " after " << time_str << " " << (int)(1000.0*num_records_/diff.total_milliseconds()) << " rec/s ";

    // create new extract job
    std::shared_ptr<ASTERIXExtractRecordsJob> extract_job {new ASTERIXExtractRecordsJob(current_framing_, data)};
    logdbg << "ASTERIXImporterTask: addDecodedASTERIX: data " << data->size() << " jobs " << extract_jobs_.unsafe_size();

    connect (extract_job.get(), SIGNAL(obsoleteSignal()), this, SLOT(extractASTERIXObsoleteSlot()),
             Qt::QueuedConnection);
    connect (extract_job.get(), SIGNAL(doneSignal()), this, SLOT(extractASTERIXDoneSlot()), Qt::QueuedConnection);

    extract_jobs_.push(extract_job);
    JobManager::instance().addNonBlockingJob(extract_job);

    if (decode_job_)
        if (maxLoadReached())
            decode_job_->pause();

    updateMsgBox();
}

void ASTERIXImporterTask::extractASTERIXDoneSlot ()
{
    logdbg << "ASTERIXImporterTask: extractASTERIXDoneSlot";

    ASTERIXExtractRecordsJob* extract_job = static_cast<ASTERIXExtractRecordsJob*>(sender());
    std::shared_ptr<ASTERIXExtractRecordsJob> queued_extract_job;

    while (!extract_jobs_.try_pop(queued_extract_job))
        QThread::sleep(1);

    assert (queued_extract_job.get() == extract_job);

    logdbg << "ASTERIXImporterTask: extractASTERIXDoneSlot: update";

    for (auto& cat_cnt_it: queued_extract_job->categoryCounts())
    {
        if (category_counts_.count(cat_cnt_it.first) == 0)
            category_counts_[cat_cnt_it.first] = cat_cnt_it.second;
        else
            category_counts_[cat_cnt_it.first] += cat_cnt_it.second;
    }

    //    JSONMappingJob(std::vector<nlohmann::json>&& json_objects, const std::map <std::string, JSONObjectParser>& mappings,
    //                   size_t key_count);

    std::vector<json> json_objects = std::move(queued_extract_job->extractedRecords());
    size_t count = json_objects.size();

    assert (schema_);

    std::shared_ptr<JSONMappingJob> json_map_job =
            std::shared_ptr<JSONMappingJob> (new JSONMappingJob (std::move(json_objects),
                                                                 schema_->parsers(), key_count_));
    connect (json_map_job.get(), SIGNAL(obsoleteSignal()), this, SLOT(mapJSONObsoleteSlot()),
             Qt::QueuedConnection);
    connect (json_map_job.get(), SIGNAL(doneSignal()), this, SLOT(mapJSONDoneSlot()), Qt::QueuedConnection);

    json_map_jobs_.push(json_map_job);

    JobManager::instance().addNonBlockingJob(json_map_job);

    key_count_ += count;

    if (decode_job_)
    {
        if (maxLoadReached())
            decode_job_->pause();
        else
            decode_job_->unpause();
    }

    updateMsgBox();
}

void ASTERIXImporterTask::extractASTERIXObsoleteSlot ()
{
    logdbg << "ASTERIXImporterTask: extractASTERIXObsoleteSlot";
}

void ASTERIXImporterTask::mapJSONDoneSlot ()
{
    logdbg << "ASTERIXImporterTask: mapJSONDoneSlot";

    JSONMappingJob* map_job = static_cast<JSONMappingJob*>(sender());
    std::shared_ptr<JSONMappingJob> queued_map_job;

    while (!json_map_jobs_.try_pop(queued_map_job))
        QThread::sleep(1);

    assert (queued_map_job.get() == map_job);

    records_mapped_ += map_job->numMapped();
    records_not_mapped_ += map_job->numNotMapped();

    records_created_ += map_job->numCreated();

    std::map <std::string, std::shared_ptr<Buffer>> job_buffers = map_job->buffers();

    if (decode_job_)
    {
        if (maxLoadReached())
            decode_job_->pause();
        else
            decode_job_->unpause();
    }

    for (auto& buf_it : job_buffers)
    {
        if (buf_it.second && buf_it.second->size())
        {
            std::shared_ptr<Buffer> job_buffer = buf_it.second;

            if (buffers_.count(buf_it.first) == 0)
                buffers_[buf_it.first] = job_buffer;
            else
                buffers_.at(buf_it.first)->seizeBuffer(*job_buffer.get());
        }
    }

    updateMsgBox();

    if (!insert_active_)
    {
        for (auto& buf_it : buffers_)
        {
            if (buf_it.second->size() > 10000)
            {
                logdbg << "ASTERIXImporterTask: mapJSONDoneSlot: inserting part of parsed objects";
                insertData ();
                return;
            }
        }
    }

    if (decode_job_ == nullptr && extract_jobs_.unsafe_size() == 0 && json_map_jobs_.unsafe_size() == 0)
    {
        logdbg << "ASTERIXImporterTask: mapJSONDoneSlot: inserting parsed objects at end";
        insertData ();
    }
}

void ASTERIXImporterTask::mapJSONObsoleteSlot ()
{
    logdbg << "ASTERIXImporterTask: mapJSONObsoleteSlot";
}

void ASTERIXImporterTask::insertData ()
{
    logdbg << "ASTERIXImporterTask: insertData: inserting into database";

    while (insert_active_)
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep (10);
    }

    bool has_sac_sic = false;
    bool emit_change = (decode_job_ == nullptr && extract_jobs_.unsafe_size() == 0 && json_map_jobs_.unsafe_size() == 0);

    assert (schema_);

    for (auto& parser_it : *schema_)
    {
        if (buffers_.count(parser_it.second.dbObject().name()) != 0)
        {
            ++insert_active_;

            DBObject& db_object = parser_it.second.dbObject();
            std::shared_ptr<Buffer> buffer = buffers_.at(parser_it.second.dbObject().name());

            has_sac_sic = db_object.hasVariable("sac") && db_object.hasVariable("sic")
                    && buffer->has<char>("sac") && buffer->has<char>("sic");

            logdbg << "ASTERIXImporterTask: insertData: " << db_object.name() << " has sac/sic " << has_sac_sic;

            logdbg << "ASTERIXImporterTask: insertData: " << db_object.name() << " buffer " << buffer->size();

            connect (&db_object, &DBObject::insertDoneSignal, this, &ASTERIXImporterTask::insertDoneSlot,
                     Qt::UniqueConnection);
            connect (&db_object, &DBObject::insertProgressSignal, this, &ASTERIXImporterTask::insertProgressSlot,
                     Qt::UniqueConnection);


            if (parser_it.second.dataSourceVariableName() != "")
            {
                logdbg << "ASTERIXImporterTask: insertData: adding new data sources";

                std::string data_source_var_name = parser_it.second.dataSourceVariableName();


                // collect existing datasources
                std::set <int> datasources_existing;
                if (db_object.hasDataSources())
                    for (auto ds_it = db_object.dsBegin(); ds_it != db_object.dsEnd(); ++ds_it)
                        datasources_existing.insert(ds_it->first);

                // getting key list and distinct values
                assert (buffer->properties().hasProperty(data_source_var_name));
                assert (buffer->properties().get(data_source_var_name).dataType() == PropertyDataType::INT);

                assert(buffer->has<int>(data_source_var_name));
                NullableVector<int>& data_source_key_list = buffer->get<int> (data_source_var_name);
                std::set<int> data_source_keys = data_source_key_list.distinctValues();

                std::map <int, std::pair<char, char>> sac_sics; // keyvar->(sac,sic)
                // collect sac/sics
                if (has_sac_sic)
                {
                    NullableVector<char>& sac_list = buffer->get<char> ("sac");
                    NullableVector<char>& sic_list = buffer->get<char> ("sic");

                    size_t size = buffer->size();
                    int key_val;
                    for (unsigned int cnt=0; cnt < size; ++cnt)
                    {
                        key_val = data_source_key_list.get(cnt);

                        if (datasources_existing.count(key_val) != 0)
                            continue;

                        if (sac_sics.count(key_val) == 0)
                        {
                            logdbg << "ASTERIXImporterTask: insertData: found new ds " << key_val << " for sac/sic";

                            assert (!sac_list.isNull(cnt) && !sic_list.isNull(cnt));
                            sac_sics[key_val] = std::pair<char, char> (sac_list.get(cnt), sic_list.get(cnt));

                            loginf << "ASTERIXImporterTask: insertData: source " << key_val
                                   << " sac " << static_cast<int>(sac_list.get(cnt))
                                   << " sic " << static_cast<int>(sic_list.get(cnt));
                        }
                    }

                }

                // adding datasources
                std::map <int, std::pair<int,int>> datasources_to_add;

                for (auto ds_key_it : data_source_keys)
                    if (datasources_existing.count(ds_key_it) == 0 && added_data_sources_.count(ds_key_it) == 0)
                    {
                        if (datasources_to_add.count(ds_key_it) == 0)
                        {
                            logdbg << "ASTERIXImporterTask: insertData: adding new data source " << ds_key_it;
                            if (sac_sics.count(ds_key_it) == 0)
                                datasources_to_add[ds_key_it] = {-1,-1};
                            else
                                datasources_to_add[ds_key_it] = {sac_sics.at(ds_key_it).first,
                                                                 sac_sics.at(ds_key_it).second};

                            added_data_sources_.insert(ds_key_it);
                        }
                    }

                if (datasources_to_add.size())
                {
                    db_object.addDataSources(datasources_to_add);
                }
            }

            logdbg << "ASTERIXImporterTask: insertData: " << db_object.name() << " inserting, change" << emit_change;

            DBOVariableSet set = parser_it.second.variableList();
            db_object.insertData(set, buffer, emit_change);
            records_inserted_ += buffer->size();

            logdbg << "ASTERIXImporterTask: insertData: " << db_object.name() << " clearing";
            buffers_.erase(parser_it.second.dbObject().name());
        }
        else
            logdbg << "ASTERIXImporterTask: insertData: emtpy buffer for " << parser_it.second.dbObject().name();
    }

    assert (buffers_.size() == 0);

    logdbg << "JSONImporterTask: insertData: done";
}

void ASTERIXImporterTask::insertProgressSlot (float percent)
{
    logdbg << "ASTERIXImporterTask: insertProgressSlot: " << String::percentToString(percent) << "%";
}

void ASTERIXImporterTask::insertDoneSlot (DBObject& object)
{
    logdbg << "ASTERIXImporterTask: insertDoneSlot";
    --insert_active_;

    checkAllDone();
    updateMsgBox();

    logdbg << "ASTERIXImporterTask: insertDoneSlot: done";
}

void ASTERIXImporterTask::checkAllDone ()
{
    logdbg << "ASTERIXImporterTask: checkAllDone";

    if (!all_done_ && decode_job_ == nullptr && extract_jobs_.empty() && json_map_jobs_.empty() == 0
            && insert_active_ == 0)
    {
        stop_time_ = boost::posix_time::microsec_clock::local_time();

        boost::posix_time::time_duration diff = stop_time_ - start_time_;

        std::string time_str = std::to_string(diff.hours())+"h "+std::to_string(diff.minutes())
                +"m "+std::to_string(diff.seconds())+"s";

        loginf << "ASTERIXImporterTask: checkAllDone: read done after " << time_str;

        all_done_ = true;

//        if (widget_)
//            widget_->importDoneSlot(test_);
    }

    logdbg << "ASTERIXImporterTask: checkAllDone: done";
}

void ASTERIXImporterTask::updateMsgBox ()
{
    logdbg << "ASTERIXImporterTask: updateMsgBox";

    if (!msg_box_)
    {
        msg_box_ = new QMessageBox ();
        assert (msg_box_);
    }

    std::string msg;

    if (test_)
        msg = "Testing import of";
    else
        msg = "Importing";

    msg += " file '"+filename_+"'\n";

    stop_time_ = boost::posix_time::microsec_clock::local_time();

    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    std::string elapsed_time_str = String::timeStringFromDouble(diff.total_milliseconds()/1000.0, false);

    double records_per_second = num_records_/(diff.total_milliseconds()/1000.0);

    std::string records_rate_str_ = std::to_string(static_cast<int>(records_per_second));

    msg += "Elapsed Time: "+elapsed_time_str+"\n";

    msg += "Frames read: "+std::to_string(num_frames_)+"\n";
    msg += "Records read: "+std::to_string(num_records_)+"\n";

    msg += "\n";

    stringstream ss;

    for (auto& cat_cnt_it : category_counts_)
    {
        //        loginf << "ASTERIXImporterTask: decodeASTERIXDoneSlot: cat " << cat_cnt_it.first
        //               << " cnt " << cat_cnt_it.second;
        ss.str("");

        ss << setfill('0') << setw(3) << cat_cnt_it.first;

        msg += "CAT"+ss.str()+": "+std::to_string(cat_cnt_it.second)+"\n";
    }

    msg += "\n";

    msg += "Records mapped: "+std::to_string(records_mapped_)+"\n";
    msg += "Records not mapped: "+std::to_string(records_not_mapped_)+"\n\n";

    msg += "Records created: "+std::to_string(records_created_)+"\n";
    msg += "Records inserted: "+std::to_string(records_inserted_)+"\n\n";

    msg += "Record rate: "+records_rate_str_+" e/s";

    //    if (!all_done_ && remaining_time_str_.size())
    //        msg += "\nEstimated remaining time: "+remaining_time_str_;

    msg_box_->setText(msg.c_str());

    if (all_done_)
        msg_box_->setStandardButtons(QMessageBox::Ok);
    else
        msg_box_->setStandardButtons(QMessageBox::NoButton);

    msg_box_->show();

    logdbg << "ASTERIXImporterTask: updateMsgBox: done";
}

bool ASTERIXImporterTask::maxLoadReached ()
{
    return extract_jobs_.unsafe_size() > 5 || json_map_jobs_.unsafe_size() > 5;
}
