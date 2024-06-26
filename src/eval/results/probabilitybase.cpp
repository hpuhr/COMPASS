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

#include "eval/results/probabilitybase.h"

namespace EvaluationRequirementResult
{

namespace common
{
    /**
    */
    EvaluationRequirement::ProbabilityBase& castRequirement(const std::shared_ptr<EvaluationRequirement::Base>& base_req)
    {
        assert (base_req);

        EvaluationRequirement::ProbabilityBase* req_ptr = dynamic_cast<EvaluationRequirement::ProbabilityBase*>(base_req.get());
        assert (req_ptr);

        return *req_ptr;
    }

    /**
    */
    QVariant formatResultValue(double value)
    {
        return std::round(value * 10000.0) / 100.0;
    }

    /**
    */
    double invertProbability(double prob)
    {
        return 1.0 - prob;
    }
}

/****************************************************************************************
 * SingleProbabilityBase
 ****************************************************************************************/

/**
*/
SingleProbabilityBase::SingleProbabilityBase(const std::string& type, 
                                             const std::string& result_id,
                                             std::shared_ptr<EvaluationRequirement::Base> requirement,
                                             const SectorLayer& sector_layer,
                                             unsigned int utn,
                                             const EvaluationTargetData* target,
                                             EvaluationManager& eval_man,
                                             const EvaluationDetails& details)
:   Single(type, result_id, requirement, sector_layer, utn, target, eval_man, details)
{
}

/**
*/
SingleProbabilityBase::~SingleProbabilityBase() = default;

/**
*/
EvaluationRequirement::ProbabilityBase& SingleProbabilityBase::probRequirement() const
{
    return common::castRequirement(requirement_);
}

/**
*/
boost::optional<double> SingleProbabilityBase::computeResult() const
{
    auto result = computeResult_impl();

    //invert probability?
    if (result.has_value())
        result = invertProb(result.value());
}

/**
*/
QVariant SingleProbabilityBase::resultValue(double value) const
{
    return common::formatResultValue(value);
}

/**
*/
double SingleProbabilityBase::invertProb(double prob) const
{
    return common::invertProbability(prob);
}

/****************************************************************************************
 * JoinedProbabilityBase
 ****************************************************************************************/

/**
*/
JoinedProbabilityBase::JoinedProbabilityBase(const std::string& type, 
                                             const std::string& result_id,
                                             std::shared_ptr<EvaluationRequirement::Base> requirement,
                                             const SectorLayer& sector_layer,
                                             EvaluationManager& eval_man)
:   Joined(type, result_id, requirement, sector_layer, eval_man)
{
}

/**
*/
JoinedProbabilityBase::~JoinedProbabilityBase() = default;

/**
*/
EvaluationRequirement::ProbabilityBase& JoinedProbabilityBase::probRequirement() const
{
    return common::castRequirement(requirement_);
}

/**
*/
boost::optional<double> JoinedProbabilityBase::computeResult() const
{
    auto result = computeResult_impl();

    //invert probability?
    if (result.has_value())
        result = invertProb(result.value());
}

/**
*/
QVariant JoinedProbabilityBase::resultValue(double value) const
{
    return common::formatResultValue(value);
}

/**
*/
double JoinedProbabilityBase::invertProb(double prob) const
{
    return common::invertProbability(prob);
}

}