#pragma once

#include "json.hpp"

#include <string>

class ResultManager;

class TaskResult
{
    friend class ResultManager; // to change id if required

public:

    enum TaskResultType
    {
        Generic=0,
        Evaluation
    };

    TaskResult(unsigned int id);

    unsigned int id() const;

    std::string name() const;
    void name(const std::string& name);

    const nlohmann::json& content() const;
    nlohmann::json& content();

    TaskResultType type() const;
    void type(TaskResultType type);

protected:
    unsigned int id_{0};
    std::string name_;
    nlohmann::json content_;
    TaskResultType type_;

    void id(unsigned int id);
};

