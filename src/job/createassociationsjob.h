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

#ifndef CREATEASSOCIATIONSJOB_H
#define CREATEASSOCIATIONSJOB_H

#include "job.h"
#include "assoc/targetreport.h"
#include "assoc/target.h"
#include "global.h"

class CreateAssociationsTask;
class DBInterface;
class Buffer;
class DBObject;

class CreateAssociationsJob : public Job
{
    Q_OBJECT

signals:
    void statusSignal(QString status);

public:
    CreateAssociationsJob(CreateAssociationsTask& task, DBInterface& db_interface,
                          std::map<std::string, std::shared_ptr<Buffer>> buffers);

    virtual ~CreateAssociationsJob();

    virtual void run();

protected:
    static bool in_appimage_;

    CreateAssociationsTask& task_;
    DBInterface& db_interface_;
    std::map<std::string, std::shared_ptr<Buffer>> buffers_;

    std::map<std::string, std::map<unsigned int, std::vector<Association::TargetReport>>> target_reports_;
    //dbo name->ds_id->trs

    std::map<unsigned int, Association::Target> targets_;
    std::map<unsigned int, unsigned int> ta_2_utn_;
    unsigned int utn_cnt_ {0};

    double max_time_diff_ {15.0};
    double max_distance_quit_ {10*NM2M}; //10nm in meters // kb 5
    double max_distance_dubious_ {3*NM2M}; //kb 2.5? 2.5 lowest
    unsigned int max_positions_dubious_ {5};
    double max_distance_acceptable_trackers_ {NM2M/2.0};
    double max_distance_acceptable_sensors_ {2*NM2M};
    double max_distance_acceptable_sensors_wgs_ {max_distance_acceptable_sensors_/50000}; // 2*100 000m/deg to be safe
    double max_altitude_diff_ {300.0};
    double prob_min_time_overlap_ {0.5}; //kb 0.7
    unsigned int min_updates_ {2}; // kb 3!!!
    bool associate_ac_non_trackers_ {true};
    double max_speed_kts_ {100000};
    // target id? kb: nope
    // kb: TODO ma 1bit hamming distance, especially g (1bit wrong)/v (!->at least 1bit wrong)
    // kb: split tracker/sensor parameters

    void createTargetReports();
    void createTrackerUTNS();
    void createNonTrackerUTNS();
    void createAssociations();

    int findUTNForTarget (const Association::Target& target);
    // tries to find existing utn for target, -1 if failed
    int findUTNForTargetByTA (const Association::Target& target);
    // tries to find existing utn for target by target address, -1 if failed
    int findUTNForTargetReport (const Association::TargetReport& tr);
    // tries to find existing utn for target report, -1 if failed
    void addTarget (const Association::Target& target);
    // creates new utn, adds to targets_
    void addTargetByTargetReport (Association::TargetReport& tr);
    // creates new utn, adds to targets_, adds target report
};

#endif // CREATEASSOCIATIONSJOB_H
