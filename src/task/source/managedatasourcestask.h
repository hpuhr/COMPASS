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


#ifndef MANAGEDATASOURCESTASK_H
#define MANAGEDATASOURCESTASK_H


#include "configurable.h"
#include "task.h"

#include <QObject>

#include <memory>

class TaskManager;
class ManageDataSourcesTaskWidget;

class ManageDataSourcesTask: public Task, public Configurable
{
public:
    ManageDataSourcesTask(const std::string& class_id, const std::string& instance_id,
                     TaskManager& task_manager);

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    virtual TaskWidget* widget ();
    virtual void deleteWidget ();

    virtual bool checkPrerequisites ();
    virtual bool isRecommended ();
    virtual bool isRequired () { return false; }

protected:
    std::unique_ptr<ManageDataSourcesTaskWidget> widget_;

    virtual void checkSubConfigurables () {}
};

#endif // MANAGEDATASOURCESTASK_H
