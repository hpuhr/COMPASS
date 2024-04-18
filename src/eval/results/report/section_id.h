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

#pragma once

#include <string>

class SectorLayer;

class EvaluationManager;

namespace EvaluationRequirement
{
    class Base;
}

namespace EvaluationRequirementResult
{
    class Base;
}

namespace EvaluationResultsReport
{

/**
 * Functionality for creating unique requirement result section ids of various types.
 *   - ids for target-specific result sections
 *   - ids for sum result sections
 *   - conversion between result section types
 *   - definition of all needed strings in one place
 */
class SectionID 
{
public:
    //creation of requirement result section ids
    static std::string createForTargetResult(unsigned int utn, const EvaluationRequirementResult::Base& result);
    static std::string createForRequirementResult(const EvaluationRequirementResult::Base& result);
    static std::string createForRequirementResultSum(const EvaluationRequirementResult::Base& result);

    //conversion
    static std::string sumResult2Target(const std::string& sum_result_id, 
                                        unsigned int utn,
                                        const EvaluationManager& eval_manager);
    //tools
    static std::string prependReportResultID(const std::string& id);

    //needed section id parts
    static std::string reportResultID();
    static std::string targetID();
    static std::string targetID(unsigned int utn);
    static std::string sectorLayerID(const SectorLayer& sector_layer);
    static std::string requirementID(const EvaluationRequirement::Base& requirement);
    static std::string requirementGroupID(const EvaluationRequirement::Base& requirement);
    static std::string requirementGroupSectorID(const EvaluationRequirement::Base& requirement,
                                                const SectorLayer& sector_layer);
    static std::string resultID(const EvaluationRequirementResult::Base& result);
    static std::string targetResultID(unsigned int utn, const EvaluationRequirementResult::Base& result);
    static std::string requirementResultID(const EvaluationRequirementResult::Base& result);
    static std::string requirementResultSumID(const EvaluationRequirementResult::Base& result);

    //section name definitions
    static const std::string Sep;
    static const std::string SectionReport;
    static const std::string SectionResults;
    static const std::string SectionTargets;
    static const std::string SectionUTN;
    static const std::string SectionSum;
    static const std::string SectionSectors;
};

} // EvaluationResultsReport
