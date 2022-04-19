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

#include "configurable.h"
#include "propertylist.h"
#include "singleton.h"
#include "json.hpp"
#include "appmode.h"

#include <QObject>

#include <memory>
#include <map>
#include <set>
#include <vector>


class DBInterface;
class DBContentManager;
class DataSourceManager;
class FilterManager;
class TaskManager;
class ViewManager;
class SimpleConfig;
class EvaluationManager;
class MainWindow;

class COMPASS : public QObject, public Configurable, public Singleton
{
    Q_OBJECT

signals:
    void appModeSwitchSignal (AppMode app_mode);

public:
    virtual ~COMPASS();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    void openDBFile(const std::string& filename);
    void createNewDBFile(const std::string& filename);
    bool dbOpened();
    void closeDB();

    DBInterface& interface();
    DBContentManager& dbContentManager();
    DataSourceManager& dataSourceManager();
    FilterManager& filterManager();
    TaskManager& taskManager();
    ViewManager& viewManager();
    SimpleConfig& config();
    EvaluationManager& evaluationManager();

    void shutdown();

    MainWindow& mainWindow();

protected:
    bool db_opened_{false};
    bool shut_down_{false};

    AppMode app_mode_ {AppMode::Offline};

    std::unique_ptr<SimpleConfig> simple_config_;
    std::unique_ptr<DBInterface> db_interface_;
    std::unique_ptr<DBContentManager> dbcontent_manager_;
    std::unique_ptr<DataSourceManager> ds_manager_;
    std::unique_ptr<FilterManager> filter_manager_;
    std::unique_ptr<TaskManager> task_manager_;
    std::unique_ptr<ViewManager> view_manager_;
    std::unique_ptr<EvaluationManager> eval_manager_;

    std::string last_db_filename_;
    nlohmann::json db_file_list_;

    virtual void checkSubConfigurables();

    MainWindow* main_window_;

    COMPASS();

public:
    static COMPASS& instance()
    {
        static COMPASS instance;
        return instance;
    }
    std::string lastDbFilename() const;
    std::vector<std::string> dbFileList() const;
    void clearDBFileList();
    void addDBFileToList(const std::string filename);

    AppMode appMode() const;
    void appMode(const AppMode& app_mode);
    std::string appModeStr() const;

    static const std::map<AppMode, std::string>& appModes2Strings();
};

#endif /* COMPASS_H_ */
