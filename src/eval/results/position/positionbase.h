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

#include "eval/results/single.h"
#include "eval/results/joined.h"

#include <boost/optional.hpp>

namespace EvaluationRequirementResult
{

/**
*/
class SinglePositionBase : public Single
{
public:
    SinglePositionBase(const std::string& result_type,
                       const std::string& result_id,
                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                       const SectorLayer& sector_layer,
                       unsigned int utn,
                       const EvaluationTargetData* target,
                       EvaluationManager& eval_man,
                       const EvaluationDetails& details,
                       unsigned int num_pos,
                       unsigned int num_no_ref,
                       unsigned int num_pos_outside,
                       unsigned int num_pos_inside,
                       unsigned int num_passed,
                       unsigned int num_failed,
                       vector<double> values);

    unsigned int numPos() const;
    unsigned int numNoRef() const;
    unsigned int numPosOutside() const;
    unsigned int numPosInside() const;
    unsigned int numPassed() const;
    unsigned int numFailed() const;

    const vector<double>& values() const;

    enum DetailKey
    {
        Value,          //float
        CheckPassed,     //bool
        PosInside,       //bool
        NumPos,          //unsigned int
        NumNoRef,        //unsigned int
        NumInside,       //unsigned int
        NumOutside,      //unsigned int
        NumCheckPassed,  //unsigned int
        NumCheckFailed  //unsigned int
    };

protected:
    unsigned int num_pos_         {0};
    unsigned int num_no_ref_      {0};
    unsigned int num_pos_outside_ {0};
    unsigned int num_pos_inside_  {0};
    unsigned int num_passed_      {0};
    unsigned int num_failed_      {0};

    vector<double> values_;

    double value_min_ {0};
    double value_max_ {0};
    double value_avg_ {0};
    double value_var_ {0};
    double value_rms_ {0};

    boost::optional<float> prob_;
};

/**
*/
class JoinedPositionBase : public Joined
{
public:
    JoinedPositionBase(const std::string& result_type,
                       const std::string& result_id,
                       std::shared_ptr<EvaluationRequirement::Base> requirement,
                       const SectorLayer& sector_layer,
                       EvaluationManager& eval_man);
protected:
    unsigned int num_pos_         {0};
    unsigned int num_no_ref_      {0};
    unsigned int num_pos_outside_ {0};
    unsigned int num_pos_inside_  {0};
    unsigned int num_passed_      {0};
    unsigned int num_failed_      {0};

    //vector<double> values_;

    double value_min_ {0};
    double value_max_ {0};
    double value_avg_ {0};
    double value_var_ {0};
    double value_rms_ {0};

    boost::optional<float> prob_;

    void addToValues (std::shared_ptr<SinglePositionBase> single_result, bool do_update=true);
    virtual void update() = 0;

    virtual void join_impl(std::shared_ptr<Single> other) override;
    virtual void updatesToUseChanges_impl() override;

    vector<double> values() const;
};

}
