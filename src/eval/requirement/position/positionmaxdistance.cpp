#include "eval/requirement/position/positionmaxdistance.h"
#include "eval/results/position/single.h"
#include "evaluationdata.h"
#include "logger.h"
#include "stringconv.h"

#include <ogr_spatialref.h>

using namespace std;
using namespace Utils;

namespace EvaluationRequirement
{

PositionMaxDistance::PositionMaxDistance(const std::string& name, const std::string& short_name, const std::string& group_name,
                                         EvaluationManager& eval_man,
                                         float max_distance, float maximum_probability)
    : Base(name, short_name, group_name, eval_man),
      max_distance_(max_distance), maximum_probability_(maximum_probability)
{

}

float PositionMaxDistance::maxDistance() const
{
    return max_distance_;
}


float PositionMaxDistance::maximumProbability() const
{
    return maximum_probability_;
}

std::shared_ptr<EvaluationRequirementResult::Single> PositionMaxDistance::evaluate (
        const EvaluationTargetData& target_data, std::shared_ptr<Base> instance,
        const SectorLayer& sector_layer)
{
    logdbg << "EvaluationRequirementPositionMaxDistance '" << name_ << "': evaluate: utn " << target_data.utn_
           << " max_distance " << max_distance_ << " maximum_probability " << maximum_probability_;

    const std::multimap<float, unsigned int>& tst_data = target_data.tstData();

    int num_pos {0};
    int num_no_ref {0};
    int num_pos_ok {0};
    int num_pos_nok {0};

    std::vector<EvaluationRequirement::PositionMaxDistanceDetail> details;

    float tod{0};

    OGRSpatialReference wgs84;
    wgs84.SetWellKnownGeogCS("WGS84");

    OGRSpatialReference local;

    std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart;
    //std::unique_ptr<OGRCoordinateTransformation> ogr_cart2geo;

    EvaluationTargetPosition tst_pos;

    double x_pos, y_pos;
    bool ret;
    double distance;

    for (const auto& tst_id : tst_data)
    {
        ++num_pos;

        tod = tst_id.first;
        tst_pos = target_data.tstPosForTime(tod);

        if (!target_data.hasRefDataForTime (tod, 4))
        {
            details.push_back({tod, tst_pos, false, {}, 0.0, true, num_pos, num_no_ref, num_pos_ok, num_pos_nok});

            ++num_no_ref;
            continue;
        }

        pair<EvaluationTargetPosition, bool> ret_pos = target_data.interpolatedRefPosForTime(tod, 4);

        EvaluationTargetPosition ref_pos {ret_pos.first};
        bool ok {ret_pos.second};

        if (!ok)
        {
            details.push_back({tod, tst_pos, false, {}, 0.0, true, num_pos, num_no_ref, num_pos_ok, num_pos_nok});

            ++num_no_ref;
            continue;
        }

        local.SetStereographic(ref_pos.latitude_, ref_pos.longitude_, 1.0, 0.0, 0.0);

        ogr_geo2cart.reset(OGRCreateCoordinateTransformation(&wgs84, &local));

        if (getenv("APPDIR")) // inside appimage
        {
            x_pos = tst_pos.longitude_;
            y_pos = tst_pos.latitude_;
        }
        else
        {
            x_pos = tst_pos.latitude_;
            y_pos = tst_pos.longitude_;
        }

        ret = ogr_geo2cart->Transform(1, &x_pos, &y_pos); // wgs84 to cartesian offsets
        if (!ret)
        {
            details.push_back({tod, tst_pos, false, {}, 0.0, true, num_pos, num_no_ref, num_pos_ok, num_pos_nok});
            ++num_no_ref;
            continue;
        }

        distance = sqrt(pow(x_pos,2)+pow(y_pos,2));

        if (distance > max_distance_)
        {
            details.push_back({tod, tst_pos, true, ref_pos, distance, false,
                               num_pos, num_no_ref, num_pos_ok, num_pos_nok});

            ++num_pos_nok;
        }
        else
        {
            details.push_back({tod, tst_pos, true, ref_pos, distance, true,
                               num_pos, num_no_ref, num_pos_ok, num_pos_nok});

            ++num_pos_ok;
        }

    }

    loginf << "EvaluationRequirementPositionMaxDistance '" << name_ << "': evaluate: utn " << target_data.utn_
           << " num_pos " << num_pos << " num_no_ref " <<  num_no_ref
           << " num_pos_ok " << num_pos_ok << " num_pos_nok " << num_pos_nok;

    assert (details.size() == num_pos);

    return make_shared<EvaluationRequirementResult::SinglePositionMaxDistance>(
                "UTN:"+to_string(target_data.utn_), instance, target_data.utn_, &target_data,
                eval_man_, num_pos, num_no_ref, num_pos_ok, num_pos_nok, details);
}


}
