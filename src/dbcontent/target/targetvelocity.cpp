#include "targetvelocity.h"
#include "dbcontent/dbcontentaccessor.h"
#include "dbcontent/dbcontent.h"

using namespace std;

namespace dbContent {

// NUCr
//0 	N/A 	N/A
//1 	<10 m/s 	<15.2 m/s (50 fps)
//2 	<3 m/s 	<4.5 m/s (15 fps)
//3 	<1 m/s 	<1.5 m/s (5 fps)
//4 	<0.3 m/s 	<0.46 m/s (1.5 fps)

// NACv
//0 	N/A 	N/A
//1 	<10 m/s 	<15.2 m/s (50 fps)
//2 	<3 m/s 	<4.5 m/s (15 fps)
//3 	<1 m/s 	<1.5 m/s (5 fps)
//4 	<0.3 m/s 	<0.46 m/s (1.5 fps)

const map<int, float> adsb_nucr_nacv_accuracies {
    {1, 5},
    {2, 1.5},
    {3, 0.5},
    {4, 0.15}
};

boost::optional<TargetVelocityAccuracy> getVelocityAccuracy(
        std::shared_ptr<dbContent::DBContentAccessor> accessor, const std::string& dbcontent_name, unsigned int index)
{
    if (dbcontent_name == "CAT021")
        return getVelocityAccuracyADSB(accessor, dbcontent_name, index);
    else if (dbcontent_name == "CAT062")
        return getVelocityAccuracyTracker(accessor, dbcontent_name, index);
    else
        assert (false); // not implemented yet

}


boost::optional<TargetVelocityAccuracy> getVelocityAccuracyADSB(
        std::shared_ptr<dbContent::DBContentAccessor> accessor, const std::string& dbcontent_name, unsigned int index)
{
    NullableVector<unsigned char>& nucv_nacv_vec =
            accessor->getVar<unsigned char>(dbcontent_name, DBContent::var_cat021_nucv_nacv_);

    if (nucv_nacv_vec.isNull(index))
        return {}; // no info

    if (!adsb_nucr_nacv_accuracies.count(nucv_nacv_vec.get(index)))
        return {}; // no info

    TargetVelocityAccuracy acc (adsb_nucr_nacv_accuracies.at(nucv_nacv_vec.get(index)),
                                adsb_nucr_nacv_accuracies.at(nucv_nacv_vec.get(index)));

//    TargetVelocityAccuracy acc (30.0,
//                                30.0);

    return acc;
}

boost::optional<TargetVelocityAccuracy> getVelocityAccuracyTracker(
        std::shared_ptr<dbContent::DBContentAccessor> accessor, const std::string& dbcontent_name, unsigned int index)
{

    NullableVector<double>& vx_stddev_vec =
            accessor->getVar<double>(dbcontent_name, DBContent::var_cat062_vx_stddev_);

    NullableVector<double>& vy_stddev_vec =
            accessor->getVar<double>(dbcontent_name, DBContent::var_cat062_vy_stddev_);

    if (vx_stddev_vec.isNull(index) || vy_stddev_vec.isNull(index))
        return {}; // no info


    TargetVelocityAccuracy acc (vx_stddev_vec.get(index), vy_stddev_vec.get(index));

//    TargetVelocityAccuracy acc (1.0,
//                                1.0);

    return acc;
}

}
