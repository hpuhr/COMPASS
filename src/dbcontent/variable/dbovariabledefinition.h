#ifndef DBOVARIABLEDEFINITION_H
#define DBOVARIABLEDEFINITION_H

#include <string>

#include "configurable.h"

namespace dbContent
{

class DBContentVariableDefinition : public Configurable
{
  public:
    DBContentVariableDefinition(const std::string& class_id, const std::string& instance_id,
                          Configurable* parent)
        : Configurable(class_id, instance_id, parent)
    {
        registerParameter("dbo_name", &dbo_name_, "");
        registerParameter("dbo_variable_name", &dbo_variable_name_, "");

        // DBOVAR LOWERCASE HACK
        // boost::algorithm::to_lower(dbo_variable_name_);

        assert(dbo_variable_name_.size() > 0);
    }

    DBContentVariableDefinition& operator=(DBContentVariableDefinition&& other)
    {
        dbo_name_ = other.dbo_name_;
        other.dbo_name_ = "";

        dbo_variable_name_ = other.dbo_variable_name_;
        other.dbo_variable_name_ = "";

        return *this;
    }

    virtual ~DBContentVariableDefinition() {}

    const std::string& dboName() { return dbo_name_; }
    void dboName(const std::string& dbo_name) { dbo_name_ = dbo_name; }

    const std::string& variableName() { return dbo_variable_name_; }
    void variableName(const std::string& dbo_variable_name)
    {
        dbo_variable_name_ = dbo_variable_name;
    }

  protected:
    std::string dbo_name_;
    std::string dbo_variable_name_;
};

}

#endif // DBOVARIABLEDEFINITION_H
