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

#ifndef EVALUATIONREQUIREMENIDENTMODEARESULT_H
#define EVALUATIONREQUIREMENIDENTMODEARESULT_H

#include "eval/results/single.h"
#include "eval/requirement/mode_a/present.h"
#include "eval/requirement/presentdetail.h"

namespace EvaluationRequirementResult
{

class SingleModeAPresent : public Single
{
public:
    SingleModeAPresent(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            int num_updates, int num_no_ref_pos, int num_pos_outside, int num_pos_inside,
            int num_no_ref_id, int num_present_id, int num_missing_id,
            std::vector<EvaluationRequirement::PresentDetail> details);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    int numUpdates() const;
    int numNoRefPos() const;
    int numPosOutside() const;
    int numPosInside() const;
    int numNoRefId() const;
    int numPresent() const;
    int numMissing() const;

    std::vector<EvaluationRequirement::PresentDetail>& details();

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;


protected:
    int num_updates_ {0};
    int num_no_ref_pos_ {0};
    int num_pos_outside_ {0};
    int num_pos_inside_ {0};
    int num_no_ref_id_ {0}; // !ref
    int num_present_id_ {0}; // ref + tst
    int num_missing_id_ {0}; // ref + !tst

    bool has_p_present_ {false};
    float p_present_{0};

    std::vector<EvaluationRequirement::PresentDetail> details_;

    void updateProbabilities();
    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::Section& section, const std::string& table_name);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable ();
};

}

#endif // EVALUATIONREQUIREMENIDENTMODEARESULT_H
