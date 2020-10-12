#include "eval/requirement/identification/identificationconfig.h"
#include "eval/requirement/identification/identificationconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base.h"

using namespace std;

namespace EvaluationRequirement
{

    IdentificationConfig::IdentificationConfig(
            const std::string& class_id, const std::string& instance_id,
            Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
        : Config(class_id, instance_id, group, standard, eval_man)
    {
        registerParameter("minimum_probability", &minimum_probability_, 0.99);

    }

    IdentificationConfig::~IdentificationConfig()
    {

    }

    void IdentificationConfig::addGUIElements(QFormLayout* layout)
    {
        assert (layout);

        Config::addGUIElements(layout);
    }

    IdentificationConfigWidget* IdentificationConfig::widget()
    {
        if (!widget_)
            widget_.reset(new IdentificationConfigWidget(*this));

        return widget_.get();
    }

    std::shared_ptr<Base> IdentificationConfig::createRequirement()
    {
        shared_ptr<Identification> req = make_shared<Identification>(
                    name_, short_name_, group_.name(), eval_man_, minimum_probability_);

        return req;
    }

    float IdentificationConfig::minimumProbability() const
    {
        return minimum_probability_;
    }

    void IdentificationConfig::minimumProbability(float value)
    {
        minimum_probability_ = value;
    }
}
