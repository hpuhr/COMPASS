#ifndef MODE3AFILTER_H
#define MODE3AFILTER_H

#include "dbfilter.h"

#include <set>

class Mode3AFilter : public DBFilter
{
public:
    Mode3AFilter(const std::string& class_id, const std::string& instance_id,
               Configurable* parent);
     virtual ~Mode3AFilter();

     virtual std::string getConditionString(const std::string& dbo_name, bool& first,
                                            std::vector<std::string>& extra_from_parts,
                                            std::vector<dbContent::Variable*>& filtered_variables) override;

     virtual void generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id) override;

     virtual bool filters(const std::string& dbo_name) override;
     virtual void reset() override;

     virtual void saveViewPointConditions (nlohmann::json& filters) override;
     virtual void loadViewPointConditions (const nlohmann::json& filters) override;

    std::string valuesString() const;
    void valuesString(const std::string& values_str);

    virtual bool activeInLiveMode() override;
    virtual std::vector<size_t> filterBuffer(const std::string& dbcontent_name, std::shared_ptr<Buffer> buffer) override;

protected:
    std::string values_str_; // org string for display

    std::set<unsigned int> values_; // dec
    bool null_wanted_ {false};  // indicates NULL in values

    virtual void checkSubConfigurables() override;

    bool updateValuesFromStr(const std::string& values); // returns success
};

#endif // MODE3AFILTER_H
