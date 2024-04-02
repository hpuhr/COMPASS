#include "accuracyestimatorbase.h"
#include "datasourcemanager.h"
#include "dbdatasource.h"

#include "compass.h"

const double AccuracyEstimatorBase::PosAccStdDevFallback = 1000.0;
const dbContent::targetReport::PositionAccuracy AccuracyEstimatorBase::PosAccStdFallback
    { AccuracyEstimatorBase::PosAccStdDevFallback, AccuracyEstimatorBase::PosAccStdDevFallback, 0};

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

