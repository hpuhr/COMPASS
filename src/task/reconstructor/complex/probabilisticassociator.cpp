#include "probabilisticassociator.h"
#include "probimmreconstructor.h"
#include "logger.h"
#include "global.h"
#include "stringconv.h"

#include <Eigen/Dense>

#include <osgEarth/GeoMath>

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

    unsigned long rec_num;
    int utn;

    std::map<unsigned int, unsigned int> ta_2_utn = getTALookupMap(reconstructor_.targets_);

    reconstruction::Measurement mm;
    bool ret;
    double distance_m, bearing_rad;
    dbContent::targetReport::PositionAccuracy tr_pos_acc;
    dbContent::targetReport::PositionAccuracy mm_pos_acc;
    EllipseDef acc_ell;
    double est_std_dev;
    double mahalanobis_dist;
    bool not_use_tr_pos;

    AccuracyEstimatorBase::AssociatedDistance dist;

    for (auto& ts_it : reconstructor_.tr_timestamps_)
    {
        rec_num = ts_it.second;

        assert (reconstructor_.target_reports_.count(rec_num));

        dbContent::targetReport::ReconstructorInfo& tr = reconstructor_.target_reports_.at(rec_num);

        if (!tr.in_current_slice_)
            continue;

        // try by mode-s address
        if (tr.acad_)
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

            }

            // add associated target reports
            assert (reconstructor_.targets_.count(utn));

            reconstructor_.acc_estimator_->validate(tr, reconstructor_);

            // only if not newly created
            if (!tr.do_not_use_position_ && reconstructor_.targets_.at(utn).canPredict(tr.timestamp_))
            {
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
            continue; // done
        }
    }

    reconstructor_.acc_estimator_->analyzeAssociatedDistances();
    reconstructor_.acc_estimator_->clearAssociatedDistances();
}

void ProbabilisticAssociator::reset()
{
    logdbg << "ProbabilisticAssociator: reset";
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

    logdbg << "SimpleAssociator: getTALookupMap: done";

    return ta_2_utn;
}

        // tries to find existing utn for target report, -1 if failed
int ProbabilisticAssociator::findUTNForTargetReport (
    const dbContent::targetReport::ReconstructorInfo& tr,
    std::map<unsigned int, unsigned int> ta_2_utn,
    const std::map<unsigned int, dbContent::ReconstructorTarget>& targets)
{
    return -1;
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
