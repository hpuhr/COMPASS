#include "complexaccuracyestimator.h"
#include "compass.h"
#include "datasourcemanager.h"
#include "dbdatasource.h"
#include "adsbaccuracyestimator.h"
#include "mlataccuracyestimator.h"
#include "trackeraccuracyestimator.h"
#include "radaraccuracyestimator.h"
#include "number.h"
#include "logger.h"

using namespace std;
using namespace Utils;

//const double ComplexAccuracyEstimator::PosAccStdDevDefault = 100.0;
//const dbContent::targetReport::PositionAccuracy ComplexAccuracyEstimator::PosAccStdDefault
//    { ComplexAccuracyEstimator::PosAccStdDevDefault, ComplexAccuracyEstimator::PosAccStdDevDefault, 0};

//const double ComplexAccuracyEstimator::VelAccStdDevDefault = 30.0;
//const double ComplexAccuracyEstimator::VelAccStdDevDefaultCAT021 = 10.0;
//const double ComplexAccuracyEstimator::VelAccStdDevDefaultCAT062 = 20.0;

//const dbContent::targetReport::VelocityAccuracy ComplexAccuracyEstimator::VelAccStdDefault
//    { ComplexAccuracyEstimator::VelAccStdDevDefault, ComplexAccuracyEstimator::VelAccStdDevDefault};
//const dbContent::targetReport::VelocityAccuracy ComplexAccuracyEstimator::VelAccStdDefaultCAT021
//    { ComplexAccuracyEstimator::VelAccStdDevDefaultCAT021, ComplexAccuracyEstimator::VelAccStdDevDefaultCAT021};
//const dbContent::targetReport::VelocityAccuracy ComplexAccuracyEstimator::VelAccStdDefaultCAT062
//    { ComplexAccuracyEstimator::VelAccStdDevDefaultCAT062, ComplexAccuracyEstimator::VelAccStdDevDefaultCAT062};


//const double ComplexAccuracyEstimator::AccAccStdDevDefault = 30.0;
//const double ComplexAccuracyEstimator::AccAccStdDevDefaultCAT021 = 10.0;
//const double ComplexAccuracyEstimator::AccAccStdDevDefaultCAT062 = 20.0;

//const dbContent::targetReport::AccelerationAccuracy ComplexAccuracyEstimator::AccAccStdDefault
//    { ComplexAccuracyEstimator::AccAccStdDevDefault, ComplexAccuracyEstimator::AccAccStdDevDefault};
//const dbContent::targetReport::AccelerationAccuracy ComplexAccuracyEstimator::AccAccStdDefaultCAT021
//    { ComplexAccuracyEstimator::AccAccStdDevDefaultCAT021, ComplexAccuracyEstimator::AccAccStdDevDefaultCAT021};
//const dbContent::targetReport::AccelerationAccuracy ComplexAccuracyEstimator::AccAccStdDefaultCAT062
//    { ComplexAccuracyEstimator::AccAccStdDevDefaultCAT062, ComplexAccuracyEstimator::AccAccStdDevDefaultCAT062};

ComplexAccuracyEstimator::ComplexAccuracyEstimator()
{

}

ComplexAccuracyEstimator::~ComplexAccuracyEstimator()
{
    //loginf << "ComplexAccuracyEstimator: dtor";

    ds_acc_estimators_.clear();

    initialized_ = false;

    //loginf << "ComplexAccuracyEstimator: dtor: done";
}

void ComplexAccuracyEstimator::init(ReconstructorBase* reconstructor)
{
    loginf << "ComplexAccuracyEstimator: init";

    AccuracyEstimatorBase::init(reconstructor);

    connect(&COMPASS::instance().dataSourceManager(), &DataSourceManager::dataSourcesChangedSignal,
            this, &ComplexAccuracyEstimator::updateDataSourcesInfoSlot);

    updateDataSourcesInfoSlot();

    initialized_ = true;
}

void ComplexAccuracyEstimator::validate (
    dbContent::targetReport::ReconstructorInfo& tr, ReconstructorBase& reconstructor)
{
    assert (initialized_);
    assert (ds_acc_estimators_.count(tr.ds_id_));
    ds_acc_estimators_.at(tr.ds_id_)->validate(tr, reconstructor);
}


void ComplexAccuracyEstimator::updateDataSourcesInfoSlot()
{
    loginf << "ComplexAccuracyEstimator: updateDataSourcesInfoSlot";

    ds_acc_estimators_.clear();

    for (const auto& ds_it : COMPASS::instance().dataSourceManager().dbDataSources())
    {
        assert (ds_it);
        ds_acc_estimators_.emplace(std::piecewise_construct,
                                   std::forward_as_tuple(ds_it->id()),  // args for key
                                   std::forward_as_tuple(createAccuracyEstimator(*ds_it)));

        if (reconstructor_)
            ds_acc_estimators_.at(ds_it->id())->init(reconstructor_);
    }
}

std::unique_ptr<AccuracyEstimatorBase> ComplexAccuracyEstimator::createAccuracyEstimator(dbContent::DBDataSource& ds)
{
    string ds_type = ds.dsType();

    std::unique_ptr<AccuracyEstimatorBase> ret;

    if (ds_type == "Radar")
        ret.reset(new RadarAccuracyEstimator(ds));
    else if (ds_type == "MLAT")
        ret.reset(new MLATAccuracyEstimator(ds));
    else if (ds_type == "ADSB")
        ret.reset(new ADSBAccuracyEstimator(ds));
    else // ds_type == "Tracker", "RefTraj", "Other"
        ret.reset(new TrackerAccuracyEstimator(ds));

    return ret;
}

dbContent::targetReport::PositionAccuracy ComplexAccuracyEstimator::positionAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    assert (initialized_);
    assert (ds_acc_estimators_.count(tr.ds_id_));
    return ds_acc_estimators_.at(tr.ds_id_)->positionAccuracy(tr);
}

dbContent::targetReport::VelocityAccuracy ComplexAccuracyEstimator::velocityAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    assert (initialized_);
    assert (ds_acc_estimators_.count(tr.ds_id_));
    return ds_acc_estimators_.at(tr.ds_id_)->velocityAccuracy(tr);
}

dbContent::targetReport::AccelerationAccuracy ComplexAccuracyEstimator::accelerationAccuracy (
    const dbContent::targetReport::ReconstructorInfo& tr)
{
    assert (initialized_);
    assert (ds_acc_estimators_.count(tr.ds_id_));
    return ds_acc_estimators_.at(tr.ds_id_)->accelerationAccuracy(tr);
}

void ComplexAccuracyEstimator::addAssociatedDistance(
    dbContent::targetReport::ReconstructorInfo& tr, const AssociatedDistance& dist)
{
    assert (initialized_);
    assert (ds_acc_estimators_.count(tr.ds_id_));
    ds_acc_estimators_.at(tr.ds_id_)->addAssociatedDistance(tr, dist);
}

void ComplexAccuracyEstimator::analyzeAssociatedDistances() const
{
    for (auto& acc_it : ds_acc_estimators_)
        acc_it.second->analyzeAssociatedDistances();
}

void ComplexAccuracyEstimator::clearAssociatedDistances()
{
    for (auto& acc_it : ds_acc_estimators_)
        acc_it.second->clearAssociatedDistances();
}

void ComplexAccuracyEstimator::estimateAccuracies()
{
    for (auto& acc_it : ds_acc_estimators_)
        acc_it.second->estimateAccuracies();
}

