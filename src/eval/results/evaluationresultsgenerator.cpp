#include "evaluationresultsgenerator.h"
#include "evaluationmanager.h"
#include "evaluationdata.h"
#include "evaluationstandard.h"
#include "logger.h"

using namespace std;

EvaluationResultsGenerator::EvaluationResultsGenerator(EvaluationManager& eval_man)
    : eval_man_(eval_man)
{

}


void EvaluationResultsGenerator::evaluate (EvaluationData& data, EvaluationStandard& standard)
{
    loginf << "EvaluationResultsGenerator: evaluate";
}
