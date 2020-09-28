#include "evaluationresultsgenerator.h"
#include "evaluationmanager.h"
#include "evaluationdata.h"
#include "evaluationstandard.h"
#include "evaluationrequirementgroup.h"
#include "evaluationrequirementconfig.h"
#include "evaluationrequirement.h"
#include "evaluationrequirementresult.h"
#include "logger.h"

using namespace std;

EvaluationResultsGenerator::EvaluationResultsGenerator(EvaluationManager& eval_man)
    : eval_man_(eval_man)
{

}


void EvaluationResultsGenerator::evaluate (EvaluationData& data, EvaluationStandard& standard)
{
    loginf << "EvaluationResultsGenerator: evaluate";

    results_model_.beginReset();

    std::shared_ptr<EvaluationResultsReport::RootItem> root_item = results_model_.rootItem();

    for (auto& req_group_it : standard)
    {
        logdbg << "EvaluationResultsGenerator: evaluate: group " << req_group_it.first;




        for (auto& req_cfg_it : *req_group_it.second)
        {
            logdbg << "EvaluationResultsGenerator: evaluate: group " << req_group_it.first
                   << " req '" << req_cfg_it->name() << "'";

            std::shared_ptr<EvaluationRequirement> req = req_cfg_it->createRequirement();
            std::shared_ptr<EvaluationRequirementResult> result_sum;

            for (auto& target_data_it : data)
            {
//                if (target_data_it.utn_ != 610)
//                    continue;

                logdbg << "EvaluationResultsGenerator: evaluate: group " << req_group_it.first
                       << " req '" << req_cfg_it->name() << "' utn " << target_data_it.utn_;

                std::shared_ptr<EvaluationRequirementResult> result = req->evaluate(target_data_it, req);

                result->print();
                result->addToReport(root_item);

                if (!result_sum)
                    result_sum = result->copy();
                else
                    result_sum->join(result);
            }

            result_sum->print();
        }
    }

    results_model_.endReset();
}

EvaluationResultsReport::TreeModel& EvaluationResultsGenerator::resultsModel()
{
    return results_model_;
}
