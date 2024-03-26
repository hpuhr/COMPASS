#pragma once

#include "accuracyestimatorbase.h"


namespace dbContent
{
class DBDataSource;
}


class DataSourceAccuracyEstimator : public AccuracyEstimatorBase
{
  public:
    DataSourceAccuracyEstimator(const dbContent::DBDataSource& source);

  protected:
    std::string ds_name_;
    unsigned int ds_id_{0};
    std::string ds_type_;
};

