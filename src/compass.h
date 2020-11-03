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

#ifndef COMPASS_H_
#define COMPASS_H_

#include <map>
#include <set>
#include <vector>

#include "configurable.h"
#include "propertylist.h"
#include "singleton.h"

class DBInterface;
class DBObjectManager;
class DBSchemaManager;
class FilterManager;
class TaskManager;
class ViewManager;
class SimpleConfig;
class EvaluationManager;

class COMPASS : public Configurable, public Singleton
{
  public:
    ///@brief Destructor.
    virtual ~COMPASS();

    // void initialize ();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    DBInterface& interface();
    DBSchemaManager& schemaManager();
    DBObjectManager& objectManager();
    FilterManager& filterManager();
    TaskManager& taskManager();
    ViewManager& viewManager();
    SimpleConfig& config();
    EvaluationManager& evaluationManager();

    bool ready();

    ///@brief Shuts down the DB access.
    void shutdown();

  protected:
    // bool initialized_ {false};
    bool shut_down_{false};

    std::unique_ptr<SimpleConfig> simple_config_;
    /// DB interface, encapsulating all database functionality.
    std::unique_ptr<DBInterface> db_interface_;
    std::unique_ptr<DBObjectManager> dbo_manager_;
    std::unique_ptr<DBSchemaManager> db_schema_manager_;
    std::unique_ptr<FilterManager> filter_manager_;
    std::unique_ptr<TaskManager> task_manager_;
    std::unique_ptr<ViewManager> view_manager_;
    std::unique_ptr<EvaluationManager> eval_manager_;

    virtual void checkSubConfigurables();

    ///@brief Constructor.
    COMPASS();

  public:
    ///@brief Instance access function for Singleton.
    static COMPASS& instance()
    {
        static COMPASS instance;
        return instance;
    }
};

#endif /* COMPASS_H_ */
