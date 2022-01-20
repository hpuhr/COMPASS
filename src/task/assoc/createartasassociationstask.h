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

#ifndef CREATEARTASASSOCIATIONSTASK_H
#define CREATEARTASASSOCIATIONSTASK_H

#include "configurable.h"
#include "createartasassociationsjob.h"
#include "createartasassociationsstatusdialog.h"
#include "dbcontent/variable/variableset.h"
#include "task.h"

#include <QObject>
#include <memory>

#include "boost/date_time/posix_time/posix_time.hpp"

class TaskManager;
class CreateARTASAssociationsTaskDialog;
class CreateARTASAssociationsTaskWidget;

class DBContent;
class Buffer;

namespace dbContent {
class Variable;
class MetaVariable;
}

class CreateARTASAssociationsTask : public Task, public Configurable
{
    Q_OBJECT

public slots:
    void dialogRunSlot();
    void dialogCancelSlot();

    void createDoneSlot();
    void createObsoleteSlot();

    void loadedDataDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSlot();

    void associationStatusSlot(QString status);
    void saveAssociationsQuestionSlot(QString question_str);

    void closeStatusDialogSlot();

public:
    CreateARTASAssociationsTask(const std::string& class_id, const std::string& instance_id,
                                TaskManager& task_manager);
    virtual ~CreateARTASAssociationsTask();


    CreateARTASAssociationsTaskDialog* dialog();

    std::string currentDataSourceName() const;
    void currentDataSourceName(const std::string& currentDataSourceName);

    dbContent::Variable* trackerDsIdVar() const;
    dbContent::Variable* trackerTrackNumVar() const;
    dbContent::Variable* trackerTrackBeginVar() const;
    dbContent::Variable* trackerTrackEndVar() const;
    dbContent::Variable* trackerCoastingVar() const;
    dbContent::Variable* trackerTRIsVar() const;
    dbContent::MetaVariable* keyVar() const;
    dbContent::MetaVariable* hashVar() const;
    dbContent::MetaVariable* todVar() const;

    float endTrackTime() const;
    void endTrackTime(float end_track_time);

    float associationTimePast() const;
    void associationTimePast(float association_time_past);

    float associationTimeFuture() const;
    void associationTimeFuture(float association_time_future);

    float missesAcceptableTime() const;
    void missesAcceptableTime(float misses_acceptable_time);

    float associationsDubiousDistantTime() const;
    void associationsDubiousDistantTime(float associations_dubious_distant_time);

    float associationDubiousCloseTimePast() const;
    void associationDubiousCloseTimePast(float association_dubious_close_time_past);

    float associationDubiousCloseTimeFuture() const;
    void associationDubiousCloseTimeFuture(float association_dubious_close_time_future);

    bool ignoreTrackEndAssociations() const;
    void ignoreTrackEndAssociations(bool value);

    bool markTrackEndAssociationsDubious() const;
    void markTrackEndAssociationsDubious(bool value);

    bool ignoreTrackCoastingAssociations() const;
    void ignoreTrackCoastingAssociations(bool value);

    bool markTrackCoastingAssociationsDubious() const;
    void markTrackCoastingAssociationsDubious(bool value);

    virtual bool checkPrerequisites();
    virtual bool isRecommended();
    virtual bool isRequired() { return false; }

    bool canRun();
    void run();

    static const std::string DONE_PROPERTY_NAME;

protected:
    std::string current_data_source_name_;

    dbContent::Variable* tracker_ds_id_var_{nullptr};
    dbContent::Variable* tracker_track_num_var_{nullptr};
    dbContent::Variable* tracker_track_begin_var_{nullptr};
    dbContent::Variable* tracker_track_end_var_{nullptr};
    dbContent::Variable* tracker_track_coasting_var_{nullptr};
    dbContent::Variable* tracker_tris_var_{nullptr};

    dbContent::MetaVariable* rec_num_var_{nullptr};
    dbContent::MetaVariable* hash_var_{nullptr}; // contains artas md5 for target reports, tris for tracker
    dbContent::MetaVariable* tod_var_{nullptr};
    dbContent::MetaVariable* associations_var_{nullptr};

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    float end_track_time_{0};  // time-delta after which begin a new track

    float association_time_past_{
        0};  // time_delta for which associations are considered into past time
    float association_time_future_{
        0};  // time_delta for which associations are considered into future time

    float misses_acceptable_time_{
        0};  // time delta at beginning/end of recording where misses are acceptable

    float associations_dubious_distant_time_{0};
    // time delta of tou where association is dubious bc too distant in time
    float association_dubious_close_time_past_{0};
    // time delta of tou where association is dubious when multible hashes exist
    float association_dubious_close_time_future_{0};
    // time delta of tou where association is dubious when multible hashes exist

    bool ignore_track_end_associations_{false};
    bool mark_track_end_associations_dubious_{false};

    bool ignore_track_coasting_associations_{false};
    bool mark_track_coasting_associations_dubious_{false};

    bool save_associations_{true};

    std::map<std::string, std::shared_ptr<Buffer>> data_;

    std::unique_ptr<CreateARTASAssociationsTaskDialog> dialog_;

    std::unique_ptr<CreateARTASAssociationsStatusDialog> status_dialog_{nullptr};

    bool dbo_loading_done_{false};

    std::shared_ptr<CreateARTASAssociationsJob> create_job_;
    bool create_job_done_{false};

    void checkAndSetTrackerVariable(const std::string& name_str, dbContent::Variable** var);
    void checkAndSetTrackerVariableFromMeta(const std::string& meta_name_str, dbContent::Variable** var);
    void checkAndSetMetaVariable(const std::string& name_str, dbContent::MetaVariable** var);

    dbContent::VariableSet getReadSetFor(const std::string& dbo_name);
};

#endif  // CREATEARTASASSOCIATIONSTASK_H
