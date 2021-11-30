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

    void counts (const std::string& counts); // only for init
    std::string countsStr();
    const std::map<std::string, unsigned int>& countsMap() const;
    void addNumInserted(const std::string& db_content, unsigned int num);

//    virtual nlohmann::json getAsJSON();
//    virtual void setFromJSON(nlohmann::json& j);

    unsigned int id() const;
    void id(unsigned int id);

protected:
    unsigned int id_{0};

    nlohmann::json counts_;

    std::map<std::string, unsigned int> counts_map_;
};

} // namespace DBContent

#endif // DBCONTENT_DBDATASOURCE_H
