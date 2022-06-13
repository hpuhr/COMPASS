#ifndef PRIMARYONLYFILTER_H
#define PRIMARYONLYFILTER_H

#include "dbfilter.h"

class PrimaryOnlyFilter : public DBFilter
{
public:
    PrimaryOnlyFilter(const std::string& class_id, const std::string& instance_id,
                      Configurable* parent);
    virtual ~PrimaryOnlyFilter();

    virtual std::string getConditionString(const std::string& dbcontent_name, bool& first,
                                           std::vector<std::string>& extra_from_parts,
                                           std::vector<dbContent::Variable*>& filtered_variables) override;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    virtual bool filters(const std::string& dbcontent_name) override;
    virtual void reset() override;

    virtual void saveViewPointConditions (nlohmann::json& filters) override;
    virtual void loadViewPointConditions (const nlohmann::json& filters) override;

    virtual bool activeInLiveMode() override;
    virtual std::vector<size_t> filterBuffer(const std::string& dbcontent_name, std::shared_ptr<Buffer> buffer) override;

protected:
    virtual void checkSubConfigurables() override;
    virtual DBFilterWidget* createWidget() override;
};

#endif // PRIMARYONLYFILTER_H
