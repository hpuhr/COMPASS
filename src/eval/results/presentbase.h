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

#include <memory>
#include <string>

#include <boost/optional.hpp>

class EvaluationDetailComments;

namespace EvaluationRequirement
{
    class Base;
}

namespace EvaluationRequirementResult
{

/**
*/
class SinglePresentBase : public Single
{
public:
    SinglePresentBase(const std::string& result_type,
                      const std::string& result_id, 
                      std::shared_ptr<EvaluationRequirement::Base> requirement,
                      const SectorLayer& sector_layer,
                      unsigned int utn, 
                      const EvaluationTargetData* target, 
                      EvaluationManager& eval_man,
                      const boost::optional<EvaluationDetails>& details,
                      int num_updates, 
                      int num_no_ref_pos, 
                      int num_pos_outside, 
                      int num_pos_inside,
                      int num_no_ref_val, 
                      int num_present, 
                      int num_missing);

    int numUpdates() const;
    int numNoRefPos() const;
    int numPosOutside() const;
    int numPosInside() const;
    int numNoRefValue() const;
    int numPresent() const;
    int numMissing() const;

    static const std::string DetailRefExists;    //bool
    static const std::string DetailPosInside;    //bool 
    static const std::string DetailIsNotOk;      //bool
    static const std::string DetailNumUpdates;   //int
    static const std::string DetailNumNoRef;     //int
    static const std::string DetailNumInside;    //int
    static const std::string DetailNumOutside;   //int
    static const std::string DetailNumNoRefVal;  //int
    static const std::string DetailNumPresent;   //int
    static const std::string DetailNumMissing;   //int

protected:
    int num_updates_     {0};
    int num_no_ref_pos_  {0};
    int num_pos_outside_ {0};
    int num_pos_inside_  {0};
    int num_no_ref_val_  {0}; // !ref
    int num_present_     {0}; // ref + tst
    int num_missing_     {0}; // ref + !tst

    boost::optional<float> p_present_;
};

/**
*/
class JoinedPresentBase : public Joined
{
public:
    JoinedPresentBase(const std::string& result_type,
                      const std::string& result_id, 
                      std::shared_ptr<EvaluationRequirement::Base> requirement,
                      const SectorLayer& sector_layer, 
                      EvaluationManager& eval_man);
protected:
    int num_updates_     {0};
    int num_no_ref_pos_  {0};
    int num_pos_outside_ {0};
    int num_pos_inside_  {0};
    int num_no_ref_val_  {0}; // !ref
    int num_present_     {0}; // ref + tst
    int num_missing_     {0}; // ref + !tst

    boost::optional<float> p_present_;
};

}
