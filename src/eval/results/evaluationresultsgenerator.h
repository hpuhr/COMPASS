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

#pragma once

#include "eval/results/report/treemodel.h"
#include "eval/results/base/base.h"

#include "evaluationdata.h"

class EvaluationCalculator;
class EvaluationSettings;
class EvaluationStandard;

namespace EvaluationRequirementResult
{
    class Base;
    class Single;
}

namespace ResultReport
{
    class Report;
}

/**
 */
class EvaluationResultsGenerator
{
public:
    EvaluationResultsGenerator(EvaluationCalculator& calculator);
    virtual ~EvaluationResultsGenerator();

    void evaluate(EvaluationData& data, 
                  EvaluationStandard& standard);

    typedef std::map<std::string,
    std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>::const_iterator ResultIterator;

    ResultIterator begin() { return results_.begin(); }
    ResultIterator end() { return results_.end(); }

    const std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>& results ()
    const { return results_; } ;

    void updateToChanges();
    void generateResultsReportGUI();

    void clear();

    EvaluationResultsReport::TreeModel& resultsModel(); //@TODO: remove if no longer needed

    static const std::string EvalResultName;

protected:
    void addNonResultsContent (const std::shared_ptr<ResultReport::Report>& report);
    void updateToChanges(bool reset_viewable);

    EvaluationCalculator& calculator_;

    EvaluationResultsReport::TreeModel results_model_; //@TODO: remove if no longer needed

    // rq group+name -> id -> result, e.g. "All:PD"->"UTN:22"-> or "SectorX:PD"->"All"
    std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>> results_;
    std::vector<std::shared_ptr<EvaluationRequirementResult::Base>> results_vec_; // ordered as generated

};
