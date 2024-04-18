#include "probabilisticassociator.h"
#include "probimmreconstructor.h"
#include "logger.h"
#include "global.h"
#include "stringconv.h"
#include "timeconv.h"
#include "util/tbbhack.h"
#include "reconstructortask.h"
#include "number.h"

#include <Eigen/Dense>

#include <osgEarth/GeoMath>

#include <tuple>

using namespace std;
using namespace dbContent;
using namespace Utils;

ProbabilisticAssociator::ProbabilisticAssociator(ProbIMMReconstructor& reconstructor)
    : reconstructor_(reconstructor)
{

}


void ProbabilisticAssociator::associateNewData()
{
    loginf << "ProbabilisticAssociator: associateNewData";

    const std::set<unsigned int> debug_utns = reconstructor_.task().debugUTNs();

    if (debug_utns.size())
        loginf << "DBG tns '" << String::compress(debug_utns, ',') << "'";

    const std::set<unsigned long> debug_rec_nums = reconstructor_.task().debugRecNums();

    if (debug_rec_nums.size())
        loginf << "DBG recnums '" << String::compress(debug_rec_nums, ',') << "'";

    unsigned long rec_num;
    unsigned int ds_id;
    unsigned int dbcont_id;
    boost::posix_time::ptime timestamp;
    int utn;
    boost::posix_time::ptime timestamp_prev;

    std::map<unsigned int, unsigned int> ta_2_utn = getTALookupMap(reconstructor_.targets_);
    vector<tuple<bool, unsigned int, double>> results;
    const boost::posix_time::time_duration max_time_diff = Time::partialSeconds(5);
    const boost::posix_time::time_duration track_max_time_diff = Time::partialSeconds(300.0);
    const float max_altitude_diff = 300.0;
    const float max_mahalanobis_sec_verified_dist = 10;
    const float max_mahalanobis_sec_unknown_dist = 5;
    const float max_tgt_est_std_dev = 2000;
    //targetReport::Position ref_pos;
    //bool ok;

    std::vector<unsigned int> utn_vec;

    for (auto& target_it : reconstructor_.targets_)
        utn_vec.push_back(target_it.first);

            //bool not_use_tr_pos;

    AccuracyEstimatorBase::AssociatedDistance dist;

    for (auto& ts_it : reconstructor_.tr_timestamps_)
    {
        rec_num = ts_it.second;
        dbcont_id = Number::recNumGetDBContId(rec_num);

        assert (reconstructor_.target_reports_.count(rec_num));

        bool do_debug = debug_rec_nums.count(rec_num);

        if (do_debug)
            loginf << "DBG tr " << rec_num;

        dbContent::targetReport::ReconstructorInfo& tr = reconstructor_.target_reports_.at(rec_num);

        ds_id = tr.ds_id_;

        if (!tr.in_current_slice_)
        {
            if(do_debug)
                loginf << "DBG tr " << rec_num << " not in current slice";

            continue;
        }

        utn = -1; // not yet found

            // try by track number
        if (tr.track_number_ && (dbcont_id == 62 || dbcont_id == 255))
        {
            // track number of cat062 and reftraj are reliable enough to do utn assoc based on them

            // check if not already done by acad
            // find utn

            if (tr.acad_ && ta_2_utn.count(*tr.acad_)) // already exists based on acad
            {
                utn = ta_2_utn.at(*tr.acad_);

                if (!tn2utn_[ds_id].count(*tr.track_number_))
                    tn2utn_[ds_id][*tr.track_number_] =
                        std::pair<unsigned int, boost::posix_time::ptime>(utn, timestamp);
            }
            else if (!tn2utn_[ds_id].count(*tr.track_number_))
            {
                // create new and add

                if (reconstructor_.targets_.size())
                    utn = reconstructor_.targets_.rbegin()->first + 1; // new utn
                else
                    utn = 0;

                reconstructor_.targets_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(utn),   // args for key
                    std::forward_as_tuple(reconstructor_, utn, false));  // args for mapped value tmp_utn, false

                        // add to lookup

                tn2utn_[ds_id][*tr.track_number_] = std::pair<unsigned int, boost::posix_time::ptime>(utn, timestamp);

                if (tr.acad_)
                    ta_2_utn[*tr.acad_] = utn;

                utn_vec.push_back(utn);
            }
            else // already exists
            {
                std::tie(utn, timestamp_prev) = tn2utn_.at(ds_id).at(*tr.track_number_);

                if (timestamp - timestamp_prev > track_max_time_diff) // too old
                {
                    // create new and add

                    if (reconstructor_.targets_.size())
                        utn = reconstructor_.targets_.rbegin()->first + 1; // new utn
                    else
                        utn = 0;

                    reconstructor_.targets_.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(utn),   // args for key
                        std::forward_as_tuple(reconstructor_, utn, false));  // args for mapped value tmp_utn, false

                            // add to lookup

                    tn2utn_[ds_id][*tr.track_number_] = std::pair<unsigned int, boost::posix_time::ptime>(utn, timestamp);

                    if (tr.acad_)
                        ta_2_utn[*tr.acad_] = utn;

                    utn_vec.push_back(utn);
                }
                else // ok, just update and check acad lookup
                {
                    tn2utn_[ds_id][*tr.track_number_].second = timestamp;

                    if (tr.acad_)
                    {
                        if (ta_2_utn.count(*tr.acad_))
                        {
                            if (utn != ta_2_utn.at(*tr.acad_))
                            {
                                logwrn << "ProbabilisticAssociator: associateNewData: acad issue, "
                                       << String::hexStringFromInt(*tr.acad_, 6, '0')
                                       << " found both in utn " << utn << " and lookup utn " << ta_2_utn.at(*tr.acad_);
                                ta_2_utn.at(*tr.acad_) = utn;
                            }
                        }
                    }
                }
            }
        }

                // try by mode-s address
        if (utn == -1 && tr.acad_)
        {
            // find utn

            if (ta_2_utn.count(*tr.acad_)) // already exists
            {
                utn = ta_2_utn.at(*tr.acad_);
            }
            else // not yet existing, create & add target
            {
                if (reconstructor_.targets_.size())
                    utn = reconstructor_.targets_.rbegin()->first + 1; // new utn
                else
                    utn = 0;

                reconstructor_.targets_.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(utn),   // args for key
                    std::forward_as_tuple(reconstructor_, utn, false));  // args for mapped value tmp_utn, false

                        // add to lookup
                ta_2_utn[*tr.acad_] = utn;
                utn_vec.push_back(utn);
            }

            if(do_debug)
                loginf << "DBG tr " << rec_num << " utn by acad";
        }

        if (debug_utns.count(utn))
            do_debug = true;

        if (utn == -1) // not associated by acad
        {
            if(do_debug)
                loginf << "DBG tr " << rec_num << " no utn by acad, doing mode a/c + pos";

            unsigned int num_targets = reconstructor_.targets_.size();
            assert (utn_vec.size() == num_targets);

            results.resize(num_targets);
            timestamp = tr.timestamp_;

                    //unsigned int target_cnt=0;
                    //for (auto& target_it : reconstructor_.targets_)
            tbb::parallel_for(uint(0), num_targets, [&](unsigned int target_cnt)
                              {
                                  unsigned int other_utn = utn_vec.at(target_cnt);
                                  bool do_other_debug = debug_utns.count(other_utn);

                                  ReconstructorTarget& other = reconstructor_.targets_.at(other_utn);

                                  results[target_cnt] = tuple<bool, unsigned int, double>(false, other.utn_, 0);

//                                  if (other.hasACAD()) // not only try if not mode s - could be
//                                  {
//                                      //++target_cnt;
//                                      return;
//                                  }

                                  if (!other.isTimeInside(timestamp, max_time_diff))
                                  {
                                      //++target_cnt;
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

                                      if (do_debug || do_other_debug)
                                          loginf << "DBG tr " << rec_num << " utn " << utn << " other_utn "
                                                 << other_utn << ": possible mode a match, verified "
                                                 << mode_a_verified;


                                              // check mode c code
                                      if (tr.barometric_altitude_)
                                      {
                                          ComparisonResult mc_res = other.compareModeCCode(tr, max_time_diff, max_altitude_diff, false);

                                          if (mc_res == ComparisonResult::DIFFERENT)
                                          {
                                              //target_cnt++;
                                              return;
                                          }

                                          mode_c_checked = true;
                                      }

                                      if (do_debug || do_other_debug)
                                          loginf << "DBG tr " << rec_num << " utn " << utn << " other_utn "
                                                 << other_utn << ": possible mode c match";
                                  }

                                  if (do_debug || do_other_debug)
                                      loginf << "DBG tr " << rec_num << " utn " << utn << " other_utn "
                                             << other_utn << ": mode_a_checked " << mode_a_checked
                                             << " mode_a_verified " << mode_a_verified
                                             << " mode_c_checked " << mode_c_checked;

                                          // check positions

                                  //                tie(ref_pos, ok) = other.interpolatedPosForTimeFast(
                                  //                    timestamp, max_time_diff_sensor);

                                  reconstruction::Measurement mm;
                                  bool ret;
                                  double distance_m{0}, bearing_rad{0};
                                  dbContent::targetReport::PositionAccuracy tr_pos_acc;
                                  dbContent::targetReport::PositionAccuracy mm_pos_acc;
                                  EllipseDef acc_ell;
                                  double tr_est_std_dev{0};
                                  double tgt_est_std_dev{0};
                                  double mahalanobis_dist{0};

                                  ret = other.predict(mm, tr);
                                  assert (ret);

                                  distance_m = osgEarth::GeoMath::distance(tr.position_->latitude_ * DEG2RAD,
                                                                           tr.position_->longitude_ * DEG2RAD,
                                                                           mm.lat * DEG2RAD, mm.lon * DEG2RAD);

                                  bearing_rad = osgEarth::GeoMath::bearing(tr.position_->latitude_ * DEG2RAD,
                                                                           tr.position_->longitude_ * DEG2RAD,
                                                                           mm.lat * DEG2RAD, mm.lon * DEG2RAD);

                                  tr_pos_acc = reconstructor_.acc_estimator_->positionAccuracy(tr);
                                  estimateEllipse(tr_pos_acc, acc_ell);
                                  tr_est_std_dev = estimateAccuracyAt(acc_ell, bearing_rad);

                                  if (do_debug || do_other_debug)
                                      loginf << "DBG tr " << rec_num << " utn " << utn << " other_utn "
                                             << other_utn << ": distance_m " << distance_m
                                             << " tr_est_std_dev " << tr_est_std_dev;

                                  assert (mm.hasStdDevPosition());
                                  mm_pos_acc = mm.positionAccuracy();
                                  estimateEllipse(mm_pos_acc, acc_ell);
                                  tgt_est_std_dev = estimateAccuracyAt(acc_ell, bearing_rad);

                                  if (tgt_est_std_dev > max_tgt_est_std_dev)
                                  {
                                      if (do_debug || do_other_debug)
                                          loginf << "DBG tr " << rec_num << " utn " << utn << " other_utn "
                                                 << other_utn << " tgt_est_std_dev hit maximum";
                                      return;
                                  }

                                  if (do_debug || do_other_debug)
                                      loginf << "DBG tr " << rec_num << " utn " << utn << " other_utn "
                                             << other_utn << ": distance_m " << distance_m
                                             << " tgt_est_std_dev " << tgt_est_std_dev;

                                  mahalanobis_dist = distance_m / (tr_est_std_dev + tgt_est_std_dev);

                                          //loginf << "DBG3 distance " << distance;

                                  if (do_debug || do_other_debug)
                                      loginf << "DBG tr " << rec_num << " utn " << utn << " other_utn "
                                             << other_utn << ": distance_m " << distance_m
                                             << " est_std_dev sum " << (tr_est_std_dev + tgt_est_std_dev)
                                             << "mahalanobis_dist " << mahalanobis_dist;

                                  if (mode_a_verified)
                                  {
                                      if (mahalanobis_dist < max_mahalanobis_sec_verified_dist)
                                          results[target_cnt] = tuple<bool, unsigned int, double>
                                              (true, other.utn_, mahalanobis_dist);
                                  }
                                  else
                                  {
                                      {
                                          if (mahalanobis_dist < max_mahalanobis_sec_unknown_dist)
                                              results[target_cnt] = tuple<bool, unsigned int, double>
                                                  (true, other.utn_, mahalanobis_dist);
                                      }
                                  }

                                          //++target_cnt;
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
                utn = best_other_utn;
                //tmp_assoc_utns[tr_cnt] = best_other_utn;
            }
        }

        if (utn != -1) // estimate accuarcy and associate
        {
            // add associated target reports
            assert (reconstructor_.targets_.count(utn));

                    // check if position usable
            reconstructor_.acc_estimator_->validate(tr, reconstructor_);

                    // only if not newly created
            if (!tr.do_not_use_position_ && reconstructor_.targets_.at(utn).canPredict(tr.timestamp_))
            {
                reconstruction::Measurement mm;
                bool ret;
                double distance_m, bearing_rad;
                dbContent::targetReport::PositionAccuracy tr_pos_acc;
                dbContent::targetReport::PositionAccuracy mm_pos_acc;
                EllipseDef acc_ell;
                double est_std_dev;
                double mahalanobis_dist;

                        // predict pos from target and estimate accuracies
                ret = reconstructor_.targets_.at(utn).predict(mm, tr);
                assert (ret);

                distance_m = osgEarth::GeoMath::distance(tr.position_->latitude_ * DEG2RAD,
                                                         tr.position_->longitude_ * DEG2RAD,
                                                         mm.lat * DEG2RAD, mm.lon * DEG2RAD);

                bearing_rad = osgEarth::GeoMath::bearing(tr.position_->latitude_ * DEG2RAD,
                                                         tr.position_->longitude_ * DEG2RAD,
                                                         mm.lat * DEG2RAD, mm.lon * DEG2RAD);

                tr_pos_acc = reconstructor_.acc_estimator_->positionAccuracy(tr);
                estimateEllipse(tr_pos_acc, acc_ell);
                est_std_dev = estimateAccuracyAt(acc_ell, bearing_rad);

                assert (mm.hasStdDevPosition());
                mm_pos_acc = mm.positionAccuracy();
                estimateEllipse(mm_pos_acc, acc_ell);
                est_std_dev += estimateAccuracyAt(acc_ell, bearing_rad);

                mahalanobis_dist = distance_m / est_std_dev;

                dist.latitude_deg_ = tr.position_->latitude_;
                dist.longitude_deg_ = tr.position_->longitude_;
                dist.est_std_dev_ = est_std_dev;
                dist.distance_m_ = distance_m;
                dist.mahalanobis_distance_ = mahalanobis_dist;

                reconstructor_.acc_estimator_->addAssociatedDistance(tr, dist);

                        //                not_use_tr_pos = distance_m > 50 && mahalanobis_dist > 10;

                        //                loginf << " dist " << String::doubleToStringPrecision(distance_m, 2)
                        //                       << " est_std_dev " << String::doubleToStringPrecision(est_std_dev, 2)
                        //                       << " mahala " << String::doubleToStringPrecision(mahalanobis_dist, 2)
                        //                       << " use pos " << !not_use_tr_pos;

                        //                tr.do_not_use_position_ = not_use_tr_pos;
            }

            reconstructor_.targets_.at(utn).addTargetReport(rec_num);
        }
    }

    reconstructor_.acc_estimator_->analyzeAssociatedDistances();
    reconstructor_.acc_estimator_->clearAssociatedDistances();
}

void ProbabilisticAssociator::reset()
{
    logdbg << "ProbabilisticAssociator: reset";

    tn2utn_.clear();
}


std::map<unsigned int, unsigned int> ProbabilisticAssociator::getTALookupMap (
    const std::map<unsigned int, ReconstructorTarget>& targets)
{
    logdbg << "ProbabilisticAssociator: getTALookupMap";

    std::map<unsigned int, unsigned int> ta_2_utn;

    for (auto& target_it : targets)
    {
        if (!target_it.second.hasACAD())
            continue;

        assert (target_it.second.acads_.size() == 1);

        assert (!ta_2_utn.count(*target_it.second.acads_.begin()));

        ta_2_utn[*target_it.second.acads_.begin()] = target_it.second.utn_;
    }

    logdbg << "ProbabilisticAssociator: getTALookupMap: done";

    return ta_2_utn;
}

//int ProbabilisticAssociator::findContinuationUTNForTrackerUpdate (
//    const targetReport::ReconstructorInfo& tr,
//    const std::map<unsigned int, ReconstructorTarget>& targets)
//// tries to find existing utn for tracker update, -1 if failed
//{
//    logdbg << "ProbabilisticAssociator: findContinuationUTNForTrackerUpdate";

//    if (tr.acad_)
//        return -1;

//    const boost::posix_time::time_duration max_time_diff_tracker = Time::partialSeconds(15);
//    const double max_altitude_diff_tracker = 300.0;
//    //const double max_distance_acceptable_tracker = reconstructor_.settings_.cont_max_distance_acceptable_tracker_;

//    unsigned int num_targets = targets.size();

//    vector<tuple<bool, unsigned int, double>> results;
//    // usable, other utn, distance
//    results.resize(num_targets);

//            //    for (auto& tgt_it : targets)
//            //        loginf << "UGA " << tgt_it.first;

//    tbb::parallel_for(uint(0), num_targets, [&](unsigned int cnt)
//                      {
//                         //loginf << "ProbabilisticAssociator: findContinuationUTNForTrackerUpdate: 1";
//                          //                          assert (tracker_reconstructor_.targets_vec.at(cnt));
//                          //                          unsigned int utn = tracker_reconstructor_.targets_vec.at(cnt)->utn_;
//                          assert (targets.count(cnt));

//                          const ReconstructorTarget& other = targets.at(cnt);

//                                  //Transformation trafo;

//                          results[cnt] = tuple<bool, unsigned int, double>(false, other.utn_, 0);

//                          if (!other.numAssociated()) // check if target has associated target reports
//                              return;

//                          if (other.hasACAD()) // not for mode-s targets
//                              return;

//                          if (tr.timestamp_ <= other.timestamp_max_) // check if not recently updated
//                              return;

//                                  // tr.tod_ > other.tod_max_
//                          if (tr.timestamp_ - other.timestamp_max_ > max_time_diff_tracker) // check if last updated longer ago than threshold
//                              return;

//                                  //loginf << "ProbabilisticAssociator: findContinuationUTNForTrackerUpdate: 2";

//                          assert (reconstructor_.target_reports_.count(other.lastAssociated()));

//                          const targetReport::ReconstructorInfo& other_last_tr =
//                              reconstructor_.target_reports_.at(other.lastAssociated());

//                          assert (other_last_tr.track_end_);

//                          if (!*other_last_tr.track_end_) // check if other track was ended
//                              return;

//                          if (!other_last_tr.mode_a_code_ || !tr.mode_a_code_) // check mode a codes exist
//                              return;

//                          if (!other_last_tr.mode_a_code_->hasReliableValue() || !tr.mode_a_code_->hasReliableValue())
//                              return;  // check reliable mode a codes exist

//                          if (other_last_tr.mode_a_code_->code_ != tr.mode_a_code_->code_) // check mode-a
//                              return;

//                                  // mode a codes the same

//                                  //loginf << "ProbabilisticAssociator: findContinuationUTNForTrackerUpdate: 3";

//                          if (other_last_tr.barometric_altitude_ && tr.barometric_altitude_
//                              && other_last_tr.barometric_altitude_->hasReliableValue()
//                              && tr.barometric_altitude_ ->hasReliableValue()
//                              && fabs(other_last_tr.barometric_altitude_->altitude_
//                                      - tr.barometric_altitude_->altitude_) > max_altitude_diff_tracker)
//                              return; // check mode c codes if existing

//                          bool ok;
//                          //double x_pos, y_pos;
//                          double distance;

//                          assert (tr.position_ && other_last_tr.position_);

//                          //                          tie(ok, x_pos, y_pos) = trafo.distanceCart(
//                          //                              other_last_tr.position_->latitude_, other_last_tr.position_->longitude_,
//                          //                              tr.position_->latitude_, tr.position_->longitude_);

//                          //                          if (!ok)
//                          //                              return;

//                          //                          distance = sqrt(pow(x_pos,2)+pow(y_pos,2));

//                          distance = osgEarth::GeoMath::distance(
//                              other_last_tr.position_->latitude_ * DEG2RAD,
//                              other_last_tr.position_->longitude_  * DEG2RAD,
//                              tr.position_->latitude_ * DEG2RAD, tr.position_->longitude_ * DEG2RAD);

//                          if (distance > max_distance_acceptable_tracker)
//                              return;

//                                  //loginf << "ProbabilisticAssociator: findContinuationUTNForTrackerUpdate: 4";

//                          results[cnt] = tuple<bool, unsigned int, double>(
//                              true, other.utn_, distance);
//                      });

//    logdbg << "ProbabilisticAssociator: findContinuationUTNForTrackerUpdate: finding best matches";

//            // find best match
//    unsigned int num_matches = 0;

//    bool usable;
//    unsigned int other_utn;
//    double distance;

//    bool first = true;
//    unsigned int best_other_utn;
//    double best_distance;

//    for (auto& res_it : results) // usable, other utn, distance
//    {
//        tie(usable, other_utn, distance) = res_it;

//        if (!usable)
//            continue;

//        ++num_matches;

//        if (first || distance < best_distance)
//        {
//            best_other_utn = other_utn;
//            best_distance = distance;

//            first = false;
//        }
//    }

//    if (first)
//        return -1;

//    if (num_matches > 1)
//    {
//        logdbg << "ProbabilisticAssociator: findContinuationUTNForTrackerUpdate: " << num_matches << " found";
//        return -1;
//    }

//    logdbg << "ProbabilisticAssociator: findContinuationUTNForTrackerUpdate: continuation match utn "
//           << best_other_utn << " found, distance " << best_distance;

//    return best_other_utn;
//}

        // tries to find existing utn for target report, -1 if failed
//int ProbabilisticAssociator::findUTNForTargetReport (
//    const dbContent::targetReport::ReconstructorInfo& tr,
//    const std::vector<unsigned int>& utn_vec,
//    const std::map<unsigned int, dbContent::ReconstructorTarget>& targets)
//{
//    return -1;
//}


void ProbabilisticAssociator::estimateEllipse(dbContent::targetReport::PositionAccuracy& acc, EllipseDef& def) const
{
    Eigen::Matrix2f cov_mat;
    cov_mat << std::pow(acc.x_stddev_, 2), acc.xy_cov_, acc.xy_cov_, std::pow(acc.y_stddev_, 2);

    Eigen::JacobiSVD<Eigen::MatrixXf> svd(cov_mat, Eigen::ComputeThinU); //  | ComputeThinV

    auto singular_values = svd.singularValues();
    auto U = svd.matrixU();

    def.theta_rad = std::atan2(U(1,0), U(0, 0));
    def.rad1      = std::sqrt(singular_values(0));
    def.rad2      = std::sqrt(singular_values(1));
}

double ProbabilisticAssociator::estimateAccuracyAt (EllipseDef& def, double bearing_rad) const
{
    double x_e, y_e;

    x_e = def.rad1 * std::cos(def.theta_rad) * std::cos(bearing_rad) - def.rad2 * std::sin(def.theta_rad) * std::sin(bearing_rad);
    y_e = def.rad1 * std::sin(def.theta_rad) * std::cos(bearing_rad) + def.rad2 * std::cos(def.theta_rad) * std::sin(bearing_rad);

    return std::sqrt(std::pow(x_e, 2) + std::pow(y_e, 2));
}
