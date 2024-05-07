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


bool ProbabilisticAssociator::canGetPositionOffset(const dbContent::targetReport::ReconstructorInfo& tr,
                                                   const dbContent::ReconstructorTarget& target)
{
    return tr.position_ && target.canPredict(tr.timestamp_);
}

// distance, target acc, tr acc
std::tuple<double, double, double> ProbabilisticAssociator::getPositionOffset(const dbContent::targetReport::ReconstructorInfo& tr,
                                                                              const dbContent::ReconstructorTarget& target, 
                                                                              bool do_debug)
{
    reconstruction::Measurement mm;
    bool ret;
    double distance_m{0}, bearing_rad{0};
    dbContent::targetReport::PositionAccuracy tr_pos_acc;
    dbContent::targetReport::PositionAccuracy mm_pos_acc;
    EllipseDef acc_ell;
    double tgt_est_std_dev{0};
    double tr_est_std_dev{0};

    ret = target.predict(mm, tr);
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

    if (do_debug)
        loginf << "DBG tr " << tr.record_num_ << " other_utn "
               << target.utn_ << ": distance_m " << distance_m
               << " tr_est_std_dev " << tr_est_std_dev;

    assert (mm.hasStdDevPosition());
    mm_pos_acc = mm.positionAccuracy();
    estimateEllipse(mm_pos_acc, acc_ell);
    tgt_est_std_dev = estimateAccuracyAt(acc_ell, bearing_rad);

            // distance, target acc, tr acc
    return std::tuple<double, double, double>(distance_m, tgt_est_std_dev, tr_est_std_dev);
}

bool ProbabilisticAssociator::canGetPositionOffset(const boost::posix_time::ptime& ts,
                                                   const dbContent::ReconstructorTarget& target0,
                                                   const dbContent::ReconstructorTarget& target1)
{
    bool ok = (target0.canPredict(ts) && target1.canPredict(ts));

            // if (ok)
            // {
            //     loginf << "ProbabilisticAssociator: canGetPositionOffset: can get offset: "
            //            << "t = " << Utils::Time::toString(ts) << " "
            //            << "utn0 = " << target0.utn_ << " (" << target0.trackerCount() << ") "
            //            << "utn1 = " << target1.utn_ << " (" << target1.trackerCount() << ")";
            // }
    
    return ok;
}

// distance, target0 acc, target1 acc
std::tuple<double, double, double> ProbabilisticAssociator::getPositionOffset(const boost::posix_time::ptime& ts,
                                                                              const dbContent::ReconstructorTarget& target0,
                                                                              const dbContent::ReconstructorTarget& target1, 
                                                                              int thread_id,
                                                                              bool do_debug)
{
    reconstruction::Measurement mm0, mm1;
    bool ret;
    double distance_m{0}, bearing_rad{0};
    dbContent::targetReport::PositionAccuracy tr_pos_acc;
    dbContent::targetReport::PositionAccuracy mm0_pos_acc, mm1_pos_acc;
    EllipseDef acc_ell;
    double mm0_est_std_dev{0};
    double mm1_est_std_dev{0};

    ret = target0.predict(mm0, ts, thread_id) && target1.predict(mm1, ts, thread_id);
    assert (ret);

    distance_m = osgEarth::GeoMath::distance(mm0.lat * DEG2RAD,
                                             mm0.lon * DEG2RAD,
                                             mm1.lat * DEG2RAD, 
                                             mm1.lon * DEG2RAD);

    bearing_rad = osgEarth::GeoMath::bearing(mm0.lat * DEG2RAD,
                                             mm0.lon * DEG2RAD,
                                             mm1.lat * DEG2RAD, 
                                             mm1.lon * DEG2RAD);

    assert (mm0.hasStdDevPosition());
    mm0_pos_acc = mm0.positionAccuracy();
    estimateEllipse(mm0_pos_acc, acc_ell);
    mm0_est_std_dev = estimateAccuracyAt(acc_ell, bearing_rad);

    assert (mm1.hasStdDevPosition());
    mm1_pos_acc = mm1.positionAccuracy();
    estimateEllipse(mm1_pos_acc, acc_ell);
    mm1_est_std_dev = estimateAccuracyAt(acc_ell, bearing_rad);

    if (do_debug)
    {
        loginf << "DBG utn0 " << target0.utn_ << " utn1 " << target1.utn_ << ":"
               << " distance_m " << distance_m 
               << " stddev0 " << mm0_est_std_dev 
               << " stddev1 " << mm1_est_std_dev;
    }

            // distance, target0 acc, target1 acc
    return std::tuple<double, double, double>(distance_m, mm0_est_std_dev, mm1_est_std_dev);
}

boost::optional<bool> ProbabilisticAssociator::checkPositionOffsetAcceptable (
    const dbContent::targetReport::ReconstructorInfo& tr,
    double distance_m, double tgt_est_std_dev, double tr_est_std_dev, bool secondary_verified,
    bool do_debug)
{
    if (isTargetAccuracyAcceptable(tgt_est_std_dev)) // target estimate reliable enough to break up
    {
        double sum_est_std_dev = tgt_est_std_dev + tr_est_std_dev;

        if (sum_est_std_dev > reconstructor_.settings().max_sum_est_std_dev_)
            sum_est_std_dev = reconstructor_.settings().max_sum_est_std_dev_;
        if (sum_est_std_dev < reconstructor_.settings().min_sum_est_std_dev_)
            sum_est_std_dev = reconstructor_.settings().min_sum_est_std_dev_;

        double mahalanobis_dist = distance_m / sum_est_std_dev;

        if (secondary_verified)
        {
            if (mahalanobis_dist > reconstructor_.settings().max_mahalanobis_sec_verified_dist_)
            {
                // position offset too large
                return false;
            }
        }
        else
        {
            if (mahalanobis_dist > reconstructor_.settings().max_mahalanobis_sec_unknown_dist_)
            {
                return false;
            }
        }
    }

    return {}; // no check possible
}

boost::optional<std::pair<bool, double>> ProbabilisticAssociator::calculatePositionOffsetScore (
    const dbContent::targetReport::ReconstructorInfo& tr, unsigned int other_utn,
    double distance_m, double tgt_est_std_dev, double tr_est_std_dev, bool secondary_verified,
    bool do_debug)
{
    if (!isTargetAccuracyAcceptable(tgt_est_std_dev))
    {
        if (do_debug)
            loginf << "DBG tr " << tr.record_num_ << " other_utn "
                   << other_utn << " tgt_est_std_dev hit maximum";
        return {};
    }

    double sum_est_std_dev = tgt_est_std_dev + tr_est_std_dev;

    if (sum_est_std_dev > reconstructor_.settings().max_sum_est_std_dev_)
    {
        if (do_debug)
            loginf << "DBG tr " << tr.record_num_ << " other_utn "
                   << other_utn << " sum_est_std_dev hit maximum";

        sum_est_std_dev = reconstructor_.settings().max_sum_est_std_dev_;
    }

    if (sum_est_std_dev < reconstructor_.settings().min_sum_est_std_dev_)
        sum_est_std_dev = reconstructor_.settings().min_sum_est_std_dev_;

    if (do_debug)
        loginf << "DBG tr " << tr.record_num_ << " other_utn "
               << other_utn << ": distance_m " << distance_m
               << " sum_est_std_dev " << sum_est_std_dev;

    double mahalanobis_dist = distance_m / sum_est_std_dev;

            //loginf << "DBG3 distance " << distance;

    if (secondary_verified)
    {
        if (do_debug)
        {
            loginf << "DBG tr " << tr.record_num_ << " other_utn "
                   << other_utn << ": distance_m " << distance_m
                   << " sum_est_std_dev sum " << sum_est_std_dev
                   << " mahalanobis_dist " << mahalanobis_dist
                   << " ver ok " << (mahalanobis_dist < reconstructor_.settings().max_mahalanobis_sec_verified_dist_);
        }

        return std::pair<bool, double> (
            mahalanobis_dist < reconstructor_.settings().max_mahalanobis_sec_verified_dist_, mahalanobis_dist);
    }
    else
    {
        if (do_debug)
        {
            loginf << "DBG tr " << tr.record_num_ << " other_utn "
                   << other_utn << ": distance_m " << distance_m
                   << " sum_est_std_dev sum " << sum_est_std_dev
                   << " mahalanobis_dist " << mahalanobis_dist
                   << " nver ok " << (mahalanobis_dist < reconstructor_.settings().max_mahalanobis_sec_unknown_dist_);
        }

        return std::pair<bool, double> (
            mahalanobis_dist < reconstructor_.settings().max_mahalanobis_sec_unknown_dist_, mahalanobis_dist);
    }
}

std::tuple<ReconstructorAssociatorBase::DistanceClassification, double>
ProbabilisticAssociator::checkPositionOffsetScore (double distance_m, double sum_stddev_est, bool secondary_verified)
{
    const auto& settings = reconstructor_.settings();

    if (sum_stddev_est > settings.max_sum_est_std_dev_)
        sum_stddev_est = reconstructor_.settings().max_sum_est_std_dev_;

    if (sum_stddev_est < settings.min_sum_est_std_dev_)
        sum_stddev_est = settings.min_sum_est_std_dev_;

    double mahalanobis_dist = distance_m / sum_stddev_est;

    // subtract acceptable mahala from score to get scaled minimum after
    if (secondary_verified)
    {
        if (mahalanobis_dist > settings.max_mahalanobis_quit_verified_)
            return tuple<DistanceClassification, double>(
                DistanceClassification::Distance_NotOK,
                mahalanobis_dist - settings.max_mahalanobis_acceptable_verified_);
        if (mahalanobis_dist > settings.max_mahalanobis_dubious_verified_)
            return tuple<DistanceClassification, double>(
                DistanceClassification::Distance_Dubious,
                mahalanobis_dist - settings.max_mahalanobis_acceptable_verified_);
        if (mahalanobis_dist < settings.max_mahalanobis_acceptable_verified_)
            return tuple<DistanceClassification, double>
                (DistanceClassification::Distance_Good,
                 mahalanobis_dist - settings.max_mahalanobis_acceptable_verified_);

        return tuple<DistanceClassification, double>(
            DistanceClassification::Distance_Acceptable,
            mahalanobis_dist - settings.max_mahalanobis_acceptable_verified_);
    }
    else
    {
        if (mahalanobis_dist > settings.max_mahalanobis_quit_unverified_)
            return tuple<DistanceClassification, double>(
                DistanceClassification::Distance_NotOK,
                mahalanobis_dist - settings.max_mahalanobis_acceptable_unverified_);
        if (mahalanobis_dist > settings.max_mahalanobis_dubious_unverified_)
            return tuple<DistanceClassification, double>(
                DistanceClassification::Distance_Dubious,
                mahalanobis_dist - settings.max_mahalanobis_acceptable_unverified_);
        if (mahalanobis_dist < settings.max_mahalanobis_acceptable_unverified_)
            return tuple<DistanceClassification, double>(
                DistanceClassification::Distance_Good,
                mahalanobis_dist - settings.max_mahalanobis_acceptable_unverified_);

        return tuple<DistanceClassification, double>(
            DistanceClassification::Distance_Acceptable,
            mahalanobis_dist - settings.max_mahalanobis_acceptable_unverified_);
    }
}

bool ProbabilisticAssociator::isTargetAccuracyAcceptable(double tgt_est_std_dev)
{
    return tgt_est_std_dev <= reconstructor_.settings().max_tgt_est_std_dev_;
}

bool ProbabilisticAssociator::isTargetAverageDistanceAcceptable(double distance_score_avg, bool secondary_verified)
{
    if (secondary_verified)
        return distance_score_avg < reconstructor_.settings().max_mahalanobis_acceptable_verified_;
    else
        return distance_score_avg < reconstructor_.settings().max_mahalanobis_acceptable_unverified_;
}

ReconstructorBase& ProbabilisticAssociator::reconstructor()
{
    return reconstructor_;
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
