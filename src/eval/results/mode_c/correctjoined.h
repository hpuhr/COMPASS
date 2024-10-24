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

#ifndef JOINEVALUATIONREQUIREMENMODECCORRECTRESULT_H
#define JOINEVALUATIONREQUIREMENMODECCORRECTRESULT_H

#include "eval/results/joined.h"

#include <boost/optional.hpp>

namespace EvaluationRequirementResult
{

class SingleModeCCorrect;

class JoinedModeCCorrect : public Joined
{
public:
    JoinedModeCCorrect(const std::string& result_id,
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

protected:
    void addToValues (std::shared_ptr<SingleModeCCorrect> single_result);
    void updatePCor();
    void addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

    std::unique_ptr<nlohmann::json::object_t> getErrorsViewable ();

    virtual void join_impl(std::shared_ptr<Single> other) override;
    virtual void updatesToUseChanges_impl() override;

    unsigned int num_updates_     {0};
    unsigned int num_no_ref_pos_  {0};
    unsigned int num_no_ref_id_   {0};
    unsigned int num_pos_outside_ {0};
    unsigned int num_pos_inside_  {0};
    unsigned int num_correct_     {0};
    unsigned int num_not_correct_ {0};

    boost::optional<float> pcor_;
};

}

#endif // JOINEVALUATIONREQUIREMENMODECCORRECTRESULT_H
