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

#include "viewevaldata.h"

#include "evaluationmanager.h"

/**
*/
bool ViewEvalDataID::inResults(const EvaluationManager& eval_man) const
{
    const auto& results = eval_man.results();

    if (!results.count(eval_results_grpreq))
        return false;

    if (!results.at(eval_results_grpreq).count(eval_results_id))
        return false;

    return true;
}