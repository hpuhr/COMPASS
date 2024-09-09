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

#include "reconstructorbase.h"
#include "reconstructortask.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "logger.h"
#include "timeconv.h"
#include "datasourcemanager.h"
#include "evaluationmanager.h"

#include "kalman_chain.h"
#include "tbbhack.h"

#include "dbcontent/variable/metavariable.h"
#include "targetreportaccessor.h"
#include "number.h"

using namespace std;
using namespace Utils;

ReconstructorBase::ReconstructorBase(const std::string& class_id, 
                                     const std::string& instance_id,
                                     ReconstructorTask& task, 
                                     std::unique_ptr<AccuracyEstimatorBase>&& acc_estimator,
                                     ReconstructorBaseSettings& base_settings,
                                     unsigned int default_line_id)
    :   Configurable (class_id, instance_id, &task)
      ,   acc_estimator_(std::move(acc_estimator))
      ,   task_(task)
      ,   base_settings_(base_settings)
      ,   chain_predictors_(new reconstruction::KalmanChainPredictors)
{
    accessor_ = make_shared<dbContent::DBContentAccessor>();

    // base settings
    {
        registerParameter("ds_line", &base_settings_.ds_line, default_line_id);

        registerParameter("slice_duration_in_minutes", &base_settings_.slice_duration_in_minutes,
                          base_settings_.slice_duration_in_minutes);
        registerParameter("outdated_duration_in_minutes", &base_settings_.outdated_duration_in_minutes,
                          base_settings_.outdated_duration_in_minutes);

        registerParameter("delete_all_calc_reftraj", &base_settings_.delete_all_calc_reftraj,
                          base_settings_.delete_all_calc_reftraj);
    }

    // association stuff
    registerParameter("max_time_diff", &base_settings_.max_time_diff_, base_settings_.max_time_diff_);
    registerParameter("track_max_time_diff", &base_settings_.track_max_time_diff_, base_settings_.track_max_time_diff_);

    registerParameter("max_altitude_diff", &base_settings_.max_altitude_diff_, base_settings_.max_altitude_diff_);

    registerParameter("target_prob_min_time_overlap", &base_settings_.target_prob_min_time_overlap_,
                      base_settings_.target_prob_min_time_overlap_);
    registerParameter("target_min_updates", &base_settings_.target_min_updates_, base_settings_.target_min_updates_);
    registerParameter("target_max_positions_dubious_verified_rate", &base_settings_.target_max_positions_dubious_verified_rate_,
                      base_settings_.target_max_positions_dubious_verified_rate_);
    registerParameter("target_max_positions_dubious_unknown_rate", &base_settings_.target_max_positions_dubious_unknown_rate_,
                      base_settings_.target_max_positions_dubious_unknown_rate_);

    // reference computation
    {
        registerParameter("ref_rec_type", (int*)&ref_calc_settings_.kalman_type_assoc, (int)ReferenceCalculatorSettings().kalman_type_assoc);
        registerParameter("ref_rec_type_final", (int*)&ref_calc_settings_.kalman_type_final, (int)ReferenceCalculatorSettings().kalman_type_final);

        registerParameter("ref_Q_std", &ref_calc_settings_.Q_std.Q_std_static, ReferenceCalculatorSettings().Q_std.Q_std_static);
        registerParameter("ref_Q_std_ground", &ref_calc_settings_.Q_std.Q_std_ground, ReferenceCalculatorSettings().Q_std.Q_std_ground);
        registerParameter("ref_Q_std_air", &ref_calc_settings_.Q_std.Q_std_air, ReferenceCalculatorSettings().Q_std.Q_std_air);
        registerParameter("ref_Q_std_unknown", &ref_calc_settings_.Q_std.Q_std_unknown, ReferenceCalculatorSettings().Q_std.Q_std_unknown);

        registerParameter("dynamic_process_noise", &ref_calc_settings_.dynamic_process_noise, ReferenceCalculatorSettings().dynamic_process_noise);

        registerParameter("ref_min_chain_size", &ref_calc_settings_.min_chain_size   , ReferenceCalculatorSettings().min_chain_size);
        registerParameter("ref_min_dt"        , &ref_calc_settings_.min_dt   , ReferenceCalculatorSettings().min_dt);
        registerParameter("ref_max_dt"        , &ref_calc_settings_.max_dt   , ReferenceCalculatorSettings().max_dt);
        registerParameter("ref_max_distance"  , &ref_calc_settings_.max_distance   , ReferenceCalculatorSettings().max_distance);

        registerParameter("ref_smooth_rts", &ref_calc_settings_.smooth_rts, ReferenceCalculatorSettings().smooth_rts);

        registerParameter("ref_resample_result", &ref_calc_settings_.resample_result, ReferenceCalculatorSettings().resample_result);
        registerParameter("ref_resample_Q_std", &ref_calc_settings_.resample_Q_std.Q_std_static, ReferenceCalculatorSettings().resample_Q_std.Q_std_static);
        registerParameter("ref_resample_Q_std_ground", &ref_calc_settings_.resample_Q_std.Q_std_ground, ReferenceCalculatorSettings().resample_Q_std.Q_std_ground);
        registerParameter("ref_resample_Q_std_air", &ref_calc_settings_.resample_Q_std.Q_std_air, ReferenceCalculatorSettings().resample_Q_std.Q_std_air);
        registerParameter("ref_resample_Q_std_unknown", &ref_calc_settings_.resample_Q_std.Q_std_unknown, ReferenceCalculatorSettings().resample_Q_std.Q_std_unknown);
        registerParameter("ref_resample_dt"    , &ref_calc_settings_.resample_dt    , ReferenceCalculatorSettings().resample_dt);

        registerParameter("ref_max_proj_distance_cart", &ref_calc_settings_.max_proj_distance_cart, ReferenceCalculatorSettings().max_proj_distance_cart);

        registerParameter("ref_resample_systracks"       , &ref_calc_settings_.resample_systracks       , ReferenceCalculatorSettings().resample_systracks);
        registerParameter("ref_resample_systracks_dt"    , &ref_calc_settings_.resample_systracks_dt    , ReferenceCalculatorSettings().resample_systracks_dt);
        registerParameter("ref_resample_systracks_max_dt", &ref_calc_settings_.resample_systracks_max_dt, ReferenceCalculatorSettings().resample_systracks_max_dt);
    }

    assert (acc_estimator_);
}

/**
 */
ReconstructorBase::~ReconstructorBase()
{
    acc_estimator_ = nullptr;
}

bool ReconstructorBase::hasNextTimeSlice()
{
    if (current_slice_begin_.is_not_a_date_time())
    {
        assert (COMPASS::instance().dbContentManager().hasMinMaxTimestamp());
        std::tie(timestamp_min_, timestamp_max_) = COMPASS::instance().dbContentManager().minMaxTimestamp();

        current_slice_begin_ = timestamp_min_;
        next_slice_begin_ = timestamp_min_; // first slice

        loginf << "ReconstructorBase: hasNextTimeSlice: new min " << Time::toString(current_slice_begin_)
               << " max " << Time::toString(timestamp_max_) << " first_slice " << first_slice_;
    }

    assert (!current_slice_begin_.is_not_a_date_time());
    assert (!timestamp_max_.is_not_a_date_time());

    first_slice_ = current_slice_begin_ == timestamp_min_;

    return next_slice_begin_ < timestamp_max_;
}

int ReconstructorBase::numSlices() const
{
    if (timestamp_min_.is_not_a_date_time() || timestamp_max_.is_not_a_date_time())
        return -1;

    return (int)std::ceil(Utils::Time::partialSeconds(timestamp_max_ - timestamp_min_)
                          / Utils::Time::partialSeconds(base_settings_.sliceDuration()));
}

std::unique_ptr<ReconstructorBase::DataSlice> ReconstructorBase::getNextTimeSlice()
{
    assert (hasNextTimeSlice());

    current_slice_begin_ = next_slice_begin_;

    assert (!current_slice_begin_.is_not_a_date_time());
    assert (!timestamp_max_.is_not_a_date_time());

    assert (current_slice_begin_ <= timestamp_max_);

    boost::posix_time::ptime current_slice_end = current_slice_begin_ + base_settings_.sliceDuration();

    TimeWindow window {current_slice_begin_, current_slice_end};

    loginf << "ReconstructorBase: getNextTimeSlice: current_slice_begin " << Time::toString(current_slice_begin_)
           << " current_slice_end " << Time::toString(current_slice_end);

    first_slice_ = current_slice_begin_ == timestamp_min_;

    remove_before_time_ = current_slice_begin_ - base_settings_.outdatedDuration();
    write_before_time_ = current_slice_end - base_settings_.outdatedDuration();

    next_slice_begin_ = current_slice_end; // for next iteration

            //assert (current_slice_begin_ <= timestamp_max_); can be bigger

    std::unique_ptr<DataSlice> slice (new DataSlice());

    slice->slice_count_ = slice_cnt_;
    slice->slice_begin_ = current_slice_begin_;
    slice->next_slice_begin_ = next_slice_begin_;
    slice->timestamp_min_ = timestamp_min_;
    slice->timestamp_max_ = timestamp_max_;;
    slice->first_slice_ = first_slice_;
    slice->is_last_slice_ = !hasNextTimeSlice();

    slice->remove_before_time_ = remove_before_time_;
    slice->write_before_time_ = write_before_time_;

    slice->loading_done_ = false;
    slice->processing_done_ = false;
    slice->write_done_ = false;

    ++slice_cnt_;

    return slice;
}

void ReconstructorBase::reset()
{
    loginf << "ReconstructorBase: reset/init";

    dbContent::ReconstructorTarget::globalStats().reset();

    accessor_->clear();
    accessors_.clear();

    slice_cnt_ = 0;
    current_slice_begin_ = {};
    next_slice_begin_ = {};
    timestamp_min_ = {};
    timestamp_max_ = {};
    first_slice_ = false;

    remove_before_time_ = {};
    write_before_time_ = {};

    target_reports_.clear();
    tr_timestamps_.clear();
    tr_ds_.clear();
    targets_.clear();

    assert (acc_estimator_);
    acc_estimator_->init(this);

    cancelled_ = false;
}

/**
 */
void ReconstructorBase::initChainPredictors()
{
    if (chain_predictors_->isInit())
        return;

                //int num_threads = std::max(1, tbb::task_scheduler_init::default_num_threads());

#if TBB_VERSION_MAJOR <= 4
    int num_threads = tbb::task_scheduler_init::default_num_threads(); // TODO PHIL
#else
    int num_threads = oneapi::tbb::info::default_concurrency();
#endif

    assert (num_threads > 0);

    assert(chain_predictors_);

    chain_predictors_->init(referenceCalculatorSettings().kalman_type_assoc,
                            referenceCalculatorSettings().chainEstimatorSettings(),
                            num_threads);
}

/**
 */
void ReconstructorBase::processSlice()
{
    assert (!currentSlice().remove_before_time_.is_not_a_date_time());

    loginf << "ReconstructorBase: processSlice: " << Time::toString(currentSlice().timestamp_min_);

    logdbg << "ReconstructorBase: processSlice: first_slice " << currentSlice().first_slice_;

    processing_ = true;

    if (currentSlice().first_slice_)
    {
       //not needed at the moment
       //initChainPredictors();
    }

    if (!currentSlice().first_slice_)
    {
        logdbg << "ReconstructorBase: processSlice: removing data before "
               << Time::toString(currentSlice().remove_before_time_);

        accessor_->removeContentBeforeTimestamp(currentSlice().remove_before_time_);
    }

    logdbg << "ReconstructorBase: processSlice: adding";

    accessor_->add(currentSlice().data_);

    logdbg << "ReconstructorBase: processSlice: processing slice";

    processSlice_impl();

    processing_ = false;

    logdbg << "ReconstructorBase: processSlice: done";

    currentSlice().processing_done_ = true;

    if (task().debug() && currentSlice().is_last_slice_ )
    {
        auto& stats = dbContent::ReconstructorTarget::globalStats();

        const int Decimals = 3;

        auto perc = [ & ] (size_t num, size_t num_total)
        {
            if (num_total == 0)
                return std::string("0%");
            
            return QString::number((double)num / (double)num_total * 100.0, 'f', Decimals).toStdString() + "%";
        };

        std::string num_chain_updates_valid_p           = perc(stats.num_chain_updates_valid          , stats.num_chain_updates       );
        std::string num_chain_updates_failed_p          = perc(stats.num_chain_updates_failed         , stats.num_chain_updates       );
        std::string num_chain_updates_failed_numeric_p  = perc(stats.num_chain_updates_failed_numeric , stats.num_chain_updates_failed);
        std::string num_chain_updates_failed_badstate_p = perc(stats.num_chain_updates_failed_badstate, stats.num_chain_updates_failed);
        std::string num_chain_updates_failed_other_p    = perc(stats.num_chain_updates_failed_other   , stats.num_chain_updates_failed);
        std::string num_chain_updates_skipped_p         = perc(stats.num_chain_updates_skipped        , stats.num_chain_updates       );
        std::string num_chain_updates_proj_changed_p    = perc(stats.num_chain_updates_proj_changed   , stats.num_chain_added         );

        std::string num_chain_predictions_failed_p          = perc(stats.num_chain_predictions_failed         , stats.num_chain_predictions       );
        std::string num_chain_predictions_failed_numeric_p  = perc(stats.num_chain_predictions_failed_numeric , stats.num_chain_predictions_failed);
        std::string num_chain_predictions_failed_badstate_p = perc(stats.num_chain_predictions_failed_badstate, stats.num_chain_predictions_failed);
        std::string num_chain_predictions_failed_other_p    = perc(stats.num_chain_predictions_failed_other   , stats.num_chain_predictions_failed);
        std::string num_chain_predictions_fixed_p           = perc(stats.num_chain_predictions_fixed          , stats.num_chain_predictions       );
        std::string num_chain_predictions_proj_changed_p    = perc(stats.num_chain_predictions_proj_changed   , stats.num_chain_predictions       );

        std::string num_rec_updates_valid_p           = perc(stats.num_rec_updates_valid          , stats.num_rec_updates       );
        std::string num_rec_updates_failed_p          = perc(stats.num_rec_updates_failed         , stats.num_rec_updates       );
        std::string num_rec_updates_failed_numeric_p  = perc(stats.num_rec_updates_failed_numeric , stats.num_rec_updates_failed);
        std::string num_rec_updates_failed_badstate_p = perc(stats.num_rec_updates_failed_badstate, stats.num_rec_updates_failed);
        std::string num_rec_updates_failed_other_p    = perc(stats.num_rec_updates_failed_other   , stats.num_rec_updates_failed);
        std::string num_rec_updates_skipped_p         = perc(stats.num_rec_updates_skipped        , stats.num_rec_updates       );

        loginf << "ReconstructorBase: processSlice: last slice finished\n"
               << "\n"
               << "Reconstruction Statistics\n"
               << "\n"
               << " * Chain updates:\n"
               << "\n"
               << "   mm added:     " << stats.num_chain_added                                                                  <<  "\n"
               << "   valid:        " << stats.num_chain_updates_valid           << " (" << num_chain_updates_valid_p           << ")\n"
               << "   failed:       " << stats.num_chain_updates_failed          << " (" << num_chain_updates_failed_p          << ")\n"
               << "      numeric:   " << stats.num_chain_updates_failed_numeric  << " (" << num_chain_updates_failed_numeric_p  << ")\n"
               << "      bad state: " << stats.num_chain_updates_failed_badstate << " (" << num_chain_updates_failed_badstate_p << ")\n"
               << "      other:     " << stats.num_chain_updates_failed_other    << " (" << num_chain_updates_failed_other_p    << ")\n"
               << "   skipped:      " << stats.num_chain_updates_skipped         << " (" << num_chain_updates_skipped_p         << ")\n"
               << "   total:        " << stats.num_chain_updates                                                                <<  "\n"
               << "   proj changed: " << stats.num_chain_updates_proj_changed    << " (" << num_chain_updates_proj_changed_p    << ")\n"
               << "\n"
               << " * Chain predictions:\n"
               << "\n" 
               << "   failed:       " << stats.num_chain_predictions_failed          << " ("  << num_chain_predictions_failed_p          << ")\n"
               << "      numeric:   " << stats.num_chain_predictions_failed_numeric  << " ("   << num_chain_predictions_failed_numeric_p << ")\n"
               << "      bad state: " << stats.num_chain_predictions_failed_badstate << " (" << num_chain_predictions_failed_badstate_p  << ")\n"
               << "      other:     " << stats.num_chain_predictions_failed_other    << " ("     << num_chain_predictions_failed_other_p << ")\n"
               << "   fixed:        " << stats.num_chain_predictions_fixed           << " ("   << num_chain_predictions_fixed_p          << ")\n"
               << "   total:        " << stats.num_chain_predictions                                                                     <<  "\n"
               << "   proj changed: " << stats.num_chain_predictions_proj_changed    << " (" << num_chain_predictions_proj_changed_p     << ")\n"
               << "\n"
               << " * Rec updates:\n"
               << "\n" 
               << "   valid:   " << stats.num_rec_updates_valid                << " (" << num_rec_updates_valid_p           << ")\n"
               << "   failed:  " << stats.num_rec_updates_failed               << " (" << num_rec_updates_failed_p          << ")\n"
               << "      numeric:   " << stats.num_rec_updates_failed_numeric  << " (" << num_rec_updates_failed_numeric_p  << ")\n"
               << "      bad state: " << stats.num_rec_updates_failed_badstate << " (" << num_rec_updates_failed_badstate_p << ")\n"
               << "      other:     " << stats.num_rec_updates_failed_other    << " (" << num_rec_updates_failed_other_p    << ")\n"
               << "   skipped: " << stats.num_rec_updates_skipped              << " (" << num_rec_updates_skipped_p         << ")\n"
               << "   total:   " << stats.num_rec_updates                                                                   <<  "\n"
               << "\n"
               << " * Rec smooth steps:\n" 
               << "\n"
               << "   failed: " << stats.num_rec_smooth_failed << "\n"
               << "\n"
               << " * Rec interp steps:\n"
               << "\n" 
               << "   failed:" << stats.num_rec_interp_failed << "\n"
               << "\n";
    }
}

void ReconstructorBase::clearOldTargetReports()
{
    logdbg << "ReconstructorBase: clearOldTargetReports: remove_before_time "
           << Time::toString(currentSlice().remove_before_time_)
           << " size " << target_reports_.size();

    tr_timestamps_.clear();
    tr_ds_.clear();

    for (auto ts_it = target_reports_.begin(); ts_it != target_reports_.end() /* not hoisted */; /* no increment */)
    {
        if (ts_it->second.timestamp_ < currentSlice().remove_before_time_)
        {
            //loginf << "ReconstructorBase: clearOldTargetReports: removing " << Time::toString(ts_it->second.timestamp_);
            ts_it = target_reports_.erase(ts_it);
        }
        else
        {
            //loginf << "ReconstructorBase: clearOldTargetReports: keeping " << Time::toString(ts_it->second.timestamp_);

            ts_it->second.in_current_slice_ = false;

                    // add to lookup structures
            tr_timestamps_.insert({ts_it->second.timestamp_, ts_it->second.record_num_});
            tr_ds_[Number::recNumGetDBContId(ts_it->second.record_num_)]
                  [ts_it->second.ds_id_][ts_it->second.line_id_].push_back(
                          ts_it->second.record_num_);

            ++ts_it;
        }
    }

    loginf << "ReconstructorBase: clearOldTargetReports: size after " << target_reports_.size();

            // clear old data from targets
    for (auto& tgt_it : targets_)
        tgt_it.second.removeOutdatedTargetReports();

}

void ReconstructorBase::createTargetReports()
{
    loginf << "ReconstructorBase: createTargetReports: current_slice_begin "
           << Time::toString(currentSlice().slice_begin_);

    boost::posix_time::ptime ts;
    unsigned long record_num;

    dbContent::targetReport::ReconstructorInfo info;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    accessors_.clear();

            //unsigned int calc_ref_ds_id = Number::dsIdFrom(ds_sac_, ds_sic_);

    std::set<unsigned int> unused_ds_ids = task_.unusedDSIDs();
    std::map<unsigned int, std::set<unsigned int>> unused_lines = task_.unusedDSIDLines();

    auto& ds_man = COMPASS::instance().dataSourceManager();

    for (auto& buf_it : *accessor_)
    {
        assert (dbcont_man.existsDBContent(buf_it.first));
        unsigned int dbcont_id = dbcont_man.dbContent(buf_it.first).id();

        accessors_.emplace(dbcont_id, accessor_->targetReportAccessor(buf_it.first));

        dbContent::TargetReportAccessor& tgt_acc = accessors_.at(dbcont_id);
        unsigned int buffer_size = tgt_acc.size();

        for (unsigned int cnt=0; cnt < buffer_size; cnt++)
        {
            record_num = tgt_acc.recordNumber(cnt);
            ts = tgt_acc.timestamp(cnt);

                    //loginf << "ReconstructorBase: createTargetReports: ts " << Time::toString(ts);

            if (!tgt_acc.position(cnt))
                continue;

            if (ts >= currentSlice().slice_begin_) // insert
            {
                // base info
                info.buffer_index_ = cnt;
                info.record_num_ = record_num;
                info.dbcont_id_ = dbcont_id;
                info.ds_id_ = tgt_acc.dsID(cnt);
                info.line_id_ = tgt_acc.lineID(cnt);
                info.timestamp_ = ts;

                        // reconstructor info
                info.in_current_slice_ = true;

                info.is_calculated_reference_ = ds_man.hasDBDataSource(info.ds_id_) && 
                                                ds_man.dbDataSource(info.ds_id_).sac() == ReconstructorBaseSettings::REC_DS_SAC &&
                                                ds_man.dbDataSource(info.ds_id_).sic() == ReconstructorBaseSettings::REC_DS_SIC;

                info.acad_ = tgt_acc.acad(cnt);
                info.acid_ = tgt_acc.acid(cnt);

                info.mode_a_code_ = tgt_acc.modeACode(cnt);

                info.track_number_ = tgt_acc.trackNumber(cnt);
                info.track_begin_ = tgt_acc.trackBegin(cnt);
                info.track_end_ = tgt_acc.trackEnd(cnt);

                info.position_ = tgt_acc.position(cnt);
                info.position_accuracy_ = tgt_acc.positionAccuracy(cnt);

                info.do_not_use_position_ = !info.position().has_value()
                                            || (unused_ds_ids.count(info.ds_id_)
                                                || (unused_lines.count(info.ds_id_) && unused_lines.at(info.ds_id_).count(info.line_id_)));

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

                tr_ds_[dbcont_id][info.ds_id_][info.line_id_].push_back(record_num);
            }
            else // update buffer_index_
            {
                if (ts < currentSlice().remove_before_time_)
                {
                    logerr << "ReconstructorBase: createTargetReports: old data not removed ts "
                           << Time::toString(ts)
                           << " dbcont " << buf_it.first
                           << " buffer_size " << buffer_size
                           << " remove before " << Time::toString(currentSlice().remove_before_time_);
                }

                assert (ts >= currentSlice().remove_before_time_);

                if (!target_reports_.count(record_num))
                    logerr << "ReconstructorBase: createTargetReports: missing prev ts " << Time::toString(ts);

                assert (target_reports_.count(record_num));
                assert (target_reports_.at(record_num).record_num_ == record_num); // be sure

                target_reports_.at(record_num).buffer_index_ = cnt;
                target_reports_.at(record_num).in_current_slice_ = false;

                assert (target_reports_.at(record_num).timestamp_ == ts); // be very sure
            }
        }
    }
}

void ReconstructorBase::removeTargetAssociationsNewerThan(const boost::posix_time::ptime& ts)
{
            // remove target reports from targets & clean
    for (auto& tgt_it : targets_)
        tgt_it.second.removeTargetReportsNewerThan(ts);
}

std::map<unsigned int, std::map<unsigned long, unsigned int>> ReconstructorBase::createAssociations()
{
    loginf << "ReconstructorBase: createAssociations";

    std::map<unsigned int, std::map<unsigned long, unsigned int>> associations;
    unsigned int num_assoc {0};

    for (auto& tgt_it : targets_)
    {
        for (auto rn_it : tgt_it.second.target_reports_)
        {
            assert (target_reports_.count(rn_it));

            dbContent::targetReport::ReconstructorInfo& tr = target_reports_.at(rn_it);

            if (tr.timestamp_ < currentSlice().write_before_time_) // tr.in_current_slice_
            {
                associations[Number::recNumGetDBContId(rn_it)][rn_it] = tgt_it.first;
                ++num_assoc;
            }
        }
        tgt_it.second.associations_written_ = true;

        tgt_it.second.updateCounts();
    }

    loginf << "ReconstructorBase: createAssociations: done with " << num_assoc << " associated";

    return associations;
}

std::map<std::string, std::shared_ptr<Buffer>> ReconstructorBase::createAssociationBuffers(
    std::map<unsigned int, std::map<unsigned long,unsigned int>> associations)
{
    logdbg << "ReconstructorBase: createAssociationBuffers";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

            // write association info to buffers

    std::map<std::string, std::shared_ptr<Buffer>> assoc_data;

    for (auto& cont_assoc_it : associations) // dbcontent -> rec_nums
    {
        unsigned int num_associated {0};
        unsigned int num_not_associated {0};

        unsigned int dbcontent_id = cont_assoc_it.first;
        string dbcontent_name = dbcontent_man.dbContentWithId(cont_assoc_it.first);
        //DBContent& dbcontent = dbcontent_man.dbContent(dbcontent_name);

        std::map<unsigned long, unsigned int>& tr_associations = cont_assoc_it.second;

        logdbg << "ReconstructorBase: createAssociationBuffers: db content " << dbcontent_name;

        string rec_num_name =
            dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name).name();

        string utn_name =
            dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).getFor(dbcontent_name).name();

        PropertyList properties;
        properties.addProperty(utn_name,  DBContent::meta_var_utn_.dataType());
        properties.addProperty(rec_num_name,  DBContent::meta_var_rec_num_.dataType());

        assoc_data [dbcontent_name].reset(new Buffer(properties));

        shared_ptr<Buffer> buffer  = assoc_data.at(dbcontent_name);

        NullableVector<unsigned int>& utn_col_vec = buffer->get<unsigned int>(utn_name);
        NullableVector<unsigned long>& rec_num_col_vec = buffer->get<unsigned long>(rec_num_name);

        assert (tr_ds_.count(dbcontent_id));

        unsigned int buf_cnt = 0;
        for (auto& ds_it : tr_ds_.at(dbcontent_id))  // iterate over all rec nums
        {
            for (auto& line_it : ds_it.second)
            {
                for (auto& rn_it : line_it.second)
                {
                    assert (target_reports_.count(rn_it));

                    if (target_reports_.at(rn_it).timestamp_ >= currentSlice().write_before_time_)
                        continue;

                    rec_num_col_vec.set(buf_cnt, rn_it);

                    if (tr_associations.count(rn_it))
                    {
                        utn_col_vec.set(buf_cnt, tr_associations.at(rn_it));
                        ++num_associated;
                    }
                    else
                        ++num_not_associated;
                    // else null

                    ++buf_cnt;
                }
            }
        }

        logdbg << "ReconstructorBase: createAssociationBuffers: dcontent " << dbcontent_name
               <<  " assoc " << num_associated << " not assoc " << num_not_associated
               << " buffer size " << buffer->size();

        logdbg << "ReconstructorBase: createAssociationBuffers: dcontent " << dbcontent_name << " done";
    }

    logdbg << "ReconstructorBase: createAssociationBuffers: done";

    return assoc_data;
}

std::map<std::string, std::shared_ptr<Buffer>> ReconstructorBase::createReferenceBuffers()
{
    logdbg << "ReconstructorBase: createReferenceBuffers: num " << targets_.size();

    std::shared_ptr<Buffer> buffer;

    for (auto& tgt_it : targets_)
    {
        if (!buffer)
            buffer = tgt_it.second.getReferenceBuffer(); // also updates count
        else
        {
            auto tmp = tgt_it.second.getReferenceBuffer();
            buffer->seizeBuffer(*tmp);
        }
    }

    if (buffer && buffer->size())
    {
        NullableVector<boost::posix_time::ptime>& ts_vec = buffer->get<boost::posix_time::ptime>(
            DBContent::meta_var_timestamp_.name());

        logdbg << "ReconstructorBase: createReferenceBuffers: buffer size " << buffer->size()
               << " ts min " << Time::toString(ts_vec.get(0))
               << " max " << Time::toString(ts_vec.get(ts_vec.size()-1));

        DataSourceManager& src_man = COMPASS::instance().dataSourceManager();

        unsigned int ds_id = Number::dsIdFrom(base_settings_.ds_sac, base_settings_.ds_sic);

        if (!src_man.hasConfigDataSource(ds_id))
        {
            logdbg << "ReconstructorBase: createReferenceBuffers: creating data source";

            src_man.createConfigDataSource(ds_id);
            assert (src_man.hasConfigDataSource(ds_id));
        }

        dbContent::ConfigurationDataSource& src = src_man.configDataSource(ds_id);

        src.name(base_settings_.ds_name);
        src.dsType("RefTraj"); // same as dstype

        return std::map<std::string, std::shared_ptr<Buffer>> {{buffer->dbContentName(), buffer}};
    }
    else
    {
        logdbg << "ReconstructorBase: createReferenceBuffers: empty buffer";

        return std::map<std::string, std::shared_ptr<Buffer>> {};
    }
}

bool ReconstructorBase::processing() const
{
    return processing_;
}

void ReconstructorBase::cancel()
{
    cancelled_ = true;
}

void ReconstructorBase::saveTargets()
{
    loginf << "ReconstructorBase: saveTargets: num " << targets_.size();

    processing_ = true;

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();

    cont_man.createNewTargets(targets_);

    cont_man.saveTargets();

    processing_ = false;

    logdbg << "ReconstructorBase: saveTargets: done";
}

const dbContent::TargetReportAccessor& ReconstructorBase::accessor(
    const dbContent::targetReport::ReconstructorInfo& tr) const
{
    assert (accessors_.count(tr.dbcont_id_));
    return accessors_.at(tr.dbcont_id_);
}

ReconstructorTask& ReconstructorBase::task() const
{
    return task_;
}

ReconstructorBase::DataSlice& ReconstructorBase::currentSlice()
{
    return task_.processingSlice();
}

float ReconstructorBase::qVarForAltitude(bool fl_unknown, 
                                         bool fl_ground, 
                                         unsigned int fl_index,
                                         bool dynamic,
                                         const ReferenceCalculatorSettings::ProcessNoise& Q_std) const
{
    if (!dynamic)
        return Q_std.Q_std_static * Q_std.Q_std_static;
    if (fl_unknown)
        return Q_std.Q_std_unknown * Q_std.Q_std_unknown;
    if (fl_ground)
        return Q_std.Q_std_ground * Q_std.Q_std_ground;

    assert (ref_calc_settings_.Q_altitude_min_ft < ref_calc_settings_.Q_altitude_max_ft);

    double alt_ft       = std::max(ref_calc_settings_.Q_altitude_min_ft, std::min(ref_calc_settings_.Q_altitude_max_ft, fl_index * 100.0));
    double t            = (alt_ft - ref_calc_settings_.Q_altitude_min_ft) / (ref_calc_settings_.Q_altitude_max_ft - ref_calc_settings_.Q_altitude_min_ft);
    double Q_std_interp = (1.0 - t) * Q_std.Q_std_ground + t * Q_std.Q_std_air;

    return Q_std_interp * Q_std_interp;
}

void ReconstructorBase::createMeasurement(reconstruction::Measurement& mm, 
                                          const dbContent::targetReport::ReconstructorInfo& ri,
                                          const dbContent::ReconstructorTarget* target)
{
    mm = {};

    mm.source_id = ri.record_num_;
    mm.t         = ri.timestamp_;
    
    auto pos = ri.position();
    assert(pos.has_value());

    auto vel = ri.velocity_;

    auto pos_acc = acc_estimator_->positionAccuracy(ri);

    if (pos_acc.x_stddev_ == 0 || pos_acc.y_stddev_ == 0)
    {
        logerr << "ReconstructorBase: createMeasurement: stddevs 0,  x " << pos_acc.x_stddev_
               << " y " << pos_acc.y_stddev_ << " ds_id " << ri.ds_id_ << " dbcont_id " << ri.dbcont_id_;
        assert (false);
    }

    auto vel_acc = acc_estimator_->velocityAccuracy(ri);
    auto acc_acc = acc_estimator_->accelerationAccuracy(ri);

    //position
    mm.lat = pos.value().latitude_;
    mm.lon = pos.value().longitude_;

    //height information
    if (target)
    {
        bool fl_unknown, fl_ground;
        unsigned int fl_index;
        std::tie(fl_unknown, fl_ground, fl_index) = target->getAltitudeState(ri.timestamp_, Time::partialSeconds(base_settings_.max_time_diff_));

        //compute measurement-specific process noise from altitude state
        mm.Q_var        = qVarForAltitude(fl_unknown, fl_ground, fl_index, ref_calc_settings_.dynamic_process_noise, ref_calc_settings_.Q_std);
        mm.Q_var_interp = qVarForAltitude(fl_unknown, fl_ground, fl_index, ref_calc_settings_.dynamic_process_noise, ref_calc_settings_.resample_Q_std);
    }

    //velocity
    if (vel.has_value())
    {
        auto speed_vec = Utils::Number::speedAngle2SpeedVec(vel->speed_, vel->track_angle_);

        mm.vx = speed_vec.first;
        mm.vy = speed_vec.second;

        //@TODO: vz?
    }

    //@TODO: acceleration?

    //accuracies
    mm.x_stddev = pos_acc.x_stddev_;
    mm.y_stddev = pos_acc.y_stddev_;
    mm.xy_cov   = pos_acc.xy_cov_;

    mm.vx_stddev = vel_acc.vx_stddev_;
    mm.vy_stddev = vel_acc.vy_stddev_;

    mm.ax_stddev = acc_acc.ax_stddev_;
    mm.ay_stddev = acc_acc.ay_stddev_;
}

void ReconstructorBase::createMeasurement(reconstruction::Measurement& mm,
                                          unsigned long rec_num,
                                          const dbContent::ReconstructorTarget* target)
{
    auto it = target_reports_.find(rec_num);
    assert(it != target_reports_.end());

    createMeasurement(mm, it->second, target);
}

reconstruction::KalmanChainPredictors& ReconstructorBase::chainPredictors()
{
    assert(chain_predictors_);

    return *chain_predictors_;
}

boost::optional<unsigned int> ReconstructorBase::utnForACAD(unsigned int acad)
{
    auto it = std::find_if(targets_.begin(), targets_.end(),
                           [acad](const std::pair<const unsigned int, dbContent::ReconstructorTarget> & t) -> bool {
                               return t.second.hasACAD(acad);
                           });

    if (it != targets_.end())
        return it->first;
    else
        return {};
}
