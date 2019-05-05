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
#include "taskmanager.h"
#include "configurable.h"
#include "files.h"
#include "stringconv.h"
#include "savedfile.h"
#include "logger.h"

#include <jasterix/jasterix.h>

using namespace Utils;
using namespace nlohmann;
//using namespace jASTERIX;

ASTERIXImporterTask::ASTERIXImporterTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager* task_manager)
    : Configurable (class_id, instance_id, task_manager)
{
    registerParameter("debug_jasterix", &debug_jasterix_, false);
    registerParameter("current_filename", &current_filename_, "");
//    registerParameter("current_schema", &current_schema_, "");

    createSubConfigurables();

    std::string jasterix_definition_path = HOME_DATA_DIRECTORY+"/jasterix_definitions";

    loginf << "ASTERIXImporterTask: contructor: jasterix definition path '" << jasterix_definition_path << "'";
    assert (Files::directoryExists(jasterix_definition_path));

    jasterix_.reset(new jASTERIX::jASTERIX(jasterix_definition_path, false, debug_jasterix_));
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
//    else if (class_id == "JSONParsingSchema")
//    {
//        std::string name = configuration().getSubConfiguration(
//                    class_id, instance_id).getParameterConfigValueString("name");

//        assert (schemas_.find (name) == schemas_.end());

//        logdbg << "JSONImporterTask: generateSubConfigurable: generating schema " << instance_id
//               << " with name " << name;

//        schemas_.emplace(std::piecewise_construct,
//                     std::forward_as_tuple(name),  // args for key
//                     std::forward_as_tuple(class_id, instance_id, *this));  // args for mapped value
//    }
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

void ASTERIXImporterTask::addFile (const std::string &filename)
{
    if (file_list_.count (filename) != 0)
        throw std::invalid_argument ("ASTERIXImporterTask: addFile: name '"+filename+"' already in use");

    std::string instancename = filename;
    instancename.erase (std::remove(instancename.begin(), instancename.end(), '/'), instancename.end());

    Configuration &config = addNewSubConfiguration ("ASTERIXFile", "ASTERIXFile"+instancename);
    config.addParameterString("name", filename);
    generateSubConfigurable ("ASTERIXFile", "ASTERIXFile"+instancename);

//    if (widget_)
//        widget_->updateFileListSlot();
}

void ASTERIXImporterTask::removeCurrentFilename ()
{
    assert (current_filename_.size());
    assert (hasFile(current_filename_));

    if (file_list_.count (current_filename_) != 1)
        throw std::invalid_argument ("ASTERIXImporterTask: addFile: name '"+current_filename_+"' not in use");

    delete file_list_.at(current_filename_);
    file_list_.erase(current_filename_);

//    if (widget_)
//        widget_->updateFileListSlot();
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
