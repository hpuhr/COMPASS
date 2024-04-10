#include "datasourceaccuracyestimator.h"
#include "dbdatasource.h"
#include "reconstructorbase.h"
#include "reconstructortarget.h"
#include "logger.h"
#include "global.h"

#include <osgEarth/GeoMath>

DataSourceAccuracyEstimator::DataSourceAccuracyEstimator(const dbContent::DBDataSource& source)
{
    ds_name_ = source.name();
    ds_id_ = source.id();
    ds_type_ = source.dsType();

    name_ = ds_name_;
}

void DataSourceAccuracyEstimator::estimateAccuracies()
{
    loginf << "DataSourceAccuracyEstimator " << name_ << ": estimateAccuracies";

    assert (reconstructor_);

    for (auto& tgt_it : reconstructor_->targets_)
    {
        // dbcontent id -> ds_id -> line_id -> ts -> record_num
        for (auto& dbcont_it : tgt_it.second.tr_ds_timestamps_)
        {
            if (dbcont_it.second.count(ds_id_))
                estimateAccuracyUsing(tgt_it.second, dbcont_it.first, dbcont_it.second.at(ds_id_));
        }
    }

    finalizeEstimateAccuracy();
}

std::tuple<double, double> DataSourceAccuracyEstimator::getOffset(
    dbContent::targetReport::ReconstructorInfo& tr, dbContent::targetReport::Position& ref_pos)
{
    assert (tr.position_);

    double distance_m   = osgEarth::GeoMath::distance(
        tr.position_->latitude_ * DEG2RAD, tr.position_->longitude_ * DEG2RAD,
        ref_pos.latitude_ * DEG2RAD, ref_pos.longitude_ * DEG2RAD);
    double bearing_rad  = osgEarth::GeoMath::bearing(
        tr.position_->latitude_ * DEG2RAD, tr.position_->longitude_ * DEG2RAD,
        ref_pos.latitude_ * DEG2RAD, ref_pos.longitude_ * DEG2RAD);

    return std::tuple<double, double>{distance_m, bearing_rad};
}
