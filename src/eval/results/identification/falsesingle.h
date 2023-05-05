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

#ifndef EVALUATIONREQUIREMENTIDENTIFICATIONFALSERESULT_H
#define EVALUATIONREQUIREMENTIDENTIFICATIONFALSERESULT_H

#include "eval/results/falsebase.h"
#include "eval/results/evaluationdetail.h"

#include <boost/optional.hpp>

namespace EvaluationRequirementResult
{

class SingleIdentificationFalse : public SingleFalseBase
{
public:
    SingleIdentificationFalse(const std::string& result_id, 
                              std::shared_ptr<EvaluationRequirement::Base> requirement,
                              const SectorLayer& sector_layer,
                              unsigned int utn, 
                              const EvaluationTargetData* target, 
                              EvaluationManager& eval_man,
                              const EvaluationDetails& details,
                              int num_updates, 
                              int num_no_ref_pos, 
                              int num_no_ref, 
                              int num_pos_outside, 
                              int num_pos_inside,
                              int num_unknown, 
                              int num_correct, 
                              int num_false);

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

    void addAnnotations(nlohmann::json::object_t& viewable, bool add_ok) override;

protected:
    void updateProbabilities();
    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::Section& section, const std::string& table_name);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable ();
};

}

#endif // EVALUATIONREQUIREMENTIDENTIFICATIONFALSERESULT_H
