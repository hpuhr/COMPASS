#ifndef EVALUATIONREQUIREMENTGROUP_H
#define EVALUATIONREQUIREMENTGROUP_H

#include <QObject>

#include "configurable.h"

class EvaluationStandard;

class EvaluationRequirementGroup : public QObject, public Configurable
{
    Q_OBJECT

signals:

public slots:

public:
    EvaluationRequirementGroup(const std::string& class_id, const std::string& instance_id,
                               EvaluationStandard& standard);
    virtual ~EvaluationRequirementGroup();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

protected:
    EvaluationStandard& standard_;
    std::string name_;

    virtual void checkSubConfigurables();
};

#endif // EVALUATIONREQUIREMENTGROUP_H
