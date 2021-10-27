#ifndef DBCONTENTCONFIGURATIONDATASOURCE_H
#define DBCONTENTCONFIGURATIONDATASOURCE_H

#include "dbcontentdatasourcebase.h"
#include "configurable.h"

class DBObjectManager;

namespace DBContent
{

class ConfigurationDataSource : public Configurable, public DataSourceBase
{
public:
    ConfigurationDataSource(const std::string& class_id, const std::string& instance_id,
                            DBObjectManager& dbo_manager);
    //ConfigurationDataSource() = default;
    virtual ~ConfigurationDataSource();

protected:
    virtual void checkSubConfigurables() {}
};

}

#endif // DBCONTENTCONFIGURATIONDATASOURCE_H
