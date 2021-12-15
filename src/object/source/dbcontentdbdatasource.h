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
    bool hasActiveNumInserted() const;
    const std::map<std::string, unsigned int>& activeNumInsertedMap() const;

    void addNumInserted(unsigned int line_id, const std::string& db_content, unsigned int num);

    void addNumLoaded(unsigned int line_id, const std::string& db_content, unsigned int num);
    unsigned int activeNumLoaded (const std::string& db_content);
    void clearNumLoaded();

//    virtual nlohmann::json getAsJSON();
//    virtual void setFromJSON(nlohmann::json& j);

    unsigned int id() const;
    void id(unsigned int id);

    unsigned int activeLine() const;
    void activeLine(unsigned int active_line);

    bool loadingWanted() const;
    void loadingWanted(bool loading_wanted);

protected:
    unsigned int id_{0};

    unsigned int active_line_ {0};

    bool loading_wanted_ {true};

    nlohmann::json counts_;

    std::map<unsigned int, std::map<std::string, unsigned int>> num_loaded_;
    std::map<unsigned int, std::map<std::string, unsigned int>> counts_map_;
};

} // namespace DBContent

#endif // DBCONTENT_DBDATASOURCE_H
