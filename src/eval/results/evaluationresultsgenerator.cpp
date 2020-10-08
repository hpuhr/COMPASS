#include "evaluationresultsgenerator.h"
#include "evaluationmanager.h"
#include "evaluationdata.h"
#include "evaluationstandard.h"
#include "evaluationrequirementgroup.h"
#include "evaluationrequirementconfig.h"
#include "evaluationrequirement.h"
#include "eval/results/single.h"
#include "eval/results/detection/joined.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "logger.h"

using namespace std;
using namespace EvaluationRequirementResult;

EvaluationResultsGenerator::EvaluationResultsGenerator(EvaluationManager& eval_man)
    : eval_man_(eval_man), results_model_(eval_man_)
{

}


void EvaluationResultsGenerator::evaluate (EvaluationData& data, EvaluationStandard& standard)
{
    loginf << "EvaluationResultsGenerator: evaluate";

    results_model_.beginReset();
    results_.clear();

    std::shared_ptr<EvaluationResultsReport::RootItem> root_item = results_model_.rootItem();

    EvaluationResultsReport::Section& overview_section = root_item->getSection("Overview");
    overview_section.addText("Sample");

    EvaluationResultsReport::SectionContentText& overview_text = overview_section.getText("Sample");

    overview_text.addText("Why not visit Sweden this time of the year?");
    overview_text.addText("It has lovely lakes");
    overview_text.addText("Elk bytes\nline2");

    //string req_grp_id;
    //string result_id;

    for (auto& req_group_it : standard)
    {
        logdbg << "EvaluationResultsGenerator: evaluate: group " << req_group_it.first;

        for (auto& req_cfg_it : *req_group_it.second)
        {
            logdbg << "EvaluationResultsGenerator: evaluate: group " << req_group_it.first
                   << " req '" << req_cfg_it->name() << "'";

            std::shared_ptr<EvaluationRequirement> req = req_cfg_it->createRequirement();
            std::shared_ptr<Joined> result_sum;

            //req_grp_id = req->groupName()+":"+req->name();

            for (auto& target_data_it : data)
            {
//                if (target_data_it.utn_ != 610)
//                    continue;

                logdbg << "EvaluationResultsGenerator: evaluate: group " << req_group_it.first
                       << " req '" << req_cfg_it->name() << "' utn " << target_data_it.utn_;

                std::shared_ptr<Single> result = req->evaluate(target_data_it, req);

                //result->print();
                result->addToReport(root_item);

                // add to results
                // rq group+name -> id -> result, e.g. "All:PD"->"UTN:22"-> or "SectorX:PD"->"All"

                //result_id = "UTN:"+to_string(target_data_it.utn_);

                results_[result->reqGrpId()][result->resultId()] = result;

                if (!result_sum)
                    result_sum = result->createEmptyJoined("All");

                result_sum->join(result);
            }

            if (result_sum)
            {
                //result_sum->print();
                result_sum->addToReport(root_item);

                results_[result_sum->reqGrpId()][result_sum->resultId()] = result_sum;
            }
        }
    }

    results_model_.endReset();

    emit eval_man_.resultsChangedSignal();
}

EvaluationResultsReport::TreeModel& EvaluationResultsGenerator::resultsModel()
{
    return results_model_;
}
