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

#ifndef ASTERIXIMPORTTASK_H
#define ASTERIXIMPORTTASK_H

#include "asterixdecodejob.h"
#include "asterixpostprocess.h"
#include "configurable.h"
#include "json.hpp"
#include "asterixjsonmappingjob.h"
#include "asterixpostprocessjob.h"
#include "asterixjsonparsingschema.h"
#include "task.h"
#include "appmode.h"

#include <QObject>
#include <QMessageBox>

#include <deque>
#include <memory>
#include <mutex>

class TaskManager;

class ASTERIXCategoryConfig;
class ASTERIXStatusDialog;
class ASTERIXImportTaskDialog;

class QProgressDialog;

namespace jASTERIX
{
class jASTERIX;
}

struct ASTERIXFileInfo
{
    std::string filename_;
    //unsigned int line_id_ {0}; // TODO rework

    bool decoding_tried_ {false};
    std::string decoding_info_str_;

    nlohmann::json analysis_info_;
    bool errors_found_{false};
};


class ASTERIXImportTaskSettings
{
public:
    // registered
    bool debug_jasterix_;
    nlohmann::json file_list_;
    std::string current_file_framing_;

    unsigned int num_packets_overload_ {60};

    float override_tod_offset_{0};
    float filter_tod_min_{0};
    float filter_tod_max_{0};

    double filter_latitude_min_{0};
    double filter_latitude_max_{0};
    double filter_longitude_min_{0};
    double filter_longitude_max_{0};

    float filter_modec_min_{0};
    float filter_modec_max_{0};

    // unregistered, for passing on

    unsigned int file_line_id_ {0};
    boost::posix_time::ptime date_;

    unsigned int max_network_lines_ {4};

    bool test_{false};

    bool override_tod_active_{false};

    bool ignore_time_jumps_{false};
    bool network_ignore_future_ts_ {false};

    bool filter_tod_active_{false};
    bool filter_position_active_{false};
    bool filter_modec_active_{false};

    bool importFile() const { return import_files_; }

    std::vector<ASTERIXFileInfo>& filesInfo() { return files_info_; }
    const std::vector<ASTERIXFileInfo>& filesInfo() const { return files_info_; }

    void clearImportFilenames () { files_info_.clear(); }
    void addImportFilename(const std::string& filename, unsigned int line_id)
    {
        files_info_.push_back(ASTERIXFileInfo());
        files_info_.back().filename_ = filename;
        //files_info_.back().line_id_ = line_id;
    }

private:
    friend class ASTERIXImportTask;

    bool import_files_ {false}; // false = network, true file

    //std::string current_filename_;
    std::vector<ASTERIXFileInfo> files_info_;

};

class ASTERIXImportTask : public Task, public Configurable
{
    Q_OBJECT

  public slots:
    void dialogImportSlot();
    void dialogTestImportSlot();
    void dialogCancelSlot();

    void decodeASTERIXDoneSlot();
    void decodeASTERIXObsoleteSlot();
    void addDecodedASTERIXSlot();

    void mapJSONDoneSlot();
    void mapJSONObsoleteSlot();

    void postprocessDoneSlot();
    void postprocessObsoleteSlot();

    void insertDoneSlot();

    void appModeSwitchSlot (AppMode app_mode_previous, AppMode app_mode_current);

  public:
    ASTERIXImportTask(const std::string& class_id, const std::string& instance_id,
                      TaskManager& task_manager);
    virtual ~ASTERIXImportTask();

    ASTERIXImportTaskDialog* dialog();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    void asterixFileFraming(const std::string& asterix_framing);
    void asterixDecoderConfig(const std::string& asterix_decoder_cfg);

    bool canImportFiles();

    virtual bool canRun() override;
    virtual void run() override;

    virtual void stop() override;
    void run(bool test);
    bool isRunning() const;

    std::vector<std::string> fileList();
    void addFile(const std::string& filename);
    void clearFileList ();

    void addImportFileNames(const std::vector<std::string>& filenames, unsigned int line_id=0);
    std::vector<ASTERIXFileInfo>& filesInfo() { return settings_.filesInfo(); }
    std::string importFilenamesStr() const;
    void clearImportFilesInfo ();

    void importNetwork();
    bool isImportNetwork();

    std::shared_ptr<jASTERIX::jASTERIX> jASTERIX() { assert (jasterix_); return jasterix_; }
    void refreshjASTERIX();

    bool hasConfiguratonFor(unsigned int category);
    bool decodeCategory(unsigned int category);
    void decodeCategory(unsigned int category, bool decode);
    std::string editionForCategory(unsigned int category);
    void editionForCategory(unsigned int category, const std::string& edition);
    std::string refEditionForCategory(unsigned int category);
    void refEditionForCategory(unsigned int category, const std::string& ref);
    std::string spfEditionForCategory(unsigned int category);
    void spfEditionForCategory(unsigned int category, const std::string& spf);

    std::shared_ptr<ASTERIXJSONParsingSchema> schema() const;

    unsigned int numPacketsInProcessing() const;

    ASTERIXImportTaskSettings& settings();

    void testFileDecoding();

protected:

    std::shared_ptr<jASTERIX::jASTERIX> jasterix_;
    ASTERIXPostProcess post_process_;

    ASTERIXImportTaskSettings settings_;

    bool file_decoding_tested_ {false};
    bool file_decoding_errors_detected_ {false};

    bool running_ {false};

    unsigned int num_packets_in_processing_{0};
    unsigned int num_packets_total_{0};

    unsigned int num_records_ {0};

    boost::posix_time::ptime start_time_;
    std::unique_ptr<QProgressDialog> file_progress_dialog_;

    std::unique_ptr<ASTERIXImportTaskDialog> dialog_;

    std::map<unsigned int, ASTERIXCategoryConfig> category_configs_;

    std::shared_ptr<ASTERIXJSONParsingSchema> schema_;

    std::shared_ptr<ASTERIXDecodeJob> decode_job_;

    std::vector<std::shared_ptr<ASTERIXJSONMappingJob>> json_map_jobs_;
    std::vector<std::shared_ptr<ASTERIXPostprocessJob>> postprocess_jobs_;
    std::vector<std::map<std::string, std::shared_ptr<Buffer>>> queued_job_buffers_;

    boost::posix_time::ptime last_insert_time_;

    bool error_{false};
    std::string error_message_;

    bool insert_active_{false};
    //boost::posix_time::ptime insert_start_time_;
    //double total_insert_time_ms_ {0};

    boost::posix_time::ptime last_file_progress_time_;

    std::set<int> added_data_sources_;

    bool insert_slot_connected_ {false};
    bool all_done_{false};

    virtual void checkSubConfigurables() override;

    void insertData(); // inserts queued job buffers
    void checkAllDone();

    bool maxLoadReached();
    void updateFileProgressDialog(bool force=false);
};

#endif  // ASTERIXIMPORTTASK_H
