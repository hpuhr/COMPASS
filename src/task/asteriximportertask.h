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

#ifndef ASTERIXIMPORTERTASK_H
#define ASTERIXIMPORTERTASK_H

#include "configurable.h"
#include "json.hpp"
//#include "jsonparsingschema.h"
//#include "readjsonfilepartjob.h"

#include <QObject>

#include <memory>

#include "boost/date_time/posix_time/posix_time.hpp"

class TaskManager;
class ASTERIXImporterTaskWidget;

class ASTERIXImporterTask: public QObject, public Configurable
{
    Q_OBJECT
public:
    ASTERIXImporterTask(const std::string& class_id, const std::string& instance_id,
                        TaskManager* task_manager);
    virtual ~ASTERIXImporterTask();

    ASTERIXImporterTaskWidget* widget();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

protected:
    std::unique_ptr<ASTERIXImporterTaskWidget> widget_;

    virtual void checkSubConfigurables () {}
};

#endif // ASTERIXIMPORTERTASK_H
