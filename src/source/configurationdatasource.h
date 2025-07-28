#pragma once

#include "source/datasourcebase.h"
#include "configurable.h"

class DataSourceManager;

namespace dbContent
{

class DBDataSource;

class ConfigurationDataSource : public Configurable, public DataSourceBase
{
public:
    ConfigurationDataSource(const std::string& class_id, const std::string& instance_id,
                            DataSourceManager& ds_manager);
    //ConfigurationDataSource() = default;
    virtual ~ConfigurationDataSource();

    virtual void setFromJSON(const nlohmann::json& j);

    DBDataSource* getAsNewDBDS();

protected:
    virtual void checkSubConfigurables() {}
};

}
