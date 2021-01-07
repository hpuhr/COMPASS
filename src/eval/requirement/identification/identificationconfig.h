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

#ifndef EVALUATIONREQUIREMENTIDENTIFICATIONCONFIG_H
#define EVALUATIONREQUIREMENTIDENTIFICATIONCONFIG_H

#include "configurable.h"
#include "eval/requirement/base/baseconfig.h"
#include "eval/requirement/identification/identification.h"
#include "eval/requirement/identification/identificationconfigwidget.h"

#include <memory>

class Group;
class EvaluationStandard;

namespace EvaluationRequirement
{

    class IdentificationConfig : public BaseConfig
    {
    public:
        IdentificationConfig(const std::string& class_id, const std::string& instance_id,
                             Group& group, EvaluationStandard& standard,
                             EvaluationManager& eval_man);

        std::shared_ptr<Base> createRequirement() override;

        bool requireCorrectness() const;
        void requireCorrectness(bool value);

        bool requireCorrectnessOfAll() const;
        void requireCorrectnessOfAll(bool value);

        bool useModeA() const;
        void useModeA(bool value);

        bool useMsTa() const;
        void useMsTa(bool value);

        bool useMsTi() const;
        void useMsTi(bool value);

    protected:
        // true: correct (not false) only, false: present is ok
        bool require_correctness_ {false};

        // true: all must be correct (not false), false: at least one must be correct (not false)
        // for require_correctness_ true only
        bool require_correctness_of_all_ {false};

        // mode a ssr code
        bool use_mode_a_ {true};
        // 24-bit mode s address
        bool use_ms_ta_ {true};
        // downlinked aircraft identification
        bool use_ms_ti_ {true};

        virtual void createWidget() override;
    };

}

#endif // EVALUATIONREQUIREMENTIDENTIFICATIONCONFIG_H
