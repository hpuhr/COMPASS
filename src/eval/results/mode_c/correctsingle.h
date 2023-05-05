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

#ifndef EVALUATIONREQUIREMENMODECCORRECTRESULT_H
#define EVALUATIONREQUIREMENMODECCORRECTRESULT_H

#include "eval/results/single.h"
#include "eval/results/evaluationdetail.h"

#include <boost/optional.hpp>

namespace EvaluationRequirementResult
{

class SingleModeCCorrect : public Single
{
public:
    SingleModeCCorrect(const std::string& result_id,
                                std::shared_ptr<EvaluationRequirement::Base> requirement,
                                const SectorLayer& sector_layer,
                                unsigned int utn, 
                                const EvaluationTargetData* target, 
                                EvaluationManager& eval_man,
                                const EvaluationDetails& details,
                                unsigned int num_updates, 
                                unsigned int num_no_ref_pos, 
                                unsigned int num_no_ref_id,
                                unsigned int num_pos_outside, 
                                unsigned int num_pos_inside,
                                unsigned int num_correct, 
                                unsigned int num_not_correct);

    //irtual void print() override;
    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

    virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) override;

    unsigned int numUpdates() const;
    unsigned int numNoRefPos() const;
    unsigned int numNoRefId() const;
    unsigned int numPosOutside() const;
    unsigned int numPosInside() const;
    unsigned int numCorrect() const;
    unsigned int numNotCorrect() const;

    virtual bool hasViewableData (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::unique_ptr<nlohmann::json::object_t> viewableData(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    virtual bool hasReference (
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;
    virtual std::string reference(
            const EvaluationResultsReport::SectionContentTable& table, const QVariant& annotation) override;

    enum DetailKey
    {
        RefExists,     //bool
        PosInside,     //bool
        IsNotCorrect,  //bool
        NumUpdates,    //unsigned int
        NumNoRef,      //unsigned int
        NumInside,     //unsigned int
        NumOutside,    //unsigned int
        NumCorrect,    //unsigned int
        NumNotCorrect //unsigned int
    };

    void addAnnotations(nlohmann::json::object_t& viewable, bool add_ok) override;

protected:
    unsigned int num_updates_     {0};
    unsigned int num_no_ref_pos_  {0};
    unsigned int num_no_ref_id_   {0};
    unsigned int num_pos_outside_ {0};
    unsigned int num_pos_inside_  {0};
    unsigned int num_correct_     {0};
    unsigned int num_not_correct_ {0};

    boost::optional<float> pcor_;

    void updatePCor();
    void addTargetToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void addTargetDetailsToTable (EvaluationResultsReport::Section& section, const std::string& table_name);
    void addTargetDetailsToReport(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
    void reportDetails(EvaluationResultsReport::Section& utn_req_section);

    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable ();
};

}

#endif // EVALUATIONREQUIREMENMODECCORRECTRESULT_H
