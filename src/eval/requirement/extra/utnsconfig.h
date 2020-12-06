/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EVALUATIONREQUIREMENTEXTRAUTNSCONFIG_H
#define EVALUATIONREQUIREMENTEXTRAUTNSCONFIG_H

#include "configurable.h"
#include "eval/requirement/config.h"
#include "eval/requirement/extra/utnsconfigwidget.h"
#include "eval/requirement/extra/utns.h"

#include <memory>


class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{

    class ExtraUTNsConfig : public Config
    {
    public:
        ExtraUTNsConfig(const std::string& class_id, const std::string& instance_id,
                        Group& group, EvaluationStandard& standard,
                        EvaluationManager& eval_man);
        virtual ~ExtraUTNsConfig();

        virtual void addGUIElements(QFormLayout* layout) override;
        ExtraUTNsConfigWidget* widget() override;
        std::shared_ptr<Base> createRequirement() override;

        float maxRefTimeDiff() const;
        void maxRefTimeDiff(float value);

        float minDuration() const;
        void minDuration(float value);

        unsigned int minNumUpdates() const;
        void minNumUpdates(unsigned int value);

        bool ignorePrimaryOnly() const;
        void ignorePrimaryOnly(bool value);

        float maximumProbability() const;
        void maximumProbability(float value);

    protected:
        float max_ref_time_diff_ {0};

        float min_duration_{0};
        unsigned int min_num_updates_ {0};
        bool ignore_primary_only_ {true};
        float maximum_probability_{0};

        std::unique_ptr<ExtraUTNsConfigWidget> widget_;
    };

}

#endif // EVALUATIONREQUIREMENTEXTRAUTNSCONFIG_H
