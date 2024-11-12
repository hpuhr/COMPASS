#include "reconstructorassociatorbase.h"
#include "logger.h"
#include "global.h"
#include "stringconv.h"
#include "timeconv.h"
#include "util/tbbhack.h"
#include "reconstructortask.h"
#include "number.h"
#include "util/system.h"
#include "kalman_chain.h"

#define FIND_UTN_FOR_TARGET_MT
#define FIND_UTN_FOR_TARGET_REPORT_MT

using namespace std;
using namespace dbContent;
using namespace Utils;

ReconstructorAssociatorBase::ReconstructorAssociatorBase()
{
}

void ReconstructorAssociatorBase::associateNewData()
{
    loginf << "ReconstructorAssociatorBase: associateNewData: slice " << reconstructor().currentSlice().slice_count_
           << " run " << reconstructor().currentSlice().run_count_;

    max_time_diff_ = Time::partialSeconds(reconstructor().settings().max_time_diff_);

    assert (!unassoc_rec_nums_.size());

    if (reconstructor().isCancelled())
        return;

    boost::posix_time::ptime start_time = boost::posix_time::microsec_clock::local_time();

    associateTargetReports();

    time_assoc_trs_ += boost::posix_time::microsec_clock::local_time() - start_time;

    reconstructor().targets_container_.checkACADLookup();

    if (reconstructor().isCancelled())
        return;

    start_time = boost::posix_time::microsec_clock::local_time();

    retryAssociateTargetReports(); // try once

    time_retry_assoc_trs_ += boost::posix_time::microsec_clock::local_time() - start_time;

    if (reconstructor().isCancelled())
        return;

    reconstructor().targets_container_.checkACADLookup();

    if (reconstructor().isCancelled())
        return;

    start_time = boost::posix_time::microsec_clock::local_time();

    selfAssociateNewUTNs(); // self-associate created utns

    time_assoc_new_utns_ += boost::posix_time::microsec_clock::local_time() - start_time;

    if (reconstructor().isCancelled())
        return;

    reconstructor().targets_container_.checkACADLookup();

    if (reconstructor().isCancelled())
        return;

    start_time = boost::posix_time::microsec_clock::local_time();

    retryAssociateTargetReports();  // try second time

    time_retry_assoc_trs_ += boost::posix_time::microsec_clock::local_time() - start_time;

    countUnAssociated();

    if (reconstructor().isCancelled())
        return;

    // clear new flags
    for (auto& tgt_it : reconstructor().targets_container_.targets_)
        tgt_it.second.created_in_current_slice_ = false;

    unassoc_rec_nums_.clear();

    loginf << "ReconstructorAssociatorBase: associateNewData: time_assoc_trs " << Time::toString(time_assoc_trs_)
           << " time_assoc_new_utns " << Time::toString(time_assoc_new_utns_)
           << " time_retry_assoc_trs " << Time::toString(time_retry_assoc_trs_);

    if (reconstructor().currentSlice().is_last_slice_)
        loginf << "ReconstructorAssociatorBase: associateNewData: done, num_merges " << num_merges_;
}

void ReconstructorAssociatorBase::reset()
{
    logdbg << "ReconstructorAssociatorBase: reset";

    reconstructor().targets_container_.utn_vec_.clear();
    reconstructor().targets_container_.acad_2_utn_.clear();
    reconstructor().targets_container_.acid_2_utn_.clear();
    reconstructor().targets_container_.tn2utn_.clear();

    unassoc_rec_nums_.clear();
    assoc_counts_.clear();

    num_merges_ = 0;

    time_assoc_trs_ = {};
    time_assoc_new_utns_ = {};
    time_retry_assoc_trs_ = {};
}

void ReconstructorAssociatorBase::associateTargetReports()
{
    loginf << "ReconstructorAssociatorBase: associateTargetReports";

    unsigned long rec_num;
    int utn;

    bool do_debug = false;

    reconstructor().targets_container_.checkACADLookup();

    bool is_unreliable_primary_only;

    for (auto& ts_it : reconstructor().tr_timestamps_)
    {
        if (reconstructor().isCancelled())
            return;

        rec_num = ts_it.second;

        assert (reconstructor().target_reports_.count(rec_num));

        do_debug = reconstructor().task().debugSettings().debugRecNum(rec_num);

        if (do_debug)
            loginf << "DBG tr " << rec_num;

        dbContent::targetReport::ReconstructorInfo& tr = reconstructor().target_reports_.at(rec_num);

        if (!tr.in_current_slice_)
        {
            if (do_debug)
                loginf << "DBG tr " << rec_num << " not in current slice";

            continue;
        }

        if (reconstructor().acc_estimator_->canCorrectPosition(tr))
        {
            if (do_debug)
                loginf << "DBG correcting position";

            reconstructor().acc_estimator_->correctPosition(tr);
        }

        utn = -1;

        is_unreliable_primary_only = tr.dbcont_id_ != 62 && tr.dbcont_id_  != 255 && tr.isPrimaryOnlyDetection();

        if (do_debug)
            loginf << "is_unreliable_primary_only " << is_unreliable_primary_only;

        if (!is_unreliable_primary_only) // if unreliable primary only, delay association until retry
        {
            if (do_debug)
                loginf << "DBG !is_unreliable_primary_only finding UTN";

            utn = findUTNFor(tr);

            if (do_debug)
                loginf << "DBG !is_unreliable_primary_only got UTN " << utn;
        }

        if (utn != -1) // estimate accuracy and associate
        {
            if (do_debug)
                loginf << "DBG associating to UTN " << utn;


            associate(tr, utn);
        }
        else // not associated
        {
            if (do_debug)
                loginf << "DBG adding to unassoc_rec_nums_";

            unassoc_rec_nums_.push_back(rec_num);
        }
    }
    if (do_debug)
        loginf << "DBG associateTargetReports done";
}

void ReconstructorAssociatorBase::associateTargetReports(std::set<unsigned int> dbcont_ids)
{
    loginf << "ReconstructorAssociatorBase: associateTargetReports: dbcont_ids " << String::compress(dbcont_ids, ',');

    unsigned long rec_num;
    int utn;

    bool do_debug;

    reconstructor().targets_container_.checkACADLookup();

    for (auto& ts_it : reconstructor().tr_timestamps_)
    {
        if (reconstructor().isCancelled())
            return;

        rec_num = ts_it.second;

        assert (reconstructor().target_reports_.count(rec_num));

        do_debug = reconstructor().task().debugSettings().debugRecNum(rec_num);

        if (do_debug)
            loginf << "DBG tr " << rec_num;

        dbContent::targetReport::ReconstructorInfo& tr = reconstructor().target_reports_.at(rec_num);

        if (!dbcont_ids.count(tr.dbcont_id_))
            continue;

        if (!tr.in_current_slice_)
        {
            if(do_debug)
                loginf << "DBG tr " << rec_num << " not in current slice";

            continue;
        }

        utn = findUTNFor(tr);

        if (utn != -1) // estimate accuracy and associate
            associate(tr, utn);
        else // not associated
            unassoc_rec_nums_.push_back(rec_num);
    }

}

void ReconstructorAssociatorBase::selfAssociateNewUTNs()
{
    loginf << "ReconstructorAssociatorBase: selfAssociateNewUTNs";

    unsigned int loop_cnt {0};

    set<unsigned int> utns_to_remove;

RESTART_SELF_ASSOC:

    if (!reconstructor().isCancelled())
    {
        logdbg << "ReconstructorAssociatorBase: selfAssociateNewUTNs: loop " << loop_cnt;

        reconstructor().targets_container_.checkACADLookup();

        for (auto utn : reconstructor().targets_container_.utn_vec_)
        {
            if (utns_to_remove.count(utn))
                continue;

            assert (reconstructor().targets_container_.targets_.count(utn));
            dbContent::ReconstructorTarget& target = reconstructor().targets_container_.targets_.at(utn);

            if (!target.target_reports_.size()) // can not compare
                continue;

            logdbg << "ReconstructorAssociatorBase: selfAssociateNewUTNs: checking utn " << utn;

            std::vector<unsigned int> utns_to_merge = findUTNsForTarget(utn, utns_to_remove);

            if (utns_to_merge.size())
            {
                loginf << "ReconstructorAssociatorBase: selfAssociateNewUTNs: loop " << loop_cnt
                       << ": found utn " << utn << " merges with " << String::compress(utns_to_merge,',');

                for (auto other_utn : utns_to_merge)
                {
                    assert (reconstructor().targets_container_.targets_.count(other_utn));

                    dbContent::ReconstructorTarget& other_target =
                        reconstructor().targets_container_.targets_.at(other_utn);

                    loginf << "ReconstructorAssociatorBase: selfAssociateNewUTNs: loop " << loop_cnt
                           << ": merging utn " << utn << " '" << target.asStr()
                           << "' with " << other_utn << " '" << other_target.asStr() << "'";

                    // move target reports
                    target.addTargetReports(other_target);

                    // schedule remove from targets

                    utns_to_remove.insert(other_utn);

                    reconstructor().targets_container_.replaceInLookup(other_utn, utn);

                    ++num_merges_;
                }
            }
        }

        // remove scheduled from targets
        if (utns_to_remove.size())
        {
            for (auto other_utn : utns_to_remove)
            {
                assert (reconstructor().targets_container_.targets_.count(other_utn));

                reconstructor().targets_container_.removeUTN(other_utn);
            }

            utns_to_remove.clear();

            ++loop_cnt;

            reconstructor().targets_container_.checkACADLookup();

            goto RESTART_SELF_ASSOC; // here we go again
        }
    }

    loginf << "ReconstructorAssociatorBase: selfAssociateNewUTNs: done at loop cnt " << loop_cnt;
}

void ReconstructorAssociatorBase::retryAssociateTargetReports()
{
    loginf << "ReconstructorAssociatorBase: retryAssociateTargetReports";

    if (!unassoc_rec_nums_.size())
        return;

    unsigned long rec_num;
    unsigned int dbcont_id;
    int utn;

    bool do_debug;

    unsigned int assocated_cnt{0};


    for (auto rec_num_it = unassoc_rec_nums_.rbegin(); rec_num_it != unassoc_rec_nums_.rend();)
    {
        if (reconstructor().isCancelled())
            return;

        rec_num = *rec_num_it;

        dbcont_id = Number::recNumGetDBContId(rec_num);

        assert (reconstructor().target_reports_.count(rec_num));

        do_debug = reconstructor().task().debugSettings().debugRecNum(rec_num);

        if (do_debug)
            loginf << "DBG tr " << rec_num;

        dbContent::targetReport::ReconstructorInfo& tr = reconstructor().target_reports_.at(rec_num);

        if (!tr.in_current_slice_)
        {
            if(do_debug)
                loginf << "DBG tr " << rec_num << " not in current slice";

            ++rec_num_it;
            continue;
        }

        // check if should have been associated
        assert (!tr.acad_);
        assert (!tr.acid_);

        if (dbcont_id == 62 || dbcont_id == 255)
            assert (!tr.track_number_);

        utn = findUTNByModeACPos (tr);

        if (utn != -1) // estimate accuracy and associate
        {
            associate(tr, utn);

            ++assocated_cnt;

            // Erase the element and update the iterator
            rec_num_it = std::vector<unsigned long>::reverse_iterator(
                unassoc_rec_nums_.erase(std::next(rec_num_it).base()));
        }
        else
            ++rec_num_it;
    }

    loginf << "ReconstructorAssociatorBase: retryAssociateTargetReports: done with count " << assocated_cnt;
}

void ReconstructorAssociatorBase::associate(
    dbContent::targetReport::ReconstructorInfo& tr, int utn)
{
    assert (utn >= 0);

    bool do_debug = reconstructor().task().debugSettings().debugRecNum(tr.record_num_)
                    || reconstructor().task().debugSettings().debugUTN(utn);

    unsigned int dbcont_id  = Number::recNumGetDBContId(tr.record_num_);
    //AccuracyEstimatorBase::AssociatedDistance dist;

    if (!reconstructor().targets_container_.targets_.count(utn))
        logerr << "ReconstructorAssociatorBase: associate: utn " << utn << " missing";

    // add associated target reports
    assert (reconstructor().targets_container_.targets_.count(utn));

    if(do_debug)
        loginf << "DBG tr " << tr.asStr() << " assoc to utn "
               << reconstructor().targets_container_.targets_.at(utn).asStr();

    // check if position usable
    // if (do_debug)
    //     loginf << "DBG validate";

    reconstructor().acc_estimator_->validate(tr); // do counting

    // if (do_debug)
    //     loginf << "DBG outlier detect";

    tr.is_pos_outlier_ = false;

    if (!tr.doNotUsePosition())
        reconstructor().acc_estimator_->doOutlierDetection(tr, utn, do_debug); //124976,129072

    // if (do_debug)
    //     loginf << "DBG addTargetReport";

    reconstructor().targets_container_.targets_.at(utn).addTargetReport(tr.record_num_);

    // if (do_debug)
    //     loginf << "DBG add to lookups";

    reconstructor().targets_container_.addToLookup(utn, tr);

    assoc_counts_[tr.ds_id_][dbcont_id].first++;

    // if (do_debug)
    //     loginf << "DBG postAssociate";

    postAssociate (tr, utn);
}



void ReconstructorAssociatorBase::countUnAssociated()
{
    for (auto rn_it : unassoc_rec_nums_)
    {
        if (reconstructor().isCancelled())
            return;

        unsigned long rec_num;
        unsigned int dbcont_id;

        rec_num = rn_it;

        dbcont_id = Number::recNumGetDBContId(rec_num);

        assert (reconstructor().target_reports_.count(rec_num));

        dbContent::targetReport::ReconstructorInfo& tr = reconstructor().target_reports_.at(rec_num);

        assoc_counts_[tr.ds_id_][dbcont_id].second++;
    }
}

int ReconstructorAssociatorBase::findUTNFor (dbContent::targetReport::ReconstructorInfo& tr)
{
    int utn {-1};

    bool do_debug = reconstructor().task().debugSettings().debugRecNum(tr.record_num_);

    unsigned int dbcont_id = Number::recNumGetDBContId(tr.record_num_);

    vector<tuple<bool, unsigned int, double>> results;
    const boost::posix_time::time_duration track_max_time_diff =
        Time::partialSeconds(reconstructor().settings().track_max_time_diff_);

    assert (reconstructor().targets_container_.targets_.size() == reconstructor().targets_container_.utn_vec_.size());

START_TR_ASSOC:

    utn = -1; // not yet found

    // try by mode-s address or trustworthy track number
    // create new if not already existing
    if (tr.acad_
        || tr.acid_
        || (tr.track_number_ && (dbcont_id == 62 || dbcont_id == 255)))
    {
        // find utn

        if (reconstructor().targets_container_.canAssocByACAD(tr, do_debug)) // already exists
            utn = reconstructor().targets_container_.assocByACAD(tr, do_debug);
        else if (reconstructor().targets_container_.canAssocByACID(tr, do_debug)) // already exists
            utn = reconstructor().targets_container_.assocByACID(tr, do_debug);
        else if (reconstructor().targets_container_.canAssocByTrackNumber(tr, do_debug))
        {
            utn = reconstructor().targets_container_.assocByTrackNumber(tr, track_max_time_diff, do_debug);

//            if (utn == -1) TODO HP
//            {
//                if (do_debug)
//                    loginf << "DBG restart assoc due to track_num disassoc";

//                assert (!reconstructor().targets_container_.canAssocByTrackNumber(tr, do_debug));

//                goto START_TR_ASSOC;
//            }

            if (do_debug)
                loginf << "DBG assoc by track_num, utn " << utn;

            if (utn != -1 && reconstructor().settings().do_track_number_disassociate_using_distance_
                && canGetPositionOffsetTR(tr, reconstructor().targets_container_.targets_.at(utn)))
            {
                assert (reconstructor().targets_container_.targets_.count(utn));

                if (do_debug)
                    loginf << "DBG stored utn in tn2utn_ checking position offset";

                // check for position offsets
                boost::optional<bool> check_result = checkTrackPositionOffsetAcceptable(
                    tr, utn, true, do_debug);

                if (check_result && !*check_result)
                {
                    if (do_debug)
                        loginf << "DBG stored utn in tn2utn_ position offset not acceptable " << *check_result;

                    reconstructor().targets_container_.eraseTrackNumberLookup(tr);

                    goto START_TR_ASSOC;
                }
            }

            // do assoc
            if (do_debug)
                loginf << "DBG use stored utn in tn2utn_: " << utn;
        }
        else // not yet existing, create & add target findUTNByModeACPosOrCreateNewTarget(tr)
        {
            // check if position match to other target would exist
            utn = findUTNByModeACPos (tr);

            if (utn == -1)
            {
                // create new and add
                utn = reconstructor().targets_container_.createNewTarget(tr);

                if (reconstructor().task().debugSettings().debugUTN(utn))
                    loginf << "created new utn " << utn << " with no mode AC match using tr " << tr.asStr();

                if (do_debug)
                    loginf << "DBG use mode a/c/pos assoc to new utn " << utn;

                assert (reconstructor().targets_container_.targets_.count(utn));
            }
            else
            {
                assert (reconstructor().targets_container_.targets_.count(utn));

                if (do_debug)
                    loginf << "DBG use mode a/c/pos assoc to existing utn " << utn;
            }
        }
        assert (utn != -1);

        if(do_debug)
            loginf << "DBG tr " << tr.record_num_ << " utn by acad/acid/track num";
    }
    else // not associated by trustworty id
    {
        assert (utn == -1);

        if(do_debug)
            loginf << "DBG tr " << tr.record_num_ << " no utn by acad, doing mode a/c + pos";

        utn = findUTNByModeACPos (tr);

        if (utn != -1)
            assert (reconstructor().targets_container_.targets_.count(utn));
    }

    return utn;
}

int ReconstructorAssociatorBase::findUTNByModeACPos (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    unsigned int num_targets = reconstructor().targets_container_.targets_.size();
    assert (reconstructor().targets_container_.utn_vec_.size() == num_targets);

    vector<tuple<bool, unsigned int, double>> results;
    vector<reconstruction::PredictionStats> prediction_stats;

    results.resize(num_targets);
    prediction_stats.resize(num_targets);
    boost::posix_time::ptime timestamp = tr.timestamp_;

    const float max_altitude_diff = reconstructor().settings().max_altitude_diff_;

    bool do_debug = reconstructor().task().debugSettings().debugRecNum(tr.record_num_);

    if (do_debug)
        loginf << "ReconstructorAssociatorBase: findUTNByModeACPos: rn " << tr.record_num_;

#ifdef FIND_UTN_FOR_TARGET_REPORT_MT
    tbb::parallel_for(uint(0), num_targets, [&](unsigned int target_cnt)
#else
    for (unsigned int target_cnt = 0; target_cnt < num_targets; ++target_cnt)
#endif
                      {
                          unsigned int other_utn = reconstructor().targets_container_.utn_vec_.at(target_cnt);
                          bool do_other_debug = false; //debug_utns.count(other_utn);

                          ReconstructorTarget& other = reconstructor().targets_container_.targets_.at(other_utn);

                          results[target_cnt] = tuple<bool, unsigned int, double>(false, other.utn_, 0);

                          if (tr.acad_ && other.hasACAD()) // has to be covered outside
#ifdef FIND_UTN_FOR_TARGET_REPORT_MT
                              return;
#else
            continue;
#endif

                          if (!other.isTimeInside(timestamp, max_time_diff_))
                          {
#ifdef FIND_UTN_FOR_TARGET_REPORT_MT
                              return;
#else
            continue;
#endif
                          }

                          if (!other.canPredict(timestamp))
#ifdef FIND_UTN_FOR_TARGET_REPORT_MT
                              return;
#else
            continue;
#endif

                          bool mode_a_checked = false;
                          bool mode_a_verified = false;
                          bool mode_c_checked = false;

                          if (tr.mode_a_code_ || tr.barometric_altitude_) // mode a/c based
                          {
                              // check mode a code

                              if (tr.mode_a_code_)
                              {
                                  ComparisonResult ma_res = other.compareModeACode(tr, max_time_diff_, do_debug);

                                  if (ma_res == ComparisonResult::DIFFERENT)
                                  {
                                      //target_cnt++;
#ifdef FIND_UTN_FOR_TARGET_REPORT_MT
                                      return;
#else
                    continue;
#endif
                                  }

                                  mode_a_checked = true;
                                  mode_a_verified = ma_res == ComparisonResult::SAME;
                              }

                              if (do_debug)
                                  loginf << "DBG tr " << tr.record_num_ << " other_utn "
                                         << other_utn << ": possible mode a match, verified "
                                         << mode_a_verified;


                              // check mode c code
                              if (tr.barometric_altitude_)
                              {
                                  ComparisonResult mc_res = other.compareModeCCode(
                                      tr, max_time_diff_, max_altitude_diff, false);

                                  if (mc_res == ComparisonResult::DIFFERENT)
                                  {
                                      //target_cnt++;
#ifdef FIND_UTN_FOR_TARGET_REPORT_MT
                                      return;
#else
                    continue;
#endif
                                  }

                                  mode_c_checked = true;
                              }

                              if (do_debug)
                                  loginf << "DBG tr " << tr.record_num_ << " other_utn "
                                         << other_utn << ": possible mode c match";
                          }

                          if (do_debug)
                              loginf << "DBG tr " << tr.record_num_ << " other_utn "
                                     << other_utn << ": mode_a_checked " << mode_a_checked
                                     << " mode_a_verified " << mode_a_verified
                                     << " mode_c_checked " << mode_c_checked;

                          // check positions

                          double distance_m{0}, tgt_est_std_dev{0}, tr_est_std_dev{0};

                          if (!canGetPositionOffsetTR(tr, other))
#ifdef FIND_UTN_FOR_TARGET_REPORT_MT
                              return;
#else
            continue;
#endif
                          auto pos_offs = getPositionOffsetTR(tr, other, do_debug, {}, &prediction_stats[ target_cnt ]);

                          if (!pos_offs.has_value()) 
#ifdef FIND_UTN_FOR_TARGET_REPORT_MT
                              return;
#else
            continue;
#endif

                          std::tie(distance_m, tgt_est_std_dev, tr_est_std_dev) = pos_offs.value();

                          boost::optional<std::pair<bool, double>> check_ret = calculatePositionOffsetScore(
                              tr, other_utn, distance_m, tgt_est_std_dev, tr_est_std_dev, mode_a_verified, do_debug);

                          if (check_ret && check_ret->first)
                          {
                              results[target_cnt] = tuple<bool, unsigned int, double> (
                                  true, other.utn_, check_ret->second);
                          }
                      }
#ifdef FIND_UTN_FOR_TARGET_REPORT_MT
                      );
#endif

    //log failed predictions
    for (const auto& s : prediction_stats)
        ReconstructorTarget::addPredictionToGlobalStats(s);

    // find best match
    bool usable;
    unsigned int other_utn;

    bool first = true;
    unsigned int best_other_utn;
    double best_mahalanobis_dist;
    double mahalanobis_dist;

    for (auto& res_it : results) // usable, other utn, num updates, avg distance
    {
        tie(usable, other_utn, mahalanobis_dist) = res_it;

        if (!usable)
            continue;

        if (first || mahalanobis_dist < best_mahalanobis_dist)
        {
            best_other_utn = other_utn;
            best_mahalanobis_dist = mahalanobis_dist;

            first = false;
        }
    }

    if (!first)
    {
        if (do_debug)
        {
            loginf << "DBG tr " << tr.record_num_ << " other_utn "
                   << other_utn << ": match with best_mahalanobis_dist " << best_mahalanobis_dist;
        }

        return best_other_utn;
    }

    return -1;
}

std::vector<unsigned int> ReconstructorAssociatorBase::findUTNsForTarget (
    unsigned int utn, const std::set<unsigned int>& utns_to_ignore)
{
    if (!reconstructor().targets_container_.targets_.size()) // check if targets exist
        return {};

    assert (!utns_to_ignore.count(utn));

    assert (reconstructor().targets_container_.targets_.count(utn));

    const dbContent::ReconstructorTarget& target = reconstructor().targets_container_.targets_.at(utn);

    //    if (print_debug_target)
    //        loginf << "ReconstructorAssociatorBase: findUTNForTarget: utn " << utn;

    // dont check by mode s, should never work

    // try to find by m a/c/pos

    vector<AssociationOption> results;

    vector<reconstruction::PredictionStats> prediction_stats;

    unsigned int num_utns = reconstructor().targets_container_.utn_vec_.size();
    results.resize(num_utns);
    prediction_stats.resize(num_utns);

    const auto& settings = reconstructor().settings();

    //computes a match score for the given other target
    auto scoreUTN = [ & ] (const std::vector<size_t>& rec_nums,
                        const dbContent::ReconstructorTarget& other,
                        unsigned int result_idx,
                        bool secondary_verified,
                        bool do_debug)
    {
        vector<pair<unsigned long, double>> distance_scores;
        double distance_scores_sum {0};

        unsigned int pos_not_ok_cnt {0};
        unsigned int pos_dubious_cnt {0};
        unsigned int pos_good_cnt {0};
        unsigned int pos_skipped_cnt {0};

        double distance_m, stddev_est_target, stddev_est_other;
        float pos_not_ok_rate, pos_dubious_rate;
        bool pos_not_ok_rate_acceptable, pos_dubious_rate_acceptable;

        if (do_debug)
            loginf << "\ttarget " << utn << " other utn " << other.utn_ << " rec_nums " << rec_nums.size();

        for (auto rn_it : rec_nums)
        {
            assert (reconstructor().target_reports_.count(rn_it));

            const dbContent::targetReport::ReconstructorInfo& tr = reconstructor().target_reports_.at(rn_it);

            if (tr.doNotUsePosition() || !canGetPositionOffsetTargets(tr.timestamp_, target, other))
            {
                ++pos_skipped_cnt;
                continue;
            }

            //@TODO: debug flag
            auto pos_offs = getPositionOffsetTargets(tr.timestamp_, target, other, false,
                                                     {}, &prediction_stats[ result_idx ]);
            if (!pos_offs.has_value())
            {
                ++pos_skipped_cnt;
                continue;
            }

            tie(distance_m, stddev_est_target, stddev_est_other) = pos_offs.value();

            bool target_acc_acc = *isTargetAccuracyAcceptable(stddev_est_target, utn, tr.timestamp_, do_debug)
                                  && *isTargetAccuracyAcceptable(stddev_est_other, other.utn_, tr.timestamp_, do_debug);

            double sum_stddev_est = stddev_est_target + stddev_est_other;
            ReconstructorAssociatorBase::DistanceClassification score_class;
            double distance_score;

            std::tie(score_class, distance_score) = checkPositionOffsetScore (
                distance_m, sum_stddev_est, secondary_verified, target_acc_acc);

            if (score_class == DistanceClassification::Distance_Dubious)
                ++pos_dubious_cnt;
            else if (score_class == DistanceClassification::Distance_Good)
                ++pos_good_cnt;
            else if (score_class == DistanceClassification::Distance_NotOK)
            {
                // distance_scores.clear();
                // pos_good_cnt = 0;

                // if (do_debug)
                // {
                //     loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                //            << " max mahalanobis distance failed,  score " << distance_score;
                // }

                // break;

                ++pos_not_ok_cnt;
            }

            //loginf << "\tdist " << distance;

            distance_scores.push_back({rn_it, distance_score});
            distance_scores_sum += distance_score;
        }

        if (pos_not_ok_cnt)
            pos_not_ok_rate = (float) pos_not_ok_cnt / (float) pos_good_cnt;
        else
            pos_not_ok_rate = 0;

        if (pos_good_cnt)
            pos_dubious_rate = (float) pos_dubious_cnt / (float) pos_good_cnt;
        else
            pos_dubious_rate = 0;

        if (do_debug)
            loginf << "\ttarget " << utn << " other utn " << other.utn_
                   << " pos_not_ok_rate " << pos_not_ok_rate << " pos_not_ok_cnt " << pos_not_ok_cnt
                   << " pos_dubious_rate " << pos_dubious_rate << " pos_dubious_cnt " << pos_dubious_cnt
                   << " pos_good_cnt " << pos_good_cnt;

        pos_not_ok_rate_acceptable =
            secondary_verified ? pos_not_ok_rate < settings.target_max_positions_not_ok_verified_rate_
                               : pos_not_ok_rate < settings.target_max_positions_not_ok_unknown_rate_;

        pos_dubious_rate_acceptable =
            secondary_verified ? pos_dubious_rate < settings.target_max_positions_dubious_verified_rate_
                               : pos_dubious_rate < settings.target_max_positions_dubious_unknown_rate_;

        if (do_debug)
            loginf << "\ttarget " << utn << " other utn " << other.utn_
                   << " pos_good_cnt " << pos_good_cnt
                   << " pos_not_ok_rate_acceptable " << pos_not_ok_rate_acceptable
                   << " pos_dubious_rate_acceptable " << pos_dubious_rate_acceptable
                   << " (distance_scores " << distance_scores.size() << " < target_min_updates "
                   << settings.target_min_updates_ << ") "
                   << (distance_scores.size() >= settings.target_min_updates_);

        if (pos_good_cnt
            && pos_not_ok_rate_acceptable && pos_dubious_rate_acceptable
            && distance_scores.size() >= settings.target_min_updates_)
        {
            double distance_score_avg = distance_scores_sum / (float) distance_scores.size();

            if (isTargetAverageDistanceAcceptable(distance_score_avg, secondary_verified))
            {
                if (do_debug)
                {
                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                           << " next utn " << num_utns << " dist avg " << distance_score_avg
                           << " num " << distance_scores.size();
                }

                results[result_idx] = AssociationOption(
                    true, other.utn_, distance_scores.size(), true, distance_score_avg);
            }
            else
            {
                if (do_debug)
                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                           << " distance_scoring failed" << distance_score_avg;
            }
        }
        else
        {
            if (do_debug)
                loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                       << " same distances failed "
                       << distance_scores.size() << " < " << settings.target_min_updates_;
        }
    };

#ifdef FIND_UTN_FOR_TARGET_MT
    tbb::parallel_for(uint(0), num_utns, [&](unsigned int cnt)
#else
    for (unsigned int cnt=0; cnt < num_utns; ++cnt)
#endif
                      {
                          unsigned int other_utn = reconstructor().targets_container_.utn_vec_.at(cnt);

                          //bool print_debug_target = debug_utns.count(utn) && debug_utns.count(other_utn);

                          // only check for previous targets
                          const dbContent::ReconstructorTarget& other =
                              reconstructor().targets_container_.targets_.at(other_utn);

                          results[cnt] = AssociationOption(false, other.utn_, 0, false, 0);

                          bool do_debug = reconstructor().task().debugSettings().debugUTN(utn)
                                          && reconstructor().task().debugSettings().debugUTN(other_utn);

                          if (utn == other_utn || utns_to_ignore.count(other_utn))
#ifdef FIND_UTN_FOR_TARGET_MT
                              return;
#else
            continue;
#endif

                          if (!other.created_in_current_slice_)
                          {
                              if (do_debug)
                                  loginf << "\ttarget " << target.utn_ << " other " << other_utn
                                         << " other not in current slice";

#ifdef FIND_UTN_FOR_TARGET_MT
                              return;
#else
                              continue;
#endif
                          }

                          if (target.hasACAD() && other.hasACAD())
                          {
                              if (do_debug)
                                  loginf << "\ttarget " << target.utn_ << " other " << other_utn
                                         << " both have acads, first " << target.hasACAD()
                                         << " " << target.acadsStr()
                                         << " other " << other.hasACAD()
                                         << " " << other.acadsStr();

#ifdef FIND_UTN_FOR_TARGET_MT
                              return;
#else
            continue;
#endif
                          }

                          if (do_debug)
                          {
                              loginf << "\ttarget " << target.utn_ << " " << target.timeStr()
                                     << " checking other " << other.utn_ << " " << other.timeStr()
                                     << " overlaps " << target.timeOverlaps(other)
                                     << " prob " << target.probTimeOverlaps(other);
                          }

                          if (target.timeOverlaps(other)
                              && target.probTimeOverlaps(other) >= settings.target_prob_min_time_overlap_)
                          {
                              // check based on mode a/c/pos

                              if (do_debug)
                                  loginf << "\ttarget " << target.utn_ << " other " << other.utn_ << " overlap passed";

                              vector<unsigned long> ma_unknown; // record numbers
                              vector<unsigned long> ma_same;
                              vector<unsigned long> ma_different;

                              tie (ma_unknown, ma_same, ma_different) = target.compareModeACodes(
                                  other, max_time_diff_, do_debug);

                              if (do_debug)
                              {
                                  loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                         << " ma unknown " << ma_unknown.size()
                                         << " same " << ma_same.size() << " diff " << ma_different.size();
                              }

                              if (ma_same.size() > ma_different.size()
                                  && ma_same.size() >= settings.target_min_updates_)
                              {
                                  if (do_debug)
                                      loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                             << " mode a check passed";

                                  // check mode c codes

                                  vector<unsigned long> mc_unknown;
                                  vector<unsigned long> mc_same;
                                  vector<unsigned long> mc_different;

                                  tie (mc_unknown, mc_same, mc_different) = target.compareModeCCodes(
                                      other, ma_same, max_time_diff_, settings.max_altitude_diff_, do_debug);

                                  if (do_debug)
                                  {
                                      loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                             << " ma same " << ma_same.size() << " diff " << ma_different.size()
                                             << " mc same " << mc_same.size() << " diff " << mc_different.size();
                                  }

                                  if (mc_same.size() > mc_different.size()
                                      && mc_same.size() >= settings.target_min_updates_)
                                  {
                                      if (do_debug)
                                          loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                                 << " mode c check passed";

                                      // check positions
                                      scoreUTN(mc_same, other, cnt, true, do_debug);
                                  }
                                  else
                                  {
                                      if (do_debug)
                                          loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                                 << " mode c check failed, checking on position only";

                                      // check based on pos only
                                      scoreUTN(target.target_reports_, other, cnt, false, do_debug);
                                  }
                              }
                              else if (!ma_different.size())
                              {
                                  if (do_debug)
                                      loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                             << " mode a check failed, checking on position only";

                                  // check based on pos only
                                  scoreUTN(target.target_reports_, other, cnt, false, do_debug);
                              }
                              else
                                  if (do_debug)
                                      loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                             << " mode a check failed";
                          }
                          else
                          {
                              if (do_debug)
                                  loginf << "\ttarget " << target.utn_ << " other " << other.utn_ << " no overlap";
                          }
#ifdef FIND_UTN_FOR_TARGET_MT
                      });
#else
        }
#endif

    //log failed predictions
    for (const auto& s : prediction_stats)
        ReconstructorTarget::addPredictionToGlobalStats(s);

    bool do_debug = false; //debug_utns.count(utn);
    std::vector<unsigned int> utns_to_merge;

    // TODO rework to 1?

    bool best_found = false;
    unsigned int best_other_utn;
    unsigned int best_num_updates;
    unsigned int best_avg_distance;


    for (auto& res_it : results) // usable, other utn, num updates, avg distance
    {
        do_debug = reconstructor().task().debugSettings().debugUTN(utn)
                   || reconstructor().task().debugSettings().debugUTN(res_it.other_utn_);

        if (!res_it.usable_)
        {
            // if (do_debug)
            //     loginf << "\ttarget " << target.utn_ << " other " << other_utn << " not usable";

            continue;
        }

        if (do_debug)
            loginf << "\ttarget " << target.utn_ << " other " << res_it.other_utn_
                   << " do_assoc " << res_it.associate_based_on_secondary_attributes_
                   << " avg dist " << String::doubleToStringPrecision(res_it.avg_distance_, 2);

        if (res_it.associate_based_on_secondary_attributes_ || res_it.avg_distance_ <= 0)
        {
            if (do_debug)
                loginf << "\ttarget " << target.utn_ << " other " << res_it.other_utn_
                   << " merging, based on sec.at. " << res_it.associate_based_on_secondary_attributes_
                   << " avg dist " << String::doubleToStringPrecision(res_it.avg_distance_, 2);

            //utns_to_merge.push_back(res_it.other_utn_);

            if (best_found)
            {
                if (best_num_updates < res_it.num_updates_
                    && best_avg_distance > res_it.avg_distance_)
                {
                    best_other_utn = res_it.other_utn_;
                    best_num_updates = res_it.num_updates_;
                    best_avg_distance = res_it.avg_distance_;
                }
            }
            else
            {
                best_other_utn = res_it.other_utn_;
                best_num_updates = res_it.num_updates_;
                best_avg_distance = res_it.avg_distance_;
            }

            best_found = true;
        }
    }

    if (best_found)
        utns_to_merge.push_back(best_other_utn);

    return utns_to_merge;
}

const std::map<unsigned int, std::map<unsigned int,
                                      std::pair<unsigned int, unsigned int>>>& ReconstructorAssociatorBase::assocAounts() const
{
    return assoc_counts_;
}
