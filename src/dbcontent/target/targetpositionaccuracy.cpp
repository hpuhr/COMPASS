#include "targetpositionaccuracy.h"
#include "dbcontent/dbcontentcache.h"
#include "dbcontent/dbcontent.h"

using namespace std;

namespace dbContent {

// NUCp | HPL                | RCu                 |
// 9   & < 7.5 m            & < 3 m             & 1.5  \\ \hline
// 8   & < 25 m             & < 10 m            & 5  \\ \hline
// 7   & < 0.1 NM (185 m)   & < 0.05 NM (93 m)  & 46.5  \\ \hline
// 6   & < 0.2 NM (370 m)   & < 0.1 NM (185 m)  & 92.5  \\ \hline
// 5   & < 0.5 NM (926 m)   & < 0.25 NM (463 m) & 231.5  \\ \hline
// 4   & < 1 NM (1852 m)    & < 0.5 NM (926 m)  & 463  \\ \hline
// 3   & < 2 NM (3704 m)    & < 1 NM (1852 m)   & 926  \\ \hline
// 2   & < 10 NM (18520 m)  & < 5 NM (9260 m)   & 4630  \\ \hline
// 1   & < 20 NM (37040 m)  & < 10 NM (18520 m) & 9260  \\ \hline
// 0   & > 20 NM (37040 m)  & > 10 NM (18520 m) & -  \\ \hline



const map<int, float> adsb_v0_accuracies {
    {1, 9260},
    {2, 4630},
    {3, 926},
    {4, 463},
    {5, 231.5},
    {6, 92.5},
    {7, 46.5},
    {8, 5},
    {9, 1.5}
};

//    NACp | EPU (HFOM)         | VEPU (VFOM) |
//    11  & < 3 m              & < 4 m  & 1.5 \\ \hline
//    10  & < 10 m             & < 15 m & 5 \\ \hline
//    9   & < 30 m             & < 45 m & 15 \\ \hline
//    8   & < 0.05 NM (93 m)   & & 46.5 \\ \hline
//    7   & < 0.1 NM (185 m)   & & 92.5 \\ \hline
//    6   & < 0.3 NM (556 m)   & & 278 \\ \hline
//    5   & < 0.5 NM (926 m)   & & 463 \\ \hline
//    4   & < 1.0 NM (1852 m)  & & 926 \\ \hline
//    3   & < 2 NM (3704 m)    & & 1852 \\ \hline
//    2   & < 4 NM (7408 m)    & & 3704 \\ \hline
//    1   & < 10 NM (18520 km) & & 9260 \\ \hline
//    0   & > 10 NM or Unknown & & - \\ \hline|

const map<int, float> adsb_v12_accuracies {
    {1, 9260},
    {2, 3704},
    {3, 1852},
    {4, 926},
    {5, 463},
    {6, 278},
    {7, 92.5},
    {8, 46.5},
    {9, 15},
    {10, 5},
    {11, 1.5}
};

boost::optional<TargetPositionAccuracy> getPositionAccuracy(
        std::shared_ptr<dbContent::Cache> cache, const std::string& dbcontent_name, unsigned int index)
{
    if (dbcontent_name == "CAT021")
        return getADSBPositionAccuracy(cache, dbcontent_name, index);
    else if (dbcontent_name == "CAT001" || dbcontent_name == "CAT048")
        return getRadarPositionAccuracy(cache, dbcontent_name, index);
    else // cat010, cat020, cat062, reftraj
        return getXYPositionAccuracy(cache, dbcontent_name, index);
}

boost::optional<TargetPositionAccuracy> getRadarPositionAccuracy(
        std::shared_ptr<dbContent::Cache> cache, const std::string& dbcontent_name, unsigned int index)
{
    boost::optional<TargetPositionAccuracy> ret;

    // TODO, not yet implemented
    assert (false);

    return ret;
}

boost::optional<TargetPositionAccuracy> getADSBPositionAccuracy(
        std::shared_ptr<dbContent::Cache> cache, const std::string& dbcontent_name, unsigned int index)
{
    boost::optional<TargetPositionAccuracy> ret;

    NullableVector<unsigned char>& mops_version_vec =
            cache->getVar<unsigned char>(dbcontent_name, DBContent::var_cat021_mops_version_);

    NullableVector<unsigned char>& nac_p_vec =
            cache->getVar<unsigned char>(dbcontent_name, DBContent::var_cat021_nacp_);

    NullableVector<unsigned char>& nucp_nic_vec =
            cache->getVar<unsigned char>(dbcontent_name, DBContent::var_cat021_nucp_nic_);

    if (mops_version_vec.isNull(index)
            || (nac_p_vec.isNull(index) && nucp_nic_vec.isNull(index)))
        return ret; // no info

    double x_stddev, y_stddev; // xy_cov always 0

    unsigned int mops_version;
    unsigned int nuc_p;
    unsigned int nac_p;

    mops_version = mops_version_vec.get(index);

    if (mops_version == 0)
    {
        assert (!nucp_nic_vec.isNull(index));

        nuc_p = nucp_nic_vec.get(index);

        if (!adsb_v0_accuracies.count(nuc_p)) // value unknown, also for 0 (undefined)
            return ret;

        x_stddev = adsb_v0_accuracies.at(nuc_p);
        y_stddev = adsb_v0_accuracies.at(nuc_p);
    }
    else if (mops_version == 1 || mops_version == 2)
    {
        assert (!nac_p_vec.isNull(index));

        nac_p = nac_p_vec.get(index);

        if (!adsb_v12_accuracies.count(nac_p))
            return ret; // value unknown, also for 0 (undefined)

        x_stddev = adsb_v12_accuracies.at(nac_p);
        y_stddev = adsb_v12_accuracies.at(nac_p);
    }
    else
        return ret; // unknown mops version


    ret = TargetPositionAccuracy {x_stddev, y_stddev, 0};

    return ret;
}

// cat010, cat020, cat062, reftraj
boost::optional<TargetPositionAccuracy> getXYPositionAccuracy(
        std::shared_ptr<dbContent::Cache> cache, const std::string& dbcontent_name, unsigned int index)
{
    boost::optional<TargetPositionAccuracy> ret;

    double x_stddev, y_stddev, xy_cov {0};

    NullableVector<double>& pos_std_dev_x_m =
            cache->getMetaVar<double>(dbcontent_name, DBContent::meta_var_x_stddev_);

    NullableVector<double>& pos_std_dev_y_m =
            cache->getMetaVar<double>(dbcontent_name, DBContent::meta_var_y_stddev_);

    NullableVector<double>& pos_std_dev_xy_corr_coeff =
            cache->getMetaVar<double>(dbcontent_name, DBContent::meta_var_xy_cov_);

    if (pos_std_dev_x_m.isNull(index) || pos_std_dev_y_m.isNull(index))
        return ret;

    x_stddev = pos_std_dev_x_m.get(index);
    y_stddev = pos_std_dev_y_m.get(index);

    if (!pos_std_dev_xy_corr_coeff.isNull(index)) // else 0
    {
        xy_cov = pos_std_dev_xy_corr_coeff.get(index); // is covariance component

        if (xy_cov < 0)
            xy_cov = - pow(xy_cov, 2);
        else
            xy_cov = pow(xy_cov, 2);
    }

    ret = TargetPositionAccuracy {x_stddev, y_stddev, xy_cov};

    return ret;
}

} // namespace dbContent
