#include "accuracyestimatorbase.h"
#include "datasourcemanager.h"
#include "dbdatasource.h"
#include "compass.h"
#include "logger.h"
#include "stringconv.h"


using namespace Utils;

const double AccuracyEstimatorBase::PosAccStdDevMin = 1.0;
const dbContent::targetReport::PositionAccuracy AccuracyEstimatorBase::PosAccStdMin
    { AccuracyEstimatorBase::PosAccStdDevMin, AccuracyEstimatorBase::PosAccStdDevMin, 0};

const double AccuracyEstimatorBase::VelAccStdDevMin = 2.0;
const dbContent::targetReport::VelocityAccuracy AccuracyEstimatorBase::VelAccStdMin
    { AccuracyEstimatorBase::VelAccStdDevMin, AccuracyEstimatorBase::VelAccStdDevMin};

const double AccuracyEstimatorBase::AccAccStdDevMin = 3.0;
const dbContent::targetReport::AccelerationAccuracy AccuracyEstimatorBase::AccAccStdMin
    { AccuracyEstimatorBase::AccAccStdDevMin, AccuracyEstimatorBase::AccAccStdDevMin};

const double AccuracyEstimatorBase::PosAccStdDevFallback = 1000.0;
const dbContent::targetReport::PositionAccuracy AccuracyEstimatorBase::PosAccStdFallback
    { AccuracyEstimatorBase::PosAccStdDevFallback, AccuracyEstimatorBase::PosAccStdDevFallback, 0};

const double AccuracyEstimatorBase::PosAccStdDevMax = 5000.0;
const dbContent::targetReport::PositionAccuracy AccuracyEstimatorBase::PosAccStdMax
    { AccuracyEstimatorBase::PosAccStdDevMax, AccuracyEstimatorBase::PosAccStdDevMax, 0};

const double AccuracyEstimatorBase::VelAccStdDevFallback = 1000.0;

const dbContent::targetReport::VelocityAccuracy AccuracyEstimatorBase::VelAccStdFallback
    { AccuracyEstimatorBase::VelAccStdDevFallback, AccuracyEstimatorBase::VelAccStdDevFallback};


const double AccuracyEstimatorBase::AccAccStdDevFallback = 1000.0;

const dbContent::targetReport::AccelerationAccuracy AccuracyEstimatorBase::AccAccStdFallback
    { AccuracyEstimatorBase::AccAccStdDevFallback, AccuracyEstimatorBase::AccAccStdDevFallback};

AccuracyEstimatorBase::AccuracyEstimatorBase()
//    : ds_man_ (COMPASS::instance().dataSourceManager())
{
    //    connect (&ds_man_, &DataSourceManager::dataSourcesChangedSignal,
    //            this, &AccuracyEstimatorBase::updateDataSourcesInfoSlot);
}

void AccuracyEstimatorBase::init(ReconstructorBase* reconstructor)
{
    logdbg << "AccuracyEstimatorBase: init";

    assert (reconstructor);
    reconstructor_ = reconstructor;
    distances_.clear();
}

void AccuracyEstimatorBase::addAssociatedDistance(
    dbContent::targetReport::ReconstructorInfo& tr, const AssociatedDistance& dist)
{
    distances_.push_back(dist);
}

void AccuracyEstimatorBase::analyzeAssociatedDistances() const
{
    if (!distances_.size())
        return;

    std::function<double(const AssociatedDistance&)> dist_lambda =
        [] (const AssociatedDistance& dist) { return dist.distance_m_; };

    std::function<double(const AssociatedDistance&)> estacc_lambda =
        [] (const AssociatedDistance& dist) { return dist.est_std_dev_; };

    std::function<double(const AssociatedDistance&)> ma_lambda =
        [] (const AssociatedDistance& dist) { return dist.mahalanobis_distance_; };

//    printStatistics("distance ", dist_lambda);
//    printStatistics("est acc ", estacc_lambda);
//    printStatistics("maha dist", ma_lambda);

//    loginf << "";
}

void AccuracyEstimatorBase::clearAssociatedDistances()
{
    distances_.clear();
}

void AccuracyEstimatorBase::printStatistics (
    const std::string name, std::function<double(const AssociatedDistance&)>& lambda)  const
{
    if (!distances_.size())
        return;

    double sum = std::accumulate(distances_.begin(), distances_.end(), 0.0,
                                 [lambda] (double sum, const AssociatedDistance& dist) { return sum + lambda(dist); });
    double mean = sum / distances_.size();

    std::vector<double> diff(distances_.size());
    std::transform(distances_.begin(), distances_.end(), diff.begin(),
                   [mean, lambda](const AssociatedDistance& dist) { return lambda(dist) - mean; });
    double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);
    double stdev = std::sqrt(sq_sum / distances_.size());

    auto comp = [lambda] (const AssociatedDistance& a, const AssociatedDistance& b) { return lambda(a) < lambda(b); };

    double min = lambda(*std::min_element(distances_.begin(), distances_.end(), comp));
    double max = lambda(*std::max_element(distances_.begin(), distances_.end(), comp));

    loginf << name << " SRC " << name_ << ": "  << " avg " << String::doubleToStringPrecision(mean, 2)
           << " stddev " << String::doubleToStringPrecision(stdev, 2) << " min " << min << " max " << max;
}
