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
#include "asterixstatusdialog.h"
#include "compass.h"
#include "buffer.h"
#include "configurable.h"
#include "createartasassociationstask.h"
#include "dbinterface.h"
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "dbovariable.h"
#include "files.h"
#include "jobmanager.h"
#include "logger.h"
//#include "postprocesstask.h"
#include "radarplotpositioncalculatortask.h"
#include "savedfile.h"
#include "stringconv.h"
#include "system.h"
#include "taskmanager.h"
#include "taskmanagerwidget.h"

#include <jasterix/category.h>
#include <jasterix/edition.h>
#include <jasterix/jasterix.h>
#include <jasterix/refedition.h>

#include <QApplication>
#include <QCoreApplication>
#include <QMessageBox>
#include <QThread>
#include <algorithm>

using namespace Utils;
using namespace nlohmann;
using namespace std;

const unsigned int unlimited_chunk_size = 10000;
const unsigned int limited_chunk_size = 5000;

// const unsigned int unlimited_num_json_jobs_ = 2;
// const unsigned int limited_num_json_jobs_ = 1;

const std::string DONE_PROPERTY_NAME = "asterix_data_imported";

const float ram_threshold = 4.0;

ASTERIXImportTask::ASTERIXImportTask(const std::string& class_id, const std::string& instance_id,
                                     TaskManager& task_manager)
    : Task("ASTERIXImportTask", "Import ASTERIX Data", false, false, task_manager),
      Configurable(class_id, instance_id, &task_manager, "task_import_asterix.json")
{
    tooltip_ = "Allows importing of ASTERIX data recording files into the opened database.";

    registerParameter("debug_jasterix", &debug_jasterix_, false);
    registerParameter("limit_ram", &limit_ram_, false);
    registerParameter("current_filename", &current_filename_, "");
    registerParameter("current_framing", &current_framing_, "");

    registerParameter("override_sac_org", &post_process_.override_sac_org_, 0);
    registerParameter("override_sic_org", &post_process_.override_sic_org_, 0);
    registerParameter("override_sac_new", &post_process_.override_sac_new_, 1);
    registerParameter("override_sic_new", &post_process_.override_sic_new_, 1);
    registerParameter("override_tod_offset", &post_process_.override_tod_offset_, 0.0);

    std::string jasterix_definition_path = HOME_DATA_DIRECTORY + "jasterix_definitions";

    loginf << "ASTERIXImportTask: constructor: jasterix definition path '"
           << jasterix_definition_path << "'";
    assert(Files::directoryExists(jasterix_definition_path));

    if (limit_ram_)
    {
        jASTERIX::frame_chunk_size = limited_chunk_size;
        jASTERIX::data_block_chunk_size = limited_chunk_size;
    }
    else
    {
        jASTERIX::frame_chunk_size = unlimited_chunk_size;
        jASTERIX::data_block_chunk_size = unlimited_chunk_size;
    }

    jasterix_ = std::make_shared<jASTERIX::jASTERIX>(jasterix_definition_path, false,
                                                     debug_jasterix_, true);

    createSubConfigurables();

    std::vector<std::string> framings = jasterix_->framings();
    if (std::find(framings.begin(), framings.end(), current_framing_) == framings.end())
    {
        loginf << "ASTERIXImportTask: constructor: resetting to no framing";
        current_framing_ = "";
    }
}

ASTERIXImportTask::~ASTERIXImportTask()
{
    for (auto it : file_list_)
        delete it.second;

    file_list_.clear();
}

void ASTERIXImportTask::generateSubConfigurable(const std::string& class_id,
                                                const std::string& instance_id)
{
    if (class_id == "ASTERIXFile")
    {
        SavedFile* file = new SavedFile(class_id, instance_id, this);
        assert(file_list_.count(file->name()) == 0);
        file_list_.insert(std::pair<std::string, SavedFile*>(file->name(), file));
    }
    else if (class_id == "ASTERIXCategoryConfig")
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

void ASTERIXImportTask::asterixFraming(const std::string& asterix_framing)
{
    loginf << "ASTERIXImportTask: asterixFraming: framing '" << asterix_framing << "'";

    assert (jasterix_);
    std::vector<std::string> framings = jasterix_->framings();

    if (asterix_framing != ""
            && std::find(framings.begin(), framings.end(), asterix_framing) == framings.end())
        throw runtime_error ("ASTERIXImportTask: unknown framing '"+asterix_framing+"'");

    current_framing_ = asterix_framing;
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

        //        if (cat_cfg.contains("mapping"))
        //        {
        //            if (!cat_cfg.at("mapping").is_string())
        //                throw runtime_error("ASTERIXImportTask: asterixDecoderConfig: cat "+to_string(cat)
        //                                    +" mapping is not a string");

        //            string mapping = cat_cfg.at("mapping");

        //            std::vector<std::string> mappings = getPossibleMappings (cat);

        //            if (std::find(mappings.begin(), mappings.end(), mapping) == mappings.end())
        //                throw runtime_error ("ASTERIXImportTask: unknown mapping '"+mapping+"'");

        //            loginf << "ASTERIXImportTask: asterixDecoderConfig: setting cat " << cat
        //                   << " mapping '" << mapping << "'";

        //             setActiveMapping (cat, mapping);
        //        }
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

TaskWidget* ASTERIXImportTask::widget()
{
    if (!widget_)
    {
        widget_.reset(new ASTERIXImportTaskWidget(*this));

        connect(&task_manager_, &TaskManager::expertModeChangedSignal, widget_.get(),
                &ASTERIXImportTaskWidget::expertModeChangedSlot);
    }

    assert(widget_);
    return widget_.get();
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
    if (std::find(framings.begin(), framings.end(), current_framing_) == framings.end())
    {
        loginf << "ASTERIXImportTask: refreshjASTERIX: resetting to no framing";
        current_framing_ = "";
    }
}

void ASTERIXImportTask::addFile(const std::string& filename)
{
    loginf << "ASTERIXImportTask: addFile: filename '" << filename << "'";

    if (file_list_.count(filename) != 0)
        throw std::invalid_argument("ASTERIXImportTask: addFile: name '" + filename +
                                    "' already in use");

    std::string instancename = filename;
    instancename.erase(std::remove(instancename.begin(), instancename.end(), '/'),
                       instancename.end());

    Configuration& config = addNewSubConfiguration("ASTERIXFile", "ASTERIXFile" + instancename);
    config.addParameterString("name", filename);
    generateSubConfigurable("ASTERIXFile", "ASTERIXFile" + instancename);

    current_filename_ = filename;

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void ASTERIXImportTask::removeCurrentFilename()
{
    loginf << "ASTERIXImportTask: removeCurrentFilename: filename '" << current_filename_ << "'";

    assert(current_filename_.size());
    assert(hasFile(current_filename_));

    if (file_list_.count(current_filename_) != 1)
        throw std::invalid_argument("ASTERIXImportTask: removeCurrentFilename: name '" +
                                    current_filename_ + "' not in use");

    delete file_list_.at(current_filename_);
    file_list_.erase(current_filename_);
    current_filename_ = "";

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void ASTERIXImportTask::removeAllFiles ()
{
    loginf << "ASTERIXImportTask: removeAllFiles";

    while (file_list_.size())
    {
        delete file_list_.begin()->second;
        file_list_.erase(file_list_.begin());
    }

    current_filename_ = "";

    emit statusChangedSignal(name_);

    if (widget_)
        widget_->updateFileListSlot();
}

void ASTERIXImportTask::currentFilename(const std::string& filename)
{
    loginf << "ASTERIXImportTask: currentFilename: filename '" << filename << "'";

    bool had_filename = canImportFile();

    current_filename_ = filename;

    if (!had_filename)  // not on re-select
        emit statusChangedSignal(name_);
}

const std::string& ASTERIXImportTask::currentFraming() const { return current_framing_; }

void ASTERIXImportTask::currentFraming(const std::string& current_framing)
{
    current_framing_ = current_framing;
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

bool ASTERIXImportTask::limitRAM() const { return limit_ram_; }

void ASTERIXImportTask::limitRAM(bool limit_ram)
{
    limit_ram_ = limit_ram;

    if (limit_ram_)
    {
        jASTERIX::frame_chunk_size = limited_chunk_size;
        jASTERIX::data_block_chunk_size = limited_chunk_size;
    }
    else
    {
        jASTERIX::frame_chunk_size = unlimited_chunk_size;
        jASTERIX::data_block_chunk_size = unlimited_chunk_size;
    }

    if (widget_)
        widget_->updateLimitRAM();
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

    if (COMPASS::instance().objectManager().hasData())
        return false;

    return true;
}

bool ASTERIXImportTask::isRequired() { return false; }

bool ASTERIXImportTask::overrideActive() const { return post_process_.override_active_; }

void ASTERIXImportTask::overrideActive(bool value)
{
    loginf << "ASTERIXImportTask: overrideActive: value " << value;

    post_process_.override_active_ = value;
}

unsigned int ASTERIXImportTask::overrideSacOrg() const { return post_process_.override_sac_org_; }

void ASTERIXImportTask::overrideSacOrg(unsigned int value)
{
    loginf << "ASTERIXImportTask: overrideSacOrg: value " << value;

    post_process_.override_sac_org_ = value;
}

unsigned int ASTERIXImportTask::overrideSicOrg() const { return post_process_.override_sic_org_; }

void ASTERIXImportTask::overrideSicOrg(unsigned int value)
{
    loginf << "ASTERIXImportTask: overrideSicOrg: value " << value;

    post_process_.override_sic_org_ = value;
}

unsigned int ASTERIXImportTask::overrideSacNew() const { return post_process_.override_sac_new_; }

void ASTERIXImportTask::overrideSacNew(unsigned int value)
{
    loginf << "ASTERIXImportTask: overrideSacNew: value " << value;

    post_process_.override_sac_new_ = value;
}

unsigned int ASTERIXImportTask::overrideSicNew() const { return post_process_.override_sic_new_; }

void ASTERIXImportTask::overrideSicNew(unsigned int value)
{
    loginf << "ASTERIXImportTask: overrideSicNew: value " << value;

    post_process_.override_sic_new_ = value;
}

float ASTERIXImportTask::overrideTodOffset() const { return post_process_.override_tod_offset_; }

void ASTERIXImportTask::overrideTodOffset(float value)
{
    loginf << "ASTERIXImportTask: overrideTodOffset: value " << value;

    post_process_.override_tod_offset_ = value;
}

//std::vector<std::string> ASTERIXImportTask::getPossibleMappings (unsigned int cat)
//{
//    std::vector<std::string> list;

//    list.push_back(""); // no mapping

//    assert (schema_);

//    for (auto& par_it : schema_->parsers())
//    {
//        if (par_it.second->category() == cat) // if same, add
//           list.push_back(String::categoryString(par_it.first));
//    }

//    return list;
//}

//std::string ASTERIXImportTask::getActiveMapping (unsigned int cat)
//{
//    assert (schema_);

//    for (auto& par_it : schema_->parsers())
//    {
//        if (par_it.second->category() == cat) // if same, add
//           return String::categoryString(par_it.first);
//    }


//    // none found
//    return "";
//}

//void ASTERIXImportTask::setActiveMapping (unsigned int cat, const std::string& mapping_name)
//{
//    loginf << "ASTERIXImportTask: setActiveMapping: cat " << cat << " mapping '" << mapping_name << "'";

//    assert (schema_);

//    std::string tmp_str;
//    unsigned int tmp_cat;

//    for (auto& par_it : schema_->parsers())
//    {
//        if (par_it.second->category() == cat) // if same, change
//            par_it.second->active(par_it.first == mapping_name);
//    }
//}

void ASTERIXImportTask::deleteWidget() { widget_.reset(nullptr); }

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

bool ASTERIXImportTask::canRun() { return canImportFile(); }

void ASTERIXImportTask::run()
{
    run (false);
}

void ASTERIXImportTask::run(bool test) // , bool create_mapping_stubs
{
    test_ = test;
    //create_mapping_stubs_ = create_mapping_stubs;

    done_ = false; // since can be run multiple times
    num_radar_inserted_ = 0;

    float free_ram = System::getFreeRAMinGB();

    loginf << "ASTERIXImportTask: run: filename " << current_filename_ << " test " << test_
              //<< " create stubs " << create_mapping_stubs_
           << " free RAM " << free_ram << " GB";

    if (free_ram < ram_threshold && !limit_ram_)
    {
        loginf << "ASTERIXImportTask: run: only " << free_ram
               << " GB free ram, recommending limiting";

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
                    nullptr, "RAM Limiting",
                    "There is only " + QString::number(free_ram) + " GB free RAM available.\n" +
                    "This will result in decreased decoding performance.\n\n" +
                    "Do you agree to limiting RAM usage?",
                    QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            limitRAM(true);
        }
    }
    else if (free_ram >= ram_threshold && limit_ram_)
    {
        loginf << "ASTERIXImportTask: run: " << free_ram
               << " GB free ram, recommending not limiting";

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(
                    nullptr, "RAM Limiting",
                    "There is " + QString::number(free_ram) + " GB free RAM available.\n" +
                    "This will result in increased decoding performance.\n\n" +
                    "Do you agree to increased RAM usage?",
                    QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes)
        {
            limitRAM(false);
        }
    }

    if (test_)
        task_manager_.appendInfo("ASTERIXImportTask: test import of file '" + current_filename_ +
                                 "' started");
    //    else if (create_mapping_stubs_)
    //        task_manager_.appendInfo("ASTERIXImportTask: create mappings stubs using file '" +
    //                                 current_filename_ + "' started");
    else
        task_manager_.appendInfo("ASTERIXImportTask: import of file '" + current_filename_ +
                                 "' started");

    if (widget_)
        widget_->runStarted();

    assert(canImportFile());

    if (status_widget_)
    {
        logwrn << "ASTERIXImportTask: run: status widget still active";
        status_widget_ = nullptr;
    }
    assert(!status_widget_);

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    status_widget_ = nullptr;

    status_widget_.reset(new ASTERIXStatusDialog(current_filename_, test_));
    connect(status_widget_.get(), &ASTERIXStatusDialog::closeSignal, this,
            &ASTERIXImportTask::closeStatusDialogSlot);
    status_widget_->markStartTime();

    insert_active_ = 0;

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
        //        loginf << "ASTERIXImportTask: importFile: setting cat " <<  cat_it.first
        //               << " edition " << cat_it.second.edition();
        jasterix_->category(cat_it.first)->setCurrentEdition(cat_it.second.edition());
        jasterix_->category(cat_it.first)->setCurrentREFEdition(cat_it.second.ref());
        jasterix_->category(cat_it.first)->setCurrentSPFEdition(cat_it.second.spf());

        // TODO mapping?
    }

    loginf << "ASTERIXImportTask: run: starting decode job";

    assert(decode_job_ == nullptr);

    decode_job_ = make_shared<ASTERIXDecodeJob>(*this, current_filename_, current_framing_, test_,
                                                post_process_);

    connect(decode_job_.get(), &ASTERIXDecodeJob::obsoleteSignal, this,
            &ASTERIXImportTask::decodeASTERIXObsoleteSlot, Qt::QueuedConnection);
    connect(decode_job_.get(), &ASTERIXDecodeJob::doneSignal, this,
            &ASTERIXImportTask::decodeASTERIXDoneSlot, Qt::QueuedConnection);
    connect(decode_job_.get(), &ASTERIXDecodeJob::decodedASTERIXSignal, this,
            &ASTERIXImportTask::addDecodedASTERIXSlot, Qt::QueuedConnection);

    JobManager::instance().addBlockingJob(decode_job_);

    return;
}

void ASTERIXImportTask::decodeASTERIXDoneSlot()
{
    loginf << "ASTERIXImportTask: decodeASTERIXDoneSlot";

    assert(decode_job_);

    if (decode_job_->error())
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
    assert(status_widget_);

    if (status_widget_->numErrors())
        task_manager_.appendError(
                    "ASTERIXImportTask: " + std::to_string(status_widget_->numErrors()) +
                    " decoding errors occured");

    task_manager_.appendInfo("ASTERIXImportTask: decoding done with " +
                             std::to_string(status_widget_->numFrames()) + " frames, " +
                             std::to_string(status_widget_->numRecords()) + " records");

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
    logdbg << "ASTERIXImportTask: addDecodedASTERIX";

    assert(decode_job_);
    assert(status_widget_);

    logdbg << "ASTERIXImportTask: addDecodedASTERIX: errors " << decode_job_->numErrors();

    status_widget_->numFrames(jasterix_->numFrames());
    status_widget_->numRecords(jasterix_->numRecords());
    status_widget_->numErrors(jasterix_->numErrors());
    status_widget_->setCategoryCounts(decode_job_->categoryCounts());

    status_widget_->show();

    std::unique_ptr<nlohmann::json> extracted_data = std::move(decode_job_->extractedData());

    //        map_jobs_mutex_.lock();
    //        json_map_jobs_.push_back(json_map_job);
    //        map_jobs_mutex_.unlock();

    while (json_map_job_)  // only one can exist at a time
    {
        if (decode_job_)
            decode_job_->pause();

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(1);
    }

    assert(!json_map_job_);

    assert(schema_);

    std::vector<std::string> keys;

    if (current_framing_ == "")
        keys = {"data_blocks", "content", "records"};
    else
        keys = {"frames", "content", "data_blocks", "content", "records"};

    json_map_job_ =
            make_shared<ASTERIXJSONMappingJob>(std::move(extracted_data), keys, schema_->parsers());

    assert(!extracted_data);

    connect(json_map_job_.get(), &ASTERIXJSONMappingJob::obsoleteSignal, this,
            &ASTERIXImportTask::mapJSONObsoleteSlot, Qt::QueuedConnection);
    connect(json_map_job_.get(), &ASTERIXJSONMappingJob::doneSignal, this,
            &ASTERIXImportTask::mapJSONDoneSlot, Qt::QueuedConnection);

    JobManager::instance().addNonBlockingJob(json_map_job_);

    if (decode_job_)
    {
        if (maxLoadReached())
            decode_job_->pause();
        else
            decode_job_->unpause();
    }
}

void ASTERIXImportTask::mapJSONDoneSlot()
{
    logdbg << "ASTERIXImportTask: mapJSONDoneSlot";

    assert(status_widget_);

    //    JSONMappingJob* map_job = static_cast<JSONMappingJob*>(sender());
    //    std::shared_ptr<JSONMappingJob> queued_map_job;

    //    map_jobs_mutex_.lock();
    //    waiting_for_map_ = true;

    assert(json_map_job_);

    //    queued_map_job = json_map_jobs_.front();
    //    json_map_jobs_.pop_front();

    //    map_jobs_mutex_.unlock();
    //    waiting_for_map_ = false;

    //    assert (queued_map_job.get() == map_job);

    status_widget_->addNumMapped(json_map_job_->numMapped());
    status_widget_->addNumNotMapped(json_map_job_->numNotMapped());
    status_widget_->addMappedCounts(json_map_job_->categoryMappedCounts());
    status_widget_->addNumCreated(json_map_job_->numCreated());

    std::map<std::string, std::shared_ptr<Buffer>> job_buffers =
            std::move(json_map_job_->buffers());
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

    insertData(std::move(job_buffers));
}

void ASTERIXImportTask::mapJSONObsoleteSlot()
{
    logdbg << "ASTERIXImportTask: mapJSONObsoleteSlot";
    // TODO
}

//void ASTERIXImportTask::mapStubsDoneSlot()
//{
//    logdbg << "ASTERIXImportTask: mapStubsDoneSlot";

//    JSONMappingStubsJob* map_stubs_job = static_cast<JSONMappingStubsJob*>(sender());
//    assert(json_map_stub_job_.get() == map_stubs_job);

//    json_map_stub_job_ = nullptr;

//    schema_->updateMappings();

//    checkAllDone();
//}
//void ASTERIXImportTask::mapStubsObsoleteSlot()
//{
//    logdbg << "ASTERIXImportTask: mapStubsObsoleteSlot";
//    json_map_stub_job_ = nullptr;
//}

void ASTERIXImportTask::insertData(std::map<std::string, std::shared_ptr<Buffer>> job_buffers)
{
    logdbg << "ASTERIXImportTask: insertData: inserting into database";

    assert (!test_);

    assert(status_widget_);

    if (!dbo_variable_sets_.size())  // initialize if empty
    {
        for (auto& parser_it : *schema_)
        {
            std::string dbo_name = parser_it.second->dbObject().name();

            //            DBObject& db_object = parser_it.second->dbObject();

            //            std::string data_source_var_name = parser_it.second.dataSourceVariableName();
            //            assert(data_source_var_name.size());

            DBOVariableSet set = parser_it.second->variableList();

            assert (dbo_variable_sets_.count(dbo_name));  // add variables
            std::get<1>(dbo_variable_sets_.at(dbo_name)).add(set);

            //            {
            //                //assert(std::get<0>(dbo_variable_sets_.at(dbo_name)) == data_source_var_name);

            //            }
            //            else  // create it
            //                dbo_variable_sets_[dbo_name] = std::make_tuple(data_source_var_name, set);
        }
    }

    while (insert_active_)
    {
        waiting_for_insert_ = true;
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        QThread::msleep(1);
    }

    waiting_for_insert_ = false;

    //bool has_sac_sic = false;

    assert(schema_);

    DBObjectManager& object_manager = COMPASS::instance().objectManager();

    for (auto& buf_it : job_buffers)
    {
        std::string dbo_name = buf_it.first;
        assert(dbo_variable_sets_.count(dbo_name));
        std::shared_ptr<Buffer> buffer = buf_it.second;

        if (!buffer->size())
        {
            loginf << "ASTERIXImportTask: insertData: dbo " << buf_it.first
                   << " with empty buffer";
            continue;
        }

        assert(object_manager.existsObject(dbo_name));
        DBObject& db_object = object_manager.object(dbo_name);

        ++insert_active_;

        connect(&db_object, &DBObject::insertDoneSignal, this, &ASTERIXImportTask::insertDoneSlot,
                Qt::UniqueConnection);
        connect(&db_object, &DBObject::insertProgressSignal, this,
                &ASTERIXImportTask::insertProgressSlot, Qt::UniqueConnection);

        DBOVariableSet& set = std::get<1>(dbo_variable_sets_.at(dbo_name));
        db_object.insertData(set, buffer, false);

        status_widget_->addNumInserted(db_object.name(), buffer->size());

        if (db_object.name() == "Radar")
            num_radar_inserted_ += buffer->size(); // store for later check
    }

    checkAllDone();

    logdbg << "JSONImporterTask: insertData: done";
}

void ASTERIXImportTask::insertProgressSlot(float percent)
{
    logdbg << "ASTERIXImportTask: insertProgressSlot: " << String::percentToString(percent)
           << "%";
}

void ASTERIXImportTask::insertDoneSlot(DBObject& object)
{
    logdbg << "ASTERIXImportTask: insertDoneSlot";
    --insert_active_;

    bool test = test_; // test_ cleared by checkAllDone

    checkAllDone();

    logdbg << "ASTERIXImportTask: insertDoneSlot: check done";

    if (all_done_ && !test)
    {
        logdbg << "ASTERIXImportTask: insertDoneSlot: finalizing";

        // in case data was imported, clear other task done properties
        if (num_radar_inserted_)
        {
            bool has_null_positions = COMPASS::instance().interface().areColumnsNull(
                        COMPASS::instance().objectManager().object("Radar").dbTableName(),
                        {"pos_lat_deg","pos_long_deg"});

            loginf << "ASTERIXImportTask: insertDoneSlot: radar has null positions " << has_null_positions;

            COMPASS::instance().interface().setProperty(
                        RadarPlotPositionCalculatorTask::DONE_PROPERTY_NAME, to_string(!has_null_positions));
        }

        //COMPASS::instance().interface().setProperty(PostProcessTask::DONE_PROPERTY_NAME, "0");
        COMPASS::instance().interface().setProperty(
                    CreateARTASAssociationsTask::DONE_PROPERTY_NAME, "0");

        COMPASS::instance().interface().setProperty(DONE_PROPERTY_NAME, "1");

        emit doneSignal(name_);
    }

    logdbg << "ASTERIXImportTask: insertDoneSlot: done";
}

void ASTERIXImportTask::checkAllDone()
{
    logdbg << "ASTERIXImportTask: checkAllDone: all done " << all_done_ << " decode "
           << (decode_job_ == nullptr)
              //<< " wait map " << !waiting_for_map_
           << " map job " << (json_map_job_ == nullptr)
           << " wait insert " << !waiting_for_insert_
           << " insert active " << (insert_active_ == 0);

    if (!all_done_ && decode_job_ == nullptr && json_map_job_ == nullptr &&
            !waiting_for_insert_ && insert_active_ == 0)
    {
        loginf << "ASTERIXImportTask: checkAllDone: setting all done";

        assert(status_widget_);
        status_widget_->setDone();

        all_done_ = true;

        QApplication::restoreOverrideCursor();

        logdbg << "ASTERIXImportTask: checkAllDone: refresh";

        refreshjASTERIX();

        logdbg << "ASTERIXImportTask: checkAllDone: widget done";

        assert(widget_);
        widget_->runDone();

        logdbg << "ASTERIXImportTask: checkAllDone: dbo content";

        if (!test_)
            emit COMPASS::instance().interface().databaseContentChangedSignal();

        task_manager_.appendInfo("ASTERIXImportTask: inserted " +
                                 std::to_string(status_widget_->numRecordsInserted()) +
                                 " records, rate " + status_widget_->recordsInsertedRateStr());

        for (auto& db_cnt_it : status_widget_->dboInsertedCounts())
            task_manager_.appendInfo("ASTERIXImportTask: inserted " +
                                     std::to_string(db_cnt_it.second) + " " + db_cnt_it.first +
                                     " records");

        logdbg << "ASTERIXImportTask: checkAllDone: status logging";

        if (test_)
            task_manager_.appendSuccess("ASTERIXImportTask: import test done after " +
                                        status_widget_->elapsedTimeStr());
        else
        {
            task_manager_.appendSuccess("ASTERIXImportTask: import done after " +
                                        status_widget_->elapsedTimeStr());
        }

        //test_ = false;  // set again by widget

        if (!show_done_summary_)
        {
            logdbg << "ASTERIXImportTask: checkAllDone: deleting status widget";

            status_widget_->close();
            status_widget_ = nullptr;
        }
    }

    logdbg << "ASTERIXImportTask: checkAllDone: done";
}

void ASTERIXImportTask::closeStatusDialogSlot()
{
    loginf << "ASTERIXImportTask: closeStatusDialogSlot";

    assert(status_widget_);
    status_widget_->close();
    status_widget_ = nullptr;

    loginf << "ASTERIXImportTask: closeStatusDialogSlot: done";
}

bool ASTERIXImportTask::maxLoadReached()
{
    return insert_active_ >= 2;

    //    if (limit_ram_)
    //        return json_map_jobs_.size() > limited_num_json_jobs_;
    //    else
    //        return json_map_jobs_.size() > unlimited_num_json_jobs_;
}
