#ifndef UTNFILTER_H
#define UTNFILTER_H

#include "dbfilter.h"

class UTNFilter : public DBFilter
{
public:
    UTNFilter(const std::string& class_id, const std::string& instance_id,
              Configurable* parent);
    virtual ~UTNFilter();

    virtual std::string getConditionString(const std::string& dbo_name, bool& first,
                                           std::vector<DBOVariable*>& filtered_variables);

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    virtual bool filters(const std::string& dbo_name);
    virtual void reset();

    virtual void saveViewPointConditions (nlohmann::json& filters);
    virtual void loadViewPointConditions (const nlohmann::json& filters);

    std::string utns() const;
    void utns(const std::string& utns);

protected:
    std::string utns_str_;
    std::vector<unsigned int> utns_;

    virtual void checkSubConfigurables();

    bool updateUTNSFromStr(const std::string& utns); // returns success
};

#endif // UTNFILTER_H
