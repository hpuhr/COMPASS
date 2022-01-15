#ifndef DBCONTENTCONFIGURATIONDATASOURCE_H
#define DBCONTENTCONFIGURATIONDATASOURCE_H

#include "dbcontent/source/datasourcebase.h"
#include "configurable.h"

class DBContentManager;

namespace dbContent
{

class DBDataSource;

class ConfigurationDataSource : public Configurable, public DataSourceBase
{
public:
    ConfigurationDataSource(const std::string& class_id, const std::string& instance_id,
                            DBContentManager& dbo_manager);
    //ConfigurationDataSource() = default;
    virtual ~ConfigurationDataSource();

    virtual nlohmann::json getAsJSON();
    virtual void setFromJSON(nlohmann::json& j);

    DBDataSource* getAsNewDBDS();

protected:
    virtual void checkSubConfigurables() {}
};

}

#endif // DBCONTENTCONFIGURATIONDATASOURCE_H
