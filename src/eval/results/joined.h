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

#ifndef JOINEDEVALUATIONREQUIREMENTRESULT_H
#define JOINEDEVALUATIONREQUIREMENTRESULT_H

#include "eval/results/base.h"

namespace EvaluationRequirementResult
{

class Single;

class Joined : public Base
{
public:
    Joined(const std::string& type, 
           const std::string& result_id,
           std::shared_ptr<EvaluationRequirement::Base> requirement,
           const SectorLayer& sector_layer,
           EvaluationManager& eval_man);
    virtual ~Joined();

    virtual BaseType baseType() const override { return BaseType::Joined; }

    void join(std::shared_ptr<Single> other);

    virtual void addToReport (std::shared_ptr<EvaluationResultsReport::RootItem> root_item) = 0;

    std::vector<std::shared_ptr<Single>>& results();

    void updatesToUseChanges();

    unsigned int numResults();
    unsigned int numUsableResults();
    unsigned int numUnusableResults();

protected:
    void addCommonDetails (EvaluationResultsReport::SectionContentTable& sector_details_table);

    virtual void join_impl(std::shared_ptr<Single> other) = 0;
    virtual void updatesToUseChanges_impl() = 0;

    std::vector<std::shared_ptr<Single>> results_;
};

}
#endif // JOINEDEVALUATIONREQUIREMENTRESULT_H
