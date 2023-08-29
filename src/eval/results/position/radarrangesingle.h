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

#ifndef EVALUATIONREQUIREMENPOSITIONSINGLEPOSITIONRADARRANGE_H
#define EVALUATIONREQUIREMENPOSITIONSINGLEPOSITIONRADARRANGE_H

#include "eval/results/position/positionbase.h"

namespace EvaluationRequirement
{
    class PositionRadarRange;
}

namespace EvaluationRequirementResult
{

class SinglePositionRadarRange : public SinglePositionBase
{
public:
    SinglePositionRadarRange(const std::string& result_id,
                             std::shared_ptr<EvaluationRequirement::Base> requirement,
                             const SectorLayer& sector_layer,
                             unsigned int utn,
                             const EvaluationTargetData* target,
                             EvaluationManager& eval_man,
                             const EvaluationDetails& details,
                             unsigned int num_pos,
                             unsigned int num_no_ref,
                             unsigned int num_pos_outside,
                             unsigned int num_pos_inside,
                             unsigned int num_comp_passed,
                             unsigned int num_comp_failed,
                             vector<double> values,
                             vector<double> ref_range_values, vector<double> tst_range_values);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    EvaluationRequirement::PositionRadarRange* req ();

    void addAnnotations(nlohmann::json::object_t& viewable, bool overview, bool add_ok) override;

    const vector<double>& refRangeValues() const;
    const vector<double>& tstRangeValues() const;

protected:

    vector<double> ref_range_values_;
    vector<double> tst_range_values_;

    QVariant range_bias_;
    QVariant range_gain_;

    void update();

    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::Section& section, const std::string& table_name);
    void addTargetDetailsToTableADSB (EvaluationResultsReport::Section& section, const std::string& table_name);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable (bool add_highlight=false);

};

}

#endif // EVALUATIONREQUIREMENPOSITIONSINGLEPOSITIONRADARRANGE_H