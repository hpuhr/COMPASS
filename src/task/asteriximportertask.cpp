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
#include "asterixstatusdialog.h"
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
#include "atsdb.h"
#include "dbinterface.h"

#include <jasterix/jasterix.h>
#include <jasterix/category.h>

#include <algorithm>

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
    qRegisterMetaType<std::shared_ptr<std::vector <nlohmann::json>>>("std::shared_ptr<std::vector <nlohmann::json>>");

    registerParameter("debug_jasterix", &debug_jasterix_, false);
    registerParameter("current_filename", &current_filename_, "");
    registerParameter("current_framing", &current_framing_, "");

    createSubConfigurables();

    std::string jasterix_definition_path = HOME_DATA_DIRECTORY+"/jasterix_definitions";

    loginf << "ASTERIXImporterTask: contructor: jasterix definition path '" << jasterix_definition_path << "'";
    assert (Files::directoryExists(jasterix_definition_path));

    jASTERIX::frame_chunk_size = 10000;
    jASTERIX::record_chunk_size = 10000;

    jasterix_ = std::make_shared<jASTERIX::jASTERIX> (jasterix_definition_path, false, debug_jasterix_, true);

    std::vector<std::string> framings = jasterix_->framings();
    if (std::find(framings.begin(), framings.end(), current_framing_) == framings.end())
    {
        loginf << "ASTERIXImporterTask: contructor: resetting to no framing";
        current_framing_ = "";
    }
}


ASTERIXImporterTask::~ASTERIXImporterTask()
{
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
        unsigned int category = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueUint("category");

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

    jasterix_ = std::make_shared<jASTERIX::jASTERIX> (jasterix_definition_path, false, debug_jasterix_, true);

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

bool ASTERIXImporterTask::hasConfiguratonFor (unsigned int category)
{
    return category_configs_.count(category) > 0;
}

bool ASTERIXImporterTask::decodeCategory (unsigned int category)
{
    assert (hasConfiguratonFor(category));
    return category_configs_.at(category).decode();
}

void ASTERIXImporterTask::decodeCategory (unsigned int category, bool decode)
{
    assert (jasterix_->hasCategory(category));

    loginf << "ASTERIXImporterTask: decodeCategory: cat " << category << " decode " << decode;

    if (!hasConfiguratonFor(category))
    {
        Configuration &new_cfg = configuration().addNewSubConfiguration ("ASTERIXCategoryConfig");
        new_cfg.addParameterUnsignedInt ("category", category);
        new_cfg.addParameterBool ("decode", decode);
        new_cfg.addParameterString ("edition", jasterix_->category(category)->defaultEdition());

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert (hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).decode(decode);
}

std::string ASTERIXImporterTask::editionForCategory (unsigned int category)
{
    assert (hasConfiguratonFor(category));

    // check if edition exists, otherwise rest to default
    if (jasterix_->category(category)->editions().count(category_configs_.at(category).edition()) == 0)
    {
        loginf << "ASTERIXImporterTask: editionForCategory: cat " << category << " reset to default edition";
        category_configs_.at(category).edition(jasterix_->category(category)->defaultEdition());
    }

    return category_configs_.at(category).edition();
}

void ASTERIXImporterTask::editionForCategory (unsigned int category, const std::string& edition)
{
    assert (jasterix_->hasCategory(category));

    loginf << "ASTERIXImporterTask: decodeCategory: cat " << category << " edition " << edition;

    if (!hasConfiguratonFor(category))
    {
        Configuration &new_cfg = configuration().addNewSubConfiguration ("ASTERIXCategoryConfig");
        new_cfg.addParameterUnsignedInt ("category", category);
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

bool ASTERIXImporterTask::debug() const
{
    return debug_jasterix_;
}

void ASTERIXImporterTask::debug(bool debug_jasterix)
{
    debug_jasterix_ = debug_jasterix;

    assert (jasterix_);
    jasterix_->setDebug(debug_jasterix_);

    loginf << "ASTERIXImporterTask: debug " << debug_jasterix_;
}

bool ASTERIXImporterTask::test() const
{
    return test_;
}

void ASTERIXImporterTask::test(bool test)
{
    test_ = test;
}

bool ASTERIXImporterTask::createMappingStubs() const
{
    return create_mapping_stubs_;
}

void ASTERIXImporterTask::createMappingStubs(bool create_mapping_stubs)
{
    create_mapping_stubs_ = create_mapping_stubs;
}

bool ASTERIXImporterTask::canImportFile (const std::string& filename)
{
    if (!Files::fileExists(filename))
    {
        loginf << "ASTERIXImporterTask: canImportFile: not possible since file does not exist";
        return false;
    }

    return true;
}

void ASTERIXImporterTask::importFile(const std::string& filename)
{
    loginf << "ASTERIXImporterTask: importFile: filename " << filename << " test " << test_;

    assert (canImportFile(filename));

    filename_ = filename;

    assert (!status_widget_);

    status_widget_ = new ASTERIXStatusDialog (filename_, test_);
    connect(status_widget_, &ASTERIXStatusDialog::closeSignal, this, &ASTERIXImporterTask::closeStatusDialogSlot);
    status_widget_->markStartTime();

    key_count_ = 0;
    insert_active_ = 0;

    all_done_ = false;

    added_data_sources_.clear();

    assert (schema_);

    for (auto& map_it : *schema_)
        if (!map_it.second.initialized())
            map_it.second.initialize();

    loginf << "ASTERIXImporterTask: importFile: setting categories";

    // set category configs
    jasterix_->decodeNoCategories();

    for (auto& cat_it : category_configs_)
    {
        loginf << "ASTERIXImporterTask: importFile: setting category " << cat_it.first;

        if (!jasterix_->hasCategory(cat_it.first))
        {
            logwrn << "ASTERIXImporterTask: importFile: cat " << cat_it.first << " not defined in decoder";
            continue;
        }

        if (!jasterix_->category(cat_it.first)->hasEdition(cat_it.second.edition()))
        {
            logwrn << "ASTERIXImporterTask: importFile: cat " << cat_it.first << " edition "
                   << cat_it.second.edition() << " not defined in decoder";
            continue;
        }

        loginf << "ASTERIXImporterTask: importFile: setting decode flag";
        jasterix_->setDecodeCategory(cat_it.first, cat_it.second.decode());
        loginf << "ASTERIXImporterTask: importFile: setting edition";
        jasterix_->category(cat_it.first)->setCurrentEdition(cat_it.second.edition());

        loginf << "ASTERIXImporterTask: importFile: setting mapping";
        if (cat_it.first == 1)
        {
            jasterix_->category(cat_it.first)->setCurrentMapping("atsdb");
            loginf << "ASTERIXImporterTask: importFile: set cat " << cat_it.first
                   << " decode " <<  cat_it.second.decode()
                   << " edition " << cat_it.second.edition() << " mapping 'atsdb'";
        }
        else
            loginf << "ASTERIXImporterTask: importFile: set cat " << cat_it.first
                   << " decode " <<  cat_it.second.decode()
                   << " edition " << cat_it.second.edition();
        // TODO mapping?
    }

    loginf << "ASTERIXImporterTask: importFile: filename " << filename;

    assert (decode_job_ == nullptr);
    decode_job_ = make_shared<ASTERIXDecodeJob> (*this, filename, current_framing_, test_);

    connect (decode_job_.get(), SIGNAL(obsoleteSignal()), this, SLOT(decodeASTERIXObsoleteSlot()),
             Qt::QueuedConnection);
    connect (decode_job_.get(), SIGNAL(doneSignal()), this, SLOT(decodeASTERIXDoneSlot()), Qt::QueuedConnection);
    connect (decode_job_.get(), SIGNAL(decodedASTERIXSignal(std::shared_ptr<std::vector<nlohmann::json>>)),
             this, SLOT(addDecodedASTERIXSlot(std::shared_ptr<std::vector<nlohmann::json>>)), Qt::QueuedConnection);

    JobManager::instance().addBlockingJob(decode_job_);

    return;
}

void ASTERIXImporterTask::decodeASTERIXDoneSlot ()
{
    loginf << "ASTERIXImporterTask: decodeASTERIXDoneSlot";

    assert (decode_job_);

    if (decode_job_->error())
    {
        error_ = decode_job_->error();
        error_message_ = decode_job_->errorMessage();

        QMessageBox msgBox;
        msgBox.setText(("Decoding error: "+error_message_+"\n\nPlease quit the application.").c_str());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }

    decode_job_ = nullptr;
}
void ASTERIXImporterTask::decodeASTERIXObsoleteSlot ()
{
    decode_job_ = nullptr;
}

void ASTERIXImporterTask::addDecodedASTERIXSlot (std::shared_ptr<std::vector<nlohmann::json>> extracted_records)
{
    loginf << "ASTERIXImporterTask: addDecodedASTERIX";

    assert (decode_job_);
    assert (status_widget_);

    status_widget_->addNumFrames(decode_job_->numFrames());
    status_widget_->addNumRecords(decode_job_->numRecords());
    status_widget_->setCategoryCounts(decode_job_->categoryCounts());

    status_widget_->show();

    if (!create_mapping_stubs_) // test or import
    {
        size_t count = extracted_records->size();

        assert (schema_);

        std::shared_ptr<JSONMappingJob> json_map_job =
                std::shared_ptr<JSONMappingJob> (new JSONMappingJob (extracted_records, schema_->parsers(),
                                                                     key_count_));

        extracted_records = nullptr;

        connect (json_map_job.get(), &JSONMappingJob::obsoleteSignal, this, &ASTERIXImporterTask::mapJSONObsoleteSlot,
                 Qt::QueuedConnection);
        connect (json_map_job.get(), &JSONMappingJob::doneSignal, this, &ASTERIXImporterTask::mapJSONDoneSlot,
                 Qt::QueuedConnection);

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
    }
    else // create mappings
    {
        while (json_map_stub_job_) // only one can exist at a time
        {
            if (decode_job_)
                decode_job_->pause();

            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            QThread::msleep(1);
        }

        assert (json_map_stub_job_ == nullptr);

        json_map_stub_job_ = std::shared_ptr<JSONMappingStubsJob> (
                    new JSONMappingStubsJob (extracted_records, schema_->parsers()));

        connect (json_map_stub_job_.get(), &JSONMappingStubsJob::obsoleteSignal,
                 this, &ASTERIXImporterTask::mapStubsObsoleteSlot, Qt::QueuedConnection);
        connect (json_map_stub_job_.get(), &JSONMappingStubsJob::doneSignal,
                 this, &ASTERIXImporterTask::mapStubsDoneSlot, Qt::QueuedConnection);

        //json_map_stubs_jobs_.push(json_map_stubs_job);

        JobManager::instance().addNonBlockingJob(json_map_stub_job_);

        if (decode_job_)
            decode_job_->unpause();
    }
}

void ASTERIXImporterTask::mapJSONDoneSlot ()
{
    loginf << "ASTERIXImporterTask: mapJSONDoneSlot";

    assert (status_widget_);

    JSONMappingJob* map_job = static_cast<JSONMappingJob*>(sender());
    std::shared_ptr<JSONMappingJob> queued_map_job;

    while (!json_map_jobs_.try_pop(queued_map_job))
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(1);
    }

    assert (queued_map_job.get() == map_job);

    status_widget_->addNumMapped(map_job->numMapped());
    status_widget_->addNumNotMapped(map_job->numNotMapped());
    status_widget_->addMappedCounts(queued_map_job->categoryMappedCounts());
    status_widget_->addNumCreated(map_job->numCreated());

    std::map <std::string, std::shared_ptr<Buffer>> job_buffers = map_job->buffers();

    if (decode_job_)
    {
        if (maxLoadReached())
            decode_job_->pause();
        else
            decode_job_->unpause();
    }

    if (test_) // ???
    {
        status_widget_->setDone();
        return;
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

    if (decode_job_ == nullptr && json_map_jobs_.unsafe_size() == 0)
    {
        logdbg << "ASTERIXImporterTask: mapJSONDoneSlot: inserting parsed objects at end";
        insertData ();
    }
}

void ASTERIXImporterTask::mapJSONObsoleteSlot ()
{
    logdbg << "ASTERIXImporterTask: mapJSONObsoleteSlot";
}

void ASTERIXImporterTask::mapStubsDoneSlot ()
{
    logdbg << "ASTERIXImporterTask: mapStubsDoneSlot";

    JSONMappingStubsJob* map_stubs_job = static_cast<JSONMappingStubsJob*>(sender());
    assert (json_map_stub_job_.get() == map_stubs_job);

    json_map_stub_job_ = nullptr;

    schema_->updateMappings();

    checkAllDone ();
}
void ASTERIXImporterTask::mapStubsObsoleteSlot ()
{
    logdbg << "ASTERIXImporterTask: mapStubsObsoleteSlot";
    json_map_stub_job_ = nullptr;
}

void ASTERIXImporterTask::insertData ()
{
    loginf << "ASTERIXImporterTask: insertData: inserting into database";

    assert (status_widget_);

    while (insert_active_)
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep (1);
    }

    bool has_sac_sic = false;
    //bool emit_change = (decode_job_ == nullptr && json_map_jobs_.unsafe_size() == 0);

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

            logdbg << "ASTERIXImporterTask: insertData: " << db_object.name() << " inserting";

            DBOVariableSet set = parser_it.second.variableList();
            db_object.insertData(set, buffer, false);

            status_widget_->addNumInserted(db_object.name(), buffer->size());

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

    checkAllDone ();

    logdbg << "ASTERIXImporterTask: insertDoneSlot: done";
}

void ASTERIXImporterTask::checkAllDone ()
{
    logdbg << "ASTERIXImporterTask: checkAllDone";

    if (!all_done_ && decode_job_ == nullptr && json_map_jobs_.empty() && json_map_stub_job_ == nullptr
            && buffers_.size() == 0 && insert_active_ == 0)
    {
        assert (status_widget_);
        status_widget_->setDone();

        all_done_ = true;

        buffers_.clear();
        refreshjASTERIX();


        if (!create_mapping_stubs_)
            emit ATSDB::instance().interface().databaseContentChangedSignal();
    }

    logdbg << "ASTERIXImporterTask: checkAllDone: done";
}

void ASTERIXImporterTask::closeStatusDialogSlot()
{
    assert (status_widget_);
    status_widget_->close();
    delete status_widget_;
    status_widget_ = nullptr;
}

bool ASTERIXImporterTask::maxLoadReached ()
{
    return json_map_jobs_.unsafe_size() > 3;
}
