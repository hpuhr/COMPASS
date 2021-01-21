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
#include "dbovariableset.h"
#include "task.h"
#include "global.h"

#include <QObject>
#include <memory>

#include "boost/date_time/posix_time/posix_time.hpp"

class TaskManager;
class CreateAssociationsTaskWidget;
class DBOVariable;
class MetaDBOVariable;
class DBObject;
class Buffer;
class CreateAssociationsJob;

class CreateAssociationsTask : public Task, public Configurable
{
    Q_OBJECT

public slots:
    void createDoneSlot();
    void createObsoleteSlot();

    void newDataSlot(DBObject& object);
    void loadingDoneSlot(DBObject& object);

    void associationStatusSlot(QString status);

    void closeStatusDialogSlot();

public:
    CreateAssociationsTask(const std::string& class_id, const std::string& instance_id,
                           TaskManager& task_manager);
    virtual ~CreateAssociationsTask();

    TaskWidget* widget();
    virtual void deleteWidget();

    MetaDBOVariable* keyVar() const;
    MetaDBOVariable* dsIdVar() const;
    MetaDBOVariable* todVar() const;
    MetaDBOVariable* targetAddrVar() const;
    MetaDBOVariable* targetIdVar() const;
    MetaDBOVariable* trackNumVar() const;
    MetaDBOVariable* mode3AVar() const;
    MetaDBOVariable* modeCVar() const;
    MetaDBOVariable* latitudeVar() const;
    MetaDBOVariable* longitudeVar() const;

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

protected:
    std::string key_var_str_;
    MetaDBOVariable* key_var_{nullptr};

    std::string ds_id_var_str_;
    MetaDBOVariable* ds_id_var_{nullptr};

    std::string tod_var_str_;
    MetaDBOVariable* tod_var_{nullptr};

    std::string target_addr_var_str_;
    MetaDBOVariable* target_addr_var_{nullptr};

    std::string target_id_var_str_;
    MetaDBOVariable* target_id_var_{nullptr};

    std::string track_num_var_str_;
    MetaDBOVariable* track_num_var_{nullptr};

    std::string mode_3a_var_str_;
    MetaDBOVariable* mode_3a_var_{nullptr};

    std::string mode_c_var_str_;
    MetaDBOVariable* mode_c_var_{nullptr};

    std::string latitude_var_str_;
    MetaDBOVariable* latitude_var_{nullptr};

    std::string longitude_var_str_;
    MetaDBOVariable* longitude_var_{nullptr};

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

    // sensor
    double max_time_diff_sensor_ {15.0};
    double max_distance_acceptable_sensor_ {2*NM2M};
    double max_altitude_diff_sensor_ {300.0};

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;

    std::unique_ptr<CreateAssociationsTaskWidget> widget_;

    std::unique_ptr<CreateAssociationsStatusDialog> status_dialog_{nullptr};

    std::map<std::string, bool> dbo_loading_done_flags_;
    bool dbo_loading_done_{false};

    std::shared_ptr<CreateAssociationsJob> create_job_;
    bool create_job_done_{false};

    void checkAndSetMetaVariable(std::string& name_str, MetaDBOVariable** var);

    DBOVariableSet getReadSetFor(const std::string& dbo_name);
};

#endif // CREATEASSOCIATIONSTASK_H
