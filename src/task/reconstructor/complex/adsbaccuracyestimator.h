#pragma once

#include "datasourceaccuracyestimator.h"

class ADSBAccuracyEstimator : public DataSourceAccuracyEstimator
{
  public:

    ADSBAccuracyEstimator(const dbContent::DBDataSource& source);
    virtual ~ADSBAccuracyEstimator() {};

    virtual void validate (dbContent::targetReport::ReconstructorInfo& tr, ReconstructorBase& reconstructor) override;

    virtual dbContent::targetReport::PositionAccuracy positionAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::VelocityAccuracy velocityAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;
    virtual dbContent::targetReport::AccelerationAccuracy accelerationAccuracy (
        const dbContent::targetReport::ReconstructorInfo& tr) override;

  protected:
    virtual void estimateAccuracyUsing(
        dbContent::ReconstructorTarget& target, unsigned int dbcont_id,
        const std::map<unsigned int, std::multimap<boost::posix_time::ptime, unsigned long>>& tr_ds_timestamps_)
         override;
    virtual void finalizeEstimateAccuracy() override;
};

