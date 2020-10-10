#ifndef EVALUATIONRESULTSGENERATOR_H
#define EVALUATIONRESULTSGENERATOR_H

#include "eval/results/report/treemodel.h"

class EvaluationManager;
class EvaluationStandard;
class EvaluationData;

namespace EvaluationRequirementResult
{
    class Base;
}

class EvaluationResultsGenerator
{
public:
    EvaluationResultsGenerator(EvaluationManager& eval_man);

    void evaluate (EvaluationData& data, EvaluationStandard& standard);

    EvaluationResultsReport::TreeModel& resultsModel();

    typedef std::map<std::string,
      std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>::const_iterator ResultIterator;

    ResultIterator begin() { return results_.begin(); }
    ResultIterator end() { return results_.end(); }

    const std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>>& results ()
    const { return results_; } ;

    void updateToUseChangeOf (unsigned int utn);

    void generateResultsReport();

protected:
    EvaluationManager& eval_man_;

    EvaluationResultsReport::TreeModel results_model_;

    // rq group+name -> id -> result, e.g. "All:PD"->"UTN:22"-> or "SectorX:PD"->"All"
    std::map<std::string, std::map<std::string, std::shared_ptr<EvaluationRequirementResult::Base>>> results_;
    std::vector<std::shared_ptr<EvaluationRequirementResult::Base>> results_vec_; // ordered as generated
};

#endif // EVALUATIONRESULTSGENERATOR_H
