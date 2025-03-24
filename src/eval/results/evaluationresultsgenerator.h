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
#include "evaluationresultsgeneratorwidget.h"

//#include "sectorlayer.h"
//#include "logger.h"

//#include <tbb/tbb.h>

class EvaluationManager;
class EvaluationManagerSettings;
class EvaluationStandard;

namespace EvaluationRequirementResult
{
    class Base;
    class Single;
}

class EvaluationResultsGenerator
{
public:
    EvaluationResultsGenerator(EvaluationManager& eval_man, EvaluationManagerSettings& eval_settings);
    virtual ~EvaluationResultsGenerator();

    void evaluate (EvaluationData& data, EvaluationStandard& standard);

    EvaluationResultsReport::TreeModel& resultsModel();

    typedef std::map<std::string,
    std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>::const_iterator ResultIterator;

    ResultIterator begin() { return results_.begin(); }
    ResultIterator end() { return results_.end(); }

    const std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>& results ()
    const { return results_; } ;

    void updateToChanges();

    void generateResultsReportGUI();

    void clear();

    EvaluationResultsGeneratorWidget* widget(); // has to take ownership

protected:
    void addNonResultsContent (std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void updateToChanges(bool reset_viewable);

    EvaluationManager& eval_man_;
    EvaluationManagerSettings& eval_settings_;

    EvaluationResultsReport::TreeModel results_model_;

    // rq group+name -> id -> result, e.g. "All:PD"->"UTN:22"-> or "SectorX:PD"->"All"
    std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>> results_;
    std::vector<std::shared_ptr<EvaluationRequirementResult::Base>> results_vec_; // ordered as generated

    
};
