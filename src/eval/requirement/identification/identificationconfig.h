#ifndef EVALUATIONREQUIREMENTIDENTIFICATIONCONFIG_H
#define EVALUATIONREQUIREMENTIDENTIFICATIONCONFIG_H

#include "configurable.h"
#include "eval/requirement/config.h"
#include "eval/requirement/identification/identification.h"
#include "eval/requirement/identification/identificationconfigwidget.h"

#include <memory>


class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{

    class IdentificationConfig : public Config
    {
    public:
        IdentificationConfig(const std::string& class_id, const std::string& instance_id,
                        Group& group, EvaluationStandard& standard,
                        EvaluationManager& eval_man);
        virtual ~IdentificationConfig();

        virtual void addGUIElements(QFormLayout* layout) override;
        IdentificationConfigWidget* widget() override;
        std::shared_ptr<Base> createRequirement() override;

        float minimumProbability() const;
        void minimumProbability(float value);

    protected:
        float minimum_probability_{0};

        std::unique_ptr<IdentificationConfigWidget> widget_;
    };

}

#endif // EVALUATIONREQUIREMENTIDENTIFICATIONCONFIG_H
