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

#ifndef EVALUATIONREQUIREMENPOSITIONALONGACROSS_H
#define EVALUATIONREQUIREMENPOSITIONALONGACROSS_H

#include "eval/requirement/base.h"
#include "evaluationtargetposition.h"

#include <QVariant>

namespace EvaluationRequirement
{

    class PositionAlongAcrossDetail
    {
    public:
        PositionAlongAcrossDetail(
                float tod, EvaluationTargetPosition tst_pos,
                bool has_ref_pos, EvaluationTargetPosition ref_pos,
                QVariant pos_inside, QVariant distance, bool pos_ok,
                int num_pos, int num_no_ref, int num_inside, int num_outside, int num_pos_ok, int num_pos_nok,
                const std::string& comment)
            : tod_(tod), tst_pos_(tst_pos), has_ref_pos_(has_ref_pos), ref_pos_(ref_pos),
              distance_(distance), pos_ok_(pos_ok), pos_inside_(pos_inside),
              num_pos_(num_pos), num_no_ref_(num_no_ref),
              num_inside_(num_inside), num_outside_(num_outside), num_pos_ok_(num_pos_ok), num_pos_nok_(num_pos_nok),
              comment_(comment)
        {
        }

        float tod_ {0};

        EvaluationTargetPosition tst_pos_;

        bool has_ref_pos_ {false};
        EvaluationTargetPosition ref_pos_;

        QVariant distance_ {0}; // only set if has_ref_pos_
        bool pos_ok_ {false};

        QVariant pos_inside_ {false};

        int num_pos_ {0};
        int num_no_ref_ {0};
        int num_inside_ {0};
        int num_outside_ {0};
        int num_pos_ok_ {0};
        int num_pos_nok_ {0};

        std::string comment_;
    };


    class PositionAlongAcross : public Base
    {
    public:
        PositionAlongAcross(
                const std::string& name, const std::string& short_name, const std::string& group_name,
                EvaluationManager& eval_man,
                float max_time_diff, float max_distance, float minimum_probability);

        float maxRefTimeDiff() const;
        float maxDistance() const;
        float minimumProbability() const;

        virtual std::shared_ptr<EvaluationRequirementResult::Single> evaluate (
                const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
                const SectorLayer& sector_layer) override;


    protected:
        static bool in_appimage_;

        float max_ref_time_diff_ {0};
        float max_distance_ {0};
        float minimum_probability_{0};
    };

}

#endif // EVALUATIONREQUIREMENPOSITIONALONGACROSS_H
