#include "eval/results/identification/single.h"
#include "eval/results/identification/joined.h"
#include "eval/requirement/base.h"
#include "eval/requirement/identification/identification.h"
#include "evaluationtargetdata.h"
#include "evaluationmanager.h"
#include "eval/results/report/rootitem.h"
#include "eval/results/report/section.h"
#include "eval/results/report/sectioncontenttext.h"
#include "eval/results/report/sectioncontenttable.h"
#include "logger.h"
#include "stringconv.h"

#include <cassert>

using namespace std;
using namespace Utils;

namespace EvaluationRequirementResult
{

JoinedIdentification::JoinedIdentification(
        const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
        const SectorLayer& sector_layer, EvaluationManager& eval_man)
    : Joined("JoinedIdentification", result_id, requirement, sector_layer, eval_man)
{
}


void JoinedIdentification::join(std::shared_ptr<Base> other)
{
    Joined::join(other);

    std::shared_ptr<SingleIdentification> other_sub =
            std::static_pointer_cast<SingleIdentification>(other);
    assert (other_sub);

    addToValues(other_sub);
}

void JoinedIdentification::addToValues (std::shared_ptr<SingleIdentification> single_result)
{
    assert (single_result);

    if (!single_result->use())
        return;

    num_updates_ += single_result->numUpdates();
    num_no_ref_pos_ += single_result->numNoRefPos();
    num_no_ref_id_ += single_result->numNoRefId();
    num_pos_outside_ += single_result->numPosOutside();
    num_pos_inside_ += single_result->numPosInside();
    num_unknown_id_ += single_result->numUnknownId();
    num_correct_id_ += single_result->numCorrectId();
    num_false_id_ += single_result->numFalseId();

    updatePID();
}

void JoinedIdentification::updatePID()
{
    assert (num_updates_ - num_no_ref_pos_ == num_pos_inside_ + num_pos_outside_);
    assert (num_pos_inside_ == num_no_ref_id_+num_unknown_id_+num_correct_id_+num_false_id_);

    if (num_correct_id_+num_false_id_)
    {
        pid_ = (float)num_correct_id_/(float)(num_correct_id_+num_false_id_);
        has_pid_ = true;
    }
    else
    {
        pid_ = 0;
        has_pid_ = false;
    }
}

void JoinedIdentification::print()
{
    std::shared_ptr<EvaluationRequirement::Identification> req =
            std::static_pointer_cast<EvaluationRequirement::Identification>(requirement_);
    assert (req);

    if (has_pid_)
        loginf << "JoinedIdentification: print: req. name " << req->name()
               << " pid " << String::percentToString(100.0 * pid_)
               << " passed " << (pid_ >= req->minimumProbability());
    else
        loginf << "JoinedIdentification: print: req. name " << req->name()
               << " has no data";
}

void JoinedIdentification::addToReport (
        std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    logdbg << "JoinedIdentification " <<  requirement_->name() <<": addToReport";

    if (!results_.size()) // some data must exist
    {
        logerr << "JoinedIdentification " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    logdbg << "JoinedIdentification " <<  requirement_->name() << ": addToReport: adding joined result";

    EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

    // condition
    std::shared_ptr<EvaluationRequirement::Identification> req =
            std::static_pointer_cast<EvaluationRequirement::Identification>(requirement_);
    assert (req);

    string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

    // pd
    QVariant pd_var;

    string result {"Unknown"};

    if (has_pid_)
    {
        pd_var = String::percentToString(pid_ * 100.0).c_str();

        result = pid_ >= req->minimumProbability() ? "Passed" : "Failed";
    }

    // "Req.", "Group", "Result", "Condition", "Result"
    ov_table.addRow({sector_layer_.name().c_str(), requirement_->shortname().c_str(),
                     requirement_->groupName().c_str(), {num_correct_id_+num_false_id_},
                     pd_var, condition.c_str(), result.c_str()}, this, {});
}

bool JoinedIdentification::hasViewableData (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == "req_overview")
        return true;
    else
        return false;
}

std::unique_ptr<nlohmann::json::object_t> JoinedIdentification::viewableData(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasViewableData(table, annotation));
    return eval_man_.getViewableForEvaluation(req_grp_id_, result_id_);

    return nullptr;
}

bool JoinedIdentification::hasReference (
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    if (table.name() == "req_overview")
        return true;
    else
        return false;;
}

std::string JoinedIdentification::reference(
        const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation)
{
    assert (hasReference(table, annotation));
    return "Report:Results:"+getRequirementSectionID();

    return nullptr;
}

void JoinedIdentification::updatesToUseChanges()
{
    loginf << "JoinedIdentification: updatesToUseChanges: prev num_updates " << num_updates_
           << " num_no_ref_pos " << num_no_ref_pos_ << " num_no_ref_id " << num_no_ref_id_
           << " num_unknown_id " << num_unknown_id_
           << " num_correct_id " << num_correct_id_ << " num_false_id " << num_false_id_;

    if (has_pid_)
        loginf << "JoinedIdentification: updatesToUseChanges: prev result " << result_id_
               << " pid " << 100.0 * pid_;
    else
        loginf << "JoinedIdentification: updatesToUseChanges: prev result " << result_id_ << " has no data";

    num_updates_ = 0;
    num_no_ref_pos_ = 0;
    num_no_ref_id_ = 0;
    num_pos_outside_ = 0;
    num_pos_inside_ = 0;
    num_unknown_id_ = 0;
    num_correct_id_ = 0;
    num_false_id_ = 0;

    for (auto result_it : results_)
    {
        std::shared_ptr<SingleIdentification> result =
                std::static_pointer_cast<SingleIdentification>(result_it);
        assert (result);

        addToValues(result);
    }

    loginf << "JoinedIdentification: updatesToUseChanges: updt num_updates " << num_updates_
           << " num_no_ref_pos " << num_no_ref_pos_ << " num_no_ref_id " << num_no_ref_id_
           << " num_unknown_id " << num_unknown_id_
           << " num_correct_id " << num_correct_id_ << " num_false_id " << num_false_id_;

    if (has_pid_)
        loginf << "JoinedIdentification: updatesToUseChanges: updt result " << result_id_
               << " pid " << 100.0 * pid_;
    else
        loginf << "JoinedIdentification: updatesToUseChanges: updt result " << result_id_ << " has no data";
}

}
