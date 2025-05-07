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

#pragma once

#include "taskresult.h"

#include <memory>

class EvaluationCalculator;

/**
 */
class EvaluationTaskResult : public TaskResult
{
public:
    EvaluationTaskResult(unsigned int id, 
                         TaskManager& task_man);
    virtual ~EvaluationTaskResult();
    
    task::TaskResultType type() const override final { return task::TaskResultType::Evaluation; }

protected:
    bool loadOnDemandFigure(ResultReport::SectionContentFigure* figure) const override;
    bool loadOnDemandTable(ResultReport::SectionContentTable* table) const override;

    void toJSON_impl(nlohmann::json& root_node) const override final;
    bool fromJSON_impl(const nlohmann::json& j) override final;

private:
    mutable std::unique_ptr<EvaluationCalculator> calculator_;
};
