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

#ifndef EVALUATIONREQUIREMENIDENTMODECRESULT_H
#define EVALUATIONREQUIREMENIDENTMODECRESULT_H

#include "eval/results/single.h"
#include "eval/requirement/mode_c/modec.h"
#include "eval/requirement/checkdetail.h"

namespace EvaluationRequirementResult
{

class SingleModeC : public Single
{
public:
    SingleModeC(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            int num_updates, int num_no_ref_pos, int num_no_ref, int num_pos_outside, int num_pos_inside,
            int num_unknown, int num_correct, int num_false,
            std::vector<EvaluationRequirement::CheckDetail> details);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    int numUpdates() const;
    int numNoRefPos() const;
    int numNoRefValue() const;
    int numPosOutside() const;
    int numPosInside() const;
    int numUnknown() const;
    int numCorrect() const;
    int numFalse() const;

    std::vector<EvaluationRequirement::CheckDetail>& details();

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
    int num_no_ref_val_ {0};
    int num_pos_outside_ {0};
    int num_pos_inside_ {0};
    int num_unknown_ {0};
    int num_correct_ {0};
    int num_false_ {0};

    // min exist
    bool use_p_present_req_ {false};
    float p_present_min_{0};

    bool has_p_present_ {false};
    float p_present_{0};

    // max false
    bool use_p_false_req_ {false};
    float p_false_max_{0};

    bool has_p_false_ {false};
    float p_false_{0};

    std::vector<EvaluationRequirement::CheckDetail> details_;

    void updateProbabilities();
    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::SectionContentTable& target_table);
    //void addTargetDetailsToTableADSB (EvaluationResultsReport::SectionContentTable& target_table);

    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable ();
};

}

#endif // EVALUATIONREQUIREMENIDENTMODECRESULT_H
