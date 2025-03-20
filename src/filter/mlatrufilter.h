#ifndef MLATRUFILTER_H
#define MLATRUFILTER_H

#include "dbfilter.h"

class MLATRUFilter : public DBFilter
{
public:
    MLATRUFilter(const std::string& class_id, const std::string& instance_id,
                 Configurable* parent);
    virtual ~MLATRUFilter();

    virtual std::string getConditionString(const std::string& dbcontent_name, bool& first) override;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    virtual bool filters(const std::string& dbcontent_name) override;
    virtual void reset() override;

    virtual void saveViewPointConditions (nlohmann::json& filters) override;
    virtual void loadViewPointConditions (const nlohmann::json& filters) override;

    std::string rus() const;
    void rus(const std::string& rus_str);

protected:
    std::string rus_str_;
    std::vector<unsigned int> values_;
    bool null_wanted_ {false};

    virtual void checkSubConfigurables() override;
    virtual DBFilterWidget* createWidget() override;

    bool updateRUsFromStr(const std::string& values_str); // returns success
};

#endif // MLATRUFILTER_H
