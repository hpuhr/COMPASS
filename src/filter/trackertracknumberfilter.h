#pragma once

#include "dbfilter.h"
#include "json_fwd.hpp"

#include <QObject>

class TrackerTrackNumberFilter : public QObject, public DBFilter
{
    Q_OBJECT

public slots:
    void updateDataSourcesSlot();


public:
    TrackerTrackNumberFilter(const std::string& class_id, const std::string& instance_id,
                             Configurable* parent);
    virtual ~TrackerTrackNumberFilter();

    virtual std::string getConditionString(const std::string& dbcontent_name, bool& first) override;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    virtual bool filters(const std::string& dbcontent_name) override;

    virtual void saveViewPointConditions (nlohmann::json& filters) override;
    virtual void loadViewPointConditions (const nlohmann::json& filters) override;

    void setTrackerTrackNum(unsigned int ds_id, unsigned int line_id, const std::string& value);
    std::map<unsigned int, std::map<unsigned int, std::string>> getActiveTrackerTrackNums ();
    std::map<std::string, std::map<std::string, std::string>> getActiveTrackerTrackNumsStr ();
    // ds_id -> line -> track nums

protected:
    nlohmann::json tracker_track_nums_; // ds_id -> line -> track nums

    virtual void checkSubConfigurables() override;
    virtual DBFilterWidget* createWidget() override;
};

