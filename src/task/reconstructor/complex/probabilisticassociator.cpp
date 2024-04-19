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

    checkACADLookup();

    vector<tuple<bool, unsigned int, double>> results;
    const boost::posix_time::time_duration max_time_diff =
        Time::partialSeconds(reconstructor_.settings().max_time_diff_);
    const boost::posix_time::time_duration track_max_time_diff =
        Time::partialSeconds(reconstructor_.settings().track_max_time_diff_);
    const float max_altitude_diff = reconstructor_.settings().max_altitude_diff_;
    const float max_mahalanobis_sec_verified_dist = reconstructor_.settings().max_mahalanobis_sec_verified_dist_;
    const float max_mahalanobis_sec_unknown_dist = reconstructor_.settings().max_mahalanobis_sec_unknown_dist_;
    const float max_tgt_est_std_dev = reconstructor_.settings().max_tgt_est_std_dev_;

    assert (reconstructor_.targets_.size() == utn_vec_.size());

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

            if (!tr.acad_ && tn2utn_[ds_id].count(*tr.track_number_)) // no acad but already mapped
            {

                // check/add using track number mapping
                std::tie(utn, timestamp_prev) = tn2utn_.at(ds_id).at(*tr.track_number_);

                        // TODO could also check for position offsets

                        // check for larger time offset
                if (timestamp - timestamp_prev > track_max_time_diff) // too old
                {
                    // create new and add
                    utn = createNewTarget(tr);
                }
                else // time ok, just update and check acad lookup
                {
                    tn2utn_[ds_id][*tr.track_number_].second = timestamp;

                            // check acad mapping
                    if (tr.acad_ && acad_2_utn_.count(*tr.acad_))
                    {
                        if (utn != acad_2_utn_.at(*tr.acad_))
                        {
                            logwrn << "ProbabilisticAssociator: associateNewData: acad issue, "
                                   << String::hexStringFromInt(*tr.acad_, 6, '0')
                                   << " found both in utn " << utn << " and lookup utn " << acad_2_utn_.at(*tr.acad_);

                            acad_2_utn_.at(*tr.acad_) = utn;
                        }
                    }
                }
            }
            else if (tr.acad_) // has mode s address, may already be mapped by track number
            {
                if (!acad_2_utn_.count(*tr.acad_) && tn2utn_[ds_id].count(*tr.track_number_)) // not already existing, create
                {
                    utn = createNewTarget(tr);
                }
                else if (acad_2_utn_.count(*tr.acad_)) // already mapped by acad
                    utn = acad_2_utn_.at(*tr.acad_);
                else if (tn2utn_[ds_id].count(*tr.track_number_)) // already mapped by tn
                {
                    // check/add using track number mapping
                    std::tie(utn, timestamp_prev) = tn2utn_.at(ds_id).at(*tr.track_number_);

                            // TODO could also check for position offsets

                            // check for larger time offset
                    if (timestamp - timestamp_prev > track_max_time_diff) // too old
                    {
                        // create new and add

                        utn = createNewTarget(tr);
                    }
                    else // time ok, just update and check acad lookup
                    {
                        tn2utn_[ds_id][*tr.track_number_].second = timestamp;

                                // check acad mapping
                        if (tr.acad_ && acad_2_utn_.count(*tr.acad_))
                        {
                            if (utn != acad_2_utn_.at(*tr.acad_))
                            {
                                logwrn << "ProbabilisticAssociator: associateNewData: acad issue, "
                                       << String::hexStringFromInt(*tr.acad_, 6, '0')
                                       << " found both in utn " << utn << " and lookup utn " << acad_2_utn_.at(*tr.acad_);

                                acad_2_utn_.at(*tr.acad_) = utn;
                            }
                        }
                    }
                }
                else // track num, acad present - but not yet mapped
                {

                        // not mapped, create new

                    utn = createNewTarget(tr);
                }
            }
            else // have unmapped track number and no mode s - do by mode a/c and position
            {
                utn = findUTNForTargetReport (tr, utn_vec_, debug_rec_nums, debug_utns);

                if (utn == -1)
                { // do based on track number alone
                    // create new and add

                   utn = createNewTarget(tr);
                }
            }
        }

                // try by mode-s address
        if (utn == -1 && tr.acad_)
        {
            // find utn

            if (acad_2_utn_.count(*tr.acad_)) // already exists
            {
                utn = acad_2_utn_.at(*tr.acad_);
            }
            else // not yet existing, create & add target
            {
                utn = createNewTarget(tr);
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

            utn = findUTNForTargetReport (tr, utn_vec_, debug_rec_nums, debug_utns);

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

    checkACADLookup();

    reconstructor_.acc_estimator_->analyzeAssociatedDistances();
    reconstructor_.acc_estimator_->clearAssociatedDistances();
}

void ProbabilisticAssociator::reset()
{
    logdbg << "ProbabilisticAssociator: reset";

    utn_vec_.clear();
    acad_2_utn_.clear();
    tn2utn_.clear();
}


        // tries to find existing utn for target report, based on mode a/c and position, -1 if failed
int ProbabilisticAssociator::findUTNForTargetReport (
    const dbContent::targetReport::ReconstructorInfo& tr,
    const std::vector<unsigned int>& utn_vec,
    const std::set<unsigned long>& debug_rec_nums,
    const std::set<unsigned int>& debug_utns)
{
    unsigned int num_targets = reconstructor_.targets_.size();
    assert (utn_vec.size() == num_targets);

    vector<tuple<bool, unsigned int, double>> results;

    results.resize(num_targets);
    boost::posix_time::ptime timestamp = tr.timestamp_;

    const boost::posix_time::time_duration max_time_diff =
        Time::partialSeconds(reconstructor_.settings().max_time_diff_);
    const float max_altitude_diff = reconstructor_.settings().max_altitude_diff_;

    const float max_mahalanobis_sec_verified_dist = reconstructor_.settings().max_mahalanobis_sec_verified_dist_;
    const float max_mahalanobis_sec_unknown_dist = reconstructor_.settings().max_mahalanobis_sec_unknown_dist_;
    const float max_tgt_est_std_dev = reconstructor_.settings().max_tgt_est_std_dev_;

    bool do_debug = debug_rec_nums.count(tr.record_num_);

    tbb::parallel_for(uint(0), num_targets, [&](unsigned int target_cnt)
                      {
                          unsigned int other_utn = utn_vec.at(target_cnt);
                          bool do_other_debug = debug_utns.count(other_utn);

                          ReconstructorTarget& other = reconstructor_.targets_.at(other_utn);

                          results[target_cnt] = tuple<bool, unsigned int, double>(false, other.utn_, 0);

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

                              if (do_debug || do_other_debug)
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

                              if (do_debug || do_other_debug)
                                  loginf << "DBG tr " << tr.record_num_ << " other_utn "
                                         << other_utn << ": possible mode c match";
                          }

                          if (do_debug || do_other_debug)
                              loginf << "DBG tr " << tr.record_num_ << " other_utn "
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
                              loginf << "DBG tr " << tr.record_num_ << " other_utn "
                                     << other_utn << ": distance_m " << distance_m
                                     << " tr_est_std_dev " << tr_est_std_dev;

                          assert (mm.hasStdDevPosition());
                          mm_pos_acc = mm.positionAccuracy();
                          estimateEllipse(mm_pos_acc, acc_ell);
                          tgt_est_std_dev = estimateAccuracyAt(acc_ell, bearing_rad);

                          if (tgt_est_std_dev > max_tgt_est_std_dev)
                          {
                              if (do_debug || do_other_debug)
                                  loginf << "DBG tr " << tr.record_num_ << " other_utn "
                                         << other_utn << " tgt_est_std_dev hit maximum";
                              return;
                          }

                          if (do_debug || do_other_debug)
                              loginf << "DBG tr " << tr.record_num_ << " other_utn "
                                     << other_utn << ": distance_m " << distance_m
                                     << " tgt_est_std_dev " << tgt_est_std_dev;

                          mahalanobis_dist = distance_m / (tr_est_std_dev + tgt_est_std_dev);

                                  //loginf << "DBG3 distance " << distance;

                          if (do_debug || do_other_debug)
                              loginf << "DBG tr " << tr.record_num_ << " other_utn "
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
        return best_other_utn;
    }

    return -1;
}

unsigned int ProbabilisticAssociator::createNewTarget(const dbContent::targetReport::ReconstructorInfo& tr)
{
    unsigned int utn;

    if (reconstructor_.targets_.size())
        utn = reconstructor_.targets_.rbegin()->first + 1; // new utn
    else
        utn = 0;

    reconstructor_.targets_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(utn),   // args for key
        std::forward_as_tuple(reconstructor_, utn, false));  // args for mapped value tmp_utn, false

            // add to lookup

    if (tr.track_number_)
        tn2utn_[tr.ds_id_][*tr.track_number_] = std::pair<unsigned int, boost::posix_time::ptime>(utn, tr.timestamp_);

    if (tr.acad_)
        acad_2_utn_[*tr.acad_] = utn;

    utn_vec_.push_back(utn);

    return utn;
}

void ProbabilisticAssociator::checkACADLookup()
{
    unsigned int acad;

    unsigned int count = 0;

    for (auto& target_it : reconstructor_.targets_)
    {
        if (!target_it.second.hasACAD())
            continue;

        assert (target_it.second.acads_.size() == 1);

        acad = *target_it.second.acads_.begin();

        if (!acad_2_utn_.count(acad))
        {
            logerr << "ProbabilisticAssociator: getTALookupMap: acad "
                   << String::hexStringFromInt(acad, 6, '0')
                   << " not in lookup";
        }

        ++count;
    }

    assert (acad_2_utn_.size() == count);
}

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
