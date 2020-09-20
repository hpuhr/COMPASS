#ifndef EVALUATIONREQUIREMENTGROUP_H
#define EVALUATIONREQUIREMENTGROUP_H

#include <QObject>

#include "configurable.h"

class EvaluationStandard;
class EvaluationRequirementConfig;

class EvaluationRequirementGroup : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void configsChangedSignal();

public slots:

public:
    EvaluationRequirementGroup(const std::string& class_id, const std::string& instance_id,
                               EvaluationStandard& standard);
    virtual ~EvaluationRequirementGroup();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    std::string name() const;

    bool hasRequirementConfig (const std::string& name);
    void addRequirementConfig (const std::string& name);
    EvaluationRequirementConfig& requirementConfig (const std::string& name);
    void removeRequirementConfig (const std::string& name);

    using EvaluationRequirementConfigIterator = typename std::map<std::string, EvaluationRequirementConfig*>::iterator;
    EvaluationRequirementConfigIterator begin() { return configs_.begin(); }
    EvaluationRequirementConfigIterator end() { return configs_.end(); }
    unsigned int size () { return configs_.size(); };

protected:
    EvaluationStandard& standard_;
    std::string name_;

    std::map<std::string, EvaluationRequirementConfig*> configs_;

    virtual void checkSubConfigurables();
};

#endif // EVALUATIONREQUIREMENTGROUP_H
