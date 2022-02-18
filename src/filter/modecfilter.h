#ifndef MODECFILTER_H
#define MODECFILTER_H

#include "dbfilter.h"

#include <set>

class ModeCFilter : public DBFilter
{
public:
    ModeCFilter(const std::string& class_id, const std::string& instance_id,
                Configurable* parent);
    virtual ~ModeCFilter();

    virtual std::string getConditionString(const std::string& dbo_name, bool& first,
                                           std::vector<std::string>& extra_from_parts,
                                           std::vector<dbContent::Variable*>& filtered_variables) override;

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    virtual bool filters(const std::string& dbo_name) override;
    virtual void reset() override;

    virtual void saveViewPointConditions (nlohmann::json& filters) override;
    virtual void loadViewPointConditions (const nlohmann::json& filters) override;


    virtual bool activeInLiveMode() override;
    virtual std::vector<size_t> filterBuffer(const std::string& dbcontent_name, std::shared_ptr<Buffer> buffer) override;

    float minValue() const;
    void minValue(float min_value);

    float maxValue() const;
    void maxValue(float max_value);

    bool nullWanted() const;
    void nullWanted(bool null_wanted);

protected:
    float min_value_ {0};
    float max_value_ {0};
    bool null_wanted_ {false};

    virtual void checkSubConfigurables() override;
};

#endif // MODECFILTER_H
