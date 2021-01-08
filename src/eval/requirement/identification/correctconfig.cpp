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

#include "eval/requirement/identification/correctconfig.h"
#include "eval/requirement/identification/correctconfigwidget.h"
#include "eval/requirement/group.h"
#include "eval/requirement/base/base.h"

using namespace std;

namespace EvaluationRequirement
{

    IdentificationCorrectConfig::IdentificationCorrectConfig(
            const std::string& class_id, const std::string& instance_id,
            Group& group, EvaluationStandard& standard, EvaluationManager& eval_man)
        : BaseConfig(class_id, instance_id, group, standard, eval_man)
    {
    }

    std::shared_ptr<Base> IdentificationCorrectConfig::createRequirement()
    {
        shared_ptr<IdentificationCorrect> req = make_shared<IdentificationCorrect>(
                    name_, short_name_, group_.name(), prob_, prob_check_type_, eval_man_,
                    require_correctness_of_all_,
                    use_mode_a_, use_ms_ta_, use_ms_ti_);

        return req;
    }
    
    bool IdentificationCorrectConfig::requireCorrectnessOfAll() const
    {
        return require_correctness_of_all_;
    }

    void IdentificationCorrectConfig::requireCorrectnessOfAll(bool value)
    {
        require_correctness_of_all_ = value;
    }

    bool IdentificationCorrectConfig::useModeA() const
    {
        return use_mode_a_;
    }
    
    void IdentificationCorrectConfig::useModeA(bool value)
    {
        use_mode_a_ = value;
    }
    
    bool IdentificationCorrectConfig::useMsTa() const
    {
        return use_ms_ta_;
    }
    
    void IdentificationCorrectConfig::useMsTa(bool value)
    {
        use_ms_ta_ = value;
    }
    
    bool IdentificationCorrectConfig::useMsTi() const
    {
        return use_ms_ti_;
    }
    
    void IdentificationCorrectConfig::useMsTi(bool value)
    {
        use_ms_ti_ = value;
    }
    
    void IdentificationCorrectConfig::createWidget()
    {
        assert (!widget_);
        widget_.reset(new IdentificationCorrectConfigWidget(*this));
        assert (widget_);
    }
}
