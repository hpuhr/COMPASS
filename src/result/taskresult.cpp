#include "taskresult.h"

TaskResult::TaskResult(unsigned int id)
    : id_(id)
{}

unsigned int TaskResult::id() const
{
    return id_;
}

void TaskResult::id(unsigned int id)
{
    id_ = id;
}

std::string TaskResult::name() const
{
    return name_;
}

void TaskResult::name(const std::string& name)
{
    name_ = name;
}

const nlohmann::json& TaskResult::content() const
{
    return content_;
}

nlohmann::json& TaskResult::content()
{
    return content_;
}

TaskResult::TaskResultType TaskResult::type() const
{
    return type_;
}

void TaskResult::type(TaskResultType type)
{
    type_ = type;
}
