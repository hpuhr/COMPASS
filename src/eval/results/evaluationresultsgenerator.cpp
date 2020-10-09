#include "evaluationresultsgenerator.h"
#include "evaluationmanager.h"
#include "evaluationdata.h"
#include "evaluationstandard.h"
#include "eval/requirement/group.h"
#include "eval/requirement/config.h"
#include "eval/requirement/base.h"
#include "eval/results/single.h"
#include "eval/results/detection/joined.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "logger.h"
#include "stringconv.h"

#include "boost/date_time/posix_time/posix_time.hpp"

#include <tbb/tbb.h>

using namespace std;
using namespace EvaluationRequirementResult;
using namespace Utils;

EvaluationResultsGenerator::EvaluationResultsGenerator(EvaluationManager& eval_man)
    : eval_man_(eval_man), results_model_(eval_man_)
{

}


void EvaluationResultsGenerator::evaluate (EvaluationData& data, EvaluationStandard& standard)
{
    loginf << "EvaluationResultsGenerator: evaluate";

    boost::posix_time::ptime loading_start_time_;
    boost::posix_time::ptime loading_stop_time_;

    loading_start_time_ = boost::posix_time::microsec_clock::local_time();

    // clear everything
    results_model_.beginReset();
    results_model_.clear();
    results_.clear();
    results_model_.endReset();

    // prepare for new data
    results_model_.beginReset();

    std::shared_ptr<EvaluationResultsReport::RootItem> root_item = results_model_.rootItem();

    EvaluationResultsReport::Section& overview_section = root_item->getSection("Overview");
    overview_section.addText("Sample");

    EvaluationResultsReport::SectionContentText& overview_text = overview_section.getText("Sample");

    overview_text.addText("Why not visit Sweden this time of the year?");
    overview_text.addText("It has lovely lakes");
    overview_text.addText("Elk bytes\nline2");

    //string req_grp_id;
    //string result_id;

    vector<unsigned int> utns;

    for (auto& target_data_it : data)
        utns.push_back(target_data_it.utn_);

    unsigned int num_utns = utns.size();

    for (auto& req_group_it : standard)
    {
        logdbg << "EvaluationResultsGenerator: evaluate: group " << req_group_it.first;

        for (auto& req_cfg_it : *req_group_it.second)
        {
            logdbg << "EvaluationResultsGenerator: evaluate: group " << req_group_it.first
                   << " req '" << req_cfg_it->name() << "'";

            std::shared_ptr<EvaluationRequirement::Base> req = req_cfg_it->createRequirement();
            std::shared_ptr<Joined> result_sum;

            //req_grp_id = req->groupName()+":"+req->name();

            vector<shared_ptr<Single>> results;
            results.resize(num_utns);

            // generate results
            tbb::parallel_for(uint(0), num_utns, [&](unsigned int utn_cnt)
            {
                results[utn_cnt] = req->evaluate(data.targetData(utns.at(utn_cnt)), req);
            });

            for (auto& result_it : results)
            {
                //result->print();
                result_it->addToReport(root_item);

                // add to results
                // rq group+name -> id -> result, e.g. "All:PD"->"UTN:22"-> or "SectorX:PD"->"All"

                //result_id = "UTN:"+to_string(target_data_it.utn_);

                results_[result_it->reqGrpId()][result_it->resultId()] = result_it;

                if (!result_sum)
                    result_sum = result_it->createEmptyJoined("All");

                if (result_it->use())
                    result_sum->join(result_it);
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

    loading_stop_time_ = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = loading_stop_time_ - loading_start_time_;
    load_time = diff.total_milliseconds() / 1000.0;

    loginf << "EvaluationResultsGenerator: evaluate done " << String::timeStringFromDouble(load_time, true);

    // 00:06:22.852 with no parallel

    emit eval_man_.resultsChangedSignal();
}

EvaluationResultsReport::TreeModel& EvaluationResultsGenerator::resultsModel()
{
    return results_model_;
}
