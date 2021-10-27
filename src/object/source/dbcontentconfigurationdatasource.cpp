#include "dbcontentconfigurationdatasource.h"
#include "dbobjectmanager.h"
#include "logger.h"

#include <algorithm>

using namespace std;

namespace DBContent
{

ConfigurationDataSource::ConfigurationDataSource(const std::string& class_id, const std::string& instance_id,
                                                 DBObjectManager& dbo_manager)
    : Configurable(class_id, instance_id, &dbo_manager)
{
    registerParameter("db_content_type", &db_content_type_, "");
    registerParameter("sac", &sac_, 0);
    registerParameter("sic", &sic_, 0);

    registerParameter("name", &name_, "");
    registerParameter("has_short_name", &has_short_name_, false);
    registerParameter("short_name", &short_name_, "");

    registerParameter("info", &info_, {});

    assert (db_content_type_.size());
    assert (find(DBObjectManager::db_content_types_.begin(),
                 DBObjectManager::db_content_types_.end(), db_content_type_)
            != DBObjectManager::db_content_types_.end());

    assert (name_.size());

    if (has_short_name_)
        assert (short_name_.size());

}

ConfigurationDataSource::~ConfigurationDataSource()
{

}

}
