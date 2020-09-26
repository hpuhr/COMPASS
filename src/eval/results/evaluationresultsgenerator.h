#ifndef EVALUATIONRESULTSGENERATOR_H
#define EVALUATIONRESULTSGENERATOR_H

class EvaluationManager;
class EvaluationStandard;
class EvaluationData;

class EvaluationResultsGenerator
{
public:
    EvaluationResultsGenerator(EvaluationManager& eval_man);

    void evaluate (EvaluationData& data, EvaluationStandard& standard);

protected:
    EvaluationManager& eval_man_;
};

#endif // EVALUATIONRESULTSGENERATOR_H
