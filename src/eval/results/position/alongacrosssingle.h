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

#ifndef EVALUATIONREQUIREMENPOSITIONALONGACROSSRESULT_H
#define EVALUATIONREQUIREMENPOSITIONALONGACROSSRESULT_H

#include "eval/results/single.h"
#include "eval/requirement/position/alongacross.h"

namespace EvaluationRequirementResult
{

class SinglePositionAlongAcross : public Single
{
public:
    SinglePositionAlongAcross(
            const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
            const SectorLayer& sector_layer,
            unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man,
            unsigned int num_pos, unsigned int num_no_ref,
            unsigned int num_pos_outside, unsigned int num_pos_inside,
            unsigned int num_along_ok, unsigned int num_along_nok,
            unsigned int num_across_ok, unsigned int num_across_nok,
            tuple<vector<double>, vector<double>, vector<double>, vector<double>, vector<double>> distance_values,
            std::vector<EvaluationRequirement::PositionAlongAcrossDetail> details);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    unsigned int numPos() const;
    unsigned int numNoRef() const;
    unsigned int numPosOutside() const;
    unsigned int numPosInside() const;
    unsigned int numAlongOk() const;
    unsigned int numAlongNOk() const;
    unsigned int numAcrossOk() const;
    unsigned int numAcrossNOk() const;

    const tuple<vector<double>, vector<double>, vector<double>, vector<double>, vector<double>>& distanceValues() const;

    std::vector<EvaluationRequirement::PositionAlongAcrossDetail>& details();

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;


protected:
    unsigned int num_pos_ {0};
    unsigned int num_no_ref_ {0};
    unsigned int num_pos_outside_ {0};
    unsigned int num_pos_inside_ {0};
    unsigned int num_along_ok_ {0};
    unsigned int num_along_nok_ {0};
    unsigned int num_across_ok_ {0};
    unsigned int num_across_nok_ {0};

    tuple<vector<double>, vector<double>, vector<double>, vector<double>, vector<double>> distance_values_;
    // dx, dy, dalong, dacross, along, along latency

    double along_min_ {0};
    double along_max_ {0};
    double along_avg_ {0};
    double along_var_ {0};

    double across_min_ {0};
    double across_max_ {0};
    double across_avg_ {0};
    double across_var_ {0};

    double latency_min_ {0};
    double latency_max_ {0};
    double latency_avg_ {0};
    double latency_var_ {0};

    bool has_p_min_along_ {false};
    float p_min_along_{0};

    bool has_p_min_across_ {false};
    float p_min_across_{0};

    std::vector<EvaluationRequirement::PositionAlongAcrossDetail> details_;

    void update();

    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::SectionContentTable& target_table);
    void addTargetDetailsToTableADSB (EvaluationResultsReport::SectionContentTable& target_table);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable ();
};

}

#endif // EVALUATIONREQUIREMENPOSITIONALONGACROSSRESULT_H
