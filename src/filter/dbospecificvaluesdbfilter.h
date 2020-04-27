#ifndef DBOSPECIFICVALUESDBFILTER_H
#define DBOSPECIFICVALUESDBFILTER_H

#include "dbfilter.h"

class DBObject;
class DBOVariable;

class DBOSpecificValuesDBFilter : public DBFilter
{
public:
    DBOSpecificValuesDBFilter(const std::string& class_id, const std::string& instance_id,
                              Configurable* parent);
    virtual ~DBOSpecificValuesDBFilter() override;

    virtual std::string getConditionString(const std::string& dbo_name, bool& first,
                                           std::vector<DBOVariable*>& filtered_variables) override;

    virtual bool filters(const std::string& dbo_name) override;

    const std::string& dbObjectName() { return dbo_name_; }

protected:
  std::string dbo_name_;
  std::string variable_name_;
  std::string condition_operator_; // operator to be used in generated conditions

  DBObject* object_{nullptr};
  DBOVariable* variable_ {nullptr};

  std::string ds_column_name_;

  virtual void checkSubConfigurables() override;
};

#endif // DBOSPECIFICVALUESDBFILTER_H
