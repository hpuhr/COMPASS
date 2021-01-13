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

#ifndef SINGLEEVALUATIONREQUIREMENTDETECTIONRESULT_H
#define SINGLEEVALUATIONREQUIREMENTDETECTIONRESULT_H

#include "eval/results/base.h"

namespace EvaluationRequirementResult
{
    using namespace std;

    class Joined;

    class Single : public Base
    {
    public:
        Single(const std::string& type, const std::string& result_id,
               std::shared_ptr<EvaluationRequirement::Base> requirement, const SectorLayer& sector_layer,
               unsigned int utn, const EvaluationTargetData* target, EvaluationManager& eval_man);

        virtual bool isSingle() const override { return true; }
        virtual bool isJoined() const override { return false; }

        virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) = 0;

        virtual std::shared_ptr<Joined> createEmptyJoined(const std::string& result_id) = 0;

        unsigned int utn() const;
        const EvaluationTargetData* target() const;

        void updateUseFromTarget ();

        const static std::string tr_details_table_name_;
        const static std::string target_table_name_;

    protected:
        unsigned int utn_; // used to generate result
        const EvaluationTargetData* target_; // used to generate result

        bool result_usable_ {true}; // whether valid data exists, changed in subclass

        std::string getTargetSectionID();
        std::string getTargetRequirementSectionID();

        virtual std::string getRequirementSectionID () override;

        void addCommonDetails (shared_ptr<EvaluationResultsReport::RootItem> root_item);
    };

}

#endif // SINGLEEVALUATIONREQUIREMENTDETECTIONRESULT_H
