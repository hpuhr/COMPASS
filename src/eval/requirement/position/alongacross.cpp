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

        int num_pos {0};
        int num_no_ref {0};
        int num_pos_outside {0};
        int num_pos_inside {0};
        int num_pos_ok {0};
        int num_pos_nok {0};

        std::vector<EvaluationRequirement::PositionAlongAcrossDetail> details;

        float tod{0};

        OGRSpatialReference wgs84;
        wgs84.SetWellKnownGeogCS("WGS84");

        OGRSpatialReference local;

        std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart;
        //std::unique_ptr<OGRCoordinateTransformation> ogr_cart2geo;

        EvaluationTargetPosition tst_pos;

        double x_pos, y_pos;
        double distance;

        bool is_inside;
        pair<EvaluationTargetPosition, bool> ret_pos;
        EvaluationTargetPosition ref_pos;
        bool ok;

        unsigned int num_distances {0};
        double error_min{0}, error_max{0}, error_sum{0};

        for (const auto& tst_id : tst_data)
        {
            ++num_pos;

            tod = tst_id.first;
            tst_pos = target_data.tstPosForTime(tod);

            if (!target_data.hasRefDataForTime (tod, max_ref_time_diff_))
            {
                details.push_back({tod, tst_pos,
                                   false, {},
                                   {}, {}, true, // pos_inside, distance, pos_ok
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside, num_pos_ok, num_pos_nok,
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
                                   false, {},
                                   {}, {}, true, // pos_inside, distance, pos_ok
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside, num_pos_ok, num_pos_nok,
                                   "No reference position"});

                ++num_no_ref;
                continue;
            }

            is_inside = sector_layer.isInside(ref_pos);

            if (!is_inside)
            {
                details.push_back({tod, tst_pos,
                                   true, ref_pos,
                                   is_inside, {}, true, // pos_inside, distance, pos_ok
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside, num_pos_ok, num_pos_nok,
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
                                   false, {},
                                   is_inside, {}, true, // pos_inside, distance, pos_ok
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside, num_pos_ok, num_pos_nok,
                                   "Position transformation error"});
                ++num_no_ref;
                continue;
            }

            distance = sqrt(pow(x_pos,2)+pow(y_pos,2));

            if (!num_distances)
            {
                error_min = distance;
                error_max = distance;
                error_sum = distance;
            }
            else
            {
                error_min = min(error_min, distance);
                error_max = max(error_max, distance);
                error_sum += distance;
            }
            ++num_distances;

            if (distance > max_distance_)
            {
                details.push_back({tod, tst_pos,
                                   true, ref_pos,
                                   is_inside, distance, false, // pos_inside, distance, pos_ok
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside, num_pos_ok, num_pos_nok,
                                   "Position not OK"});

                ++num_pos_nok;
            }
            else
            {
                details.push_back({tod, tst_pos,
                                   true, ref_pos,
                                   is_inside, distance, true, //pos_inside, distance, pos_ok
                                   num_pos, num_no_ref, num_pos_inside, num_pos_outside, num_pos_ok, num_pos_nok,
                                   "Position OK"});

                ++num_pos_ok;
            }

        }

        logdbg << "EvaluationRequirementPositionAlongAcross '" << name_ << "': evaluate: utn " << target_data.utn_
               << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
               << " num_pos_outside " << num_pos_outside << " num_pos_inside " << num_pos_inside
               << " num_pos_ok " << num_pos_ok << " num_pos_nok " << num_pos_nok
               << " num_distances " << num_distances;

        assert (num_distances == num_pos_nok+num_pos_ok);
        assert (num_no_ref <= num_pos);
        assert (num_pos - num_no_ref == num_pos_inside + num_pos_outside);
        assert (num_pos_inside == num_pos_ok + num_pos_nok);

        assert (details.size() == num_pos);

        float error_avg {0};
        if (num_distances)
            error_avg = error_sum/(double)num_distances;

        return make_shared<EvaluationRequirementResult::SinglePositionAlongAcross>(
                    "UTN:"+to_string(target_data.utn_), instance, sector_layer, target_data.utn_, &target_data,
                    eval_man_, num_pos, num_no_ref, num_pos_outside, num_pos_inside, num_pos_ok, num_pos_nok,
                    error_min, error_max, error_avg, details);
    }

    float PositionAlongAcross::maxRefTimeDiff() const
    {
        return max_ref_time_diff_;
    }


}
