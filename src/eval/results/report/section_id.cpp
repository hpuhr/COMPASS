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

#include "section_id.h"

#include "sectorlayer.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"
#include "eval/results/base/base.h"
#include "evaluationmanager.h"

#include <QString>

namespace EvaluationResultsReport
{

const std::string SectionID::Sep            = ":";
const std::string SectionID::SectionReport  = "Report";
const std::string SectionID::SectionResults = "Results";
const std::string SectionID::SectionTargets = "Targets";
const std::string SectionID::SectionUTN     = "UTN ";
const std::string SectionID::SectionSum     = "Sum";
const std::string SectionID::SectionSectors = "Sectors";

/**
*/
std::string SectionID::prependReportResultID(const std::string& id)
{
    return reportResultID() + Sep + id;
}

/**
*/
std::vector<std::string> SectionID::subSections(const std::string& section_id) 
{
    return Utils::String::split(section_id, Sep[ 0 ]);
}

/**
*/
std::string SectionID::sectionID(const std::vector<std::string>& sub_sections)
{
    return Utils::String::compress(sub_sections, Sep[ 0 ]);
}

/**
*/
std::string SectionID::sectionID(const std::string& section0, const std::string& section1)
{
    assert(!section0.empty() && !section1.empty());

    return (section0 + Sep + section1);
}

/**
*/
std::string SectionID::sectionIDWithoutResults(const std::string& section_id)
{
    //root?
    if (section_id == SectionResults)
        return "";

    const std::string ResultsHeader = SectionResults + Sep;

    //valid?
    assert (section_id.rfind(ResultsHeader, 0) == 0);

    //chop result header
    std::string ret = section_id;
    ret.erase(0,ResultsHeader.size());

    return ret;
}

/**
*/
std::string SectionID::sectionID2Path(const std::string& section_id)
{
    std::string p = section_id;

    boost::replace_all(p, ":", "/");
    boost::replace_all(p, " ", "_");

    return (p + "/");
}

/**
*/
std::string SectionID::reportResultID()
{
    return SectionReport + Sep + SectionResults;
}

/**
*/
std::string SectionID::targetResultsID()
{
    return SectionResults + Sep + SectionTargets;
}

/**
*/
std::string SectionID::targetID()
{
    return SectionTargets + Sep + SectionUTN;
}

/**
*/
std::string SectionID::targetID(unsigned int utn)
{
    return targetID() + std::to_string(utn);
}

/**
*/
std::string SectionID::sectorLayerID(const SectorLayer& sector_layer)
{
    return sector_layer.name();
}

/**
*/
std::string SectionID::requirementID(const EvaluationRequirement::Base& requirement)
{
    return requirement.name();
}

/**
*/
std::string SectionID::requirementGroupID(const EvaluationRequirement::Base& requirement)
{
    return requirement.groupName();
}

/**
*/
std::string SectionID::requirementGroupResultID(const EvaluationRequirementResult::Base& result)
{
    return sectorLayerID(result.sectorLayer()) + Sep + 
           requirementGroupID(*result.requirement()) + Sep + 
           requirementID(*result.requirement());
}

/**
*/
std::string SectionID::requirementGroupSectorID(const EvaluationRequirement::Base& requirement,
                                                const SectorLayer& sector_layer)
{
    return requirementGroupID(requirement) + " " + sectorLayerID(sector_layer);
}

/**
*/
std::string SectionID::resultID(const EvaluationRequirementResult::Base& result)
{
    return result.resultId();
}

/**
*/
std::string SectionID::targetResultID(unsigned int utn, const EvaluationRequirementResult::Base& result)
{
    assert(result.requirement());
    return targetID(utn) + Sep + requirementGroupResultID(result);
}

/**
*/
std::string SectionID::requirementResultID(const EvaluationRequirementResult::Base& result)
{
    assert(result.requirement());
    return SectionSectors + Sep + 
           requirementGroupSectorID(*result.requirement(), result.sectorLayer()) + Sep + 
           resultID(result) + Sep + 
           requirementID(*result.requirement());
}

/**
*/
std::string SectionID::requirementResultSumID(const EvaluationRequirementResult::Base& result)
{
    assert(result.requirement());
    return SectionSectors + Sep + 
           requirementGroupSectorID(*result.requirement(), result.sectorLayer()) + Sep + 
           SectionSum + Sep + 
           requirementID(*result.requirement());
}

/**
*/
std::string SectionID::createForTargetResult(unsigned int utn,
                                             const EvaluationRequirementResult::Base& result)
{
    return reportResultID() + Sep + 
           targetResultID(utn, result);
}

/**
*/
std::string SectionID::createForRequirementResult(const EvaluationRequirementResult::Base& result)
{
    return reportResultID() + Sep + 
           requirementResultID(result);
}

/**
*/
std::string SectionID::createForRequirementResultSum(const EvaluationRequirementResult::Base& result)
{
    return reportResultID() + Sep + 
           requirementResultSumID(result);
}

/**
 * Converts the given sum result id to a target result id for the given utn.
 */
std::string SectionID::sumResult2Target(const std::string& sum_result_id, 
                                        unsigned int utn, 
                                        const EvaluationManager& eval_manager)
{
    //iterate over all results
    for (const auto& elem : eval_manager.results())
    {
        for (const auto& elem2 : elem.second)
        {
            if (elem2.second->isJoined())
            {
                //if the result yields our sum result id => create a target result id utilizing info from the same result
                if (sum_result_id == createForRequirementResultSum(*elem2.second))
                    return createForTargetResult(utn, *elem2.second);
            }
        }
    }

    return "";
}

}
