#ifndef EVALUATIONREQUIREMENTCONFIG_H
#define EVALUATIONREQUIREMENTCONFIG_H

#include <QObject>

#include "configurable.h"
#include "evaluationstandardtreeitem.h"

class EvaluationRequirementGroup;

class EvaluationRequirementConfig : public QObject, public Configurable, public EvaluationStandardTreeItem
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

    std::string name() const;

    virtual EvaluationStandardTreeItem *child(int row) override;
    virtual int childCount() const override;
    virtual int columnCount() const override;
    virtual QVariant data(int column) const override;
    virtual int row() const override;

protected:
    EvaluationRequirementGroup& group_;
    std::string name_;

    virtual void checkSubConfigurables();
};

#endif // EVALUATIONREQUIREMENTCONFIG_H
