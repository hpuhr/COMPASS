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

#include "asteriximporttask.h"
#include "asteriximporttaskwidget.h"
#include "asterixstatusdialog.h"
#include "asterixcategoryconfig.h"
#include "taskmanager.h"
#include "taskmanagerwidget.h"
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
#include "buffer.h"
#include "radarplotpositioncalculatortask.h"
#include "createartasassociationstask.h"
#include "postprocesstask.h"
#include "system.h"

#include <jasterix/jasterix.h>
#include <jasterix/category.h>
#include <jasterix/edition.h>
#include <jasterix/refedition.h>

#include <algorithm>

#include <QMessageBox>
#include <QCoreApplication>
#include <QThread>
#include <QApplication>

using namespace Utils;
using namespace nlohmann;
using namespace std;

const unsigned int unlimited_chunk_size = 10000;
const unsigned int limited_chunk_size = 5000;

//const unsigned int unlimited_num_json_jobs_ = 2;
//const unsigned int limited_num_json_jobs_ = 1;

const std::string DONE_PROPERTY_NAME = "asterix_data_imported";

const float ram_threshold = 4.0;

ASTERIXImportTask::ASTERIXImportTask(const std::string& class_id, const std::string& instance_id,
                                     TaskManager& task_manager)
    : Task("ASTERIXImportTask", "Import ASTERIX Data", false, false, task_manager),
      Configurable (class_id, instance_id, &task_manager, "task_import_asterix.json")
{
    tooltip_ = "Allows importing of ASTERIX data recording files into the opened database.";

    registerParameter("debug_jasterix", &debug_jasterix_, false);
    registerParameter("limit_ram", &limit_ram_, false);
    registerParameter("current_filename", &current_filename_, "");
    registerParameter("current_framing", &current_framing_, "");

    createSubConfigurables();

    std::string jasterix_definition_path = HOME_DATA_DIRECTORY+"/jasterix_definitions";

    loginf << "ASTERIXImporterTask: contructor: jasterix definition path '" << jasterix_definition_path << "'";
    assert (Files::directoryExists(jasterix_definition_path));

    if (limit_ram_)
    {
        jASTERIX::frame_chunk_size = limited_chunk_size;
        jASTERIX::record_chunk_size = limited_chunk_size;
    }
    else
    {
        jASTERIX::frame_chunk_size = unlimited_chunk_size;
        jASTERIX::record_chunk_size = unlimited_chunk_size;
    }

    jasterix_ = std::make_shared<jASTERIX::jASTERIX> (jasterix_definition_path, false, debug_jasterix_, true);

    std::vector<std::string> framings = jasterix_->framings();
    if (std::find(framings.begin(), framings.end(), current_framing_) == framings.end())
    {
        loginf << "ASTERIXImporterTask: contructor: resetting to no framing";
        current_framing_ = "";
    }
}


ASTERIXImportTask::~ASTERIXImportTask()
{
    for (auto it : file_list_)
        delete it.second;

    file_list_.clear();
}

void ASTERIXImportTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
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

        logdbg << "ASTERIXImporterTask: generateSubConfigurable: cat " << category
               << " decode " <<  category_configs_.at(category).decode()
               << " edition '" << category_configs_.at(category).edition()
               << "' ref '" << category_configs_.at(category).ref() << "'";
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

void ASTERIXImportTask::checkSubConfigurables ()
{
    if (schema_ == nullptr)
    {
        Configuration &config = addNewSubConfiguration ("JSONParsingSchema", "JSONParsingSchemajASTERIX0");
        config.addParameterString("name", "jASTERIX");
        generateSubConfigurable ("JSONParsingSchema", "JSONParsingSchemajASTERIX0");
    }
}

TaskWidget* ASTERIXImportTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new ASTERIXImportTaskWidget (*this));

        connect (&task_manager_, &TaskManager::expertModeChangedSignal,
                 widget_.get(), &ASTERIXImportTaskWidget::expertModeChangedSlot);
    }

    assert (widget_);
    return widget_.get();
}

void ASTERIXImportTask::refreshjASTERIX()
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

void ASTERIXImportTask::addFile (const std::string& filename)
{
    loginf << "ASTERIXImporterTask: addFile: filename '" << filename << "'";

    if (file_list_.count (filename) != 0)
        throw std::invalid_argument ("ASTERIXImporterTask: addFile: name '"+filename+"' already in use");

    std::string instancename = filename;
    instancename.erase (std::remove(instancename.begin(), instancename.end(), '/'), instancename.end());

    Configuration &config = addNewSubConfiguration ("ASTERIXFile", "ASTERIXFile"+instancename);
    config.addParameterString("name", filename);
    generateSubConfigurable ("ASTERIXFile", "ASTERIXFile"+instancename);

    current_filename_ = filename;

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void ASTERIXImportTask::removeCurrentFilename ()
{
    loginf << "ASTERIXImporterTask: removeCurrentFilename: filename '" << current_filename_ << "'";

    assert (current_filename_.size());
    assert (hasFile(current_filename_));

    if (file_list_.count (current_filename_) != 1)
        throw std::invalid_argument ("ASTERIXImporterTask: removeCurrentFilename: name '"
                                     +current_filename_+"' not in use");

    delete file_list_.at(current_filename_);
    file_list_.erase(current_filename_);
    current_filename_ = "";

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void ASTERIXImportTask::currentFilename (const std::string& filename)
{
    loginf << "ASTERIXImporterTask: currentFilename: filename '" << filename << "'";

    bool had_filename = canImportFile();

    current_filename_ = filename;

    if (!had_filename) // not on re-select
        emit statusChangedSignal(name_);
}

const std::string& ASTERIXImportTask::currentFraming() const
{
    return current_framing_;
}

void ASTERIXImportTask::currentFraming(const std::string& current_framing)
{
    current_framing_ = current_framing;
}

bool ASTERIXImportTask::hasConfiguratonFor (unsigned int category)
{
    return category_configs_.count(category) > 0;
}

bool ASTERIXImportTask::decodeCategory (unsigned int category)
{
    assert (hasConfiguratonFor(category));
    return category_configs_.at(category).decode();
}

void ASTERIXImportTask::decodeCategory (unsigned int category, bool decode)
{
    assert (jasterix_->hasCategory(category));

    loginf << "ASTERIXImporterTask: decodeCategory: cat " << category << " decode " << decode;

    if (!hasConfiguratonFor(category))
    {
        Configuration &new_cfg = configuration().addNewSubConfiguration ("ASTERIXCategoryConfig");
        new_cfg.addParameterUnsignedInt ("category", category);
        new_cfg.addParameterBool ("decode", decode);
        new_cfg.addParameterString ("edition", jasterix_->category(category)->defaultEdition());
        new_cfg.addParameterString ("ref", jasterix_->category(category)->defaultREFEdition());

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert (hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).decode(decode);
}

std::string ASTERIXImportTask::editionForCategory (unsigned int category)
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

void ASTERIXImportTask::editionForCategory (unsigned int category, const std::string& edition)
{
    assert (jasterix_->hasCategory(category));

    loginf << "ASTERIXImporterTask: editionForCategory: cat " << category << " edition " << edition;

    if (!hasConfiguratonFor(category))
    {
        Configuration &new_cfg = configuration().addNewSubConfiguration ("ASTERIXCategoryConfig");
        new_cfg.addParameterUnsignedInt ("category", category);
        new_cfg.addParameterBool ("decode", false);
        new_cfg.addParameterString ("edition", edition);
        new_cfg.addParameterString ("ref", jasterix_->category(category)->defaultREFEdition());

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert (hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).edition(edition);
}

std::string ASTERIXImportTask::refEditionForCategory (unsigned int category)
{
    assert (hasConfiguratonFor(category));

    // check if edition exists, otherwise rest to default
    if (category_configs_.at(category).ref().size() && // if value set and not exist in jASTERIX
            jasterix_->category(category)->refEditions().count(category_configs_.at(category).ref()) == 0)
    {
        loginf << "ASTERIXImporterTask: refForCategory: cat " << category << " reset to default ref";
        category_configs_.at(category).ref(jasterix_->category(category)->defaultREFEdition());
    }

    return category_configs_.at(category).ref();
}

void ASTERIXImportTask::refEditionForCategory (unsigned int category, const std::string& ref)
{
    assert (jasterix_->hasCategory(category));

    loginf << "ASTERIXImporterTask: refForCategory: cat " << category << " ref '" << ref << "'";

    if (!hasConfiguratonFor(category))
    {
        Configuration &new_cfg = configuration().addNewSubConfiguration ("ASTERIXCategoryConfig");
        new_cfg.addParameterUnsignedInt ("category", category);
        new_cfg.addParameterBool ("decode", false);
        new_cfg.addParameterString ("edition", jasterix_->category(category)->defaultEdition());
        new_cfg.addParameterString ("ref", ref);

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert (hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).ref(ref);
}

std::string ASTERIXImportTask::spfEditionForCategory (unsigned int category)
{
    assert (hasConfiguratonFor(category));

    // check if edition exists, otherwise rest to default
    if (category_configs_.at(category).spf().size() && // if value set and not exist in jASTERIX
            jasterix_->category(category)->spfEditions().count(category_configs_.at(category).spf()) == 0)
    {
        loginf << "ASTERIXImporterTask: spfEditionForCategory: cat " << category << " reset to default spf";
        category_configs_.at(category).spf(jasterix_->category(category)->defaultSPFEdition());
    }

    return category_configs_.at(category).spf();
}

void ASTERIXImportTask::spfEditionForCategory (unsigned int category, const std::string& spf)
{
    assert (jasterix_->hasCategory(category));

    loginf << "ASTERIXImporterTask: spfEditionForCategory: cat " << category << " spf '" << spf << "'";

    if (!hasConfiguratonFor(category))
    {
        Configuration &new_cfg = configuration().addNewSubConfiguration ("ASTERIXCategoryConfig");
        new_cfg.addParameterUnsignedInt ("category", category);
        new_cfg.addParameterBool ("decode", false);
        new_cfg.addParameterString ("edition", jasterix_->category(category)->defaultEdition());
        new_cfg.addParameterString ("spf", spf);

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert (hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).spf(spf);
}

std::shared_ptr<JSONParsingSchema> ASTERIXImportTask::schema() const
{
    return schema_;
}

bool ASTERIXImportTask::debug() const
{
    return debug_jasterix_;
}

void ASTERIXImportTask::debug(bool debug_jasterix)
{
    debug_jasterix_ = debug_jasterix;

    assert (jasterix_);
    jasterix_->setDebug(debug_jasterix_);

    loginf << "ASTERIXImporterTask: debug " << debug_jasterix_;
}

bool ASTERIXImportTask::test() const
{
    return test_;
}

void ASTERIXImportTask::test(bool test)
{
    test_ = test;
}

bool ASTERIXImportTask::createMappingStubs() const
{
    return create_mapping_stubs_;
}

void ASTERIXImportTask::createMappingStubs(bool create_mapping_stubs)
{
    create_mapping_stubs_ = create_mapping_stubs;
}

bool ASTERIXImportTask::limitRAM() const
{
    return limit_ram_;
}

void ASTERIXImportTask::limitRAM(bool limit_ram)
{
    limit_ram_ = limit_ram;

    if (limit_ram_)
    {
        jASTERIX::frame_chunk_size = limited_chunk_size;
        jASTERIX::record_chunk_size = limited_chunk_size;
    }
    else
    {
        jASTERIX::frame_chunk_size = unlimited_chunk_size;
        jASTERIX::record_chunk_size = unlimited_chunk_size;
    }

    if (widget_)
        widget_->updateLimitRAM();
}

bool ASTERIXImportTask::checkPrerequisites ()
{
    if (!ATSDB::instance().interface().ready())  // must be connected
        return false;

    if (ATSDB::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = ATSDB::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    return true;
}

bool ASTERIXImportTask::isRecommended ()
{
    if (!checkPrerequisites())
        return false;

    if (ATSDB::instance().objectManager().hasData())
        return false;

    return true;
}

bool ASTERIXImportTask::isRequired ()
{
    return false;
}

void ASTERIXImportTask::deleteWidget ()
{
    widget_.reset(nullptr);
}

bool ASTERIXImportTask::canImportFile ()
{
    if (!current_filename_.size())
        return false;

    if (!Files::fileExists(current_filename_))
    {
        loginf << "ASTERIXImporterTask: canImportFile: not possible since file '" << current_filename_
               << "'does not exist";
        return false;
    }

    return true;
}

bool ASTERIXImportTask::canRun()
{
    return canImportFile();
}

void ASTERIXImportTask::run()
{
    float free_ram = System::getFreeRAMinGB();

    loginf << "ASTERIXImporterTask: run: filename " << current_filename_ << " test " << test_
           << " create stubs " << create_mapping_stubs_ << " free RAM " << free_ram << " GB";

    if (free_ram < ram_threshold && !limit_ram_)
    {
        loginf << "ASTERIXImporterTask: run: only " << free_ram << " GB free ram, recommending limiting";

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, "RAM Limiting",
                                      "There is only "+QString::number(free_ram)+" GB free RAM available.\n"
                                      +"This will result in decreased decoding performance.\n\n"
                                      +"Do you agree to limiting RAM usage?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            limitRAM(true);
        }
    }
    else if (free_ram >= ram_threshold && limit_ram_)
    {
        loginf << "ASTERIXImporterTask: ASTERIXImporterTask: " << free_ram << " GB free ram, recommending not limiting";

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(nullptr, "RAM Limiting",
                                      "There is "+QString::number(free_ram)+" GB free RAM available.\n"
                                      +"This will result in increased decoding performance.\n\n"
                                      +"Do you agree to increased RAM usage?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            limitRAM(false);
        }
    }

    if (test_)
        task_manager_.appendInfo("ASTERIXImporterTask: test import of file '"+current_filename_+"' started");
    else if (create_mapping_stubs_)
        task_manager_.appendInfo("ASTERIXImporterTask: create mappings stubs using file '"+current_filename_
                                 +"' started");
    else
        task_manager_.appendInfo("ASTERIXImporterTask: import of file '"+current_filename_+"' started");

    if (widget_)
        widget_->runStarted();

    assert (canImportFile());
    assert (!status_widget_);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    status_widget_ = nullptr;

    status_widget_.reset(new ASTERIXStatusDialog (current_filename_, test_, create_mapping_stubs_));
    connect(status_widget_.get(), &ASTERIXStatusDialog::closeSignal, this, &ASTERIXImportTask::closeStatusDialogSlot);
    status_widget_->markStartTime();

    insert_active_ = 0;

    all_done_ = false;

    added_data_sources_.clear();

    assert (schema_);

    for (auto& map_it : *schema_)
        if (!map_it.second.initialized())
            map_it.second.initialize();

    loginf << "ASTERIXImporterTask: run: setting categories";

    jASTERIX::add_artas_md5_hash = true;

    // set category configs
    jasterix_->decodeNoCategories();

    for (auto& cat_it : category_configs_)
    {
        //loginf << "ASTERIXImporterTask: importFile: setting category " << cat_it.first;

        loginf << "ASTERIXImporterTask: run: setting cat " << cat_it.first
               << " decode " <<  cat_it.second.decode()
               << " edition '" << cat_it.second.edition()
               << "' ref '" << cat_it.second.ref() << "'";

        if (!jasterix_->hasCategory(cat_it.first))
        {
            logwrn << "ASTERIXImporterTask: run: cat '" << cat_it.first << "' not defined in decoder";
            continue;
        }

        if (!jasterix_->category(cat_it.first)->hasEdition(cat_it.second.edition()))
        {
            logwrn << "ASTERIXImporterTask: run: cat " << cat_it.first << " edition '"
                   << cat_it.second.edition() << "' not defined in decoder";
            continue;
        }

        if (cat_it.second.ref().size() && // only if value set
                !jasterix_->category(cat_it.first)->hasREFEdition(cat_it.second.ref()))
        {
            logwrn << "ASTERIXImporterTask: run: cat " << cat_it.first << " ref '"
                   << cat_it.second.ref() << "' not defined in decoder";
            continue;
        }

        if (cat_it.second.spf().size() && // only if value set
                !jasterix_->category(cat_it.first)->hasSPFEdition(cat_it.second.spf()))
        {
            logwrn << "ASTERIXImporterTask: run: cat " << cat_it.first << " spf '"
                   << cat_it.second.spf() << "' not defined in decoder";
            continue;
        }

        //        loginf << "ASTERIXImporterTask: importFile: setting cat " <<  cat_it.first
        //               << " decode flag " << cat_it.second.decode();
        jasterix_->setDecodeCategory(cat_it.first, cat_it.second.decode());
        //        loginf << "ASTERIXImporterTask: importFile: setting cat " <<  cat_it.first
        //               << " edition " << cat_it.second.edition();
        jasterix_->category(cat_it.first)->setCurrentEdition(cat_it.second.edition());
        jasterix_->category(cat_it.first)->setCurrentREFEdition(cat_it.second.ref());
        jasterix_->category(cat_it.first)->setCurrentSPFEdition(cat_it.second.spf());

        // TODO mapping?
    }

    loginf << "ASTERIXImporterTask: run: starting decode job";

    assert (decode_job_ == nullptr);
    decode_job_ = make_shared<ASTERIXDecodeJob> (*this, current_filename_, current_framing_, test_);

    connect (decode_job_.get(), &ASTERIXDecodeJob::obsoleteSignal, this,
             &ASTERIXImportTask::decodeASTERIXObsoleteSlot, Qt::QueuedConnection);
    connect (decode_job_.get(), &ASTERIXDecodeJob::doneSignal, this, &ASTERIXImportTask::decodeASTERIXDoneSlot,
             Qt::QueuedConnection);
    connect (decode_job_.get(), &ASTERIXDecodeJob::decodedASTERIXSignal,
             this, &ASTERIXImportTask::addDecodedASTERIXSlot, Qt::QueuedConnection);

    JobManager::instance().addBlockingJob(decode_job_);

    return;
}

void ASTERIXImportTask::decodeASTERIXDoneSlot ()
{
    logdbg << "ASTERIXImporterTask: decodeASTERIXDoneSlot";

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

    assert (status_widget_);

    if (status_widget_->numErrors())
        task_manager_.appendError("ASTERIXImporterTask: "+std::to_string(status_widget_->numErrors())
                                  +" decoding errors occured");

    task_manager_.appendInfo("ASTERIXImporterTask: decoding done with "+std::to_string(status_widget_->numFrames())
                             +" frames, "+std::to_string(status_widget_->numRecords())+" records");

    decode_job_ = nullptr;

    checkAllDone();
}
void ASTERIXImportTask::decodeASTERIXObsoleteSlot ()
{
    logdbg << "ASTERIXImporterTask: decodeASTERIXObsoleteSlot";
    decode_job_ = nullptr;
}

void ASTERIXImportTask::addDecodedASTERIXSlot ()
{
    logdbg << "ASTERIXImporterTask: addDecodedASTERIX";

    assert (decode_job_);
    assert (status_widget_);

    logdbg << "ASTERIXImporterTask: addDecodedASTERIX: errors " << decode_job_->numErrors();

    status_widget_->numFrames(jasterix_->numFrames());
    status_widget_->numRecords(jasterix_->numRecords());
    status_widget_->numErrors(jasterix_->numErrors());
    status_widget_->setCategoryCounts(decode_job_->categoryCounts());

    status_widget_->show();

    std::unique_ptr<nlohmann::json> extracted_data = std::move (decode_job_->extractedData());

    if (!create_mapping_stubs_) // test or import
    {
//        map_jobs_mutex_.lock();
//        json_map_jobs_.push_back(json_map_job);
//        map_jobs_mutex_.unlock();

        while (json_map_job_) // only one can exist at a time
        {
            if (decode_job_)
                decode_job_->pause();

            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
            QThread::msleep(1);
        }

        assert (!json_map_job_);

        assert (schema_);

        std::vector<std::string> keys;

        if (current_framing_ == "")
            keys = {"data_blocks", "content", "records"};
        else
            keys = {"frames", "content", "data_blocks", "content", "records"};

        json_map_job_ = make_shared<JSONMappingJob> (std::move(extracted_data), keys, schema_->parsers());

        assert (!extracted_data);

        connect (json_map_job_.get(), &JSONMappingJob::obsoleteSignal, this, &ASTERIXImportTask::mapJSONObsoleteSlot,
                 Qt::QueuedConnection);
        connect (json_map_job_.get(), &JSONMappingJob::doneSignal, this, &ASTERIXImportTask::mapJSONDoneSlot,
                 Qt::QueuedConnection);

        JobManager::instance().addNonBlockingJob(json_map_job_);

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

        std::vector<std::string> keys;

        if (current_framing_ == "")
            keys = {"data_blocks", "content", "records"};
        else
            keys = {"frames", "content", "data_blocks", "content", "records"};

        json_map_stub_job_ = make_shared<JSONMappingStubsJob> (std::move(extracted_data), keys, schema_->parsers());
        assert (!extracted_data);

        connect (json_map_stub_job_.get(), &JSONMappingStubsJob::obsoleteSignal,
                 this, &ASTERIXImportTask::mapStubsObsoleteSlot, Qt::QueuedConnection);
        connect (json_map_stub_job_.get(), &JSONMappingStubsJob::doneSignal,
                 this, &ASTERIXImportTask::mapStubsDoneSlot, Qt::QueuedConnection);

        JobManager::instance().addNonBlockingJob(json_map_stub_job_);

        if (decode_job_)
            decode_job_->unpause();
    }
}

void ASTERIXImportTask::mapJSONDoneSlot ()
{
    logdbg << "ASTERIXImporterTask: mapJSONDoneSlot";

    assert (status_widget_);

//    JSONMappingJob* map_job = static_cast<JSONMappingJob*>(sender());
//    std::shared_ptr<JSONMappingJob> queued_map_job;

//    map_jobs_mutex_.lock();
//    waiting_for_map_ = true;

    assert (json_map_job_);

//    queued_map_job = json_map_jobs_.front();
//    json_map_jobs_.pop_front();

//    map_jobs_mutex_.unlock();
//    waiting_for_map_ = false;

//    assert (queued_map_job.get() == map_job);

    status_widget_->addNumMapped(json_map_job_->numMapped());
    status_widget_->addNumNotMapped(json_map_job_->numNotMapped());
    status_widget_->addMappedCounts(json_map_job_->categoryMappedCounts());
    status_widget_->addNumCreated(json_map_job_->numCreated());

    std::map <std::string, std::shared_ptr<Buffer>> job_buffers = std::move(json_map_job_->buffers());
    json_map_job_ = nullptr;

    if (decode_job_)
    {
        if (maxLoadReached())
            decode_job_->pause();
        else
            decode_job_->unpause();
    }

    if (test_)
    {
        checkAllDone();
        return;
    }

    insertData (std::move(job_buffers));
}

void ASTERIXImportTask::mapJSONObsoleteSlot ()
{
    logdbg << "ASTERIXImporterTask: mapJSONObsoleteSlot";
    // TODO
}

void ASTERIXImportTask::mapStubsDoneSlot ()
{
    logdbg << "ASTERIXImporterTask: mapStubsDoneSlot";

    JSONMappingStubsJob* map_stubs_job = static_cast<JSONMappingStubsJob*>(sender());
    assert (json_map_stub_job_.get() == map_stubs_job);

    json_map_stub_job_ = nullptr;

    schema_->updateMappings();

    checkAllDone ();
}
void ASTERIXImportTask::mapStubsObsoleteSlot ()
{
    logdbg << "ASTERIXImporterTask: mapStubsObsoleteSlot";
    json_map_stub_job_ = nullptr;
}

void ASTERIXImportTask::insertData (std::map <std::string, std::shared_ptr<Buffer>> job_buffers)
{
    logdbg << "ASTERIXImporterTask: insertData: inserting into database";

    assert (status_widget_);

    if (!dbo_variable_sets_.size()) // initialize if empty
    {
        for (auto& parser_it : *schema_)
        {
            std::string dbo_name = parser_it.second.dbObject().name();

            DBObject& db_object = parser_it.second.dbObject();
            assert (db_object.hasCurrentDataSourceDefinition());

            std::string data_source_var_name = parser_it.second.dataSourceVariableName();
            assert (data_source_var_name.size());
            assert (db_object.currentDataSourceDefinition().localKey() == data_source_var_name);

            DBOVariableSet set = parser_it.second.variableList();

            if (dbo_variable_sets_.count(dbo_name)) // add variables
            {
                assert (std::get<0>(dbo_variable_sets_.at(dbo_name)) == data_source_var_name);
                std::get<1>(dbo_variable_sets_.at(dbo_name)).add(set);
            }
            else // create it
                dbo_variable_sets_[dbo_name] = std::make_tuple(data_source_var_name, set);
        }
    }

    while (insert_active_)
    {
        waiting_for_insert_ = true;
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep (1);
    }

    waiting_for_insert_ = false;

    bool has_sac_sic = false;

    assert (schema_);

    DBObjectManager& object_manager = ATSDB::instance().objectManager();

    for (auto& buf_it : job_buffers)
    {
        std::string dbo_name = buf_it.first;
        assert (dbo_variable_sets_.count(dbo_name));
        std::shared_ptr<Buffer> buffer = buf_it.second;

        if (!buffer->size())
        {
            logdbg << "ASTERIXImporterTask: insertData: dbo " << buf_it.first << " with empty buffer";
            continue;
        }

        assert (object_manager.existsObject(dbo_name));
        DBObject& db_object = object_manager.object(dbo_name);

        assert (db_object.hasCurrentDataSourceDefinition());

        ++insert_active_;

        has_sac_sic = db_object.hasVariable("sac") && db_object.hasVariable("sic")
                && buffer->has<unsigned char>("sac") && buffer->has<unsigned char>("sic");

        logdbg << "ASTERIXImporterTask: insertData: " << db_object.name() << " has sac/sic " << has_sac_sic
               << " buffer size " << buffer->size();

        connect (&db_object, &DBObject::insertDoneSignal, this, &ASTERIXImportTask::insertDoneSlot,
                 Qt::UniqueConnection);
        connect (&db_object, &DBObject::insertProgressSignal, this, &ASTERIXImportTask::insertProgressSlot,
                 Qt::UniqueConnection);

        std::string data_source_var_name = std::get<0>(dbo_variable_sets_.at(dbo_name));

        logdbg << "ASTERIXImporterTask: insertData: adding new data sources in dbo " << db_object.name()
               << " ds varname '" << data_source_var_name << "'";

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

        std::map <int, std::pair<unsigned char, unsigned char>> sac_sics; // keyvar->(sac,sic)
        // collect sac/sics
        if (has_sac_sic)
        {
            NullableVector<unsigned char>& sac_list = buffer->get<unsigned char> ("sac");
            NullableVector<unsigned char>& sic_list = buffer->get<unsigned char> ("sic");

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
                    sac_sics[key_val] = std::pair<unsigned char, unsigned char> (sac_list.get(cnt), sic_list.get(cnt));

                    logdbg << "ASTERIXImporterTask: insertData: source " << key_val
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

        DBOVariableSet& set = std::get<1>(dbo_variable_sets_.at(dbo_name));
        db_object.insertData(set, buffer, false);

        status_widget_->addNumInserted(db_object.name(), buffer->size());

    }

    logdbg << "JSONImporterTask: insertData: done";
}

void ASTERIXImportTask::insertProgressSlot (float percent)
{
    logdbg << "ASTERIXImporterTask: insertProgressSlot: " << String::percentToString(percent) << "%";
}

void ASTERIXImportTask::insertDoneSlot (DBObject& object)
{
    logdbg << "ASTERIXImporterTask: insertDoneSlot";
    --insert_active_;

    checkAllDone ();

    logdbg << "ASTERIXImporterTask: insertDoneSlot: done";
}

void ASTERIXImportTask::checkAllDone ()
{
    logdbg << "ASTERIXImporterTask: checkAllDone: all done " << all_done_ << " decode " << (decode_job_ == nullptr)
           //<< " wait map " << !waiting_for_map_
           << " map job " << (json_map_job_ == nullptr) << " map stubs " << (json_map_stub_job_ == nullptr)
           << " wait insert " << ! waiting_for_insert_ << " insert active " << (insert_active_ == 0);

    if (!all_done_ && decode_job_ == nullptr && json_map_job_ == nullptr
            && json_map_stub_job_ == nullptr && !waiting_for_insert_ && insert_active_ == 0)
    {
        loginf << "ASTERIXImporterTask: checkAllDone: setting all done";

        assert (status_widget_);
        status_widget_->setDone();

        all_done_ = true;

        QApplication::restoreOverrideCursor();

        refreshjASTERIX();

        assert (widget_);
        widget_->runDone();

        if (!create_mapping_stubs_ && !test_)
            emit ATSDB::instance().interface().databaseContentChangedSignal();

        task_manager_.appendInfo("ASTERIXImporterTask: inserted "+std::to_string(status_widget_->numRecordsInserted())
                                 +" records ("+status_widget_->recordsInsertedRateStr()+" rec/s)");

        for (auto& db_cnt_it : status_widget_->dboInsertedCounts())
            task_manager_.appendInfo("ASTERIXImporterTask: inserted "+std::to_string(db_cnt_it.second)
                                     +" "+db_cnt_it.first+" records");

        if (test_)
            task_manager_.appendSuccess("ASTERIXImporterTask: import test done after "
                                        +status_widget_->elapsedTimeStr());
        else if (create_mapping_stubs_)
            task_manager_.appendSuccess("ASTERIXImporterTask: create mapping stubs done after "
                                        +status_widget_->elapsedTimeStr());
        else
        {
            task_manager_.appendSuccess("ASTERIXImporterTask: import done after "+status_widget_->elapsedTimeStr());

            bool was_done = done_;
            done_ = true;

            // in case data was imported, clear other task done properties
            if (status_widget_->dboInsertedCounts().count("Radar"))
                ATSDB::instance().interface().setProperty(RadarPlotPositionCalculatorTask::DONE_PROPERTY_NAME, "0");

            ATSDB::instance().interface().setProperty(PostProcessTask::DONE_PROPERTY_NAME, "0");
            ATSDB::instance().interface().setProperty(CreateARTASAssociationsTask::DONE_PROPERTY_NAME, "0");

            ATSDB::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");

            if (!was_done) // only emit if first time done
                emit doneSignal(name_);
        }

        test_ = false; // set again by widget

        if (!show_done_summary_)
        {
            status_widget_->close();
            status_widget_ = nullptr;
        }
    }

    logdbg << "ASTERIXImporterTask: checkAllDone: done";
}

void ASTERIXImportTask::closeStatusDialogSlot()
{
    assert (status_widget_);
    status_widget_->close();
    status_widget_ = nullptr;
}

bool ASTERIXImportTask::maxLoadReached ()
{
    return insert_active_ >= 2;

//    if (limit_ram_)
//        return json_map_jobs_.size() > limited_num_json_jobs_;
//    else
//        return json_map_jobs_.size() > unlimited_num_json_jobs_;
}
