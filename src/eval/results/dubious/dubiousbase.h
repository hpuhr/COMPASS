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
class SingleDubiousBase : public Single
{
public:
    SingleDubiousBase(const std::string& result_type,
                      const std::string& result_id, 
                      std::shared_ptr<EvaluationRequirement::Base> requirement,
                      const SectorLayer& sector_layer,
                      unsigned int utn, 
                      const EvaluationTargetData* target, 
                      EvaluationManager& eval_man,
                      const boost::optional<EvaluationDetails>& details,
                      unsigned int num_updates,
                      unsigned int num_pos_outside, 
                      unsigned int num_pos_inside, 
                      unsigned int num_pos_inside_dubious);
    virtual ~SingleDubiousBase();

    unsigned int numUpdates() const;
    unsigned int numPosOutside() const;
    unsigned int numPosInside() const;
    unsigned int numPosInsideDubious() const;

    static const std::string DetailCommentGroupDubious;

    static const std::string DetailUTN;             //unsigned int
    static const std::string DetailTrackNum;        //unsigned int
    static const std::string DetailFirstInside;     //bool
    static const std::string DetailTODBegin;        //ptime
    static const std::string DetailTODEnd;          //ptime
    static const std::string DetailDuration;        //time_duration
    static const std::string DetailNumPosInside;    //int
    static const std::string DetailNumPosInsideDub; //int
    static const std::string DetailHasModeAC;       //bool
    static const std::string DetailHasModeS;        //bool
    static const std::string DetailLeftSector;      //bool
    static const std::string DetailIsDubious;       //bool

protected:
    std::unique_ptr<nlohmann::json::object_t> getTargetErrorsViewable ();

    static std::string dubiousReasonsString(const EvaluationDetailComments& comments);

    unsigned int num_updates_            {0};
    unsigned int num_pos_outside_        {0};
    unsigned int num_pos_inside_         {0};
    unsigned int num_pos_inside_dubious_ {0};

    boost::optional<float> p_dubious_update_;
};

/**
*/
class JoinedDubiousBase : public Joined
{
public:
    JoinedDubiousBase(const std::string& result_type,
                      const std::string& result_id, 
                      std::shared_ptr<EvaluationRequirement::Base> requirement,
                      const SectorLayer& sector_layer, 
                      EvaluationManager& eval_man);
protected:
    unsigned int num_updates_            {0};
    unsigned int num_pos_outside_        {0};
    unsigned int num_pos_inside_         {0};
    unsigned int num_pos_inside_dubious_ {0};

    float duration_all_     {0};
    float duration_nondub_  {0};
    float duration_dubious_ {0};

    boost::optional<float> p_dubious_;
    boost::optional<float> p_dubious_update_;
};

}
