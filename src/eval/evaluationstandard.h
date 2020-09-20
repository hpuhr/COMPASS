#ifndef EVALUATIONSTANDARD_H
#define EVALUATIONSTANDARD_H

#include <QObject>

#include "configurable.h"

class EvaluationManager;

class EvaluationStandard : public QObject, public Configurable
{
    Q_OBJECT

signals:

public slots:

public:
    EvaluationStandard(const std::string& class_id, const std::string& instance_id, EvaluationManager& eval_man);
    virtual ~EvaluationStandard();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id);

protected:
    EvaluationManager& eval_man_;
    std::string name_;

    virtual void checkSubConfigurables();
};

#endif // EVALUATIONSTANDARD_H
