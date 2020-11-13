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

#ifndef JOINEVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H
#define JOINEVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H

#include "eval/results/joined.h"

namespace EvaluationRequirementResult
{
    class SinglePositionMaxDistance;

    class JoinedPositionMaxDistance : public Joined
    {
    public:
        JoinedPositionMaxDistance(
                const std::string& result_id, std::shared_ptr<EvaluationRequirement::Base> requirement,
                const SectorLayer& sector_layer, EvaluationManager& eval_man);

        virtual void join(std::shared_ptr<Base> other) override;

        //virtual void print() override;
        virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) override;

        virtual void updatesToUseChanges() override;

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

        bool first_ {true};
        double error_min_ {0};
        double error_max_ {0};
        double error_avg_ {0};

        bool has_p_min_pos_ {false};
        float p_min_pos_{0};

        void addToValues (std::shared_ptr<SinglePositionMaxDistance> single_result);
        void updatePMinPos();
        void addToOverviewTable(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);
        void addDetails(std::shared_ptr<EvaluationResultsReport::RootItem> root_item);

        std::unique_ptr<nlohmann::json::object_t> getErrorsViewable ();
    };

}

#endif // JOINEVALUATIONREQUIREMENPOSITIONMAXDISTANCERESULT_H
