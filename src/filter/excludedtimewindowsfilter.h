#pragma once

#include "dbfilter.h"
#include "util/timewindow.h"

class ExcludedTimeWindowsFilter : public DBFilter
{
public:
    ExcludedTimeWindowsFilter(const std::string& class_id, const std::string& instance_id,
                              Configurable* parent);
    virtual ~ExcludedTimeWindowsFilter();

    virtual std::string getConditionString(const std::string& dbcontent_name, bool& first) override;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    virtual bool filters(const std::string& dbcontent_name) override;
    virtual void reset() override;

    virtual void saveViewPointConditions (nlohmann::json& filters) override;
    virtual void loadViewPointConditions (const nlohmann::json& filters) override;

    Utils::TimeWindowCollection& timeWindows();

protected:
    nlohmann::json time_windows_json_;
    Utils::TimeWindowCollection time_windows_;

    virtual void checkSubConfigurables() override;
    virtual DBFilterWidget* createWidget() override;
};

