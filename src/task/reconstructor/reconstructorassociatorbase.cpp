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

    if (reconstructor().isCancelled())
        return;

    checkACADLookup();

    if (reconstructor().isCancelled())
        return;

    start_time = boost::posix_time::microsec_clock::local_time();

    // self-associate created utns
    selfAccociateNewUTNs();

    time_assoc_new_utns_ += boost::posix_time::microsec_clock::local_time() - start_time;

    if (reconstructor().isCancelled())
        return;

    checkACADLookup();

    if (reconstructor().isCancelled())
        return;

    start_time = boost::posix_time::microsec_clock::local_time();

    retryAssociateTargetReports();

    time_retry_assoc_trs_ += boost::posix_time::microsec_clock::local_time() - start_time;

    countUnAssociated();

    if (reconstructor().isCancelled())
        return;

    // clear new flags
    for (auto& tgt_it : reconstructor().targets_)
        tgt_it.second.created_in_current_slice_ = false;

    unassoc_rec_nums_.clear();

    //    reconstructor().acc_estimator_->analyzeAssociatedDistances();
    //    reconstructor().acc_estimator_->clearAssociatedDistances();

    loginf << "ReconstructorAssociatorBase: associateNewData: time_assoc_trs " << Time::toString(time_assoc_trs_)
           << " time_assoc_new_utns " << Time::toString(time_assoc_new_utns_)
           << " time_retry_assoc_trs " << Time::toString(time_retry_assoc_trs_);

    if (reconstructor().currentSlice().is_last_slice_)
        loginf << "ReconstructorAssociatorBase: associateNewData: done, num_merges " << num_merges_;
}

void ReconstructorAssociatorBase::reset()
{
    logdbg << "ReconstructorAssociatorBase: reset";

    utn_vec_.clear();
    acad_2_utn_.clear();
    acid_2_utn_.clear();
    tn2utn_.clear();

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

    checkACADLookup();

    bool is_unreliable_primary_only;

    for (auto& ts_it : reconstructor().tr_timestamps_)
    {
        if (reconstructor().isCancelled())
            return;

        rec_num = ts_it.second;

        assert (reconstructor().target_reports_.count(rec_num));

        //do_debug = reconstructor().task().debugRecNum(rec_num);

        if (do_debug)
            loginf << "DBG tr " << rec_num;

        dbContent::targetReport::ReconstructorInfo& tr = reconstructor().target_reports_.at(rec_num);

        // #if DO_RECONSTRUCTOR_PEDANTIC_CHECKING
        //         loginf << "ReconstructorAssociatorBase: associateTargetReports: tr " << tr.asStr();
        // #endif

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
                loginf << "DBG finding UTN";

            utn = findUTNFor(tr);

            if (do_debug)
                loginf << "DBG got UTN " << utn;
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

    checkACADLookup();

    for (auto& ts_it : reconstructor().tr_timestamps_)
    {
        if (reconstructor().isCancelled())
            return;

        rec_num = ts_it.second;

        assert (reconstructor().target_reports_.count(rec_num));

        do_debug = reconstructor().task().debugRecNum(rec_num);

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

void ReconstructorAssociatorBase::selfAccociateNewUTNs()
{
    loginf << "ReconstructorAssociatorBase: selfAccociateNewUTNs";

    unsigned int run_cnt {0};
    bool do_it_again {true};

    int other_utn;

    while (do_it_again && !reconstructor().isCancelled())
    {
        loginf << "ReconstructorAssociatorBase: selfAccociateNewUTNs: run " << run_cnt;

        checkACADLookup();

        do_it_again = false;

        for (auto utn_it = utn_vec_.begin(); utn_it != utn_vec_.end(); )
        {
            unsigned int utn = *utn_it;

            logdbg << "ReconstructorAssociatorBase: selfAccociateNewUTNs: checking utn " << utn;

            assert (reconstructor().targets_.count(utn));

            if (!reconstructor().targets_.at(utn).created_in_current_slice_)
            {
                utn_it++;
                continue;
            }

            other_utn = findUTNForTarget(utn);

            if (other_utn != -1)
            {
                loginf << "ReconstructorAssociatorBase: selfAccociateNewUTNs: run " << run_cnt
                       << ": found merge utn " << utn << " with " << other_utn;

                assert (reconstructor().targets_.count(other_utn));

                dbContent::ReconstructorTarget& target = reconstructor().targets_.at(utn);
                dbContent::ReconstructorTarget& other_target = reconstructor().targets_.at(other_utn);

                // move target reports
                other_target.addTargetReports(target);

                // remove from targets
                assert (reconstructor().targets_.count(utn));
                reconstructor().targets_.erase(utn);

                // remove from utn list
                utn_it = utn_vec_.erase(utn_it);

                // remove from acad lookup
                for (auto& acad_it : acad_2_utn_)
                {
                    if (acad_it.second == utn)
                        acad_2_utn_[acad_it.first] = other_utn;
                }

                // remove from acid lookup
                for (auto& acid_it : acid_2_utn_)
                {
                    if (acid_it.second == utn)
                        acid_2_utn_[acid_it.first] = other_utn;
                }

                // remove in tn2utn_ lookup
                // ds_id -> line id -> track num -> utn, last tod
                for (auto& ds_it : tn2utn_)
                {
                    for (auto& line_it : ds_it.second)
                    {
                        for (auto& tn_it : line_it.second)
                        {
                            if (tn_it.second.first == utn)
                            {
                                // replace utn
                                tn2utn_[ds_it.first][line_it.first][tn_it.first] =
                                    std::pair<unsigned int, boost::posix_time::ptime> (
                                        (unsigned int) other_utn, tn_it.second.second);
                            }
                        }
                    }

                }

                checkACADLookup();

                do_it_again = true;

                ++num_merges_;
            }
            else
                utn_it++;
        }

        ++run_cnt;
    }

    loginf << "ReconstructorAssociatorBase: selfAccociateNewUTNs: done at run " << run_cnt;
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

    for (auto rec_num_it = unassoc_rec_nums_.rbegin(); rec_num_it != unassoc_rec_nums_.rend(); ++rec_num_it)
    {
        if (reconstructor().isCancelled())
            return;

        rec_num = *rec_num_it;

        dbcont_id = Number::recNumGetDBContId(rec_num);

        assert (reconstructor().target_reports_.count(rec_num));

        do_debug = reconstructor().task().debugRecNum(rec_num);

        if (do_debug)
            loginf << "DBG tr " << rec_num;

        dbContent::targetReport::ReconstructorInfo& tr = reconstructor().target_reports_.at(rec_num);

        if (!tr.in_current_slice_)
        {
            if(do_debug)
                loginf << "DBG tr " << rec_num << " not in current slice";

            continue;
        }

        // check if should have been associated
        assert (!tr.acad_);
        assert (!tr.acid_);

        if (dbcont_id == 62 || dbcont_id == 255)
            assert (!tr.track_number_);

        utn = findUTNByModeACPos (tr, utn_vec_);

        if (utn != -1) // estimate accuracy and associate
        {
            associate(tr, utn);

            ++assocated_cnt;
        }
    }

    loginf << "ReconstructorAssociatorBase: retryAssociateTargetReports: done with count " << assocated_cnt;
}

void ReconstructorAssociatorBase::associate(
    dbContent::targetReport::ReconstructorInfo& tr, int utn)
{
    assert (utn >= 0);

    bool do_debug = reconstructor().task().debugRecNum(tr.record_num_)
                    || reconstructor().task().debugUTN(utn);

    unsigned int dbcont_id  = Number::recNumGetDBContId(tr.record_num_);
    //AccuracyEstimatorBase::AssociatedDistance dist;

    if (!reconstructor().targets_.count(utn))
        logerr << "ReconstructorAssociatorBase: associate: utn " << utn << " missing";

    // add associated target reports
    assert (reconstructor().targets_.count(utn));

    // check if position usable
    if (do_debug)
        loginf << "DBG validate";

    reconstructor().acc_estimator_->validate(tr);

    if (do_debug)
        loginf << "DBG outlier detect";

    doOutlierDetection(tr, utn, do_debug); //124976,129072

    if (do_debug)
        loginf << "DBG addTargetReport";

    reconstructor().targets_.at(utn).addTargetReport(tr.record_num_);

    if (do_debug)
        loginf << "DBG add to lookups";

    if (tr.track_number_ && (dbcont_id == 62 || dbcont_id == 255))
        tn2utn_[tr.ds_id_][tr.line_id_][*tr.track_number_] =
            std::pair<unsigned int, boost::posix_time::ptime>(utn, tr.timestamp_);

    if (tr.acad_)
        acad_2_utn_[*tr.acad_] = utn;

    if (tr.acid_)
        acid_2_utn_[*tr.acid_] = utn;

    assoc_counts_[tr.ds_id_][dbcont_id].first++;

    if (do_debug)
        loginf << "DBG postAssociate";

    postAssociate (tr, utn);
}

void ReconstructorAssociatorBase::checkACADLookup()
{
    unsigned int acad;

    unsigned int count = 0;

    for (auto& target_it : reconstructor().targets_)
    {
        if (!target_it.second.hasACAD())
            continue;

        if (target_it.second.acads_.size() != 1)
            logerr << "ReconstructorAssociatorBase: checkACADLookup: double acad in target "
                   << target_it.second.asStr();

        assert (target_it.second.acads_.size() == 1);

        acad = *target_it.second.acads_.begin();

        if (!acad_2_utn_.count(acad))
        {
            logerr << "ReconstructorAssociatorBase: getTALookupMap: acad "
                   << String::hexStringFromInt(acad, 6, '0')
                   << " not in lookup";
        }

        ++count;
    }

    for (auto& acad_it : acad_2_utn_)
    {
        assert (reconstructor().targets_.count(acad_it.second));
        assert (reconstructor().targets_.at(acad_it.second).hasACAD(acad_it.first));
    }

    assert (acad_2_utn_.size() == count);
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

    bool do_debug = reconstructor().task().debugRecNum(tr.record_num_);

    unsigned int dbcont_id = Number::recNumGetDBContId(tr.record_num_);

    vector<tuple<bool, unsigned int, double>> results;
    const boost::posix_time::time_duration track_max_time_diff =
        Time::partialSeconds(reconstructor().settings().track_max_time_diff_);

    assert (reconstructor().targets_.size() == utn_vec_.size());

    //double distance_m{0}, tgt_est_std_dev{0}, tr_est_std_dev{0};

    bool reset_tr_assoc {false};

    auto canAssocByACAD = [ & ] (dbContent::targetReport::ReconstructorInfo& tr)
    {
        if (!tr.acad_)
            return false;

        return (bool) acad_2_utn_.count(*tr.acad_);
    };

    auto assocByACAD = [ & ] (dbContent::targetReport::ReconstructorInfo& tr)
    {
        assert (tr.acad_);
        assert (acad_2_utn_.count(*tr.acad_));

        utn = acad_2_utn_.at(*tr.acad_);
        assert (reconstructor().targets_.count(utn));
        assert (reconstructor().targets_.at(utn).hasACAD(*tr.acad_));
    };

    auto canAssocByACID = [ & ] (dbContent::targetReport::ReconstructorInfo& tr)
    {
        if (!tr.acid_)
            return false;

        if (*tr.acid_ == "00000000" || *tr.acid_ == "????????" || *tr.acid_ == "        ")
            return false;

        if (acid_2_utn_.count(*tr.acid_)) // already exists, but check if mode s address changed
        {
            assert (reconstructor().targets_.count(acid_2_utn_.at(*tr.acid_)));

            // tr has acad, target has an acad but not the target reports
            // happens if same callsign is used by 2 different transponders
            if (tr.acad_ && reconstructor().targets_.at(acid_2_utn_.at(*tr.acid_)).hasACAD()
                && !reconstructor().targets_.at(acid_2_utn_.at(*tr.acid_)).hasACAD(!tr.acad_))
                return false;
        }

        return (bool) acid_2_utn_.count(*tr.acid_);
    };

    auto assocByACID = [ & ] (dbContent::targetReport::ReconstructorInfo& tr)
    {
        assert (tr.acid_);
        assert (acid_2_utn_.count(*tr.acid_));

        utn = acid_2_utn_.at(*tr.acid_);
        assert (reconstructor().targets_.count(utn));
        assert (reconstructor().targets_.at(utn).hasACID(*tr.acid_));
    };

    auto canAssocByTrackNumber = [ & ] (dbContent::targetReport::ReconstructorInfo& tr)
    {
        if (!tr.track_number_) // only if track number is set
            return false;

        if (dbcont_id != 62 && dbcont_id != 255) // only if trustworty track numbers in 62 and reftraj
            return false;

        return (bool) tn2utn_[tr.ds_id_][tr.line_id_].count(*tr.track_number_);
    };

    auto assocByTrackNumber = [ & ] (dbContent::targetReport::ReconstructorInfo& tr)
    {
        boost::posix_time::ptime timestamp_prev;

        assert (tr.track_number_);

        assert (tn2utn_.at(tr.ds_id_).at(tr.line_id_).count(*tr.track_number_));
        std::tie(utn, timestamp_prev) = tn2utn_.at(tr.ds_id_).at(tr.line_id_).at(*tr.track_number_);
        assert (reconstructor().targets_.count(utn));

        // check for larger time offset
        if (tr.timestamp_ - timestamp_prev > track_max_time_diff) // too old
        {
            // remove previous track number assoc
            assert (tn2utn_[tr.ds_id_][tr.line_id_].count(*tr.track_number_));
            tn2utn_[tr.ds_id_][tr.line_id_].erase(*tr.track_number_);

            // create new and add
            utn = createNewTarget(tr);
            assert (reconstructor().targets_.count(utn));
        }
        else if (canGetPositionOffsetTR(tr, reconstructor().targets_.at(utn)))
        {
            assert (reconstructor().targets_.count(utn));

            // check for position offsets
            //            auto pos_offs = getPositionOffset(tr, reconstructor().targets_.at(utn), do_debug);

            //            if (pos_offs.has_value())
            //            {
            //                std::tie(distance_m, tgt_est_std_dev, tr_est_std_dev) = pos_offs.value();

            boost::optional<bool> check_result = checkPositionOffsetAcceptable(
                tr, utn, true, do_debug);

            if (check_result && !*check_result)
            {
                tn2utn_[tr.ds_id_][tr.line_id_].erase(*tr.track_number_);
                reset_tr_assoc = true;
            }
            //            }
        }
    };

    auto findUTNByModeACPosOrCreateNewTarget = [ & ] (dbContent::targetReport::ReconstructorInfo& tr)
    {
        // check if position match to other target would exist
        utn = findUTNByModeACPos (tr, utn_vec_);

        if (utn == -1)
        {
            // create new and add
            utn = createNewTarget(tr);
            assert (reconstructor().targets_.count(utn));
        }
        else
            assert (reconstructor().targets_.count(utn));
    };

START_TR_ASSOC:

    utn = -1; // not yet found
    reset_tr_assoc = false;

    // try by mode-s address or trustworthy track number
    // create new if not already existing
    if (tr.acad_
        || tr.acid_
        || (tr.track_number_ && (dbcont_id == 62 || dbcont_id == 255)))
    {
        // find utn

        if (canAssocByACAD(tr)) // already exists
            assocByACAD(tr);
        else if (canAssocByACID(tr)) // already exists
            assocByACID(tr);
        else if (canAssocByTrackNumber(tr))
        {
            assocByTrackNumber(tr);

            if (reset_tr_assoc)
                goto START_TR_ASSOC;
        }
        else // not yet existing, create & add target
            findUTNByModeACPosOrCreateNewTarget(tr);

        assert (utn != -1);

        if(do_debug)
            loginf << "DBG tr " << tr.record_num_ << " utn by acad";
    }
    else // not associated by trustworty id
    {
        assert (utn == -1);

        if(do_debug)
            loginf << "DBG tr " << tr.record_num_ << " no utn by acad, doing mode a/c + pos";

        utn = findUTNByModeACPos (tr, utn_vec_);

        if (utn != -1)
            assert (reconstructor().targets_.count(utn));
    }

    return utn;
}

int ReconstructorAssociatorBase::findUTNByModeACPos (
    const dbContent::targetReport::ReconstructorInfo& tr, const std::vector<unsigned int>& utn_vec)
{
    unsigned int num_targets = reconstructor().targets_.size();
    assert (utn_vec.size() == num_targets);

    vector<tuple<bool, unsigned int, double>> results;
    vector<reconstruction::PredictionStats> prediction_stats;

    results.resize(num_targets);
    prediction_stats.resize(num_targets);
    boost::posix_time::ptime timestamp = tr.timestamp_;

    const float max_altitude_diff = reconstructor().settings().max_altitude_diff_;

    bool do_debug = reconstructor().task().debugRecNum(tr.record_num_);

    if (do_debug)
        loginf << "ReconstructorAssociatorBase: findUTNByModeACPos: rn " << tr.record_num_;

#ifdef FIND_UTN_FOR_TARGET_REPORT_MT
    tbb::parallel_for(uint(0), num_targets, [&](unsigned int target_cnt)
#else
    for (unsigned int target_cnt = 0; target_cnt < num_targets; ++target_cnt)
#endif
                      {
                          unsigned int other_utn = utn_vec.at(target_cnt);
                          bool do_other_debug = false; //debug_utns.count(other_utn);

                          ReconstructorTarget& other = reconstructor().targets_.at(other_utn);

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
                                  ComparisonResult ma_res = other.compareModeACode(tr, max_time_diff_);

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

                          //                tie(ref_pos, ok) = other.interpolatedPosForTimeFast(
                          //                    timestamp, max_time_diff_sensor);

                          double distance_m{0}, tgt_est_std_dev{0}, tr_est_std_dev{0}; //, sum_est_std_dev{0};
                          //double mahalanobis_dist{0};

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

int ReconstructorAssociatorBase::findUTNForTarget (unsigned int utn)
{
    if (!reconstructor().targets_.size()) // check if targets exist
        return -1;

    assert (reconstructor().targets_.count(utn));

    const dbContent::ReconstructorTarget& target = reconstructor().targets_.at(utn);

    bool print_debug_target = false; //debug_utns.count(utn);

    //    if (print_debug_target)
    //        loginf << "ReconstructorAssociatorBase: findUTNForTarget: utn " << utn;

    // dont check by mode s, should never work

    //    int tmp_utn = findUTNForTargetByTA(target, targets);

    //    if (tmp_utn != -1) // either mode s, so
    //        return tmp_utn;

    //    if (!task_.associateNonModeS())
    //        return -1;

    // try to find by m a/c/pos

    vector<tuple<bool, unsigned int, unsigned int, double>> results;
    // usable, other utn, num updates, avg distance

    vector<reconstruction::PredictionStats> prediction_stats;

    unsigned int num_utns = utn_vec_.size();
    results.resize(num_utns);
    prediction_stats.resize(num_utns);

    const auto& settings = reconstructor().settings();

    //const boost::posix_time::time_duration max_time_diff_tracker = Utils::Time::partialSeconds(settings.max_time_diff_);

    //computes a match score for the given other target
    auto scoreUTN = [ & ] (const std::vector<size_t>& rec_nums,
                        const dbContent::ReconstructorTarget& other,
                        unsigned int result_idx,
                        bool secondary_verified,
                        //                        double max_mahal_dist_accept,
                        //                        double max_mahal_dist_dub,
                        //                        double max_mahal_dist_quit,
                        //double max_pos_dubious_rate,
                        bool print_debug)
    {
        vector<pair<unsigned long, double>> distance_scores;
        double distance_scores_sum {0};

        unsigned int pos_dubious_cnt {0};
        unsigned int pos_good_cnt {0};
        unsigned int pos_skipped_cnt {0};

        double distance_m, stddev_est_target, stddev_est_other;
        float pos_dubious_rate;
        bool pos_dubious_rate_acceptable;

        for (auto rn_it : rec_nums)
        {
            assert (reconstructor().target_reports_.count(rn_it));

            const dbContent::targetReport::ReconstructorInfo& tr = reconstructor().target_reports_.at(rn_it);

            if (tr.do_not_use_position_ || !canGetPositionOffsetTargets(tr.timestamp_, target, other))
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

            if (!*isTargetAccuracyAcceptable(stddev_est_target, utn, tr.timestamp_)
                || !*isTargetAccuracyAcceptable(stddev_est_other, other.utn_, tr.timestamp_))
            {
                ++pos_skipped_cnt;
                continue;
            }

            double sum_stddev_est = stddev_est_target + stddev_est_other;
            ReconstructorAssociatorBase::DistanceClassification score_class;
            double distance_score;

            std::tie(score_class, distance_score) = checkPositionOffsetScore (
                distance_m, sum_stddev_est, secondary_verified);

            if (score_class == DistanceClassification::Distance_Dubious)
                ++pos_dubious_cnt;
            else if (score_class == DistanceClassification::Distance_Good)
                ++pos_good_cnt;
            else if (score_class == DistanceClassification::Distance_NotOK)
            {
                distance_scores.clear();
                pos_good_cnt = 0;

                if (print_debug)
                {
                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                           << " max mahalanobis distance failed,  score " << distance_score;
                }

                break;
            }

            //loginf << "\tdist " << distance;

            distance_scores.push_back({rn_it, distance_score});
            distance_scores_sum += distance_score;
        }

        pos_dubious_rate = (float) pos_dubious_cnt / (float) pos_good_cnt;

        pos_dubious_rate_acceptable =
            secondary_verified ? pos_dubious_rate < settings.target_max_positions_dubious_verified_rate_
                               : pos_dubious_rate < settings.target_max_positions_dubious_unknown_rate_;

        if (pos_good_cnt
            && pos_dubious_rate_acceptable
            && distance_scores.size() >= settings.target_min_updates_)
        {
            double distance_score_avg = distance_scores_sum / (float) distance_scores.size();

            if (isTargetAverageDistanceAcceptable(distance_score_avg, secondary_verified))
            {
                if (print_debug)
                {
                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                           << " next utn " << num_utns << " dist avg " << distance_score_avg
                           << " num " << distance_scores.size();
                }

                results[result_idx] = tuple<bool, unsigned int, unsigned int, double>(
                    true, other.utn_, distance_scores.size(), distance_score_avg);
            }
            else
            {
                if (print_debug)
                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                           << " distance_score_avg failed " << distance_score_avg;
            }
        }
        else
        {
            if (print_debug)
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
                          unsigned int other_utn = utn_vec_.at(cnt);

                          //bool print_debug_target = debug_utns.count(utn) && debug_utns.count(other_utn);

                          // only check for previous targets
                          const dbContent::ReconstructorTarget& other = reconstructor().targets_.at(other_utn);

                          results[cnt] = tuple<bool, unsigned int, unsigned int, double>(false, other.utn_, 0, 0);

                          if (utn == other_utn)
#ifdef FIND_UTN_FOR_TARGET_MT
                              return;
#else
            continue;
#endif
                          if (target.hasACAD() && other.hasACAD())
#ifdef FIND_UTN_FOR_TARGET_MT
                              return;
#else
            continue;
#endif
                          Transformation trafo;

                          bool print_debug = reconstructor().task().debugUTN(utn)
                                             && reconstructor().task().debugUTN(other_utn);

                          if (print_debug)
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

                              if (print_debug)
                                  loginf << "\ttarget " << target.utn_ << " other " << other.utn_ << " overlap passed";

                              vector<unsigned long> ma_unknown; // record numbers
                              vector<unsigned long> ma_same;
                              vector<unsigned long> ma_different;

                              tie (ma_unknown, ma_same, ma_different) = target.compareModeACodes(
                                  other, max_time_diff_);

                              if (print_debug)
                              {
                                  loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                         << " ma unknown " << ma_unknown.size()
                                         << " same " << ma_same.size() << " diff " << ma_different.size();
                              }

                              if (ma_same.size() > ma_different.size()
                                  && ma_same.size() >= settings.target_min_updates_)
                              {
                                  if (print_debug)
                                      loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                             << " mode a check passed";

                                  // check mode c codes

                                  vector<unsigned long> mc_unknown;
                                  vector<unsigned long> mc_same;
                                  vector<unsigned long> mc_different;

                                  tie (mc_unknown, mc_same, mc_different) = target.compareModeCCodes(
                                      other, ma_same, max_time_diff_, settings.max_altitude_diff_, print_debug);

                                  if (print_debug)
                                  {
                                      loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                             << " ma same " << ma_same.size() << " diff " << ma_different.size()
                                             << " mc same " << mc_same.size() << " diff " << mc_different.size();
                                  }

                                  if (mc_same.size() > mc_different.size()
                                      && mc_same.size() >= settings.target_min_updates_)
                                  {
                                      if (print_debug)
                                          loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                                 << " mode c check passed";

                                      // check positions
                                      scoreUTN(mc_same, other, cnt, true, print_debug);
                                  }
                                  else
                                  {
                                      if (print_debug)
                                          loginf << "\ttarget " << target.utn_ << " other " << other.utn_ << " mode c check failed";
                                  }
                              }
                              else if (!ma_different.size())
                              {
                                  // check based on pos only
                                  scoreUTN(target.target_reports_, other, cnt, false, print_debug);
                              }
                              else
                                  if (print_debug)
                                      loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                             << " mode a check failed";
                          }
                          else
                          {
                              if (print_debug)
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

    // find best match
    bool usable;
    unsigned int other_utn;
    unsigned int num_updates;
    double distance_score_avg;
    double score;

    bool first = true;
    unsigned int best_other_utn {0};
    unsigned int best_num_updates;
    double best_distance_score_avg;
    double best_score;

    //    if (print_debug_target)
    //        loginf << "\ttarget " << target.utn_ << " checking results";

    for (auto& res_it : results) // usable, other utn, num updates, avg distance
    {
        tie(usable, other_utn, num_updates, distance_score_avg) = res_it;

        if (!usable)
        {
            //            if (print_debug_target)
            //                loginf << "\ttarget " << target.utn_ << " result utn " << other_utn << " not usable";
            continue;
        }

        score = (double) num_updates * distance_score_avg;

        if (first || score < best_score)
        {
            if (print_debug_target)
                loginf << "ReconstructorAssociatorBase: findUTNForTarget: target "
                       << target.utn_ << " result utn " << other_utn
                       << " marked as best, score " << score;

            best_other_utn = other_utn;
            best_num_updates = num_updates;
            best_distance_score_avg = distance_score_avg;
            best_score = score;

            first = false;
        }
    }

    if (first)
    {
        if (print_debug_target)
            loginf << "ReconstructorAssociatorBase: findUTNForTarget: target " << target.utn_ << " no match found";
        return -1;
    }
    else
    {
        //if (print_debug_target)
        loginf << "ReconstructorAssociatorBase: findUTNForTarget: target " << target.utn_
               << " match found best other " << best_other_utn
               << " best score " << fixed << best_score
               << " dist score avg " << best_distance_score_avg
               << " num " << best_num_updates;

        return best_other_utn;
    }
}

unsigned int ReconstructorAssociatorBase::createNewTarget(const dbContent::targetReport::ReconstructorInfo& tr)
{
    unsigned int utn;

    if (reconstructor().targets_.size())
        utn = reconstructor().targets_.rbegin()->first + 1; // new utn
    else
        utn = 0;

    //assert (utn != 261);
    if (tr.track_number_)
        assert (!tn2utn_[tr.ds_id_][tr.line_id_].count(*tr.track_number_));

    reconstructor().targets_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(utn),   // args for key
        std::forward_as_tuple(reconstructor(), utn, false, true, true));  // args for mapped value tmp_utn, false

    // add to lookup

    reconstructor().targets_.at(utn).created_in_current_slice_ = true;

    utn_vec_.push_back(utn);

    return utn;
}

const std::map<unsigned int, std::map<unsigned int,
                                      std::pair<unsigned int, unsigned int>>>& ReconstructorAssociatorBase::assocAounts() const
{
    return assoc_counts_;
}
