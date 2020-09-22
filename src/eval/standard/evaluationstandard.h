#ifndef EVALUATIONSTANDARD_H
#define EVALUATIONSTANDARD_H

#include <QObject>

#include <memory>

#include "configurable.h"

class EvaluationManager;
class EvaluationRequirementGroup;
class EvaluationStandardWidget;

class EvaluationStandard : public QObject, public Configurable
{
    Q_OBJECT

signals:
    void groupsChangedSignal();

public slots:

public:
    EvaluationStandard(const std::string& class_id, const std::string& instance_id, EvaluationManager& eval_man);
    virtual ~EvaluationStandard();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

    std::string name() const;

    bool hasGroup (const std::string& name);
    void addGroup (const std::string& name);
    EvaluationRequirementGroup& group (const std::string& name);
    void removeGroup (const std::string& name);

    using EvaluationRequirementGroupIterator = typename std::map<std::string, EvaluationRequirementGroup*>::iterator;
    EvaluationRequirementGroupIterator begin() { return groups_.begin(); }
    EvaluationRequirementGroupIterator end() { return groups_.end(); }
    unsigned int size () { return groups_.size(); };

    EvaluationStandardWidget* widget();

protected:
    EvaluationManager& eval_man_;
    std::string name_;

    std::unique_ptr<EvaluationStandardWidget> widget_;

    std::map<std::string, EvaluationRequirementGroup*> groups_;

    virtual void checkSubConfigurables();
};

#endif // EVALUATIONSTANDARD_H
