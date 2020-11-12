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

#ifndef EVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H
#define EVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H

#include "eval/results/single.h"
#include "eval/requirement/position/positionmaxdistance.h"

namespace EvaluationRequirementResult
{

class SinglePositionMaxDistance : public Single
{
public:
    SinglePositionMaxDistance(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            int num_pos, int num_no_ref, int num_pos_outside, int num_pos_inside, int num_pos_ok, int num_pos_nok,
            double error_min, double error_max, double error_avg,
            std::vector<EvaluationRequirement::PositionMaxDistanceDetail> details);

    virtual void print() override;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    int numPos() const;
    int numNoRef() const;
    int numPosOutside() const;
    int numPosInside() const;
    int numPosOk() const;
    int numPosNOk() const;

    double errorMin() const;
    double errorMax() const;
    double errorAvg() const;

    std::vector<EvaluationRequirement::PositionMaxDistanceDetail>& details();

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;


protected:
    int num_pos_ {0};
    int num_no_ref_ {0};
    int num_pos_outside_ {0};
    int num_pos_inside_ {0};
    int num_pos_ok_ {0};
    int num_pos_nok_ {0};

    double error_min_ {0};
    double error_max_ {0};
    double error_avg_ {0};

    bool has_p_min_pos_ {false};
    float p_min_pos_{0};

    std::vector<EvaluationRequirement::PositionMaxDistanceDetail> details_;

    void updatePMinPos();
    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::SectionContentTable& target_table);
    void addTargetDetailsToTableADSB (EvaluationResultsReport::SectionContentTable& target_table);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable ();
};

}

#endif // EVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H
