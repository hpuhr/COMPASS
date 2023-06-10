#ifndef REFTRAJACCURCYFILTER_H
#define REFTRAJACCURCYFILTER_H

#include "dbfilter.h"

#include <set>

class RefTrajAccuracyFilter : public DBFilter
{
public:
    RefTrajAccuracyFilter(const std::string& class_id, const std::string& instance_id,
                Configurable* parent);
    virtual ~RefTrajAccuracyFilter();

    virtual std::string getConditionString(const std::string& dbcontent_name, bool& first) override;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    virtual bool filters(const std::string& dbcontent_name) override;
    virtual void reset() override;

    virtual void saveViewPointConditions (nlohmann::json& filters) override;
    virtual void loadViewPointConditions (const nlohmann::json& filters) override;


    float minValue() const;
    void minValue(float min_value);

protected:
    float min_value_ {0};

    virtual void checkSubConfigurables() override;
    virtual DBFilterWidget* createWidget() override;
};

#endif // REFTRAJACCURCYFILTER_H
