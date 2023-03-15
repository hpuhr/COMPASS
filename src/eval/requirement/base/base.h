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

#ifndef EVALUATIONREQUIREMENT_H
#define EVALUATIONREQUIREMENT_H

#include "eval/requirement/base/comparisontype.h"
#include "viewpoint.h"

#include "boost/date_time/posix_time/posix_time.hpp"
#include "boost/date_time/time_duration.hpp"

#include <string>
#include <memory>

class EvaluationTargetData;
class EvaluationManager;
class SectorLayer;

namespace EvaluationRequirementResult {
class Single;
}

namespace Evaluation
{
    class DataID;
}

namespace EvaluationRequirement
{

enum ValueComparisonResult
{
    Unknown_NoRefData=0,
    Unknown_NoTstData,
    Same,
    Different
};

class Base
{
public:
    Base(const std::string& name, const std::string& short_name, const std::string& group_name,
         float prob, COMPARISON_TYPE prob_check_type, EvaluationManager& eval_man);
    virtual ~Base();

    virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
            const EvaluationTargetData& target_data, 
            std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer) = 0;
    // instance is the self-reference for the result (we want to pass the shared pointer to the result)

    std::string name() const;
    std::string shortname() const;
    std::string groupName() const;

    float prob() const;
    unsigned int getNumProbDecimals() const;

    COMPARISON_TYPE probCheckType() const;

    std::string getConditionStr () const;
    std::string getResultConditionStr (float prob) const;

protected:
    //static bool in_appimage_;

    std::string name_;
    std::string short_name_;
    std::string group_name_;

    float prob_ {0};
    COMPARISON_TYPE prob_check_type_ {COMPARISON_TYPE::GREATER_THAN_OR_EUQAL};
    //std::string prob_name_{"Minimum Probability [1]"};

    EvaluationManager& eval_man_;

    bool compareValue (double val, double threshold, COMPARISON_TYPE check_type);

    std::pair<ValueComparisonResult, std::string> compareTi (
            const Evaluation::DataID& id_tst, const EvaluationTargetData& target_data,
            boost::posix_time::time_duration max_ref_time_diff); // tst timestamp
    std::pair<ValueComparisonResult, std::string> compareTa (
            const Evaluation::DataID& id_tst, const EvaluationTargetData& target_data,
            boost::posix_time::time_duration max_ref_time_diff); // tst timestamp
    std::pair<ValueComparisonResult, std::string> compareModeA (
            const Evaluation::DataID& id_tst, const EvaluationTargetData& target_data,
            boost::posix_time::time_duration max_ref_time_diff); // tst timestamp
    std::pair<ValueComparisonResult, std::string> compareModeC (
            const Evaluation::DataID& id_tst, const EvaluationTargetData& target_data,
            boost::posix_time::time_duration max_ref_time_diff, float max_val_diff); // tod tst
};

}

#endif // EVALUATIONREQUIREMENT_H
