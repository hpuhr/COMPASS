#ifndef DBCONTENT_CONFIGURATIONDATASOURCE_H
#define DBCONTENT_CONFIGURATIONDATASOURCE_H

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

#endif // DBCONTENT_CONFIGURATIONDATASOURCE_H
