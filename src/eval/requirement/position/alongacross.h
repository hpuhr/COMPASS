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
                QVariant pos_inside, QVariant distance_along, bool distance_along_ok,
                QVariant distance_across, bool distance_across_ok,
                unsigned int num_pos, unsigned int num_no_ref, unsigned int num_inside, unsigned int num_outside,
                unsigned int num_along_ok, unsigned int num_along_nok,
                unsigned int num_across_ok, unsigned int num_across_nok,
                const std::string& comment)
            : tod_(tod), tst_pos_(tst_pos), has_ref_pos_(has_ref_pos), ref_pos_(ref_pos),
              distance_along_(distance_along), distance_along_ok_(distance_along_ok),
              distance_across_(distance_across), distance_across_ok_(distance_across_ok),
              pos_inside_(pos_inside), num_pos_(num_pos), num_no_ref_(num_no_ref),
              num_inside_(num_inside), num_outside_(num_outside),
              num_along_ok_(num_along_ok), num_along_nok_(num_along_nok),
              num_across_ok_(num_across_ok), num_across_nok_(num_across_nok),
              comment_(comment)
        {
        }

        float tod_ {0};

        EvaluationTargetPosition tst_pos_;

        bool has_ref_pos_ {false};
        EvaluationTargetPosition ref_pos_;

        QVariant distance_along_ {0}; // only set if has_ref_pos_
        bool distance_along_ok_ {false};
        QVariant distance_across_ {0}; // only set if has_ref_pos_
        bool distance_across_ok_ {false};

        QVariant pos_inside_ {false};

        unsigned int num_pos_ {0};
        unsigned int num_no_ref_ {0};
        unsigned int num_inside_ {0};
        unsigned int num_outside_ {0};

        unsigned int num_along_ok_ {0};
        unsigned int num_along_nok_ {0};
        unsigned int num_across_ok_ {0};
        unsigned int num_across_nok_ {0};

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