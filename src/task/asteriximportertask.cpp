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

#include <jasterix/jasterix.h>
#include <jasterix/category.h>

#include <algorithm>

using namespace Utils;
using namespace nlohmann;
//using namespace jASTERIX;

ASTERIXImporterTask::ASTERIXImporterTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager* task_manager)
    : Configurable (class_id, instance_id, task_manager)
{
    registerParameter("debug_jasterix", &debug_jasterix_, false);
    registerParameter("current_filename", &current_filename_, "");
    registerParameter("current_framing", &current_framing_, "");

    createSubConfigurables();

    std::string jasterix_definition_path = HOME_DATA_DIRECTORY+"/jasterix_definitions";

    loginf << "ASTERIXImporterTask: contructor: jasterix definition path '" << jasterix_definition_path << "'";
    assert (Files::directoryExists(jasterix_definition_path));

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
    else
        throw std::runtime_error ("JSONImporterTask: generateSubConfigurable: unknown class_id "+class_id );
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
//    archive_ = false;
//    test_ = test;
//    all_done_ = false;

//    objects_read_ = 0;
//    objects_parsed_ = 0;
//    objects_parse_errors_ = 0;

//    objects_mapped_ = 0;
//    objects_not_mapped_ = 0;

//    objects_created_ = 0;
//    objects_inserted_ = 0;

//    assert (schemas_.count(current_schema_));

//    for (auto& map_it : schemas_.at(current_schema_))
//        if (!map_it.second.initialized())
//            map_it.second.initialize();

//    start_time_ = boost::posix_time::microsec_clock::local_time();

//    read_json_job_ = std::shared_ptr<ReadJSONFilePartJob> (new ReadJSONFilePartJob (filename, false, 10000));
//    connect (read_json_job_.get(), SIGNAL(obsoleteSignal()), this, SLOT(readJSONFilePartObsoleteSlot()),
//             Qt::QueuedConnection);
//    connect (read_json_job_.get(), SIGNAL(doneSignal()), this, SLOT(readJSONFilePartDoneSlot()), Qt::QueuedConnection);

//    JobManager::instance().addNonBlockingJob(read_json_job_);

//    updateMsgBox();

//    logdbg << "JSONImporterTask: importFile: filename " << filename << " test " << test << " done";

    return;
}
