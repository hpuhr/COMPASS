#ifndef EVALUATIONREQUIREMENTGROUP_H
#define EVALUATIONREQUIREMENTGROUP_H

#include "configurable.h"
#include "evaluationstandardtreeitem.h"

#include <QObject>
#include <QMenu>

#include <memory>

class EvaluationStandard;
class EvaluationRequirementConfig;

class EvaluationRequirementGroup : public QObject, public Configurable, public EvaluationStandardTreeItem
{
    Q_OBJECT

signals:
    void configsChangedSignal();

public slots:
    void deleteGroupSlot();
    void addRequirementSlot();


public:
    EvaluationRequirementGroup(const std::string& class_id, const std::string& instance_id,
                               EvaluationStandard& standard);
    virtual ~EvaluationRequirementGroup();

    virtual void generateSubConfigurable(const std::string& class_id,
                                         const std::string& instance_id) override;

    std::string name() const;

    bool hasRequirementConfig (const std::string& name);
    void addRequirementConfig (const std::string& class_id, const std::string& name, const std::string& short_name);
    EvaluationRequirementConfig& requirementConfig (const std::string& name);
    void removeRequirementConfig (const std::string& name);

    using EvaluationRequirementConfigIterator =
    typename std::vector<std::unique_ptr<EvaluationRequirementConfig>>::iterator;

    EvaluationRequirementConfigIterator begin() { return configs_.begin(); }
    EvaluationRequirementConfigIterator end() { return configs_.end(); }
    unsigned int size () { return configs_.size(); };

    virtual EvaluationStandardTreeItem *child(int row) override;
    virtual int childCount() const override;
    virtual int columnCount() const override;
    virtual QVariant data(int column) const override;
    virtual int row() const override;

    void showMenu ();

protected:
    EvaluationStandard& standard_;
    std::string name_;

    std::vector<std::unique_ptr<EvaluationRequirementConfig>> configs_;

    virtual void checkSubConfigurables() override;

    void sortConfigs();
};

#endif // EVALUATIONREQUIREMENTGROUP_H
