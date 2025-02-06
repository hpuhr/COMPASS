#ifndef ACIDFILTER_H
#define ACIDFILTER_H


#include "dbfilter.h"

#include <set>

class ACIDFilter : public DBFilter
{
public:
    ACIDFilter(const std::string& class_id, const std::string& instance_id,
               Configurable* parent);
     virtual ~ACIDFilter();

     virtual std::string getConditionString(const std::string& dbcontent_name, bool& first) override;

     virtual void generateSubConfigurable(const std::string& class_id,
                                          const std::string& instance_id) override;

     virtual bool filters(const std::string& dbcontent_name) override;
     virtual void reset() override;

     virtual void saveViewPointConditions (nlohmann::json& filters) override;
     virtual void loadViewPointConditions (const nlohmann::json& filters) override;

    std::string valuesString() const;
    void valuesString(const std::string& values_str);

    virtual bool activeInLiveMode() override;
    virtual std::vector<unsigned int> filterBuffer(const std::string& dbcontent_name, std::shared_ptr<Buffer> buffer) override;

protected:
    std::string values_str_; // org string for display

    std::set<std::string> values_; // parts w/o %
    bool null_wanted_ {false};  // indicates NULL in values

    virtual void checkSubConfigurables() override;
    virtual DBFilterWidget* createWidget() override;

    bool updateValuesFromStr(const std::string& values); // returns success
};

#endif // ACIDFILTER_H
