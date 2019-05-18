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
#include "asterixcategoryconfig.h"
#include "taskmanager.h"
#include "configurable.h"
#include "files.h"
#include "stringconv.h"
#include "savedfile.h"
#include "logger.h"
#include "jobmanager.h"

#include <jasterix/jasterix.h>
#include <jasterix/category.h>

#include <algorithm>
#include <iomanip>

#include <QMessageBox>

using namespace Utils;
using namespace nlohmann;
using namespace std;
//using namespace jASTERIX;

ASTERIXImporterTask::ASTERIXImporterTask(const std::string& class_id, const std::string& instance_id,
                                   TaskManager* task_manager)
    : Configurable (class_id, instance_id, task_manager)
{
    qRegisterMetaType<std::shared_ptr<nlohmann::json>>("std::shared_ptr<nlohmann::json>");

    registerParameter("debug_jasterix", &debug_jasterix_, false);
    registerParameter("current_filename", &current_filename_, "");
    registerParameter("current_framing", &current_framing_, "");

    createSubConfigurables();

    std::string jasterix_definition_path = HOME_DATA_DIRECTORY+"/jasterix_definitions";

    loginf << "ASTERIXImporterTask: contructor: jasterix definition path '" << jasterix_definition_path << "'";
    assert (Files::directoryExists(jasterix_definition_path));

    jasterix_.reset(new jASTERIX::jASTERIX(jasterix_definition_path, false, debug_jasterix_));

    std::vector<std::string> framings = jasterix_->framings();
    if (std::find(framings.begin(), framings.end(), current_framing_) == framings.end())
    {
        loginf << "ASTERIXImporterTask: contructor: resetting to no framing";
        current_framing_ = "";
    }
}


ASTERIXImporterTask::~ASTERIXImporterTask()
{
//    if (msg_box_)
//    {
//        delete msg_box_;
//        msg_box_ = nullptr;
//    }

    for (auto it : file_list_)
        delete it.second;

    file_list_.clear();
}

void ASTERIXImporterTask::generateSubConfigurable (const std::string &class_id, const std::string &instance_id)
{
    if (class_id == "ASTERIXFile")
    {
        SavedFile *file = new SavedFile (class_id, instance_id, this);
        assert (file_list_.count (file->name()) == 0);
        file_list_.insert (std::pair <std::string, SavedFile*> (file->name(), file));
    }
    else if (class_id == "ASTERIXCategoryConfig")
    {
        std::string category = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueString("category");

        assert (category_configs_.find (category) == category_configs_.end());

        logdbg << "ASTERIXImporterTask: generateSubConfigurable: generating asterix config " << instance_id
               << " with cat " << category;

        category_configs_.emplace(std::piecewise_construct,
                     std::forward_as_tuple(category),  // args for key
                     std::forward_as_tuple(category, class_id, instance_id, this));  // args for mapped value
    }
    else if (class_id == "JSONParsingSchema")
    {
        std::string name = configuration().getSubConfiguration(
                    class_id, instance_id).getParameterConfigValueString("name");

        assert (schema_ == nullptr);
        assert (name == "jASTERIX");

        logdbg << "ASTERIXImporterTask: generateSubConfigurable: generating schema " << instance_id
               << " with name " << name;

        schema_.reset(new JSONParsingSchema(class_id, instance_id, this));
    }
    else
        throw std::runtime_error ("ASTERIXImporterTask: generateSubConfigurable: unknown class_id "+class_id );
}

void ASTERIXImporterTask::checkSubConfigurables ()
{
    if (schema_ == nullptr)
    {
        Configuration &config = addNewSubConfiguration ("JSONParsingSchema", "JSONParsingSchemajASTERIX0");
        config.addParameterString("name", "jASTERIX");
        generateSubConfigurable ("JSONParsingSchema", "JSONParsingSchemajASTERIX0");
    }
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

void ASTERIXImporterTask::refreshjASTERIX()
{
    std::string jasterix_definition_path = HOME_DATA_DIRECTORY+"/jasterix_definitions";

    loginf << "ASTERIXImporterTask: refreshjASTERIX: jasterix definition path '" << jasterix_definition_path << "'";
    assert (Files::directoryExists(jasterix_definition_path));

    jasterix_.reset(new jASTERIX::jASTERIX(jasterix_definition_path, false, debug_jasterix_));

    std::vector<std::string> framings = jasterix_->framings();
    if (std::find(framings.begin(), framings.end(), current_framing_) == framings.end())
    {
        loginf << "ASTERIXImporterTask: refreshjASTERIX: resetting to no framing";
        current_framing_ = "";
    }
}

void ASTERIXImporterTask::addFile (const std::string &filename)
{
    if (file_list_.count (filename) != 0)
        throw std::invalid_argument ("ASTERIXImporterTask: addFile: name '"+filename+"' already in use");

    std::string instancename = filename;
    instancename.erase (std::remove(instancename.begin(), instancename.end(), '/'), instancename.end());

    Configuration &config = addNewSubConfiguration ("ASTERIXFile", "ASTERIXFile"+instancename);
    config.addParameterString("name", filename);
    generateSubConfigurable ("ASTERIXFile", "ASTERIXFile"+instancename);

    if (widget_)
        widget_->updateFileListSlot();
}

void ASTERIXImporterTask::removeCurrentFilename ()
{
    assert (current_filename_.size());
    assert (hasFile(current_filename_));

    if (file_list_.count (current_filename_) != 1)
        throw std::invalid_argument ("ASTERIXImporterTask: addFile: name '"+current_filename_+"' not in use");

    delete file_list_.at(current_filename_);
    file_list_.erase(current_filename_);

    if (widget_)
        widget_->updateFileListSlot();
}

const std::string& ASTERIXImporterTask::currentFraming() const
{
    return current_framing_;
}

void ASTERIXImporterTask::currentFraming(const std::string &current_framing)
{
    current_framing_ = current_framing;
}

bool ASTERIXImporterTask::hasConfiguratonFor (const std::string& category)
{
    return category_configs_.count(category) > 0;
}

bool ASTERIXImporterTask::decodeCategory (const std::string& category)
{
    assert (hasConfiguratonFor(category));
    return category_configs_.at(category).decode();
}

void ASTERIXImporterTask::decodeCategory (const std::string& category, bool decode)
{
    assert (jasterix_->hasCategory(category));

    loginf << "ASTERIXImporterTask: decodeCategory: cat " << category << " decode " << decode;

    if (!hasConfiguratonFor(category))
    {
        Configuration &new_cfg = configuration().addNewSubConfiguration ("ASTERIXCategoryConfig");
        new_cfg.addParameterString ("category", category);
        new_cfg.addParameterBool ("decode", decode);
        new_cfg.addParameterString ("edition", jasterix_->categories().at(category).defaultEdition());

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert (hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).decode(decode);
}

std::string ASTERIXImporterTask::editionForCategory (const std::string& category)
{
    assert (hasConfiguratonFor(category));

    // check if edition exists, otherwise rest to default
    if (jasterix_->categories().at(category).editions().count(category_configs_.at(category).edition()) == 0)
    {
        loginf << "ASTERIXImporterTask: editionForCategory: cat " << category << " reset to default edition";
        category_configs_.at(category).edition(jasterix_->categories().at(category).defaultEdition());
    }

    return category_configs_.at(category).edition();
}

void ASTERIXImporterTask::editionForCategory (const std::string& category, const std::string& edition)
{
    assert (jasterix_->hasCategory(category));

    loginf << "ASTERIXImporterTask: decodeCategory: cat " << category << " edition " << edition;

    if (!hasConfiguratonFor(category))
    {
        Configuration &new_cfg = configuration().addNewSubConfiguration ("ASTERIXCategoryConfig");
        new_cfg.addParameterString ("category", category);
        new_cfg.addParameterBool ("decode", false);
        new_cfg.addParameterString ("edition", edition);

        generateSubConfigurable("ASTERIXCategoryConfig", new_cfg.getInstanceId());
        assert (hasConfiguratonFor(category));
    }
    else
        category_configs_.at(category).edition(edition);
}

std::shared_ptr<JSONParsingSchema> ASTERIXImporterTask::schema() const
{
    return schema_;
}

bool ASTERIXImporterTask::canImportFile (const std::string& filename)
{
    if (!Files::fileExists(filename))
    {
        loginf << "ASTERIXImporterTask: canImportFile: not possible since file does not exist";
        return false;
    }

//    if (!ATSDB::instance().objectManager().existsObject("ADSB"))
//    {
//        loginf << "ASTERIXImporterTask: canImportFile: not possible since DBObject does not exist";
//        return false;
//    }

//    if (!current_schema_.size())
//        return false;

//    if (!schemas_.count(current_schema_))
//    {
//        current_schema_ = "";
//        return false;
//    }

    return true;
}

void ASTERIXImporterTask::importFile(const std::string& filename, bool test)
{
    loginf << "ASTERIXImporterTask: importFile: filename " << filename << " test " << test;

    assert (canImportFile(filename));

    filename_ = filename;
    test_ = test;

    num_frames_ = 0;
    num_records_ = 0;

    start_time_ = boost::posix_time::microsec_clock::local_time();

    // set category configs
    jasterix_->decodeNoCategories();

    for (auto& cat_it : category_configs_)
    {
        if (!jasterix_->hasCategory(cat_it.first))
        {
            logwrn << "ASTERIXImporterTask: importFile: cat " << cat_it.first << " not defined in decoder";
            continue;
        }

        if (!jasterix_->hasEdition(cat_it.first, cat_it.second.edition()))
        {
            logwrn << "ASTERIXImporterTask: importFile: cat " << cat_it.first << " edition "
                   << cat_it.second.edition() << " not defined in decoder";
            continue;
        }

        jasterix_->setDecodeCategory(cat_it.first, cat_it.second.decode());
        jasterix_->setEdition(cat_it.first, cat_it.second.edition());
        loginf << "ASTERIXImporterTask: importFile: set cat " << cat_it.first << " decode " <<  cat_it.second.decode()
               << " edition " << cat_it.second.edition();
        // TODO mapping
    }

    loginf << "ASTERIXImporterTask: importFile: filename " << filename;


    assert (decode_job_ == nullptr);
    decode_job_.reset(new ASTERIXDecodeJob(*this, filename, current_framing_, test));

    connect (decode_job_.get(), SIGNAL(obsoleteSignal()), this, SLOT(decodeASTERIXObsoleteSlot()),
             Qt::QueuedConnection);
    connect (decode_job_.get(), SIGNAL(doneSignal()), this, SLOT(decodeASTERIXDoneSlot()), Qt::QueuedConnection);
    connect (decode_job_.get(), SIGNAL(decodedASTERIXSignal(std::shared_ptr<nlohmann::json>)),
             this, SLOT(addDecodedASTERIXSlot(std::shared_ptr<nlohmann::json>)), Qt::QueuedConnection);

    JobManager::instance().addNonBlockingJob(decode_job_);

    return;
}

void ASTERIXImporterTask::decodeASTERIXDoneSlot ()
{
    // TODO never received
    loginf << "ASTERIXImporterTask: decodeASTERIXDoneSlot";

    assert (decode_job_);

    all_done_ = true;
    decode_job_ = nullptr;

    updateMsgBox();
}
void ASTERIXImporterTask::decodeASTERIXObsoleteSlot ()
{
    decode_job_ = nullptr;
}

void ASTERIXImporterTask::addDecodedASTERIXSlot (std::shared_ptr<nlohmann::json> data)
{
    loginf << "ASTERIXImporterTask: addDecodedASTERIX";

    assert (decode_job_);

    num_frames_ += decode_job_->numFrames();
    num_records_ += decode_job_->numRecords();

    //category_counts_ = decode_job_->categoryCounts();

//    stop_time_ = boost::posix_time::microsec_clock::local_time();

//    boost::posix_time::time_duration diff = stop_time_ - start_time_;

//    std::string time_str = std::to_string(diff.hours())+"h "+std::to_string(diff.minutes())
//            +"m "+std::to_string(diff.seconds())+"s";

//    loginf << "ASTERIXImporterTask: addDecodedASTERIX: num frames " << num_frames_ << " records " << num_records_
//           << " after " << time_str << " " << (int)(1000.0*num_records_/diff.total_milliseconds()) << " rec/s ";

    // create new extract job
    std::shared_ptr<ASTERIXExtractRecordsJob> extract_job {new ASTERIXExtractRecordsJob(current_framing_, data)};
    loginf << "ASTERIXImporterTask: addDecodedASTERIX: data " << data->size() << " jobs " << extract_jobs_.unsafe_size();

    connect (extract_job.get(), SIGNAL(obsoleteSignal()), this, SLOT(extractASTERIXObsoleteSlot()),
             Qt::QueuedConnection);
    connect (extract_job.get(), SIGNAL(doneSignal()), this, SLOT(extractASTERIXDoneSlot()), Qt::QueuedConnection);

    extract_jobs_.push(extract_job);
    JobManager::instance().addBlockingJob(extract_job);

    if (extract_jobs_.unsafe_size() >= 10)
        decode_job_->pause();

    updateMsgBox();
}

void ASTERIXImporterTask::extractASTERIXDoneSlot ()
{
    loginf << "ASTERIXImporterTask: extractASTERIXDoneSlot";

    ASTERIXExtractRecordsJob* extract_job = static_cast<ASTERIXExtractRecordsJob*>(sender());
    std::shared_ptr<ASTERIXExtractRecordsJob> queued_extract_job;

    while (!extract_jobs_.try_pop(queued_extract_job))
        QThread::sleep(1);

    assert (queued_extract_job.get() == extract_job);

    loginf << "ASTERIXImporterTask: extractASTERIXDoneSlot: update";

    for (auto& cat_cnt_it: queued_extract_job->categoryCounts())
    {
        if (category_counts_.count(cat_cnt_it.first) == 0)
            category_counts_[cat_cnt_it.first] = cat_cnt_it.second;
        else
            category_counts_[cat_cnt_it.first] += cat_cnt_it.second;
    }

    if (decode_job_ && extract_jobs_.unsafe_size() < 10)
        decode_job_->unpause();

    updateMsgBox();
}

void ASTERIXImporterTask::extractASTERIXObsoleteSlot ()
{
    loginf << "ASTERIXImporterTask: extractASTERIXObsoleteSlot";
}

void ASTERIXImporterTask::updateMsgBox ()
{
    loginf << "ASTERIXImporterTask: updateMsgBox";

    if (!msg_box_)
    {
        msg_box_ = new QMessageBox ();
        assert (msg_box_);
    }

    std::string msg;

    if (test_)
        msg = "Testing import of";
    else
        msg = "Importing";

    msg += " file '"+filename_+"'\n";

    stop_time_ = boost::posix_time::microsec_clock::local_time();

    boost::posix_time::time_duration diff = stop_time_ - start_time_;

    std::string elapsed_time_str = String::timeStringFromDouble(diff.total_milliseconds()/1000.0, false);

    double records_per_second = num_records_/(diff.total_milliseconds()/1000.0);

    std::string records_rate_str_ = std::to_string(static_cast<int>(records_per_second));

    // calculate insert rate
//    double objects_per_second = 0.0;
//    bool objects_per_second_updated = false;
//    if (objects_inserted_ && statistics_calc_objects_inserted_ != objects_inserted_)
//    {
//        objects_per_second = objects_inserted_/(diff.total_milliseconds()/1000.0);
//        objects_per_second_updated = true;

//        statistics_calc_objects_inserted_ = objects_inserted_;
//        object_rate_str_ = std::to_string(static_cast<int>(objects_per_second));
//    }

//    // calculate remaining time
//    if (objects_per_second_updated && bytes_to_read_ && objects_parsed_ && objects_mapped_)
//    {
//        double avg_time_per_obj_s = 1.0/objects_per_second;

//        double avg_mapped_obj_bytes = static_cast<double>(bytes_read_)/static_cast<double>(objects_mapped_);
//        double num_obj_total = static_cast<double>(bytes_to_read_)/avg_mapped_obj_bytes;

//        double remaining_obj_num = 0.0;

//        if (objects_not_mapped_ < objects_parsed_) // skipped objects ok
//        {
//            double not_skipped_ratio =
//                    static_cast<double>(objects_parsed_-objects_not_mapped_)/static_cast<double>(objects_parsed_);
//            remaining_obj_num = (num_obj_total*not_skipped_ratio)-objects_inserted_;

//    //        loginf << "UGA avg bytes " << avg_obj_bytes << " num total " << num_obj_total << " not skipped ratio "
//    //               << not_skipped_ratio << " all mapped " << num_obj_total*not_skipped_ratio
//    //               << " obj ins " << objects_inserted_ << " remain obj " << remaining_obj_num;
//        }
//        else // unknown number of skipped objects
//        {
//            remaining_obj_num = num_obj_total-objects_inserted_;
//        }

//        if (remaining_obj_num < 0.0)
//            remaining_obj_num = 0.0;

//        double time_remaining_s = remaining_obj_num*avg_time_per_obj_s;
//        remaining_time_str_ = String::timeStringFromDouble(time_remaining_s, false);
//    }

    msg += "Elapsed Time: "+elapsed_time_str+"\n";

//    if (bytes_read_ > 1e9)
//        msg += "Data read: "+String::doubleToStringPrecision(static_cast<double>(bytes_read_)*1e-9,2)+" GB";
//    else
//        msg += "Data read: "+String::doubleToStringPrecision(static_cast<double>(bytes_read_)*1e-6,2)+" MB";

//    if (bytes_to_read_)
//        msg += " ("+std::to_string(static_cast<int>(read_status_percent_))+"%)\n\n";
//    else
//        msg += "\n\n";

//    msg += "Objects read: "+std::to_string(objects_read_)+"\n";
//    msg += "Objects parsed: "+std::to_string(objects_parsed_)+"\n";
//    msg += "Objects parse errors: "+std::to_string(objects_parse_errors_)+"\n\n";

//    msg += "Objects mapped: "+std::to_string(objects_mapped_)+"\n";
//    msg += "Objects not mapped: "+std::to_string(objects_not_mapped_)+"\n\n";

    msg += "Frames read: "+std::to_string(num_frames_)+"\n";
    msg += "Records read: "+std::to_string(num_records_)+"\n";
    //msg += "Objects inserted: "+std::to_string(objects_inserted_)+"\n\n";

    msg += "\n";

    stringstream ss;

    for (auto& cat_cnt_it : category_counts_)
    {
//        loginf << "ASTERIXImporterTask: decodeASTERIXDoneSlot: cat " << cat_cnt_it.first
//               << " cnt " << cat_cnt_it.second;
        ss.str("");

        ss << setfill('0') << setw(3) << cat_cnt_it.first;

        msg += "CAT"+ss.str()+": "+std::to_string(cat_cnt_it.second)+"\n";
    }

    msg += "\n";
    msg += "Record rate: "+records_rate_str_+" e/s";

//    if (!all_done_ && remaining_time_str_.size())
//        msg += "\nEstimated remaining time: "+remaining_time_str_;

    msg_box_->setText(msg.c_str());

    if (all_done_)
        msg_box_->setStandardButtons(QMessageBox::Ok);
    else
        msg_box_->setStandardButtons(QMessageBox::NoButton);

    msg_box_->show();

    loginf << "ASTERIXImporterTask: updateMsgBox: done";
}
