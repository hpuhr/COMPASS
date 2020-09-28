#ifndef EVALUATIONRESULTSGENERATOR_H
#define EVALUATIONRESULTSGENERATOR_H

#include "eval/results/report/treemodel.h"

class EvaluationManager;
class EvaluationStandard;
class EvaluationData;

class EvaluationResultsGenerator
{

public:
    EvaluationResultsGenerator(EvaluationManager& eval_man);

    void evaluate (EvaluationData& data, EvaluationStandard& standard);

    EvaluationResultsReport::TreeModel& resultsModel();

protected:
    EvaluationManager& eval_man_;

    EvaluationResultsReport::TreeModel results_model_;
};

#endif // EVALUATIONRESULTSGENERATOR_H
