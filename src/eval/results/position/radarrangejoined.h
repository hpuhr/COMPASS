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

#ifndef EVALUATIONREQUIREMENPOSITIONJOINEDPOSITIONRADARRANGE_H
#define EVALUATIONREQUIREMENPOSITIONJOINEDPOSITIONRADARRANGE_H

#include "eval/results/position/positionbase.h"

namespace EvaluationRequirementResult
{
using namespace std;

class SinglePositionRadarRange;

class JoinedPositionRadarRange : public JoinedPositionBase
{
public:
    JoinedPositionRadarRange(const std::string& result_id,
                           std::shared_ptr<EvaluationRequirement::Base> requirement,
                           const SectorLayer& sector_layer,
                           EvaluationManager& eval_man);

    //virtual void print() override;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    void exportAsCSV();

protected:

    QVariant range_gain_;
    QVariant range_bias_;

    //void addToValues (std::shared_ptr<SinglePositionRadarRange> single_result);
    void update() override;

    void addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

    std::unique_ptr<nlohmann::json::object_t> getErrorsViewable ();

//    virtual void join_impl(std::shared_ptr<Single> other) override;
//    virtual void updatesToUseChanges_impl() override;

    vector<double> refRangeValues() const;
    vector<double> tstRangeValues() const;
};

}

#endif // EVALUATIONREQUIREMENPOSITIONJOINEDPOSITIONRADARRANGE_H
