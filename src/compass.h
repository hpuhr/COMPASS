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
class FFTManager;
class LicenseManager;

namespace rtcommand
{
    class RTCommandRunner;
}

class COMPASS : public QObject, public Configurable, public Singleton
{
    Q_OBJECT

signals:
    void databaseOpenedSignal();
    void databaseClosedSignal();
    void appModeSwitchSignal (AppMode app_mode_previous, AppMode app_mode_current);

public:
    virtual ~COMPASS();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;
    std::string getPath() const override final;

    void openDBFile(const std::string& filename);
    void createNewDBFile(const std::string& filename);
    void exportDBFile(const std::string& filename);
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
    rtcommand::RTCommandRunner& rtCmdRunner();
    FFTManager& fftManager();
    LicenseManager& licenseManager();

    void init();
    void shutdown();

    MainWindow& mainWindow();

    std::string lastUsedPath();
    void lastUsedPath(const std::string& last_path);

    std::string versionString(bool open_ats = true, 
                              bool license_type = true) const;
    std::string licenseeString(bool licensed_to = true) const;

protected:
    bool db_opened_{false};
    bool expert_mode_ {false};

    AppMode app_mode_ {AppMode::Offline};
    AppState app_state_ = AppState::Starting;

    bool hide_evaluation_ {false};
    bool hide_viewpoints_ {false};
    bool disable_live_to_offline_switch_ {false};
    bool disable_menu_config_save_ {false};
    bool disable_geographicview_rotate_ {false};
    bool disable_add_remove_views_ {false};
    bool disable_confirm_reset_views_ {false};

    static const bool is_app_image_;

    unsigned int auto_live_running_resume_ask_time_ {60}; // minutes
    unsigned int auto_live_running_resume_ask_wait_time_ {1}; // minutes

    unsigned int max_fps_ {30};

    std::unique_ptr<SimpleConfig> simple_config_;
    std::unique_ptr<DBInterface> db_interface_;
    std::unique_ptr<DBContentManager> dbcontent_manager_;
    std::unique_ptr<DataSourceManager> ds_manager_;
    std::unique_ptr<FilterManager> filter_manager_;
    std::unique_ptr<TaskManager> task_manager_;
    std::unique_ptr<ViewManager> view_manager_;
    std::unique_ptr<EvaluationManager> eval_manager_;
    std::unique_ptr<FFTManager> fft_manager_;
    std::unique_ptr<LicenseManager> license_manager_;

    std::unique_ptr<rtcommand::RTCommandRunner> rt_cmd_runner_;

    std::string last_db_filename_;
    nlohmann::json db_file_list_;

    std::string last_path_;

    bool db_export_in_progress_ {false};

    virtual void checkSubConfigurables() override;

    MainWindow* main_window_;

    COMPASS();

private:
    friend class Client;

    void setAppState(AppState state);

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

    AppState appState() const { return app_state_; }

    static bool isAppImage() { return is_app_image_; }

    static const std::map<AppMode, std::string>& appModes2Strings();

    bool expertMode() const;
    void expertMode(bool expert_mode);

    bool isShutDown() const;
    bool isRunning() const;

    bool hideEvaluation() const;
    bool hideViewpoints() const;

    unsigned int maxFPS() const;
    void maxFPS(unsigned int max_fps);

    bool disableLiveToOfflineSwitch() const;
    bool disableMenuConfigSave() const;
    bool disableGeographicViewRotate() const;
    bool disableAddRemoveViews() const;
    bool dbExportInProgress() const;

    unsigned int autoLiveRunningResumeAskTime() const; // min
    unsigned int autoLiveRunningResumeAskWaitTime() const; // min
    bool disableConfirmResetViews() const;
};

#endif /* COMPASS_H_ */
