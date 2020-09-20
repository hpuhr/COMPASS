#ifndef EVALUATIONREQUIREMENTCONFIG_H
#define EVALUATIONREQUIREMENTCONFIG_H

#include <QObject>

#include "configurable.h"

class EvaluationRequirementGroup;

class EvaluationRequirementConfig : public QObject, public Configurable
{
    Q_OBJECT

signals:

public slots:

public:
    EvaluationRequirementConfig(const std::string& class_id, const std::string& instance_id,
                                EvaluationRequirementGroup& group);
    virtual ~EvaluationRequirementConfig();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

protected:
    EvaluationRequirementGroup& group_;
    std::string name_;

    virtual void checkSubConfigurables();
};

#endif // EVALUATIONREQUIREMENTCONFIG_H
