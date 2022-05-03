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

#include "asteriximporttask.h"
#include "asterixcategoryconfig.h"
#include "asteriximporttaskwidget.h"
#include "compass.h"
#include "buffer.h"
#include "configurable.h"
#include "createartasassociationstask.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variable.h"
#include "datasourcemanager.h"
#include "files.h"
#include "jobmanager.h"
#include "logger.h"
#include "asteriximporttaskdialog.h"
#include "radarplotpositioncalculatortask.h"
#include "stringconv.h"
#include "system.h"
#include "taskmanager.h"
#include "taskmanagerwidget.h"
#include "mainwindow.h"
#include "stringconv.h"

#include <jasterix/category.h>
#include <jasterix/edition.h>
#include <jasterix/jasterix.h>
#include <jasterix/refedition.h>

#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <QThread>
#include <QProgressDialog>

#include <algorithm>

using namespace Utils;
using namespace nlohmann;
using namespace std;

const unsigned int unlimited_chunk_size = 4000;
//const unsigned int limited_chunk_size = 2000;

const std::string DONE_PROPERTY_NAME = "asterix_data_imported";

const float ram_threshold = 4.0;

ASTERIXImportTask::ASTERIXImportTask(const std::string& class_id, const std::string& instance_id,
                                     TaskManager& task_manager)
    : Task("ASTERIXImportTask", "Import ASTERIX Data", false, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_import_asterix.json")
{
    tooltip_ = "Allows importing of ASTERIX data recording files into the opened database.";

    registerParameter("debug_jasterix", &debug_jasterix_, false);

    registerParameter("file_list", &file_list_, json::array());
    registerParameter("current_file_framing", &current_file_framing_, "");

    registerParameter("override_tod_offset", &override_tod_offset_, 0.0);

    std::string jasterix_definition_path = HOME_DATA_DIRECTORY + "jasterix_definitions";

    loginf << "ASTERIXImportTask: constructor: jasterix definition path '"
           << jasterix_definition_path << "'";
    assert(Files::directoryExists(jasterix_definition_path));

    jASTERIX::frame_chunk_size = unlimited_chunk_size;
    jASTERIX::data_block_chunk_size = unlimited_chunk_size;

    jasterix_ = std::make_shared<jASTERIX::jASTERIX>(jasterix_definition_path, false,
                                                     debug_jasterix_, true);

    createSubConfigurables();

    std::vector<std::string> framings = jasterix_->framings();
    if (std::find(framings.begin(), framings.end(), current_file_framing_) == framings.end())
    {
        loginf << "ASTERIXImportTask: constructor: resetting to no framing";
        current_file_framing_ = "";
    }
}

ASTERIXImportTask::~ASTERIXImportTask()
{
    loginf << "ASTERIXImportTask: destructor";
}

void ASTERIXImportTask::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "ASTERIXCategoryConfig")
    {
        unsigned int category = configuration()
                .getSubConfiguration(class_id, instance_id)
                .getParameterConfigValueUint("category");

        assert(category_configs_.find(category) == category_configs_.end());

        logdbg << "ASTERIXImportTask: generateSubConfigurable: generating asterix config "
               << instance_id << " with cat " << category;

        category_configs_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(category),                                // args for key
                    std::forward_as_tuple(category, class_id, instance_id, this));  // args for mapped value

        logdbg << "ASTERIXImportTask: generateSubConfigurable: cat " << category << " decode "
               << category_configs_.at(category).decode() << " edition '"
               << category_configs_.at(category).edition() << "' ref '"
               << category_configs_.at(category).ref() << "'";
    }
    else if (class_id == "ASTERIXJSONParsingSchema")
    {
        std::string name = configuration()
                .getSubConfiguration(class_id, instance_id)
                .getParameterConfigValueString("name");

        assert(schema_ == nullptr);
        assert(name == "jASTERIX");

        logdbg << "ASTERIXImportTask: generateSubConfigurable: generating schema " << instance_id
               << " with name " << name;

        schema_.reset(new ASTERIXJSONParsingSchema(class_id, instance_id, *this));
    }
    else
        throw std::runtime_error("ASTERIXImportTask: generateSubConfigurable: unknown class_id " +
                                 class_id);
}

void ASTERIXImportTask::asterixFileFraming(const std::string& asterix_framing)
{
    loginf << "ASTERIXImportTask: asterixFraming: framing '" << asterix_framing << "'";

    assert (jasterix_);
    std::vector<std::string> framings = jasterix_->framings();

    if (asterix_framing != ""
            && std::find(framings.begin(), framings.end(), asterix_framing) == framings.end())
        throw runtime_error ("ASTERIXImportTask: unknown framing '"+asterix_framing+"'");

    current_file_framing_ = asterix_framing;
}

void ASTERIXImportTask::asterixDecoderConfig(const std::string& asterix_decoder_cfg)
{
    loginf << "ASTERIXImportTask: asterixDecoderConfig: config string '" << asterix_decoder_cfg << "'";

    assert (jasterix_);

    json config = json::parse(asterix_decoder_cfg);

    if (!config.is_object())
        throw runtime_error("ASTERIXImportTask: asterixDecoderConfig: json config is not an object");

    for (auto& cat_it : config.items())
    {
        std::string cat_str = cat_it.key();

        unsigned int cat = stoi (cat_str);

        if (!hasConfiguratonFor(cat))
            throw runtime_error("ASTERIXImportTask: asterixDecoderConfig: unknown cat "+to_string(cat)
                                +" from '" + cat_str + "'");

        json& cat_cfg = cat_it.value();
        if (!cat_cfg.is_object())
            throw runtime_error("ASTERIXImportTask: asterixDecoderConfig: cat "+to_string(cat)
                                +" config is not an object");

        if (cat_cfg.contains("edition"))
        {
            if (!cat_cfg.at("edition").is_string())
                throw runtime_error("ASTERIXImportTask: asterixDecoderConfig: cat "+to_string(cat)
                                    +" edition is not a string");

            string edition = cat_cfg.at("edition");

            if (!jasterix_->category(cat)->hasEdition(edition))
                throw runtime_error("ASTERIXImportTask: asterixDecoderConfig: cat "+to_string(cat)
                                    +" has no edition '"+edition+"'");

            loginf << "ASTERIXImportTask: asterixDecoderConfig: setting cat " << cat
                   << " edition " << edition;

            category_configs_.at(cat).edition(edition);
        }

        if (cat_cfg.contains("ref_edition"))
        {
            if (!cat_cfg.at("ref_edition").is_string())
                throw runtime_error("ASTERIXImportTask: asterixDecoderConfig: cat "+to_string(cat)
                                    +" ref edition is not a string");

            string ref_ed = cat_cfg.at("ref_edition");

            if (!jasterix_->category(cat)->hasREFEdition(ref_ed))
                throw runtime_error("ASTERIXImportTask: asterixDecoderConfig: cat "+to_string(cat)
                                    +" has no ref edition '"+ref_ed+"'");

            loginf << "ASTERIXImportTask: asterixDecoderConfig: setting cat " << cat
                   << " ref edition " << ref_ed;

            category_configs_.at(cat).ref(ref_ed);
        }

        if (cat_cfg.contains("spf_edition"))
        {
            if (!cat_cfg.at("spf_edition").is_string())
                throw runtime_error("ASTERIXImportTask: asterixDecoderConfig: cat "+to_string(cat)
                                    +" spf edition is not a string");

            string spf_ed = cat_cfg.at("spf_edition");

            if (!jasterix_->category(cat)->hasSPFEdition(spf_ed))
                throw runtime_error("ASTERIXImportTask: asterixDecoderConfig: cat "+to_string(cat)
                                    +" has no spf edition '"+spf_ed+"'");

            loginf << "ASTERIXImportTask: asterixDecoderConfig: setting cat " << cat
                   << " spf edition " << spf_ed;

            category_configs_.at(cat).spf(spf_ed);
        }
    }

}

void ASTERIXImportTask::checkSubConfigurables()
{
    if (schema_ == nullptr)
    {
        Configuration& config =
                addNewSubConfiguration("JSONParsingSchema", "JSONParsingSchemajASTERIX0");
        config.addParameterString("name", "jASTERIX");
        generateSubConfigurable("JSONParsingSchema", "JSONParsingSchemajASTERIX0");
    }
}

ASTERIXImportTaskDialog* ASTERIXImportTask::dialog()
{
    if (!dialog_)
    {
        dialog_.reset(new ASTERIXImportTaskDialog(*this));

        connect(dialog_.get(), &ASTERIXImportTaskDialog::testTmportSignal,
                this, &ASTERIXImportTask::dialogTestImportSlot);

        connect(dialog_.get(), &ASTERIXImportTaskDialog::importSignal,
                this, &ASTERIXImportTask::dialogImportSlot);

        connect(dialog_.get(), &ASTERIXImportTaskDialog::cancelSignal,
                this, &ASTERIXImportTask::dialogCancelSlot);
    }

    assert(dialog_);
    return dialog_.get();

}

void ASTERIXImportTask::refreshjASTERIX()
{
    std::string jasterix_definition_path = HOME_DATA_DIRECTORY + "jasterix_definitions";

    loginf << "ASTERIXImportTask: refreshjASTERIX: jasterix definition path '"
           << jasterix_definition_path << "'";
    assert(Files::directoryExists(jasterix_definition_path));

    jasterix_ = std::make_shared<jASTERIX::jASTERIX>(jasterix_definition_path, false,
                                                     debug_jasterix_, true);

    std::vector<std::string> framings = jasterix_->framings();
    if (std::find(framings.begin(), framings.end(), current_file_framing_) == framings.end())
    {
        loginf << "ASTERIXImportTask: refreshjASTERIX: resetting to no framing";
        current_file_framing_ = "";
    }
}

std::vector<std::string> ASTERIXImportTask::fileList()
{
    return file_list_.get<std::vector<string>>();
}

void ASTERIXImportTask::addFile(const std::string& filename)
{
    loginf << "ASTERIXImportTask: addFile: filename '" << filename << "'";

    vector<string> tmp_list = file_list_.get<std::vector<string>>();

    if (find(tmp_list.begin(), tmp_list.end(), filename) == tmp_list.end())
    {
        loginf << "ASTERIXImportTask: addFile: adding filename '" << filename << "'";

        tmp_list.push_back(filename);

        sort(tmp_list.begin(), tmp_list.end());

        file_list_ = tmp_list;
    }

    emit statusChangedSignal(name_);
}

void ASTERIXImportTask::clearFileList ()
{
    loginf << "ASTERIXImportTask: removeAllFiles";

    file_list_.clear();
}

void ASTERIXImportTask::importFilename(const std::string& filename)
{
    loginf << "ASTERIXImportTask: currentFilename: filename '" << filename << "'";

    current_filename_ = filename;
    import_file_ = true;

    addFile(filename);

    if (dialog_)
        dialog_->updateButtons();
}

void ASTERIXImportTask::importNetwork()
{
    loginf << "ASTERIXImportTask: importNetwork";

    current_filename_ = "";
    import_file_ = false;

    if (dialog_)
        dialog_->updateButtons();
}

bool ASTERIXImportTask::isImportNetwork()
{
    return !import_file_;
}

const std::string& ASTERIXImportTask::currentFraming() const { return current_file_framing_; }

void ASTERIXImportTask::currentFraming(const std::string& current_framing)
{
    current_file_framing_ = current_framing;
}

bool ASTERIXImportTask::hasConfiguratonFor(unsigned int category)
{
    return category_configs_.count(category) > 0;
}

bool ASTERIXImportTask::decodeCategory(unsigned int category)
{
    assert(hasConfiguratonFor(category));
    return category_configs_.at(category).decode();
}

void ASTERIXImportTask::decodeCategory(unsigned int category, bool decode)
{
    assert(jasterix_->hasCategory(category));

    loginf << "ASTERIXImportTask: decodeCategory: cat " << category << " decode " << decode;

    if (!hasConfiguratonFor(category))
    {
        Configuration& new_cfg = configuration().addNewSubConfiguration("ASTERIXCategoryConfig");
        new_cfg.addParameterUnsignedInt("category", category);
        new_cfg.addParameterBool("decode", decode);
        new_cfg.addParameterString("edition", jasterix_->category(category)->defaultEdition());
        new_cfg.addParameterString("ref", jasterix_->category(category)->defaultREFEdition());

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert(hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).decode(decode);
}

std::string ASTERIXImportTask::editionForCategory(unsigned int category)
{
    assert(hasConfiguratonFor(category));

    // check if edition exists, otherwise rest to default
    if (jasterix_->category(category)->editions().count(category_configs_.at(category).edition()) ==
            0)
    {
        loginf << "ASTERIXImportTask: editionForCategory: cat " << category
               << " reset to default edition";
        category_configs_.at(category).edition(jasterix_->category(category)->defaultEdition());
    }

    return category_configs_.at(category).edition();
}

void ASTERIXImportTask::editionForCategory(unsigned int category, const std::string& edition)
{
    assert(jasterix_->hasCategory(category));

    loginf << "ASTERIXImportTask: editionForCategory: cat " << category << " edition " << edition;

    if (!hasConfiguratonFor(category))
    {
        Configuration& new_cfg = configuration().addNewSubConfiguration("ASTERIXCategoryConfig");
        new_cfg.addParameterUnsignedInt("category", category);
        new_cfg.addParameterBool("decode", false);
        new_cfg.addParameterString("edition", edition);
        new_cfg.addParameterString("ref", jasterix_->category(category)->defaultREFEdition());

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert(hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).edition(edition);
}

std::string ASTERIXImportTask::refEditionForCategory(unsigned int category)
{
    assert(hasConfiguratonFor(category));

    // check if edition exists, otherwise rest to default
    if (category_configs_.at(category).ref().size() &&  // if value set and not exist in jASTERIX
            jasterix_->category(category)->refEditions().count(category_configs_.at(category).ref()) ==
            0)
    {
        loginf << "ASTERIXImportTask: refForCategory: cat " << category
               << " reset to default ref";
        category_configs_.at(category).ref(jasterix_->category(category)->defaultREFEdition());
    }

    return category_configs_.at(category).ref();
}

void ASTERIXImportTask::refEditionForCategory(unsigned int category, const std::string& ref)
{
    assert(jasterix_->hasCategory(category));

    loginf << "ASTERIXImportTask: refForCategory: cat " << category << " ref '" << ref << "'";

    if (!hasConfiguratonFor(category))
    {
        Configuration& new_cfg = configuration().addNewSubConfiguration("ASTERIXCategoryConfig");
        new_cfg.addParameterUnsignedInt("category", category);
        new_cfg.addParameterBool("decode", false);
        new_cfg.addParameterString("edition", jasterix_->category(category)->defaultEdition());
        new_cfg.addParameterString("ref", ref);

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert(hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).ref(ref);
}

std::string ASTERIXImportTask::spfEditionForCategory(unsigned int category)
{
    assert(hasConfiguratonFor(category));

    // check if edition exists, otherwise rest to default
    if (category_configs_.at(category).spf().size() &&  // if value set and not exist in jASTERIX
            jasterix_->category(category)->spfEditions().count(category_configs_.at(category).spf()) ==
            0)
    {
        loginf << "ASTERIXImportTask: spfEditionForCategory: cat " << category
               << " reset to default spf";
        category_configs_.at(category).spf(jasterix_->category(category)->defaultSPFEdition());
    }

    return category_configs_.at(category).spf();
}

void ASTERIXImportTask::spfEditionForCategory(unsigned int category, const std::string& spf)
{
    assert(jasterix_->hasCategory(category));

    loginf << "ASTERIXImportTask: spfEditionForCategory: cat " << category << " spf '" << spf
           << "'";

    if (!hasConfiguratonFor(category))
    {
        Configuration& new_cfg = configuration().addNewSubConfiguration("ASTERIXCategoryConfig");
        new_cfg.addParameterUnsignedInt("category", category);
        new_cfg.addParameterBool("decode", false);
        new_cfg.addParameterString("edition", jasterix_->category(category)->defaultEdition());
        new_cfg.addParameterString("spf", spf);

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert(hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).spf(spf);
}

std::shared_ptr<ASTERIXJSONParsingSchema> ASTERIXImportTask::schema() const { return schema_; }

bool ASTERIXImportTask::debug() const { return debug_jasterix_; }

void ASTERIXImportTask::debug(bool debug_jasterix)
{
    debug_jasterix_ = debug_jasterix;

    assert(jasterix_);
    jasterix_->setDebug(debug_jasterix_);

    loginf << "ASTERIXImportTask: debug " << debug_jasterix_;
}

bool ASTERIXImportTask::checkPrerequisites()
{
    if (!COMPASS::instance().interface().ready())  // must be connected
        return false;

    if (COMPASS::instance().interface().hasProperty(DONE_PROPERTY_NAME))
        done_ = COMPASS::instance().interface().getProperty(DONE_PROPERTY_NAME) == "1";

    return true;
}

bool ASTERIXImportTask::isRecommended()
{
    if (!checkPrerequisites())
        return false;

    if (COMPASS::instance().dbContentManager().hasData())
        return false;

    return true;
}

bool ASTERIXImportTask::isRequired() { return false; }

bool ASTERIXImportTask::overrideTodActive() const { return override_tod_active_; }

void ASTERIXImportTask::overrideTodActive(bool value)
{
    loginf << "ASTERIXImportTask: overrideActive: value " << value;

    override_tod_active_ = value;
}

float ASTERIXImportTask::overrideTodOffset() const { return override_tod_offset_; }

void ASTERIXImportTask::overrideTodOffset(float value)
{
    loginf << "ASTERIXImportTask: overrideTodOffset: value " << value;

    override_tod_offset_ = value;
}

unsigned int ASTERIXImportTask::fileLineID() const
{
    return file_line_id_;
}

void ASTERIXImportTask::fileLineID(unsigned int value)
{
    loginf << "ASTERIXImportTask: fileLineID: value " << value;

    file_line_id_ = value;
}

bool ASTERIXImportTask::isRunning() const
{
    return running_;
}

bool ASTERIXImportTask::canImportFile()
{
    if (!current_filename_.size())
        return false;
    
    if (!Files::fileExists(current_filename_))
    {
        loginf << "ASTERIXImportTask: canImportFile: not possible since file '"
               << current_filename_ << "'does not exist";
        return false;
    }

    return true;
}

bool ASTERIXImportTask::canRun()
{
    if (import_file_)
        return canImportFile(); // set file exists
    else
        return COMPASS::instance().dataSourceManager().getNetworkLines().size(); // there are network lines defined
}

void ASTERIXImportTask::run()
{
    run (false);
}

void ASTERIXImportTask::stop()
{
    loginf << "ASTERIXImportTask: stop";

    stopped_ = true;

    if (decode_job_)
        decode_job_->setObsolete();

    for (auto& job_it : json_map_jobs_)
        job_it->setObsolete();

    for (auto& job_it : postprocess_jobs_)
        job_it->setObsolete();

    while(decode_job_ && !decode_job_->done())
    {
        loginf << "ASTERIXImportTask: stop: waiting for decode job to finish";

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(1);
    }

    while(json_map_jobs_.size())
    {
        loginf << "ASTERIXImportTask: stop: waiting for map job to finish";

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(1);
    }

    while(postprocess_jobs_.size())
    {
        loginf << "ASTERIXImportTask: stop: waiting for post-process job to finish";

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(1);
    }

    while (waiting_for_insert_)
    {
        loginf << "ASTERIXImportTask: stop: waiting for insert to finish";

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(1);
    }

    //    done_ = true; // set by checkAllDone
    //    running_ = false;

    loginf << "ASTERIXImportTask: stop done";
}

void ASTERIXImportTask::run(bool test) // , bool create_mapping_stubs
{
    test_ = test;

    assert (!running_);

    if (!import_file_)
        COMPASS::instance().appMode(AppMode::LiveRunning); // set live mode

    running_ = true;
    done_ = false; // since can be run multiple times

    num_packets_in_processing_ = 0;
    num_packets_total_ = 0;

    num_records_ = 0;

    start_time_ = boost::posix_time::microsec_clock::local_time();

    last_insert_time_ = boost::posix_time::microsec_clock::local_time();

    float free_ram = System::getFreeRAMinGB();

    loginf << "ASTERIXImportTask: run: filename " << current_filename_ << " test " << test_
           << " free RAM " << free_ram << " GB";

    if (test_)
        task_manager_.appendInfo("ASTERIXImportTask: test import of file '" + current_filename_ +
                                 "' started");
    else
        task_manager_.appendInfo("ASTERIXImportTask: import of file '" + current_filename_ +
                                 "' started");

    assert(canRun());

    if (import_file_)
    {
        updateFileProgressDialog();

        boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

        while ((boost::posix_time::microsec_clock::local_time() - start_time).total_milliseconds() < 50)
        {
            QCoreApplication::processEvents();
        }
    }

    insert_active_ = false;

    all_done_ = false;

    added_data_sources_.clear();

    assert(schema_);

    for (auto& map_it : *schema_)
        if (!map_it.second->initialized())
            map_it.second->initialize();

    loginf << "ASTERIXImportTask: run: setting categories";

    jASTERIX::add_artas_md5_hash = true;

    // set category configs
    jasterix_->decodeNoCategories();

    for (auto& cat_it : category_configs_)
    {
        // loginf << "ASTERIXImportTask: importFile: setting category " << cat_it.first;

        loginf << "ASTERIXImportTask: run: setting cat " << cat_it.first << " decode "
               << cat_it.second.decode() << " edition '" << cat_it.second.edition() << "' ref '"
               << cat_it.second.ref() << "'";

        if (!jasterix_->hasCategory(cat_it.first))
        {
            logwrn << "ASTERIXImportTask: run: cat '" << cat_it.first
                   << "' not defined in decoder";
            continue;
        }

        if (!jasterix_->category(cat_it.first)->hasEdition(cat_it.second.edition()))
        {
            logwrn << "ASTERIXImportTask: run: cat " << cat_it.first << " edition '"
                   << cat_it.second.edition() << "' not defined in decoder";
            continue;
        }

        if (cat_it.second.ref().size() &&  // only if value set
                !jasterix_->category(cat_it.first)->hasREFEdition(cat_it.second.ref()))
        {
            logwrn << "ASTERIXImportTask: run: cat " << cat_it.first << " ref '"
                   << cat_it.second.ref() << "' not defined in decoder";
            continue;
        }

        if (cat_it.second.spf().size() &&  // only if value set
                !jasterix_->category(cat_it.first)->hasSPFEdition(cat_it.second.spf()))
        {
            logwrn << "ASTERIXImportTask: run: cat " << cat_it.first << " spf '"
                   << cat_it.second.spf() << "' not defined in decoder";
            continue;
        }

        //        loginf << "ASTERIXImportTask: importFile: setting cat " <<  cat_it.first
        //               << " decode flag " << cat_it.second.decode();
        jasterix_->setDecodeCategory(cat_it.first, cat_it.second.decode());
        loginf << "ASTERIXImportTask: run: setting cat " <<  cat_it.first
               << " edition " << cat_it.second.edition();
        jasterix_->category(cat_it.first)->setCurrentEdition(cat_it.second.edition());
        jasterix_->category(cat_it.first)->setCurrentREFEdition(cat_it.second.ref());
        jasterix_->category(cat_it.first)->setCurrentSPFEdition(cat_it.second.spf());
    }

    loginf << "ASTERIXImportTask: run: starting decode job";

    assert(decode_job_ == nullptr);

    decode_job_ = make_shared<ASTERIXDecodeJob>(*this, test_, post_process_);

    if (import_file_)
        decode_job_->setDecodeFile(current_filename_, current_file_framing_); // do file import
    else
    {
        COMPASS::instance().dataSourceManager().createNetworkDBDataSources();
        decode_job_->setDecodeUDPStreams(COMPASS::instance().dataSourceManager().getNetworkLines()); // record from network
    }


    connect(decode_job_.get(), &ASTERIXDecodeJob::obsoleteSignal, this,
            &ASTERIXImportTask::decodeASTERIXObsoleteSlot, Qt::QueuedConnection);
    connect(decode_job_.get(), &ASTERIXDecodeJob::doneSignal, this,
            &ASTERIXImportTask::decodeASTERIXDoneSlot, Qt::QueuedConnection);
    connect(decode_job_.get(), &ASTERIXDecodeJob::decodedASTERIXSignal, this,
            &ASTERIXImportTask::addDecodedASTERIXSlot, Qt::QueuedConnection);

    JobManager::instance().addBlockingJob(decode_job_);

    return;
}

void ASTERIXImportTask::dialogImportSlot()
{
    loginf << "ASTERIXImportTask: dialogImportSlot";

    assert (dialog_);
    dialog_->hide();

    assert (canRun());
    run (false);
}

void ASTERIXImportTask::dialogTestImportSlot()
{
    loginf << "ASTERIXImportTask: dialogImportSlot";

    assert (dialog_);
    dialog_->hide();

    assert (canRun());

    run (true);
}

void ASTERIXImportTask::dialogCancelSlot()
{
    loginf << "ASTERIXImportTask: dialogCancelSlot";

    assert (dialog_);
    dialog_->hide();
}

void ASTERIXImportTask::decodeASTERIXDoneSlot()
{
    logdbg << "ASTERIXImportTask: decodeASTERIXDoneSlot";

    if (!decode_job_) // called twice?
        return;

    assert(decode_job_);

    if (!stopped_ && decode_job_->error())
    {
        loginf << "ASTERIXImportTask: decodeASTERIXDoneSlot: error";
        error_ = decode_job_->error();
        error_message_ = decode_job_->errorMessage();

        QMessageBox msgBox;
        msgBox.setText(
                    ("Decoding error: " + error_message_ + "\n\nPlease check the decoder settings.")
                    .c_str());
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.exec();
    }

    decode_job_ = nullptr;

    checkAllDone();
}
void ASTERIXImportTask::decodeASTERIXObsoleteSlot()
{
    logdbg << "ASTERIXImportTask: decodeASTERIXObsoleteSlot";

    decode_job_ = nullptr;
}

void ASTERIXImportTask::addDecodedASTERIXSlot()
{
    logdbg << "ASTERIXImportTask: addDecodedASTERIXSlot";

    if (stopped_)
    {
        checkAllDone();
        return;
    }

    assert(decode_job_);

    logdbg << "ASTERIXImportTask: addDecodedASTERIXSlot: errors " << decode_job_->numErrors()
           << " num records " << jasterix_->numRecords();

    if (import_file_)
    {
        updateFileProgressDialog();

        if (file_progress_dialog_->wasCanceled())
        {
            stop();
            return;
        }
    }

    // remove if break here for testing
    if (import_file_ && maxLoadReached())
    {
        logdbg << "ASTERIXImportTask: addDecodedASTERIXSlot: returning since max load reached";
        return;
    }

    if (stopped_)
        return;

    logdbg << "ASTERIXImportTask: addDecodedASTERIXSlot: processing data";

    std::unique_ptr<nlohmann::json> extracted_data {decode_job_->extractedData()};
    ++num_packets_in_processing_;
    ++num_packets_total_;

    loginf << "ASTERIXImportTask: addDecodedASTERIXSlot: processing data,"
           << " num_packets_in_processing_ " << num_packets_in_processing_
           << " num_packets_total_ " << num_packets_total_;

    if (!extracted_data)
    {
        logwrn << "ASTERIXImportTask: addDecodedASTERIXSlot: processing data empty";
        return;
    }

    // break here for testing
    //return;

    if (stopped_)
        return;

    //assert(!json_map_job_);

    assert(schema_);

    std::vector<std::string> keys;

    if (current_file_framing_ == "" || !import_file_) // force netto when doing network import
        keys = {"data_blocks", "content", "records"};
    else
        keys = {"frames", "content", "data_blocks", "content", "records"};

    std::shared_ptr<ASTERIXJSONMappingJob> json_map_job =
            make_shared<ASTERIXJSONMappingJob>(std::move(extracted_data), keys, schema_->parsers());

    json_map_jobs_.push_back(json_map_job);

    assert(!extracted_data);

    connect(json_map_job.get(), &ASTERIXJSONMappingJob::obsoleteSignal, this,
            &ASTERIXImportTask::mapJSONObsoleteSlot, Qt::QueuedConnection);
    connect(json_map_job.get(), &ASTERIXJSONMappingJob::doneSignal, this,
            &ASTERIXImportTask::mapJSONDoneSlot, Qt::QueuedConnection);

    JobManager::instance().addNonBlockingJob(json_map_job);
}

void ASTERIXImportTask::mapJSONDoneSlot()
{
    logdbg << "ASTERIXImportTask: mapJSONDoneSlot";

    // break here for testing
    //    json_map_job_ = nullptr;
    //    num_packets_in_processing_--;

    //    if (decode_job_ && decode_job_->hasData())
    //        addDecodedASTERIXSlot(); // load next chunk
    //    else
    //        checkAllDone();

    //    return;

    if (stopped_)
    {
        logdbg << "ASTERIXImportTask: mapJSONDoneSlot: stopping";

        json_map_jobs_.clear();

        checkAllDone();

        return;
    }

    ASTERIXJSONMappingJob* map_job = dynamic_cast<ASTERIXJSONMappingJob*>(QObject::sender());
    assert(map_job);

    std::map<std::string, std::shared_ptr<Buffer>> job_buffers {map_job->buffers()};

    assert (json_map_jobs_.size());
    assert (json_map_jobs_.begin()->get() == map_job);
    map_job = nullptr;
    json_map_jobs_.erase(json_map_jobs_.begin()); // remove

    logdbg << "ASTERIXImportTask: mapJSONDoneSlot: processing, num buffers " << job_buffers.size();

    if (!job_buffers.size())
        return;

    if (!test_)
    {
        std::shared_ptr<ASTERIXPostprocessJob> postprocess_job =
                make_shared<ASTERIXPostprocessJob>(std::move(job_buffers),
                                                   override_tod_active_, override_tod_offset_,
                                                   !import_file_);

        postprocess_jobs_.push_back(postprocess_job);

        // check for future when net import

        connect(postprocess_job.get(), &ASTERIXPostprocessJob::obsoleteSignal, this,
                &ASTERIXImportTask::postprocessObsoleteSlot, Qt::QueuedConnection);
        connect(postprocess_job.get(), &ASTERIXPostprocessJob::doneSignal, this,
                &ASTERIXImportTask::postprocessDoneSlot, Qt::QueuedConnection);

        JobManager::instance().addNonBlockingJob(postprocess_job);
    }

    if (test_)
    {
        checkAllDone();
    }

    logdbg << "ASTERIXImportTask: mapJSONDoneSlot: done";
}

void ASTERIXImportTask::mapJSONObsoleteSlot()
{
    logdbg << "ASTERIXImportTask: mapJSONObsoleteSlot";

    ASTERIXJSONMappingJob* map_job = dynamic_cast<ASTERIXJSONMappingJob*>(QObject::sender());
    assert(map_job);

    assert (json_map_jobs_.size());
    assert (json_map_jobs_.begin()->get() == map_job);
    map_job = nullptr;
    json_map_jobs_.erase(json_map_jobs_.begin()); // remove

    checkAllDone();

}

void ASTERIXImportTask::postprocessDoneSlot()
{
    logdbg << "ASTERIXImportTask: postprocessDoneSlot: import_file " << import_file_;

    if (stopped_)
    {
        postprocess_jobs_.clear();

        checkAllDone();

        return;
    }

    ASTERIXPostprocessJob* post_job = dynamic_cast<ASTERIXPostprocessJob*>(QObject::sender());
    assert(post_job);

    std::map<std::string, std::shared_ptr<Buffer>> job_buffers {post_job->buffers()};

    assert (postprocess_jobs_.size());
    assert (postprocess_jobs_.begin()->get() == post_job);
    post_job = nullptr;
    postprocess_jobs_.erase(postprocess_jobs_.begin()); // remove


    if (import_file_) // cache and insert only if time elapsed
    {
        //        // merge into existing buffers

        //        for (auto& buf_it : job_buffers)
        //        {
        //            if (!buffer_cache_.count(buf_it.first)) // set if first
        //                buffer_cache_[buf_it.first] = buf_it.second;
        //            else // append by seize if existing
        //                buffer_cache_.at(buf_it.first)->seizeBuffer(*buf_it.second.get());
        //        }

        //        job_buffers.clear();

        //        // insert if time elapsed or all other jobs are done
        //        if ((boost::posix_time::microsec_clock::local_time() - last_insert_time_).total_milliseconds() > 2000
        //                || (decode_job_ == nullptr && json_map_job_ == nullptr && postprocess_job_ == nullptr))
        //        {
        //            logdbg << "ASTERIXImportTask: postprocessDoneSlot: doing file insert";

        //            last_insert_time_ = boost::posix_time::microsec_clock::local_time();

        //            std::map<std::string, std::shared_ptr<Buffer>> buffer_cache_cpy = move(buffer_cache_);
        //            buffer_cache_.clear(); // to be safe

        //            insertData(std::move(buffer_cache_cpy));
        //        }

        insertData(std::move(job_buffers));
    }
    else // insert immediately for network
    {
        logdbg << "ASTERIXImportTask: postprocessDoneSlot: doing network insert";

        insertData(std::move(job_buffers));
    }
}

void ASTERIXImportTask::postprocessObsoleteSlot()
{
    ASTERIXPostprocessJob* post_job = dynamic_cast<ASTERIXPostprocessJob*>(QObject::sender());
    assert(post_job);

    assert (postprocess_jobs_.size());
    assert (postprocess_jobs_.begin()->get() == post_job);
    post_job = nullptr;
    postprocess_jobs_.erase(postprocess_jobs_.begin()); // remove
}

void ASTERIXImportTask::insertData(std::map<std::string, std::shared_ptr<Buffer>> job_buffers)
{
    logdbg << "ASTERIXImportTask: insertData: inserting " << job_buffers.size() << " into database";

    if (stopped_)
    {
        checkAllDone();
        return;
    }

    assert (!test_);

    DBContentManager& object_manager = COMPASS::instance().dbContentManager();

    while (insert_active_ && !stopped_ && object_manager.insertInProgress())
    {
        //loginf << "ASTERIXImportTask: insertData: waiting for insert";

        waiting_for_insert_ = true;

        logdbg << "ASTERIXImportTask: addDecodedASTERIXSlot: waiting on insert to finish";
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        QThread::msleep(1);

        if (stopped_)
        {
            waiting_for_insert_ = false;

            checkAllDone();
            return;
        }
    }

    waiting_for_insert_ = false;

    if (stopped_)
    {
        checkAllDone();
        return;
    }

    //loginf << "ASTERIXImportTask: insertData: no more waiting for insert";

    assert(schema_);

    for (auto& job_it : job_buffers)
        num_records_ += job_it.second->size();

    insert_active_ = true;

    connect(&object_manager, &DBContentManager::insertDoneSignal,
            this, &ASTERIXImportTask::insertDoneSlot, Qt::UniqueConnection);

    object_manager.insertData(job_buffers);

    checkAllDone();

    logdbg << "JSONImporterTask: insertData: done";
}

void ASTERIXImportTask::insertDoneSlot()
{
    logdbg << "ASTERIXImportTask: insertDoneSlot";

    insert_active_ = false;

    --num_packets_in_processing_;

    if (import_file_)
    {
        logdbg << "ASTERIXImportTask: insertDoneSlot: num_packets_in_processing " << num_packets_in_processing_;

        updateFileProgressDialog();
    }

    bool test = test_; // test_ cleared by checkAllDone

    checkAllDone();

    logdbg << "ASTERIXImportTask: insertDoneSlot: check done";

    if (all_done_ && !test)
    {
        logdbg << "ASTERIXImportTask: insertDoneSlot: finalizing";

        disconnect(&COMPASS::instance().dbContentManager(), &DBContentManager::insertDoneSignal,
                   this, &ASTERIXImportTask::insertDoneSlot);

        emit doneSignal(name_);
    }

    loginf << "ASTERIXImportTask: insertDoneSlot: processed " << num_records_ << " records";

    if (decode_job_ && decode_job_->hasData())
    {
        logdbg << "ASTERIXImportTask: insertDoneSlot: starting decoding of next chunk";
        addDecodedASTERIXSlot(); // load next chunk
    }

    logdbg << "ASTERIXImportTask: insertDoneSlot: done";
}

void ASTERIXImportTask::checkAllDone()
{
    logdbg << "ASTERIXImportTask: checkAllDone: all done " << all_done_ << " decode "
           << (decode_job_ != nullptr)
           << " map jobs " << json_map_jobs_.size()
           << " post jobs " << postprocess_jobs_.size()
           << " wait insert " << waiting_for_insert_
           << " insert active " << insert_active_;

    if (!all_done_ && decode_job_ == nullptr && !json_map_jobs_.size() && !postprocess_jobs_.size()
            && !waiting_for_insert_ && !insert_active_)
    {
        logdbg << "ASTERIXImportTask: checkAllDone: setting all done: total packets " << num_packets_total_;

        if (import_file_ && file_progress_dialog_)
        {
            file_progress_dialog_ = nullptr;
        }

        all_done_ = true;
        done_ = true; // why was this not set?
        running_ = false;

        boost::posix_time::time_duration time_diff = boost::posix_time::microsec_clock::local_time() - start_time_;
        loginf << "ASTERIXImportTask: checkAllDone: import done after "
               << String::timeStringFromDouble(time_diff.total_milliseconds() / 1000.0, false);

        COMPASS::instance().mainWindow().updateMenus(); // re-enable import menu

        QApplication::restoreOverrideCursor();

        logdbg << "ASTERIXImportTask: checkAllDone: refresh";

        refreshjASTERIX();

        logdbg << "ASTERIXImportTask: checkAllDone: dbo content";

        if (!test_)
            emit COMPASS::instance().interface().databaseContentChangedSignal();

        logdbg << "ASTERIXImportTask: checkAllDone: status logging";

        if (!show_done_summary_)
        {
            logdbg << "ASTERIXImportTask: checkAllDone: deleting status widget";
        }

        COMPASS::instance().dataSourceManager().saveDBDataSources();
    }

    logdbg << "ASTERIXImportTask: checkAllDone: done";
}

bool ASTERIXImportTask::maxLoadReached()
{
    return num_packets_in_processing_> 2;
}

void ASTERIXImportTask::updateFileProgressDialog()
{
    if (stopped_)
        return;

    if (!file_progress_dialog_)
    {
        file_progress_dialog_.reset(
                    new QProgressDialog(("File '"+current_filename_+"'").c_str(), "Abort", 0, 100));
        file_progress_dialog_->setWindowTitle("Importing ASTERIX Recording");
        file_progress_dialog_->setWindowModality(Qt::WindowModal);
    }

    string text = "File '"+current_filename_+"'";
    string rec_text;
    string rem_text;

    if (decode_job_)
    {
        file_progress_dialog_->setValue(decode_job_->getFileDecodingProgress());

        rec_text = "\n\nRecords/s: "+to_string((unsigned int) decode_job_->getRecordsPerSecond());
        rem_text = "Remaining: "+String::timeStringFromDouble(decode_job_->getRemainingTime() + 1.0, false);
    }
    else
    {
        rec_text = "\n\nRecords/s: Unknown";
        rem_text = "Remaining: Unknown";
    }

    //string pack_text = "\npacks: "+to_string(num_packets_in_processing_) + " total " + to_string(num_packets_total_);

    int num_filler = text.size() - rec_text.size() - rem_text.size();
    if (num_filler < 1)
        num_filler = 1;

    file_progress_dialog_->setLabelText((text + rec_text + std::string(num_filler, ' ') + rem_text).c_str());
}
