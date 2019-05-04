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

using namespace Utils;
using namespace nlohmann;

ASTERIXImporterTask::ASTERIXImporterTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager* task_manager)
    : Configurable (class_id, instance_id, task_manager)
{
//    registerParameter("current_filename", &current_filename_, "");
//    registerParameter("current_schema", &current_schema_, "");

    createSubConfigurables();
}


ASTERIXImporterTask::~ASTERIXImporterTask()
{
//    if (msg_box_)
//    {
//        delete msg_box_;
//        msg_box_ = nullptr;
//    }

//    for (auto it : file_list_)
//        delete it.second;

//    file_list_.clear();
}

void ASTERIXImporterTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
//    if (class_id == "JSONFile")
//    {
//        SavedFile *file = new SavedFile (class_id, instance_id, this);
//        assert (file_list_.count (file->name()) == 0);
//        file_list_.insert (std::pair <std::string, SavedFile*> (file->name(), file));
//    }
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
//    else
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
