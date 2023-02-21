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

#ifndef CREATEASSOCIATIONSTASK_H
#define CREATEASSOCIATIONSTASK_H

#include "configurable.h"
#include "createassociationsstatusdialog.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/dbcontentcache.h"
#include "task.h"
#include "global.h"

#include <QObject>
#include <memory>

#include "boost/date_time/posix_time/posix_time.hpp"

class TaskManager;
class CreateAssociationsTaskDialog;
class DBContent;
class Buffer;
class CreateAssociationsJob;

namespace dbContent
{
class Variable;
class MetaVariable;

}

class CreateAssociationsTask : public Task, public Configurable
{
    Q_OBJECT

public slots:
    void dialogRunSlot();
    void dialogCancelSlot();

    void createDoneSlot();
    void createObsoleteSlot();

    void loadedDataSlot(const std::map<std::string, std::shared_ptr<Buffer>>& data, bool requires_reset);
    void loadingDoneSlot();

    void associationStatusSlot(QString status);

    void closeStatusDialogSlot();

public:
    CreateAssociationsTask(const std::string& class_id, const std::string& instance_id,
                           TaskManager& task_manager);
    virtual ~CreateAssociationsTask();

    CreateAssociationsTaskDialog* dialog();

//    dbContent::MetaVariable* keyVar() const;
//    dbContent::MetaVariable* dsIdVar() const;
//    dbContent::MetaVariable* lineIdVar() const;
//    dbContent::MetaVariable* timestampVar() const;
//    dbContent::MetaVariable* targetAddrVar() const;
//    dbContent::MetaVariable* targetIdVar() const;
//    dbContent::MetaVariable* trackNumVar() const;
//    dbContent::MetaVariable* trackEndVar() const;
//    dbContent::MetaVariable* mode3AVar() const;
//    dbContent::MetaVariable* modeCVar() const;
//    dbContent::MetaVariable* latitudeVar() const;
//    dbContent::MetaVariable* longitudeVar() const;

    virtual bool checkPrerequisites();
    virtual bool isRecommended();
    virtual bool isRequired() { return false; }

    bool canRun();
    void run();

    static const std::string DONE_PROPERTY_NAME;

    double maxTimeDiffTracker() const;
    void maxTimeDiffTracker(double value);

    double maxTimeDiffSensor() const;
    void maxTimeDiffSensor(double value);

    double maxDistanceQuitTracker() const;
    void maxDistanceQuitTracker(double value);

    double maxDistanceDubiousTracker() const;
    void maxDistanceDubiousTracker(double value);

    unsigned int maxPositionsDubiousTracker() const;
    void maxPositionsDubiousTracker(unsigned int value);

    double maxDistanceAcceptableTracker() const;
    void maxDistanceAcceptableTracker(double value);

    double maxDistanceAcceptableSensor() const;
    void maxDistanceAcceptableSensor(double value);

    double maxAltitudeDiffTracker() const;
    void maxAltitudeDiffTracker(double value);

    double maxAltitudeDiffSensor() const;
    void maxAltitudeDiffSensor(double value);

    double probMinTimeOverlapTracker() const;
    void probMinTimeOverlapTracker(double value);

    unsigned int minUpdatesTracker() const;
    void minUpdatesTracker(unsigned int value);

    bool associateNonModeS() const;
    void associateNonModeS(bool value);

    double maxSpeedTrackerKts() const;
    void maxSpeedTrackerKts(double value);

    bool cleanDubiousUtns() const;
    void cleanDubiousUtns(bool value);

    bool markDubiousUtnsUnused() const;
    void markDubiousUtnsUnused(bool value);

    bool commentDubiousUtns() const;
    void commentDubiousUtns(bool value);

    double contMaxTimeDiffTracker() const;
    void contMaxTimeDiffTracker(double value);

    double contMaxDistanceAcceptableTracker() const;
    void contMaxDistanceAcceptableTracker(double value);

protected:
//    dbContent::MetaVariable* rec_num_var_{nullptr};
//    dbContent::MetaVariable* ds_id_var_{nullptr};
//    dbContent::MetaVariable* line_id_var_{nullptr};
//    dbContent::MetaVariable* ts_var_{nullptr};
//    dbContent::MetaVariable* target_addr_var_{nullptr};
//    dbContent::MetaVariable* target_id_var_{nullptr};
//    dbContent::MetaVariable* track_num_var_{nullptr};
//    dbContent::MetaVariable* track_end_var_{nullptr};
//    dbContent::MetaVariable* mode_3a_var_{nullptr};
//    dbContent::MetaVariable* mode_c_var_{nullptr};
//    dbContent::MetaVariable* latitude_var_{nullptr};
//    dbContent::MetaVariable* longitude_var_{nullptr};
//    dbContent::MetaVariable* associations_var_{nullptr};

    bool associate_non_mode_s_ {true};
    bool clean_dubious_utns_ {true};
    bool mark_dubious_utns_unused_ {false};
    bool comment_dubious_utns_ {true};

    // tracker stuff
    double max_time_diff_tracker_ {15.0};

    double max_distance_quit_tracker_ {10*NM2M}; //10nm in meters // kb 5
    double max_distance_dubious_tracker_ {3*NM2M}; //kb 2.5? 2.5 lowest
    unsigned int max_positions_dubious_tracker_ {5};

    double max_distance_acceptable_tracker_ {NM2M/2.0};
    double max_altitude_diff_tracker_ {300.0};

    unsigned int min_updates_tracker_ {2}; // kb 3!!!

    double prob_min_time_overlap_tracker_ {0.5}; //kb 0.7

    double max_speed_tracker_kts_ {100000};

    double cont_max_time_diff_tracker_ {30.0};
    double cont_max_distance_acceptable_tracker_ {1852.0};

    // sensor
    double max_time_diff_sensor_ {15.0};
    double max_distance_acceptable_sensor_ {2*NM2M};
    double max_altitude_diff_sensor_ {300.0};

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    std::unique_ptr<CreateAssociationsTaskDialog> dialog_;

    std::unique_ptr<CreateAssociationsStatusDialog> status_dialog_;

    std::shared_ptr<dbContent::Cache> cache_;

    std::map<std::string, std::shared_ptr<Buffer>> data_;
    //bool dbo_loading_done_{false};

    std::shared_ptr<CreateAssociationsJob> create_job_;
    bool create_job_done_{false};

    //void checkAndSetMetaVariable(const std::string& name_str, dbContent::MetaVariable** var);

    dbContent::VariableSet getReadSetFor(const std::string& dbcontent_name);
};

#endif // CREATEASSOCIATIONSTASK_H
