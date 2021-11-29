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

#include <QObject>
#include <deque>
#include <memory>
#include <mutex>

#include "asterixdecodejob.h"
#include "asterixpostprocess.h"
#include "configurable.h"
#include "json.hpp"
#include "asterixjsonmappingjob.h"
#include "asterixpostprocessjob.h"
#include "jsonmappingstubsjob.h"
#include "asterixjsonparsingschema.h"
#include "task.h"

//#include <tbb/concurrent_queue.h>

class TaskManager;
class ASTERIXImportTaskWidget;
class ASTERIXCategoryConfig;
class ASTERIXStatusDialog;
class SavedFile;
class ASTERIXImportRecordingTaskDialog;

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

    void insertProgressSlot(float percent);
    void insertDoneSlot(DBObject& object);

    void closeStatusDialogSlot();

  public:
    ASTERIXImportTask(const std::string& class_id, const std::string& instance_id,
                      TaskManager& task_manager);
    virtual ~ASTERIXImportTask();

    ASTERIXImportRecordingTaskDialog* dialog();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    void asterixFraming(const std::string& asterix_framing);
    void asterixDecoderConfig(const std::string& asterix_decoder_cfg);

    bool canImportFile();
    virtual bool canRun();
    virtual void run();
    void run(bool test);

    const std::map<std::string, SavedFile*>& fileList() { return file_list_; }
    bool hasFile(const std::string& filename) { return file_list_.count(filename) > 0; }
    void addFile(const std::string& filename);
    void removeCurrentFilename();
    void removeAllFiles ();
    void currentFilename(const std::string& filename);
    const std::string& currentFilename() { return current_filename_; }

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

    virtual bool checkPrerequisites();
    virtual bool isRecommended();
    virtual bool isRequired();

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

  protected:
    bool debug_jasterix_;
    bool limit_ram_;
    std::shared_ptr<jASTERIX::jASTERIX> jasterix_;
    ASTERIXPostProcess post_process_;

    std::map<std::string, SavedFile*> file_list_;
    std::string current_filename_;
    std::string current_framing_;

    bool test_{false};

    std::unique_ptr<ASTERIXImportRecordingTaskDialog> dialog_;

    std::map<unsigned int, ASTERIXCategoryConfig> category_configs_;

    std::shared_ptr<ASTERIXJSONParsingSchema> schema_;

    std::shared_ptr<ASTERIXDecodeJob> decode_job_;

    std::shared_ptr<ASTERIXJSONMappingJob> json_map_job_;

    std::shared_ptr<ASTERIXPostprocessJob> postprocess_job_;

    bool error_{false};
    std::string error_message_;

    std::unique_ptr<ASTERIXStatusDialog> status_widget_;

    bool waiting_for_insert_{false};
    size_t insert_active_{0};

    std::map<std::string, DBOVariableSet> dbo_variable_sets_;
    std::set<int> added_data_sources_;

    unsigned int num_radar_inserted_ {0};

    bool all_done_{false};

    virtual void checkSubConfigurables();

    void insertData(std::map<std::string, std::shared_ptr<Buffer>> job_buffers);
    void checkAllDone();

    bool maxLoadReached();
};

#endif  // ASTERIXIMPORTTASK_H
