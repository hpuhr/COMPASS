#ifndef DBCONTENT_DATASOURCEBASE_H
#define DBCONTENT_DATASOURCEBASE_H

#include <json.hpp>

#include <string>


namespace DBContent {

class DataSourceBase
{
public:
    DataSourceBase();

    std::string dsType() const;
    void dsType(const std::string& ds_type);

    unsigned int sac() const;
    void sac(unsigned int sac);

    unsigned int sic() const;
    void sic(unsigned int sic);

    std::string name() const;
    void name(const std::string &name);

    bool hasShortName() const;
    void removeShortName();
    void shortName(const std::string& short_name);
    const std::string& shortName() const;

    void info(const std::string& info);
    nlohmann::json& info(); // for direct use, var->value
    std::string infoStr();

protected:
    std::string ds_type_;

    unsigned int sac_{0};
    unsigned int sic_{0};

    std::string name_;

    bool has_short_name_{false};
    std::string short_name_;

    nlohmann::json info_;
};

} // namespace DBContent

#endif // DBCONTENT_DATASOURCEBASE_H
