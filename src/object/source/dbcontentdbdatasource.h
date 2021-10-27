#ifndef DBCONTENT_DBDATASOURCE_H
#define DBCONTENT_DBDATASOURCE_H

#include "dbcontentdatasourcebase.h"

namespace DBContent {

class DBDataSource : public DataSourceBase
{
public:
    DBDataSource();
    virtual ~DBDataSource();

    nlohmann::json& counts(); // for direct use, "cat048"->uint

    virtual nlohmann::json getAsJSON();
    virtual void setFromJSON(nlohmann::json& j);

protected:
    nlohmann::json counts_;
};

} // namespace DBContent

#endif // DBCONTENT_DBDATASOURCE_H
