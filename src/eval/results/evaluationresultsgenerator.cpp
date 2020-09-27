#include "evaluationresultsgenerator.h"
#include "evaluationmanager.h"
#include "evaluationdata.h"
#include "evaluationstandard.h"
#include "evaluationrequirementgroup.h"
#include "evaluationrequirementconfig.h"
#include "evaluationrequirement.h"
#include "logger.h"

using namespace std;

EvaluationResultsGenerator::EvaluationResultsGenerator(EvaluationManager& eval_man)
    : eval_man_(eval_man)
{

}


void EvaluationResultsGenerator::evaluate (EvaluationData& data, EvaluationStandard& standard)
{
    loginf << "EvaluationResultsGenerator: evaluate";

    for (auto& req_group_it : standard)
    {
        logdbg << "EvaluationResultsGenerator: evaluate: group " << req_group_it.first;

        for (auto& req_cfg_it : *req_group_it.second)
        {
            logdbg << "EvaluationResultsGenerator: evaluate: group " << req_group_it.first
                   << " req '" << req_cfg_it->name() << "'";

            std::shared_ptr<EvaluationRequirement> req = req_cfg_it->createRequirement();

            for (auto& target_data_it : data)
            {
                logdbg << "EvaluationResultsGenerator: evaluate: group " << req_group_it.first
                       << " req '" << req_cfg_it->name() << "' utn " << target_data_it.utn_;

                req->evaluate(target_data_it);
            }
        }
    }
}
