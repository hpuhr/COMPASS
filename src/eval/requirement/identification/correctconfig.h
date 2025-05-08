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

#ifndef EVALUATIONREQUIREMENTIDENTIFICATIONCORRECTCONFIG_H
#define EVALUATIONREQUIREMENTIDENTIFICATIONCORRECTCONFIG_H

#include "eval/requirement/base/probabilitybaseconfig.h"

#include <memory>

class Group;
class EvaluationStandard;

namespace ResultReport
{
    class Report;
}

namespace EvaluationRequirement
{

class IdentificationCorrectConfig : public ProbabilityBaseConfig
{
public:
    IdentificationCorrectConfig(const std::string& class_id, const std::string& instance_id,
                                Group& group, EvaluationStandard& standard,
                                EvaluationCalculator& calculator);

    std::shared_ptr<Base> createRequirement() override;

    bool requireCorrectnessOfAll() const;
    void requireCorrectnessOfAll(bool value);

    bool useModeA() const;
    void useModeA(bool value);

    bool useMsTa() const;
    void useMsTa(bool value);

    bool useMsTi() const;
    void useMsTi(bool value);

    virtual void addToReport (std::shared_ptr<ResultReport::Report> report);

protected:
    // true: all must be correct (not false), false: at least one must be correct (not false)
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
