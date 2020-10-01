#include "evaluationrequirementdetectionresult.h"
#include "evaluationrequirement.h"
#include "evaluationrequirementdetection.h"
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

EvaluationRequirementDetectionResult::EvaluationRequirementDetectionResult(
        std::shared_ptr<EvaluationRequirement> requirement,
        std::vector<unsigned int> utns, std::vector<const EvaluationTargetData*> targets,
        EvaluationManager& eval_man,
        float sum_uis, float missed_uis, float max_gap_uis, float no_ref_uis)
    : EvaluationRequirementResult(requirement, utns, targets, eval_man), sum_uis_(sum_uis), missed_uis_(missed_uis),
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
        has_pd_ = true;
    }
    else
    {
        pd_ = 0;
        has_pd_ = false;
    }
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
                requirement_, utns_, targets_, eval_man_, sum_uis_, missed_uis_, max_gap_uis_, no_ref_uis_);
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

void EvaluationRequirementDetectionResult::addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item)
{
    loginf << "EvaluationRequirementDetectionResult " <<  requirement_->name() <<": addToReport";

    // add to main requirements overview

    if (!utns_.size()) // some data must exist
    {
        logerr << "EvaluationRequirementDetectionResult " <<  requirement_->name() <<": addToReport: no data";
        return;
    }

    if (utns_.size() > 1) // is joined. TODO how to distinguish different aggregations?
    {
        loginf << "EvaluationRequirementDetectionResult " <<  requirement_->name()
               << ": addToReport: adding joined result";

        EvaluationResultsReport::SectionContentTable& ov_table = getReqOverviewTable(root_item);

        // condition
        std::shared_ptr<EvaluationRequirementDetection> req =
                std::static_pointer_cast<EvaluationRequirementDetection>(requirement_);
        assert (req);

        string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

        // pd
        QVariant pd_var;

        string result {"Unknown"};

        if (has_pd_)
        {
            pd_var = String::percentToString(pd_ * 100.0).c_str();

            result = pd_ >= req->minimumProbability() ? "Passed" : "Failed";
        }

        // "Req.", "Group", "Result", "Condition", "Result"
        ov_table.addRow({requirement_->shortname().c_str(), requirement_->groupName().c_str(),
                         pd_var, condition.c_str(), result.c_str()}, nullptr,
                        "Report:Results:"+getRequirementSectionID()); // "Report:Results:Overview"
    }
    else // utns_.size() == 1
    {
        // add target to requirements->group->req

        loginf << "EvaluationRequirementDetectionResult " <<  requirement_->name()
               << ": addToReport: adding single result";

        assert (utns_.size() == 1);

        const EvaluationTargetData* target = targets_.at(0);

        EvaluationResultsReport::Section& tgt_overview_section = getRequirementSection(root_item);

        if (!tgt_overview_section.hasTable("target_table"))
            tgt_overview_section.addTable("target_table", 13,
            {"UTN", "Begin", "End", "Callsign", "Target Addr.", "Mode 3/A", "Mode C Min", "Mode C Max",
             "EUIs", "MUIs", "MGUIs", "NRUIs", "PD"});

        EvaluationResultsReport::SectionContentTable& target_table = tgt_overview_section.getTable("target_table");

        QVariant pd_var;

        if (has_pd_)
            pd_var = roundf(pd_ * 10000.0) / 100.0;

        string utn_req_section_heading =
                "Details:Targets:"+to_string(utns_.at(0))+":"+requirement_->groupName()+":"+requirement_->name();

        target_table.addRow(
        {utns_.at(0), target->timeBeginStr().c_str(), target->timeEndStr().c_str(),
         target->callsignsStr().c_str(), target->targetAddressesStr().c_str(),
         target->modeACodesStr().c_str(), target->modeCMinStr().c_str(),
         target->modeCMaxStr().c_str(), sum_uis_, missed_uis_, max_gap_uis_, no_ref_uis_, pd_var},
                    eval_man_.getViewableForUTN(utns_.at(0)), "Report:Results:"+utn_req_section_heading);

        // add requirement to targets->utn->requirements->group->req

        EvaluationResultsReport::Section& utn_req_section = root_item->getSection(utn_req_section_heading);

        if (!utn_req_section.hasTable("details_table"))
            utn_req_section.addTable("details_table", 3, {"Name", "comment", "Value"});

        EvaluationResultsReport::SectionContentTable& utn_req_table = utn_req_section.getTable("details_table");

        utn_req_table.addRow({"EUIs", "Expected Update Intervals", sum_uis_});
        utn_req_table.addRow({"MUIs", "Missed Update Intervals", missed_uis_});
        utn_req_table.addRow({"MGUIs", "Max. Gap Update Intervals", max_gap_uis_});
        utn_req_table.addRow({"NRUIs", "No Reference Update Intervals", no_ref_uis_});
        utn_req_table.addRow({"PD", "Probability of Dectection", pd_var});

        // condition
        std::shared_ptr<EvaluationRequirementDetection> req =
                std::static_pointer_cast<EvaluationRequirementDetection>(requirement_);
        assert (req);

        string condition = ">= "+String::percentToString(req->minimumProbability() * 100.0);

        utn_req_table.addRow({"Condition", "", condition.c_str()});

        string result {"Unknown"};

        if (has_pd_)
            result = pd_ >= req->minimumProbability() ? "Passed" : "Failed";

        utn_req_table.addRow({"Condition Fulfilled", "", result.c_str()});
    }

    // TODO add requirement description, methods
}
