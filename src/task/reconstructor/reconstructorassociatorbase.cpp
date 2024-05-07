#include "reconstructorassociatorbase.h"
#include "logger.h"
#include "global.h"
#include "stringconv.h"
#include "timeconv.h"
#include "util/tbbhack.h"
#include "reconstructortask.h"
#include "number.h"

#define FIND_UTN_FOR_TARGET_MT

using namespace std;
using namespace dbContent;
using namespace Utils;

ReconstructorAssociatorBase::ReconstructorAssociatorBase()
{

}

void ReconstructorAssociatorBase::associateNewData()
{
    loginf << "ReconstructorAssociatorBase: associateNewData";

    assert (!unassoc_rec_nums_.size());

    associateTargetReports();

    checkACADLookup();

            // self-associate created utns
    selfAccociateNewUTNs();

    checkACADLookup();

    retryAssociateTargetReports();

            // clear new flags
    for (auto& tgt_it : reconstructor().targets_)
        tgt_it.second.created_in_current_slice_ = false;

    unassoc_rec_nums_.clear();

            //    reconstructor().acc_estimator_->analyzeAssociatedDistances();
            //    reconstructor().acc_estimator_->clearAssociatedDistances();

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

    num_merges_ = 0;
}

void ReconstructorAssociatorBase::associateTargetReports()
{
    loginf << "ReconstructorAssociatorBase: associateTargetReports";

    const std::set<unsigned int> debug_utns = reconstructor().task().debugUTNs();

    if (debug_utns.size())
        loginf << "DBG tns '" << String::compress(debug_utns, ',') << "'";

    const std::set<unsigned long> debug_rec_nums = reconstructor().task().debugRecNums();

    if (debug_rec_nums.size())
        loginf << "DBG recnums '" << String::compress(debug_rec_nums, ',') << "'";

    unsigned long rec_num;
    int utn;

    bool do_debug;

    checkACADLookup();

    for (auto& ts_it : reconstructor().tr_timestamps_)
    {
        rec_num = ts_it.second;

        assert (reconstructor().target_reports_.count(rec_num));

        do_debug = debug_rec_nums.count(rec_num);

        if (do_debug)
            loginf << "DBG tr " << rec_num;

        dbContent::targetReport::ReconstructorInfo& tr = reconstructor().target_reports_.at(rec_num);

        if (!tr.in_current_slice_)
        {
            if(do_debug)
                loginf << "DBG tr " << rec_num << " not in current slice";

            continue;
        }

        utn = findUTNFor(tr, debug_rec_nums, debug_utns);

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

    const std::set<unsigned int> debug_utns = reconstructor().task().debugUTNs();

    const std::set<unsigned long> debug_rec_nums = reconstructor().task().debugRecNums();

    int other_utn;

    while (do_it_again)
    {
        loginf << "ReconstructorAssociatorBase: selfAccociateNewUTNs: run " << run_cnt;

        checkACADLookup();

        do_it_again = false;

        for (auto utn_it = utn_vec_.begin(); utn_it != utn_vec_.end(); utn_it++)
        {
            unsigned int utn = *utn_it;

            if (!reconstructor().targets_.at(utn).created_in_current_slice_)
                continue;

            other_utn = findUTNForTarget(utn, debug_rec_nums, debug_utns);

            if (other_utn != -1)
            {
                loginf << "ReconstructorAssociatorBase: selfAccociateNewUTNs: run " << run_cnt
                       << ": found merge utn " << utn << " with " << other_utn;

                dbContent::ReconstructorTarget& target = reconstructor().targets_.at(utn);
                dbContent::ReconstructorTarget& other_target = reconstructor().targets_.at(other_utn);

                        // move target reports
                other_target.addTargetReports(target.target_reports_);

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

    const std::set<unsigned int> debug_utns = reconstructor().task().debugUTNs();
    const std::set<unsigned long> debug_rec_nums = reconstructor().task().debugRecNums();

    unsigned int rec_num;
    unsigned int dbcont_id;
    int utn;

    bool do_debug;

    unsigned int assocated_cnt{0};

    for (auto rec_num_it = unassoc_rec_nums_.rbegin(); rec_num_it != unassoc_rec_nums_.rend(); ++rec_num_it)
    {
        rec_num = *rec_num_it;

        dbcont_id = Number::recNumGetDBContId(rec_num);

        assert (reconstructor().target_reports_.count(rec_num));

        do_debug = debug_rec_nums.count(rec_num);

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

        if ((dbcont_id == 62 || dbcont_id == 255))
            assert (!tr.track_number_);

        utn = findUTNByModeACPos (tr, utn_vec_, debug_rec_nums, debug_utns);

        if (utn != -1) // estimate accuracy and associate
        {
            associate(tr, utn);

            ++assocated_cnt;
        }
    }

    loginf << "ReconstructorAssociatorBase: retryAssociateTargetReports: done with count " << assocated_cnt;
}

void ReconstructorAssociatorBase::associate(dbContent::targetReport::ReconstructorInfo& tr, int utn)
{
    assert (utn >= 0);

    unsigned int dbcont_id  = Number::recNumGetDBContId(tr.record_num_);
    //AccuracyEstimatorBase::AssociatedDistance dist;

    if (!reconstructor().targets_.count(utn))
        logerr << "ReconstructorAssociatorBase: associate: utn " << utn << " missing";

            // add associated target reports
    assert (reconstructor().targets_.count(utn));

            // check if position usable
    reconstructor().acc_estimator_->validate(tr, reconstructor());

    reconstructor().targets_.at(utn).addTargetReport(tr.record_num_);

    if (tr.track_number_ && (dbcont_id == 62 || dbcont_id == 255))
        tn2utn_[tr.ds_id_][tr.line_id_][*tr.track_number_] =
            std::pair<unsigned int, boost::posix_time::ptime>(utn, tr.timestamp_);

    if (tr.acad_)
        acad_2_utn_[*tr.acad_] = utn;

    if (tr.acid_)
        acid_2_utn_[*tr.acid_] = utn;

            // TODO move to post process association or something
            // only if not newly created
    //    if (!tr.do_not_use_position_ && reconstructor().targets_.at(utn).canPredict(tr.timestamp_))
    //    {
    //        reconstruction::Measurement mm;
    //        bool ret;
    //        double distance_m, bearing_rad;
    //        dbContent::targetReport::PositionAccuracy tr_pos_acc;
    //        dbContent::targetReport::PositionAccuracy mm_pos_acc;
    //        EllipseDef acc_ell;
    //        double est_std_dev;
    //        double mahalanobis_dist;

            //                // predict pos from target and estimate accuracies
            //        ret = reconstructor().targets_.at(utn).predict(mm, tr);
            //        assert (ret);

            //        distance_m = osgEarth::GeoMath::distance(tr.position_->latitude_ * DEG2RAD,
            //                                                 tr.position_->longitude_ * DEG2RAD,
            //                                                 mm.lat * DEG2RAD, mm.lon * DEG2RAD);

            //        bearing_rad = osgEarth::GeoMath::bearing(tr.position_->latitude_ * DEG2RAD,
            //                                                 tr.position_->longitude_ * DEG2RAD,
            //                                                 mm.lat * DEG2RAD, mm.lon * DEG2RAD);

            //        tr_pos_acc = reconstructor().acc_estimator_->positionAccuracy(tr);
            //        estimateEllipse(tr_pos_acc, acc_ell);
            //        est_std_dev = estimateAccuracyAt(acc_ell, bearing_rad);

            //        assert (mm.hasStdDevPosition());
            //        mm_pos_acc = mm.positionAccuracy();
            //        estimateEllipse(mm_pos_acc, acc_ell);
            //        est_std_dev += estimateAccuracyAt(acc_ell, bearing_rad);

            //        mahalanobis_dist = distance_m / est_std_dev;

            //        dist.latitude_deg_ = tr.position_->latitude_;
            //        dist.longitude_deg_ = tr.position_->longitude_;
            //        dist.est_std_dev_ = est_std_dev;
            //        dist.distance_m_ = distance_m;
            //        dist.mahalanobis_distance_ = mahalanobis_dist;

            //        reconstructor().acc_estimator_->addAssociatedDistance(tr, dist);

            //                //                not_use_tr_pos = distance_m > 50 && mahalanobis_dist > 10;

            //                //                loginf << " dist " << String::doubleToStringPrecision(distance_m, 2)
            //                //                       << " est_std_dev " << String::doubleToStringPrecision(est_std_dev, 2)
            //                //                       << " mahala " << String::doubleToStringPrecision(mahalanobis_dist, 2)
            //                //                       << " use pos " << !not_use_tr_pos;

            //                //                tr.do_not_use_position_ = not_use_tr_pos;
            //    }
}

void ReconstructorAssociatorBase::checkACADLookup()
{
    unsigned int acad;

    unsigned int count = 0;

    for (auto& target_it : reconstructor().targets_)
    {
        if (!target_it.second.hasACAD())
            continue;

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

    assert (acad_2_utn_.size() == count);
}

int ReconstructorAssociatorBase::findUTNFor (dbContent::targetReport::ReconstructorInfo& tr,
                                            const std::set<unsigned long>& debug_rec_nums,
                                            const std::set<unsigned int>& debug_utns)
{
    int utn {-1};

    bool do_debug = debug_rec_nums.count(tr.record_num_);

    unsigned int dbcont_id = Number::recNumGetDBContId(tr.record_num_);

    vector<tuple<bool, unsigned int, double>> results;
    const boost::posix_time::time_duration track_max_time_diff =
        Time::partialSeconds(reconstructor().settings().track_max_time_diff_);

    assert (reconstructor().targets_.size() == utn_vec_.size());

    double distance_m{0}, tgt_est_std_dev{0}, tr_est_std_dev{0};

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
    };

    auto canAssocByACID = [ & ] (dbContent::targetReport::ReconstructorInfo& tr)
    {
        if (!tr.acid_)
            return false;

        return (bool) acid_2_utn_.count(*tr.acid_);
    };

    auto assocByACID = [ & ] (dbContent::targetReport::ReconstructorInfo& tr)
    {
        assert (tr.acid_);
        assert (acid_2_utn_.count(*tr.acid_));

        utn = acid_2_utn_.at(*tr.acid_);
        assert (reconstructor().targets_.count(utn));
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

        assert (tn2utn_.at(tr.ds_id_).at(tr.line_id_).count(*tr.track_number_));
        std::tie(utn, timestamp_prev) = tn2utn_.at(tr.ds_id_).at(tr.line_id_).at(*tr.track_number_);
        assert (reconstructor().targets_.count(utn));

                // check for larger time offset
        if (tr.timestamp_ - timestamp_prev > track_max_time_diff) // too old
        {
            // create new and add
            utn = createNewTarget(tr);
            assert (reconstructor().targets_.count(utn));
        }
        else if (canGetPositionOffset(tr, reconstructor().targets_.at(utn)))
        {
            assert (reconstructor().targets_.count(utn));

                    // check for position offsets
            std::tie(distance_m, tgt_est_std_dev, tr_est_std_dev) = getPositionOffset(
                tr, reconstructor().targets_.at(utn), do_debug);

            boost::optional<bool> check_result = checkPositionOffsetAcceptable(
                tr, distance_m, tgt_est_std_dev, tr_est_std_dev, true, do_debug);

            if (check_result && !*check_result)
            {
                tn2utn_[tr.ds_id_][tr.line_id_].erase(*tr.track_number_);
                reset_tr_assoc = true;
            }
        }
    };

    auto findUTNByModeACPosOrCreateNewTarget = [ & ] (dbContent::targetReport::ReconstructorInfo& tr)
    {
        // check if position match to other target would exist
        utn = findUTNByModeACPos (tr, utn_vec_, debug_rec_nums, debug_utns);

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

        utn = findUTNByModeACPos (tr, utn_vec_, debug_rec_nums, debug_utns);

        if (utn != -1)
            assert (reconstructor().targets_.count(utn));
    }

    return utn;
}

int ReconstructorAssociatorBase::findUTNByModeACPos (
    const dbContent::targetReport::ReconstructorInfo& tr,
    const std::vector<unsigned int>& utn_vec,
    const std::set<unsigned long>& debug_rec_nums,
    const std::set<unsigned int>& debug_utns)
{
    unsigned int num_targets = reconstructor().targets_.size();
    assert (utn_vec.size() == num_targets);

    vector<tuple<bool, unsigned int, double>> results;

    results.resize(num_targets);
    boost::posix_time::ptime timestamp = tr.timestamp_;

    const boost::posix_time::time_duration max_time_diff =
        Time::partialSeconds(reconstructor().settings().max_time_diff_);
    const float max_altitude_diff = reconstructor().settings().max_altitude_diff_;

    bool do_debug = debug_rec_nums.count(tr.record_num_);

    if (do_debug)
        loginf << "ReconstructorAssociatorBase: findUTNByModeACPos: rn " << tr.record_num_;

    tbb::parallel_for(uint(0), num_targets, [&](unsigned int target_cnt)
                      {
                          unsigned int other_utn = utn_vec.at(target_cnt);
                          bool do_other_debug = false; //debug_utns.count(other_utn);

                          ReconstructorTarget& other = reconstructor().targets_.at(other_utn);

                          results[target_cnt] = tuple<bool, unsigned int, double>(false, other.utn_, 0);

                          if (tr.acad_ && other.hasACAD()) // has to be covered outside
                              return;

                          if (!other.isTimeInside(timestamp, max_time_diff))
                          {
                              return;
                          }

                          if (!other.canPredict(timestamp))
                              return;

                          bool mode_a_checked = false;
                          bool mode_a_verified = false;
                          bool mode_c_checked = false;

                          if (tr.mode_a_code_ || tr.barometric_altitude_) // mode a/c based
                          {
                              // check mode a code

                              if (tr.mode_a_code_)
                              {
                                  ComparisonResult ma_res = other.compareModeACode(tr, max_time_diff);

                                  if (ma_res == ComparisonResult::DIFFERENT)
                                  {
                                      //target_cnt++;
                                      return;
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
                                      tr, max_time_diff, max_altitude_diff, false);

                                  if (mc_res == ComparisonResult::DIFFERENT)
                                  {
                                      //target_cnt++;
                                      return;
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

                          if (!canGetPositionOffset(tr, other))
                              return;

                          std::tie(distance_m, tgt_est_std_dev, tr_est_std_dev) = getPositionOffset(tr, other, do_debug);

                          boost::optional<std::pair<bool, double>> check_ret = calculatePositionOffsetScore(
                              tr, other_utn, distance_m, tgt_est_std_dev, tr_est_std_dev, mode_a_verified, do_debug);

                          if (check_ret && check_ret->first)
                          {
                              results[target_cnt] = tuple<bool, unsigned int, double> (
                                  true, other.utn_, check_ret->second);
                          }
                      });

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


int ReconstructorAssociatorBase::findUTNForTarget (unsigned int utn,
                                                  const std::set<unsigned long>& debug_rec_nums,
                                                  const std::set<unsigned int>& debug_utns)
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

    unsigned int num_utns = utn_vec_.size();
    results.resize(num_utns);

    const auto& settings = reconstructor().settings();

    const boost::posix_time::time_duration max_time_diff_tracker = Utils::Time::partialSeconds(settings.max_time_diff_);

            //computes a match score for the given other target
    auto scoreUTN = [ & ] (const std::vector<size_t>& rec_nums,
                        const dbContent::ReconstructorTarget& other,
                        unsigned int result_idx,
                        int thread_id,
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

            if (!canGetPositionOffset(tr.timestamp_, target, other))
            {
                ++pos_skipped_cnt;
                continue;
            }

                    //@TODO: debug flag
            tie(distance_m, stddev_est_target, stddev_est_other) = getPositionOffset(
                tr.timestamp_, target, other, thread_id, false);

            if (!isTargetAccuracyAcceptable(stddev_est_target)
                || !isTargetAccuracyAcceptable(stddev_est_other))
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
            secondary_verified ? pos_dubious_rate < settings.max_positions_dubious_verified_rate_
                               : pos_dubious_rate < settings.max_positions_dubious_unknown_rate_;

        if (pos_good_cnt
            && pos_dubious_rate_acceptable
            && distance_scores.size() >= settings.min_updates_tracker_)
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
                       << distance_scores.size() << " < " << settings.min_updates_tracker_;
        }
    };

#ifdef FIND_UTN_FOR_TARGET_MT
    tbb::parallel_for(uint(0), num_utns, [&](unsigned int cnt)
#else
    for (unsigned int cnt=0; cnt < num_utns; ++cnt)
#endif
                      {
#ifdef FIND_UTN_FOR_TARGET_MT
                          int thread_id = tbb::this_task_arena::current_thread_index();
#else
        int thread_id = 0;
#endif
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

                          bool print_debug = debug_utns.count(utn) && debug_utns.count(other_utn);

                          if (print_debug)
                          {
                              loginf << "\ttarget " << target.utn_ << " " << target.timeStr()
                                     << " checking other " << other.utn_ << " " << other.timeStr()
                                     << " overlaps " << target.timeOverlaps(other)
                                     << " prob " << target.probTimeOverlaps(other);
                          }

                          if (target.timeOverlaps(other)
                              && target.probTimeOverlaps(other) >= settings.prob_min_time_overlap_tracker_)
                          {
                              // check based on mode a/c/pos

                              if (print_debug)
                                  loginf << "\ttarget " << target.utn_ << " other " << other.utn_ << " overlap passed";

                              vector<unsigned long> ma_unknown; // record numbers
                              vector<unsigned long> ma_same;
                              vector<unsigned long> ma_different;

                              tie (ma_unknown, ma_same, ma_different) = target.compareModeACodes(
                                  other, max_time_diff_tracker);

                              if (print_debug)
                              {
                                  loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                         << " ma unknown " << ma_unknown.size()
                                         << " same " << ma_same.size() << " diff " << ma_different.size();
                              }

                              if (ma_same.size() > ma_different.size()
                                  && ma_same.size() >= settings.min_updates_tracker_)
                              {
                                  if (print_debug)
                                      loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                             << " mode a check passed";

                                          // check mode c codes

                                  vector<unsigned long> mc_unknown;
                                  vector<unsigned long> mc_same;
                                  vector<unsigned long> mc_different;

                                  tie (mc_unknown, mc_same, mc_different) = target.compareModeCCodes(
                                      other, ma_same, max_time_diff_tracker, settings.max_altitude_diff_, print_debug);

                                  if (print_debug)
                                  {
                                      loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                             << " ma same " << ma_same.size() << " diff " << ma_different.size()
                                             << " mc same " << mc_same.size() << " diff " << mc_different.size();
                                  }

                                  if (mc_same.size() > mc_different.size()
                                      && mc_same.size() >= settings.min_updates_tracker_)
                                  {
                                      if (print_debug)
                                          loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                                 << " mode c check passed";

                                              // check positions
                                      scoreUTN(mc_same, other, cnt, thread_id, true, print_debug);
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
                                  scoreUTN(target.target_reports_, other, cnt, thread_id, false, print_debug);
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
