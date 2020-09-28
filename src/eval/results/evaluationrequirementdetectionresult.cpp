#include "evaluationrequirementdetectionresult.h"
#include "evaluationrequirement.h"
#include "evaluationrequirementdetection.h"
#include "logger.h"
#include "stringconv.h"

#include <cassert>

using namespace std;
using namespace Utils;

EvaluationRequirementDetectionResult::EvaluationRequirementDetectionResult(
        std::shared_ptr<EvaluationRequirement> requirement, std::vector<unsigned int> utns,
        float sum_uis, float missed_uis, float max_gap_uis, float no_ref_uis)
    : EvaluationRequirementResult(requirement, utns), sum_uis_(sum_uis), missed_uis_(missed_uis),
      max_gap_uis_(max_gap_uis), no_ref_uis_(no_ref_uis)
{
    updatePD();
}


void EvaluationRequirementDetectionResult::updatePD()
{
    if (sum_uis_)
    {
        assert (sum_uis_ > max_gap_uis_ + no_ref_uis_);
        pd_ = 1.0 - (missed_uis_/(sum_uis_ - max_gap_uis_ - no_ref_uis_));
    }
    else
        pd_ = 0;
}

void EvaluationRequirementDetectionResult::join(const std::shared_ptr<EvaluationRequirementResult> other_base)
{
    logdbg << "EvaluationRequirementDetectionResult: join";

    EvaluationRequirementResult::join(other_base);

    const std::shared_ptr<EvaluationRequirementDetectionResult> other =
            std::static_pointer_cast<EvaluationRequirementDetectionResult>(other);

    assert (other);

    sum_uis_ += other->sum_uis_;
    missed_uis_ += other->missed_uis_;
    max_gap_uis_ += other->max_gap_uis_;
    no_ref_uis_ += other->no_ref_uis_;

    updatePD();
}

std::shared_ptr<EvaluationRequirementResult> EvaluationRequirementDetectionResult::copy ()
{
    loginf << "EvaluationRequirementDetectionResult: copy";

    std::shared_ptr<EvaluationRequirementDetectionResult> copy = make_shared<EvaluationRequirementDetectionResult>(
                requirement_, utns_, sum_uis_, missed_uis_, max_gap_uis_, no_ref_uis_);
    copy->updatePD();

    return copy;
}

void EvaluationRequirementDetectionResult::print()
{
    std::shared_ptr<EvaluationRequirementDetection> req =
                std::static_pointer_cast<EvaluationRequirementDetection>(requirement_);
    assert (req);

    if (sum_uis_)
        loginf << "EvaluationRequirementDetectionResult: print: req. name " << req->name()
               << " utn " << utnsString()
               << " pd " << String::percentToString(100.0 * pd_) << " passed " << (pd_ >= req->minimumProbability());
    else
        loginf << "EvaluationRequirementDetectionResult: print: req. name " << req->name()
               << " utn " << utnsString() << " has no data";
}
