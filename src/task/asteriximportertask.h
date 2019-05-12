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
#include "jsonparsingschema.h"

#include <QObject>

#include <memory>

#include "boost/date_time/posix_time/posix_time.hpp"

class TaskManager;
class ASTERIXImporterTaskWidget;
class ASTERIXCategoryConfig;
class SavedFile;

namespace jASTERIX
{
    class jASTERIX;
}

class ASTERIXImporterTask: public QObject, public Configurable
{
    Q_OBJECT
public:
    ASTERIXImporterTask(const std::string& class_id, const std::string& instance_id,
                        TaskManager* task_manager);
    virtual ~ASTERIXImporterTask();

    ASTERIXImporterTaskWidget* widget();

    virtual void generateSubConfigurable (const std::string &class_id, const std::string &instance_id);

    bool canImportFile (const std::string& filename);
    void importFile (const std::string& filename, bool test);

    const std::map <std::string, SavedFile*> &fileList () { return file_list_; }
    bool hasFile (const std::string &filename) { return file_list_.count (filename) > 0; }
    void addFile (const std::string &filename);
    void removeCurrentFilename ();
    void currentFilename (const std::string last_filename) { current_filename_ = last_filename; }
    const std::string &currentFilename () { return current_filename_; }

    std::shared_ptr<jASTERIX::jASTERIX> jASTERIX() { return jasterix_; }
    void refreshjASTERIX();

    const std::string& currentFraming() const;

    void currentFraming(const std::string &current_framing);

    bool hasConfiguratonFor (const std::string& category);
    bool decodeCategory (const std::string& category);
    void decodeCategory (const std::string& category, bool decode);
    std::string editionForCategory (const std::string& category);
    void editionForCategory (const std::string& category, const std::string& edition);

    std::shared_ptr<JSONParsingSchema> schema() const;

protected:
    bool debug_jasterix_;
    std::shared_ptr<jASTERIX::jASTERIX> jasterix_;

    std::map <std::string, SavedFile*> file_list_;
    std::string current_filename_;
    std::string current_framing_;

    std::string filename_;

    std::unique_ptr<ASTERIXImporterTaskWidget> widget_;

    std::map <std::string, ASTERIXCategoryConfig> category_configs_;

    std::shared_ptr<JSONParsingSchema> schema_;

    size_t num_records_sum_ {0};

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    virtual void checkSubConfigurables ();

    void jasterix_callback(nlohmann::json& data, size_t num_frames, size_t num_records);
};

#endif // ASTERIXIMPORTERTASK_H
