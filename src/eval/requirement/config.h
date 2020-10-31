#ifndef EVALUATIONREQUIREMENTCONFIG_H
#define EVALUATIONREQUIREMENTCONFIG_H

#include <QObject>

#include "configurable.h"
#include "evaluationstandardtreeitem.h"

class Group;
class EvaluationStandard;


class QWidget;
class QFormLayout;
class EvaluationManager;

namespace EvaluationRequirement
{

    class Base;

    class Config : public QObject, public Configurable, public EvaluationStandardTreeItem
    {
        Q_OBJECT

    signals:

    public slots:
        void changedNameSlot(const QString& value);
        void changedShortNameSlot(const QString& value);

    public:
        Config(const std::string& class_id, const std::string& instance_id,
                                    Group& group, EvaluationStandard& standard,
                                    EvaluationManager& eval_man);
        virtual ~Config();

        virtual void generateSubConfigurable(const std::string& class_id,
                                             const std::string& instance_id) override;

        std::string name() const;
        void name(const std::string& name);

        bool hasShortName () const;
        std::string shortName() const;
        void shortName(const std::string& short_name);

        virtual EvaluationStandardTreeItem *child(int row) override;
        virtual int childCount() const override;
        virtual int columnCount() const override;
        virtual QVariant data(int column) const override;
        virtual int row() const override;

        virtual QWidget* widget() = 0;
        virtual std::shared_ptr<Base> createRequirement() = 0;

    protected:
        Group& group_;
        EvaluationStandard& standard_;
        EvaluationManager& eval_man_;

        std::string name_;
        std::string short_name_;

        virtual void checkSubConfigurables() override;

        virtual void addGUIElements(QFormLayout* layout);
    };

}

#endif // EVALUATIONREQUIREMENTCONFIG_H
