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

#include "eval/requirement/position/alongacross.h"
#include "eval/results/position/alongacrosssingle.h"
#include "evaluationdata.h"
#include "logger.h"
#include "stringconv.h"
#include "sectorlayer.h"

#include <ogr_spatialref.h>

#include <algorithm>

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{
    bool PositionAlongAcross::in_appimage_ {getenv("APPDIR")};


    PositionAlongAcross::PositionAlongAcross(const std::string& name, const std::string& short_name, const std::string& group_name,
                                             EvaluationManager& eval_man,
                                             float max_ref_time_diff, float max_distance, float minimum_probability)
        : Base(name, short_name, group_name, eval_man),
          max_ref_time_diff_(max_ref_time_diff), max_distance_(max_distance), minimum_probability_(minimum_probability)
    {

    }

    float PositionAlongAcross::maxDistance() const
    {
        return max_distance_;
    }


    float PositionAlongAcross::minimumProbability() const
    {
        return minimum_probability_;
    }

    std::shared_ptr<EvaluationRequirementResult::Single> PositionAlongAcross::evaluate (
            const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
            const SectorLayer& sector_layer)
    {
        logdbg << "EvaluationRequirementPositionAlongAcross '" << name_ << "': evaluate: utn " << target_data.utn_
               << " max_distance " << max_distance_ << " minimum_probability " << minimum_probability_;

        const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

        unsigned int num_pos {0};
        unsigned int num_no_ref {0};
        unsigned int num_pos_outside {0};
        unsigned int num_pos_inside {0};
        unsigned int num_along_ok {0};
        unsigned int num_along_nok {0};
        unsigned int num_across_ok {0};
        unsigned int num_across_nok {0};

        std::vector<EvaluationRequirement::PositionAlongAcrossDetail> details;

        float tod{0};

        OGRSpatialReference wgs84;
        wgs84.SetWellKnownGeogCS("WGS84");

        OGRSpatialReference local;

        std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart;

        EvaluationTargetPosition tst_pos;

        double x_pos, y_pos;
        double distance, angle, d_along, d_across;

        bool is_inside;
        pair<EvaluationTargetPosition, bool> ret_pos;
        EvaluationTargetPosition ref_pos;
        pair<EvaluationTargetVelocity, bool> ret_spd;
        EvaluationTargetVelocity ref_spd;
        bool ok;

        bool along_ok, across_ok;

        unsigned int num_distances {0};
        string comment;

        tuple<vector<double>, vector<double>, vector<double>, vector<double>> distance_values;
        // dx, dy, dalong, dacross

        for (const auto& tst_id : tst_data)
        {
            ++num_pos;

            tod = tst_id.first;
            tst_pos = target_data.tstPosForTime(tod);

            along_ok = true;
            across_ok = true;

            if (!target_data.hasRefDataForTime (tod, max_ref_time_diff_))
            {
                details.push_back({tod, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, along_ok, // pos_inside, distance_along, distance_along_ok
                                   {}, across_ok, // distance_across, distance_across_ok
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_along_ok, num_along_nok, num_across_ok, num_across_nok,
                                   "No reference data"});

                ++num_no_ref;
                continue;
            }

            ret_pos = target_data.interpolatedRefPosForTime(tod, max_ref_time_diff_);

            ref_pos = ret_pos.first;
            ok = ret_pos.second;

            if (!ok)
            {
                details.push_back({tod, tst_pos,
                                   false, {}, // has_ref_pos, ref_pos
                                   {}, {}, along_ok, // pos_inside, distance_along, distance_along_ok
                                   {}, across_ok, // distance_across, distance_across_ok
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_along_ok, num_along_nok, num_across_ok, num_across_nok,
                                   "No reference position"});

                ++num_no_ref;
                continue;
            }

            ret_spd = target_data.interpolatedRefSpdForTime(tod, max_ref_time_diff_);

            ref_spd = ret_spd.first;
            assert (ret_pos.second); // must be set of ref pos exists

            is_inside = sector_layer.isInside(ref_pos);

            if (!is_inside)
            {
                details.push_back({tod, tst_pos,
                                   true, ref_pos, // has_ref_pos, ref_pos
                                   is_inside, {}, along_ok, // pos_inside, distance_along, distance_along_ok
                                   {}, across_ok, // distance_across, distance_across_ok
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_along_ok, num_along_nok, num_across_ok, num_across_nok,
                                   "Outside sector"});
                ++num_pos_outside;
                continue;
            }
            ++num_pos_inside;

            local.SetStereographic(ref_pos.latitude_, ref_pos.longitude_, 1.0, 0.0, 0.0);

            ogr_geo2cart.reset(OGRCreateCoordinateTransformation(&wgs84, &local));

            if (in_appimage_) // inside appimage
            {
                x_pos = tst_pos.longitude_;
                y_pos = tst_pos.latitude_;
            }
            else
            {
                x_pos = tst_pos.latitude_;
                y_pos = tst_pos.longitude_;
            }

            ok = ogr_geo2cart->Transform(1, &x_pos, &y_pos); // wgs84 to cartesian offsets
            if (!ok)
            {
                details.push_back({tod, tst_pos,
                                   true, ref_pos, // has_ref_pos, ref_pos
                                   is_inside, {}, along_ok, // pos_inside, distance_along, distance_along_ok
                                   {}, across_ok, // distance_across, distance_across_ok
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                                   num_along_ok, num_along_nok, num_across_ok, num_across_nok,
                                   "Position transformation error"});
                ++num_no_ref;
                continue;
            }

            distance = sqrt(pow(x_pos,2)+pow(y_pos,2));
            angle = ref_spd.track_angle_ - atan2(y_pos, x_pos);

            d_along = distance * cos(angle);
            d_across = distance * sin(angle);

            ++num_distances;

            if (d_along > max_distance_)
            {
                along_ok = false;
                ++num_along_nok;
                comment = "Along-track not OK";
            }
            else
            {
                ++num_along_ok;
                comment = "";
            }

            if (d_across > max_distance_)
            {
                across_ok = false;
                ++num_across_nok;

                if (comment.size())
                    comment += ", ";

                comment = "Across-track not OK";
            }
            else
                ++num_across_ok;

            details.push_back({tod, tst_pos,
                               true, ref_pos,
                               is_inside, d_along, along_ok, // pos_inside, distance_along, distance_along_ok
                               d_across, across_ok, // distance_across, distance_across_ok
                               num_pos, num_no_ref, num_pos_inside, num_pos_outside,
                               num_along_ok, num_along_nok, num_across_ok, num_across_nok,
                               comment});

            get<0>(distance_values).push_back(x_pos);
            get<1>(distance_values).push_back(y_pos);
            get<2>(distance_values).push_back(d_along);
            get<3>(distance_values).push_back(d_across);
        }

//        logdbg << "EvaluationRequirementPositionAlongAcross '" << name_ << "': evaluate: utn " << target_data.utn_
//               << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
//               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
//               << " num_pos_ok " << num_pos_ok << " num_pos_nok " << num_pos_nok
//               << " num_distances " << num_distances;

        assert (num_no_ref <= num_pos);
        assert (num_pos - num_no_ref == num_pos_inside + num_pos_outside);

        assert (num_distances == num_along_ok+num_along_nok);
        assert (num_distances == num_across_ok+num_across_nok);
        assert (num_distances == get<0>(distance_values).size());

        assert (details.size() == num_pos);

        return make_shared<EvaluationRequirementResult::SinglePositionAlongAcross>(
                    "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                    eval_man_, num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_along_ok, num_along_nok,
                    num_across_ok, num_across_nok, distance_values, details);
    }

    float PositionAlongAcross::maxRefTimeDiff() const
    {
        return max_ref_time_diff_;
    }


}
