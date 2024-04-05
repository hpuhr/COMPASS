#pragma once

#include "accuracyestimatorbase.h"


namespace dbContent
{
class DBDataSource;
class ReconstructorTarget;
}


class DataSourceAccuracyEstimator : public AccuracyEstimatorBase
{
  public:
    DataSourceAccuracyEstimator(const dbContent::DBDataSource& source);
    virtual ~DataSourceAccuracyEstimator() {};

    virtual void estimateAccuracies(); // when targets with calculated references are done

  protected:
    std::string ds_name_;
    unsigned int ds_id_{0};
    std::string ds_type_;

    virtual void estimateAccuracyUsing(dbContent::ReconstructorTarget& target, unsigned int dbcont_id,
        const std::map<unsigned int, std::multimap<boost::posix_time::ptime, unsigned long>>& tr_ds_timestamps_) = 0;
    virtual void finalizeEstimateAccuracy() = 0;

    // distance m, bearing rad
    std::tuple<double, double> getOffset(dbContent::targetReport::ReconstructorInfo& tr,
                                         dbContent::targetReport::Position& ref_pos);
};

