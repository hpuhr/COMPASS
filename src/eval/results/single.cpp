#include "eval/results/single.h"
#include "eval/requirement/base.h"
#include "eval/results/report/sectioncontenttable.h"
#include "evaluationtargetdata.h"
#include "sectorlayer.h"

namespace EvaluationRequirementResult
{

Single::Single(
        const std::string& type, const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
        const SectorLayer& sector_layer, unsigned int utn, const EvaluationTargetData* target,
        EvaluationManager& eval_man)
    : Base(type, result_id, requirement, sector_layer, eval_man), utn_(utn), target_(target)
{
}

unsigned int Single::utn() const
{
    return utn_;
}

const EvaluationTargetData* Single::target() const
{
    return target_;
}

void Single::updateUseFromTarget ()
{
    use_ = result_usable_ && target_->use();
}

std::string Single::getTargetSectionID ()
{
    return "Targets:"+to_string(utn_)+":"+sector_layer_.name()+":"+requirement_->groupName()+":"+requirement_->name();
}

void Single::addCommonDetails (EvaluationResultsReport::SectionContentTable& details_overview_table)
{
    details_overview_table.addRow({"UTN", "Unique Target Number", utn_}, this);
    details_overview_table.addRow({"Begin", "Begin time of target", target_->timeBeginStr().c_str()}, this);
    details_overview_table.addRow({"End", "End time of target", target_->timeEndStr().c_str()}, this);
    details_overview_table.addRow({"Callsign", "Mode S target identification(s)", target_->callsignsStr().c_str()}, this);
    details_overview_table.addRow({"Target Addr.", "Mode S target adress(es)", target_->targetAddressesStr().c_str()}, this);
    details_overview_table.addRow({"Mode 3/A", "Mode 3/A code(s)", target_->modeACodesStr().c_str()}, this);
    details_overview_table.addRow({"Mode C Min", "Minimum Mode C code [ft]", target_->modeCMinStr().c_str()}, this);
    details_overview_table.addRow({"Mode C Max", "Maximum Mode C code [ft]", target_->modeCMaxStr().c_str()}, this);
}

}
