#pragma once

#include <string>

#include "configurable.h"

namespace dbContent
{

class VariableDefinition : public Configurable
{
  public:
    VariableDefinition(const std::string& class_id, const std::string& instance_id,
                          Configurable* parent)
        : Configurable(class_id, instance_id, parent)
    {
        registerParameter("dbcontent_name", &dbcontent_name_, std::string());
        registerParameter("variable_name", &variable_name_, std::string());

        // DBOVAR LOWERCASE HACK
        // boost::algorithm::to_lower(dbo_variable_name_);

        assert(variable_name_.size() > 0);
    }

    VariableDefinition& operator=(VariableDefinition&& other)
    {
        dbcontent_name_ = other.dbcontent_name_;
        other.dbcontent_name_ = "";

        variable_name_ = other.variable_name_;
        other.variable_name_ = "";

        return *this;
    }

    virtual ~VariableDefinition() {}

    const std::string& dbContentName() { return dbcontent_name_; }
    void dbContentName(const std::string& dbcontent_name) { dbcontent_name_ = dbcontent_name; }

    const std::string& variableName() { return variable_name_; }
    void variableName(const std::string& dbo_variable_name)
    {
        variable_name_ = dbo_variable_name;
    }

  protected:
    std::string dbcontent_name_;
    std::string variable_name_;
};

}

