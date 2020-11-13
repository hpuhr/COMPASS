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

#ifndef EVALUATIONRESULTSGENERATOR_H
#define EVALUATIONRESULTSGENERATOR_H

#include "eval/results/report/treemodel.h"
#include "eval/requirement/base.h"
#include "eval/results/base.h"
#include "evaluationdata.h"
#include "sectorlayer.h"

#include <tbb/tbb.h>

class EvaluationManager;
class EvaluationStandard;

//namespace EvaluationRequirement {
//    class Base;
//}

namespace EvaluationRequirementResult
{
    class Base;
    class Single;
}

class EvaluateTask : public tbb::task {

public:
    EvaluateTask(std::vector<std::shared_ptr<EvaluationRequirementResult::Single>>& results,
                 std::vector<unsigned int>& utns,
                 EvaluationData& data,
                 std::shared_ptr<EvaluationRequirement::Base> req,
                 const SectorLayer& sector_layer,
                 std::vector<bool>& done_flags, bool single_thread)
        : results_(results), utns_(utns), data_(data), req_(req), sector_layer_(sector_layer), done_flags_(done_flags),
          single_thread_(single_thread)
    {
    }

    /*override*/ tbb::task* execute() {
        // Do the job

        unsigned int num_utns = utns_.size();

        if (single_thread_)
        {
            for(unsigned int utn_cnt=0; utn_cnt < num_utns; ++utn_cnt)
            {
                results_[utn_cnt] = req_->evaluate(data_.targetData(utns_.at(utn_cnt)), req_, sector_layer_);
                done_flags_[utn_cnt] = true;
            }
        }
        else
        {
            tbb::parallel_for(uint(0), num_utns, [&](unsigned int utn_cnt)
            {
                results_[utn_cnt] = req_->evaluate(data_.targetData(utns_.at(utn_cnt)), req_, sector_layer_);
                done_flags_[utn_cnt] = true;
            });
        }

        return NULL; // or a pointer to a new task to be executed immediately
    }

protected:
    std::vector<std::shared_ptr<EvaluationRequirementResult::Single>>& results_;
    std::vector<unsigned int>& utns_;
    EvaluationData& data_;
    std::shared_ptr<EvaluationRequirement::Base> req_;
    const SectorLayer& sector_layer_;
    std::vector<bool>& done_flags_;
    bool single_thread_;
};

//class GenerateResultsTask : public tbb::task {

//public:
//    GenerateResultsTask(std::shared_ptr<EvaluationResultsReport::RootItem> root_item,
//                        std::vector<std::shared_ptr<EvaluationRequirementResult::Base>>& results_vec)
//        :root_item_(root_item), results_vec_(results_vec)
//    {
//    }

//    /*override*/ tbb::task* execute() {
//        // Do the job

//        // first add all joined
//        for (auto& result_it : results_vec_)
//            if (result_it->isJoined())
//                result_it->addToReport(root_item_);

//        // then all singles
//        for (auto& result_it : results_vec_)
//            if (result_it->isSingle())
//                result_it->addToReport(root_item_);

//        done_ = true;

//        return NULL; // or a pointer to a new task to be executed immediately
//    }

//    bool done () { return done_; }

//protected:
//    std::shared_ptr<EvaluationResultsReport::RootItem> root_item_;
//    std::vector<std::shared_ptr<EvaluationRequirementResult::Base>>& results_vec_;

//    bool done_ {false};
//};

class EvaluationResultsGenerator
{
public:
    EvaluationResultsGenerator(EvaluationManager& eval_man);
    ~EvaluationResultsGenerator();

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

protected:
    EvaluationManager& eval_man_;

    EvaluationResultsReport::TreeModel results_model_;

    // rq group+name -> id -> result, e.g. "All:PD"->"UTN:22"-> or "SectorX:PD"->"All"
    std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>> results_;
    std::vector<std::shared_ptr<EvaluationRequirementResult::Base>> results_vec_; // ordered as generated
};

#endif // EVALUATIONRESULTSGENERATOR_H
