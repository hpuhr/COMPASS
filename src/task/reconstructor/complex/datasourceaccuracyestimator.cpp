#include "datasourceaccuracyestimator.h"
#include "dbdatasource.h"

DataSourceAccuracyEstimator::DataSourceAccuracyEstimator(const dbContent::DBDataSource& source)
{
    ds_name_ = source.name();
    ds_id_ = source.id();
    ds_type_ = source.dsType();

    name_ = ds_name_;
}
