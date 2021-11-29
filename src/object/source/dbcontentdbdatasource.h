#ifndef DBCONTENT_DBDATASOURCE_H
#define DBCONTENT_DBDATASOURCE_H

#include "dbcontentdatasourcebase.h"
#include "property.h"

namespace DBContent {

class DBDataSource : public DataSourceBase
{
public:

    static const std::string table_name_;

    const static Property id_column_;
    const static Property ds_type_column_;
    const static Property sac_column_;
    const static Property sic_column_;
    const static Property name_column_;
    const static Property short_name_;
    const static Property info_column_;
    const static Property counts_column_;

    DBDataSource();
    virtual ~DBDataSource();

    void counts (const std::string& counts);
    nlohmann::json& counts(); // for direct use, "cat048"->uint
    std::string countsStr();

//    virtual nlohmann::json getAsJSON();
//    virtual void setFromJSON(nlohmann::json& j);

    unsigned int id() const;
    void id(unsigned int id);

protected:
    unsigned int id_{0};

    nlohmann::json counts_;
};

} // namespace DBContent

#endif // DBCONTENT_DBDATASOURCE_H
