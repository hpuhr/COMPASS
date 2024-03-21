#include "simplereconstructor.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "targetreportaccessor.h"
#include "datasourcemanager.h"

#include "timeconv.h"

using namespace std;
using namespace Utils;

SimpleReconstructor::SimpleReconstructor(const std::string& class_id, const std::string& instance_id,
                                         ReconstructorTask& task)
    : ReconstructorBase(class_id, instance_id, task), associatior_(*this)
{
    // common
    registerParameter("associate_non_mode_s", &settings_.associate_non_mode_s_, true);
    registerParameter("clean_dubious_utns", &settings_.clean_dubious_utns_, true);
    registerParameter("mark_dubious_utns_unused", &settings_.mark_dubious_utns_unused_, false);
    registerParameter("comment_dubious_utns", &settings_.comment_dubious_utns_, true);

            // tracker stuff
    registerParameter("max_time_diff_tracker", &settings_.max_time_diff_tracker_, 15.0);

    registerParameter("max_distance_quit_tracker", &settings_.max_distance_quit_tracker_, 10*NM2M); // kb 5nm
    registerParameter("max_distance_dubious_tracker", &settings_.max_distance_dubious_tracker_, 3*NM2M);
    //kb 2.5? 2.5 lowest
    registerParameter("max_positions_dubious_tracker", &settings_.max_positions_dubious_tracker_, 5u);

    registerParameter("max_distance_acceptable_tracker", &settings_.max_distance_acceptable_tracker_, NM2M/2.0);
    registerParameter("max_altitude_diff_tracker", &settings_.max_altitude_diff_tracker_, 300.0);

    registerParameter("min_updates_tracker", &settings_.min_updates_tracker_, 2u); // kb 3!!!
    registerParameter("prob_min_time_overlap_tracker", &settings_.prob_min_time_overlap_tracker_, 0.5); //kb 0.7
    registerParameter("max_speed_tracker_kts", &settings_.max_speed_tracker_kts_, 100000.0);

    registerParameter("cont_max_time_diff_tracker", &settings_.cont_max_time_diff_tracker_, 30.0);
    registerParameter("cont_max_distance_acceptable_tracker", &settings_.cont_max_distance_acceptable_tracker_, 1852.0);

            // sensor
    registerParameter("max_time_diff_sensor", &settings_.max_time_diff_sensor_, 15.0);
    registerParameter("max_distance_acceptable_sensor", &settings_.max_distance_acceptable_sensor_, 2*NM2M);
    registerParameter("max_altitude_diff_sensor", &settings_.max_altitude_diff_sensor_, 300.0);

            // target id? kb: nope
            // kb: TODO ma 1bit hamming distance, especially g (1bit wrong)/v (!->at least 1bit wrong)
}

SimpleReconstructor::~SimpleReconstructor() {}

dbContent::VariableSet SimpleReconstructor::getReadSetFor(const std::string& dbcontent_name) const
{
    dbContent::VariableSet read_set;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

            // ds id
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ds_id_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ds_id_));

            // line id
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_line_id_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_));

            // timestamp
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));

            // aircraft address
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_acad_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acad_));

            // aircraft id
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_acid_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acid_));

            // track num
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_num_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_num_));

            // track end
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_end_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_end_));

            // mode 3a
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_));

            // mode c
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_));

    if (dbcontent_name == "CAT062")
    {
        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat062_fl_measured_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat062_fl_measured_));
    }

            // latitude
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_));

            // longitude
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_longitude_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_));

            // assoc
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_utn_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_utn_));

            // rec num, must be last for update process
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_rec_num_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_));

            // adsb mops
    if (dbcontent_name == "CAT021")
    {
        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat021_mops_version_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat021_mops_version_));
    }

    return read_set;
}

void SimpleReconstructor::reset()
{
    loginf << "SimpleReconstructor: reset";

    target_reports_.clear();
    tr_timestamps_.clear();
    tr_ds_.clear();

    ReconstructorBase::reset();
}

SimpleReconstructorSettings& SimpleReconstructor::settings()
{
    return settings_;
}

bool SimpleReconstructor::processSlice_impl()
{
    loginf << "SimpleReconstructor: processSlice_impl: current_slice_begin " << Time::toString(current_slice_begin_)
           << " end " << Time::toString(current_slice_begin_ + slice_duration_)
           << " has next " << hasNextTimeSlice();

            // remove_before_time_, new data >= current_slice_begin_

    clearOldTargetReports();

    createTargetReports();

    return true;
}


void SimpleReconstructor::clearOldTargetReports()
{
    loginf << "SimpleReconstructor: clearOldTargetReports: remove_before_time " << Time::toString(remove_before_time_);

    for (auto ts_it = target_reports_.cbegin(); ts_it != target_reports_.cend() /* not hoisted */; /* no increment */)
    {
        if (ts_it->second.timestamp_ < remove_before_time_)
        {
            //loginf << "SimpleReconstructor: clearOldTargetReports: removing " << Time::toString(ts_it->second.timestamp_);
            ts_it = target_reports_.erase(ts_it);
        }
        else
        {
            //loginf << "SimpleReconstructor: clearOldTargetReports: keeping " << Time::toString(ts_it->second.timestamp_);
            ++ts_it;
        }
    }

    for (auto ts_it = tr_timestamps_.cbegin(); ts_it != tr_timestamps_.cend() /* not hoisted */; /* no increment */)
    {
        if (ts_it->first < remove_before_time_)
            ts_it = tr_timestamps_.erase(ts_it);
        else
            ++ts_it;
    }

    // dbcontent -> ds_id -> record_num
    //std::map<unsigned int, std::map<unsigned int,std::vector<unsigned long>>>

    for (auto& dbcont_it : tr_ds_)
    {
        for (auto& ds_it : dbcont_it.second)
        {
            for (auto ts_it = ds_it.second.cbegin(); ts_it != ds_it.second.cend() /* not hoisted */; /* no increment */)
            {
                if (!target_reports_.count(*ts_it)) // TODO could be made faster
                    ts_it = ds_it.second.erase(ts_it);
                else
                    ++ts_it;

//                if (ts_it->first < remove_before_time_)
//                    ts_it = ds_it.second.erase(ts_it);
//                else
//                    ++ts_it;
            }
        }
    }
}

void SimpleReconstructor::createTargetReports()
{
    loginf << "SimpleReconstructor: createTargetReports: current_slice_begin " << Time::toString(current_slice_begin_);

    boost::posix_time::ptime ts;
    unsigned long record_num;

    dbContent::targetReport::ReconstructorInfo info;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    for (auto& buf_it : *accessor_)
    {
        dbContent::TargetReportAccessor tgt_acc = accessor_->targetReportAccessor(buf_it.first);
        unsigned int buffer_size = tgt_acc.size();

        assert (dbcont_man.existsDBContent(buf_it.first));
        unsigned int dbcont_id = dbcont_man.dbContent(buf_it.first).id();

        for (unsigned int cnt=0; cnt < buffer_size; cnt++)
        {
            record_num = tgt_acc.recordNumber(cnt);
            ts = tgt_acc.timestamp(cnt);

            //loginf << "SimpleReconstructor: createTargetReports: ts " << Time::toString(ts);

            if (ts >= current_slice_begin_) // insert
            {
                // base info
                info.buffer_index_ = cnt;
                info.record_num_ = record_num;
                info.ds_id_ = tgt_acc.dsID(cnt);
                info.line_id_ = tgt_acc.lineID(cnt);
                info.timestamp_ = ts;

                // reconstructor info
                info.in_current_slice_ = true;
                info.acad_ = tgt_acc.acad(cnt);
                info.acid_ = tgt_acc.acid(cnt);

                info.mode_a_code_ = tgt_acc.modeACode(cnt);

                info.track_number_ = tgt_acc.trackNumber(cnt);
                info.track_begin_ = tgt_acc.trackBegin(cnt);
                info.track_end_ = tgt_acc.trackEnd(cnt);

                info.position_ = tgt_acc.position(cnt);
                info.position_accuracy_ = tgt_acc.positionAccuracy(cnt);

                info.barometric_altitude_ = tgt_acc.barometricAltitude(cnt);

                info.velocity_ = tgt_acc.velocity(cnt);
                info.velocity_accuracy_ = tgt_acc.velocityAccuracy(cnt);

                info.track_angle_ = tgt_acc.trackAngle(cnt);
                info.ground_bit_ = tgt_acc.groundBit(cnt);

                // insert info
                assert (!target_reports_.count(record_num));
                target_reports_[record_num] = info;

                // insert into lookups
                tr_timestamps_.insert({ts, record_num});
                // dbcontent id -> ds_id -> ts ->  record_num

                tr_ds_[dbcont_id][info.ds_id_].push_back(record_num);
            }
            else // update buffer_index_
            {
                assert (ts > remove_before_time_);

                if (!target_reports_.count(record_num))
                    logerr << "SimpleReconstructor: createTargetReports: missing prev ts " << Time::toString(ts);

                assert (target_reports_.count(record_num));

                target_reports_.at(record_num).buffer_index_ = cnt;
                target_reports_.at(record_num).in_current_slice_ = false;

                assert (target_reports_.at(record_num).timestamp_ == ts); // just to be sure
            }
        }
    }
}

