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

#ifndef EVALUATIONREQUIREMENT_PROBABILITYBASE_H
#define EVALUATIONREQUIREMENT_PROBABILITYBASE_H

#include "eval/requirement/base/comparisontype.h"
#include "eval/requirement/base/base.h"

#include <string>
#include <memory>

class EvaluationManager;

namespace EvaluationRequirement 
{

class ProbabilityBase : public Base
{
public:
    ProbabilityBase(const std::string& name, 
                    const std::string& short_name, 
                    const std::string& group_name,
                    float prob, 
                    COMPARISON_TYPE prob_check_type, 
                    EvaluationManager& eval_man);
    virtual ~ProbabilityBase();

    float prob() const;
    unsigned int getNumProbDecimals() const;

    COMPARISON_TYPE probCheckType() const;

    std::string getConditionStr () const override;
    bool getConditionResult (float prob) const; //  true if passed
    std::string getConditionResultStr (float prob) const;

protected:
    float prob_ {0};
    COMPARISON_TYPE prob_check_type_ {COMPARISON_TYPE::GREATER_THAN_OR_EQUAL};

    bool compareValue (double val, double threshold, COMPARISON_TYPE check_type);
};

} // namespace EvaluationRequirement

#endif // EVALUATIONREQUIREMENT_PROBABILITYBASE_H
