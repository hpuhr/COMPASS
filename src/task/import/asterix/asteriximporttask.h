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
#include "jsonmappingstubsjob.h"
#include "asterixjsonparsingschema.h"
#include "task.h"

#include <QObject>

#include <deque>
#include <memory>
#include <mutex>


//#include <tbb/concurrent_queue.h>

class TaskManager;

class ASTERIXImportTaskWidget;
class ASTERIXCategoryConfig;
class ASTERIXStatusDialog;
class ASTERIXImportTaskDialog;

class QProgressDialog;

namespace jASTERIX
{
class jASTERIX;
}

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

//    void insertProgressSlot(float percent);
    void insertDoneSlot();

    void closeStatusDialogSlot();

  public:
    ASTERIXImportTask(const std::string& class_id, const std::string& instance_id,
                      TaskManager& task_manager);
    virtual ~ASTERIXImportTask();

    ASTERIXImportTaskDialog* dialog();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    void asterixFileFraming(const std::string& asterix_framing);
    void asterixDecoderConfig(const std::string& asterix_decoder_cfg);

    bool canImportFile();
    virtual bool canRun() override;
    virtual void run() override;
    virtual void stop() override;
    void run(bool test);
    bool isRunning() const;

    std::vector<std::string> fileList();
    void addFile(const std::string& filename);
    void clearFileList ();

    void importFilename(const std::string& filename);
    const std::string& importFilename() { return current_filename_; }

    void importNetwork();
    bool isImportNetwork();

    std::shared_ptr<jASTERIX::jASTERIX> jASTERIX() { return jasterix_; }
    void refreshjASTERIX();

    const std::string& currentFraming() const;

    void currentFraming(const std::string& current_framing);

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

    bool debug() const;
    void debug(bool debug);

    bool limitRAM() const;
    void limitRAM(bool value);

    virtual bool checkPrerequisites() override;
    virtual bool isRecommended() override;
    virtual bool isRequired() override;

    bool overrideActive() const;
    void overrideActive(bool value);

    unsigned int overrideSacOrg() const;
    void overrideSacOrg(unsigned int value);

    unsigned int overrideSicOrg() const;
    void overrideSicOrg(unsigned int value);

    unsigned int overrideSacNew() const;
    void overrideSacNew(unsigned int value);

    unsigned int overrideSicNew() const;
    void overrideSicNew(unsigned int value);

    float overrideTodOffset() const;
    void overrideTodOffset(float value);

    unsigned int fileLineID() const;
    void fileLineID(unsigned int value);

protected:
    bool debug_jasterix_;
    bool limit_ram_;
    std::shared_ptr<jASTERIX::jASTERIX> jasterix_;
    ASTERIXPostProcess post_process_;

    bool import_file_ {false}; // false = network, true file

    nlohmann::json file_list_;
    std::string current_filename_;
    std::string current_file_framing_;
    unsigned int file_line_id_ {0};

    bool test_{false};

    bool running_ {false};
    boost::posix_time::ptime start_time_;
    std::unique_ptr<QProgressDialog> file_progress_dialog_;

    std::unique_ptr<ASTERIXImportTaskDialog> dialog_;

    std::map<unsigned int, ASTERIXCategoryConfig> category_configs_;

    std::shared_ptr<ASTERIXJSONParsingSchema> schema_;

    std::shared_ptr<ASTERIXDecodeJob> decode_job_;

    std::shared_ptr<ASTERIXJSONMappingJob> json_map_job_;

    std::shared_ptr<ASTERIXPostprocessJob> postprocess_job_;

    std::map<std::string, std::shared_ptr<Buffer>> buffer_cache_; // to be used for file import
    boost::posix_time::ptime last_insert_time_;

    bool error_{false};
    std::string error_message_;

    //std::unique_ptr<ASTERIXStatusDialog> status_widget_;

    bool waiting_for_insert_{false};
    bool insert_active_{false};

    //std::map<std::string, DBOVariableSet> dbo_variable_sets_;
    std::set<int> added_data_sources_;

    //unsigned int num_radar_inserted_ {0};

    bool all_done_{false};

    virtual void checkSubConfigurables() override;

    void insertData(std::map<std::string, std::shared_ptr<Buffer>> job_buffers);
    void checkAllDone();

    bool maxLoadReached();
};

#endif  // ASTERIXIMPORTTASK_H
