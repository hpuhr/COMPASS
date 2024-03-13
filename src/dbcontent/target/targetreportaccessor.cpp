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

#include "targetreportaccessor.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent.h"

#include "targetreportdefs.h"

namespace dbContent 
{

/**
*/
TargetReportAccessor::TargetReportAccessor(const std::string& dbcontent_name, 
                                           const std::shared_ptr<Buffer>& buffer,
                                           const DBContentManager& dbcont_man)
:   BufferAccessor(dbcontent_name, buffer, dbcont_man)
{
    cacheVectors();
}

/**
*/
TargetReportAccessor::TargetReportAccessor(const std::shared_ptr<DBContentVariableLookup>& lookup)
:   BufferAccessor(lookup)
{
    cacheVectors();
}

/**
*/
void TargetReportAccessor::cacheVectors()
{
    const auto& dbcontent_name = lookup_->dbContentName();

    is_radar_   = (dbcontent_name == "CAT001" || dbcontent_name == "CAT048");
    is_adsb_    = (dbcontent_name == "CAT021");
    is_tracker_ = (dbcontent_name == "CAT062");

    //general
    meta_timestamp_vec_ = metaVarVector<boost::posix_time::ptime>(DBContent::meta_var_timestamp_);
    meta_rec_num_vec_   = metaVarVector<unsigned long>(DBContent::meta_var_rec_num_);
    meta_ds_id_vec_     = metaVarVector<unsigned int>(DBContent::meta_var_ds_id_);
    meta_acad_vec_      = metaVarVector<unsigned int>(DBContent::meta_var_acad_);
    meta_acid_vec_      = metaVarVector<std::string>(DBContent::meta_var_acid_);

    //position
    meta_latitude_vec_      = metaVarVector<double>(DBContent::meta_var_latitude_);
    meta_longitude_vec_     = metaVarVector<double>(DBContent::meta_var_longitude_);
    cat062_alt_trusted_vec_ = varVector<float>(DBContent::var_cat062_fl_measured_);
    cat062_alt_sec_vec_     = varVector<float>(DBContent::var_cat062_baro_alt_);
    meta_ground_bit_vec_    = metaVarVector<bool>(DBContent::meta_var_ground_bit_);

    //position accuracy
    cat021_mops_version_vec_            = varVector<unsigned char>(DBContent::var_cat021_mops_version_);
    cat021_nac_p_vec_                   = varVector<unsigned char>(DBContent::var_cat021_nacp_);
    cat021_nucp_nic_vec_                = varVector<unsigned char>(DBContent::var_cat021_nucp_nic_);
    meta_pos_std_dev_x_m_vec_           = metaVarVector<double>(DBContent::meta_var_x_stddev_);
    meta_pos_std_dev_y_m_vec_           = metaVarVector<double>(DBContent::meta_var_y_stddev_);
    meta_pos_std_dev_xy_corr_coeff_vec_ = metaVarVector<double>(DBContent::meta_var_xy_cov_);

    //velocity / angle
    meta_speed_vec_       = metaVarVector<double>(DBContent::meta_var_ground_speed_);
    meta_track_angle_vec_ = metaVarVector<double>(DBContent::meta_var_track_angle_);

    //velocity accuracy
    cat021_nucv_nacv_vec_ = varVector<unsigned char>(DBContent::var_cat021_nucv_nacv_);
    cat062_vx_stddev_vec_ = varVector<double>(DBContent::var_cat062_vx_stddev_);
    cat062_vy_stddev_vec_ = varVector<double>(DBContent::var_cat062_vy_stddev_);

    //mode a
    meta_mode_a_vec_          = metaVarVector<unsigned int>(DBContent::meta_var_m3a_);
    meta_mode_a_garbled_vec_  = metaVarVector<bool>(DBContent::meta_var_m3a_g_);
    meta_mode_a_valid_vec_    = metaVarVector<bool>(DBContent::meta_var_m3a_v_);
    meta_mode_a_smoothed_vec_ = metaVarVector<bool>(DBContent::meta_var_m3a_smoothed_);

    //mode c
    meta_mode_c_vec_         = metaVarVector<float>(DBContent::meta_var_mc_);
    meta_mode_c_garbled_vec_ = metaVarVector<bool>(DBContent::meta_var_mc_g_);
    meta_mode_c_valid_vec_   = metaVarVector<bool>(DBContent::meta_var_mc_v_);

    //track
    meta_track_num_vec_   = metaVarVector<unsigned int>(DBContent::meta_var_track_num_);
    meta_track_begin_vec_ = metaVarVector<bool>(DBContent::meta_var_track_begin_);
    meta_track_end_vec_   = metaVarVector<bool>(DBContent::meta_var_track_end_);
}

/**
*/
boost::optional<boost::posix_time::ptime> TargetReportAccessor::timestamp(unsigned int index) const
{
    return getOptional<boost::posix_time::ptime>(meta_timestamp_vec_, index);
}

/**
*/
boost::optional<unsigned long> TargetReportAccessor::recordNumber(unsigned int index) const
{
    return getOptional<unsigned long>(meta_rec_num_vec_, index);
}

/**
*/
boost::optional<unsigned int> TargetReportAccessor::dsID(unsigned int index) const
{
    return getOptional<unsigned int>(meta_ds_id_vec_, index);
}

/**
*/
boost::optional<unsigned char> TargetReportAccessor::mopsVersion(unsigned int index) const
{
    return getOptional<unsigned char>(cat021_mops_version_vec_, index);
}

/**
*/
boost::optional<unsigned int> TargetReportAccessor::acad(unsigned int index) const
{
    return getOptional<unsigned int>(meta_acad_vec_, index);
}

/**
*/
boost::optional<std::string> TargetReportAccessor::acid(unsigned int index) const
{
    return getOptional<std::string>(meta_acid_vec_, index);
}

/**
*/
boost::optional<targetReport::Position> TargetReportAccessor::position(unsigned int index) const
{
    if (!meta_latitude_vec_ || 
        !meta_longitude_vec_ ||
         meta_latitude_vec_->isNull(index) ||
         meta_longitude_vec_->isNull(index))
        return {};

    return targetReport::Position(meta_latitude_vec_->get(index),
                                         meta_longitude_vec_->get(index));
}

/**
*/
boost::optional<targetReport::PositionAccuracy> TargetReportAccessor::positionAccuracy(unsigned int index) const
{
    if (is_adsb_)
    {
        if (!cat021_mops_version_vec_ || cat021_mops_version_vec_->isNull(index))
            return {};

        auto mops_version = cat021_mops_version_vec_->get(index);

        double x_stddev, y_stddev;

        if (mops_version == 0)
        {
            if (!cat021_nucp_nic_vec_ || cat021_nucp_nic_vec_->isNull(index))
                return {};

            auto nuc_p = cat021_nucp_nic_vec_->get(index);

            if (!targetReport::AccuracyTables::adsb_v0_accuracies.count(nuc_p)) // value unknown, also for 0 (undefined)
                return {};

            x_stddev = targetReport::AccuracyTables::adsb_v0_accuracies.at(nuc_p);
            y_stddev = x_stddev;
        }
        else if (mops_version == 1 || mops_version == 2)
        {
            if (!cat021_nac_p_vec_ || cat021_nac_p_vec_->isNull(index))
                return {};

            auto nac_p = cat021_nac_p_vec_->get(index);

            if (!targetReport::AccuracyTables::adsb_v12_accuracies.count(nac_p))
                return {}; // value unknown, also for 0 (undefined)

            x_stddev = targetReport::AccuracyTables::adsb_v12_accuracies.at(nac_p);
            y_stddev = x_stddev;
        }
        else
        {
            return {}; // unknown mops version
        }

        return targetReport::PositionAccuracy(x_stddev,
                                                y_stddev,
                                                0.0);
    }
    else if (is_radar_)
    {
        //tbi
    }
    else // cat010, cat020, cat062, reftraj
    {
        if (!meta_pos_std_dev_x_m_vec_ || 
            !meta_pos_std_dev_y_m_vec_ ||
             meta_pos_std_dev_x_m_vec_->isNull(index) ||
             meta_pos_std_dev_y_m_vec_->isNull(index))
            return {};

        double xy_cov = 0.0;
        if (meta_pos_std_dev_xy_corr_coeff_vec_ && !meta_pos_std_dev_xy_corr_coeff_vec_->isNull(index))
        {
            xy_cov = meta_pos_std_dev_xy_corr_coeff_vec_->get(index);

            if (xy_cov < 0)
                xy_cov = -std::pow(xy_cov, 2);
            else
                xy_cov =  std::pow(xy_cov, 2);
        }

        return targetReport::PositionAccuracy(meta_pos_std_dev_x_m_vec_->get(index),
                                                meta_pos_std_dev_y_m_vec_->get(index),
                                                xy_cov);
    }

    //not implemented for dbcontent
    return {};
}

/**
*/
boost::optional<targetReport::BarometricAltitude> TargetReportAccessor::barometricAltitude(unsigned int index) const
{
    if (is_tracker_ && cat062_alt_trusted_vec_ && !cat062_alt_trusted_vec_->isNull(index))
    {
        return targetReport::BarometricAltitude(targetReport::BarometricAltitude::Source::Barometric_CAT062_Trusted,
                                             cat062_alt_trusted_vec_->get(index),
                                             meta_mode_c_valid_vec_ && !meta_mode_c_valid_vec_->isNull(index) ? meta_mode_c_valid_vec_->get(index) : boost::optional<bool>(),
                                             meta_mode_c_garbled_vec_ && !meta_mode_c_garbled_vec_->isNull(index) ? meta_mode_c_garbled_vec_->get(index) : boost::optional<bool>());
    }
    else if (meta_mode_c_vec_ && !meta_mode_c_vec_->isNull(index))
    {
        return targetReport::BarometricAltitude(targetReport::BarometricAltitude::Source::Barometric_ModeC,
                                             meta_mode_c_vec_->get(index),
                                             meta_mode_c_valid_vec_ && !meta_mode_c_valid_vec_->isNull(index) ? meta_mode_c_valid_vec_->get(index) : boost::optional<bool>(),
                                             meta_mode_c_garbled_vec_ && !meta_mode_c_garbled_vec_->isNull(index) ? meta_mode_c_garbled_vec_->get(index) : boost::optional<bool>());
    }
    else if (is_tracker_ && cat062_alt_sec_vec_ && !cat062_alt_sec_vec_->isNull(index))
    {
        return targetReport::BarometricAltitude(targetReport::BarometricAltitude::Source::Barometric_CAT062_Secondary,
                                             cat062_alt_sec_vec_->get(index),
                                             meta_mode_c_valid_vec_ && !meta_mode_c_valid_vec_->isNull(index) ? meta_mode_c_valid_vec_->get(index) : boost::optional<bool>(),
                                             meta_mode_c_garbled_vec_ && !meta_mode_c_garbled_vec_->isNull(index) ? meta_mode_c_garbled_vec_->get(index) : boost::optional<bool>());
    }

    //not implemented for dbcontent
    return {};
}

/**
*/
boost::optional<targetReport::Velocity> TargetReportAccessor::velocity(unsigned int index) const
{
    if (!meta_speed_vec_ || 
        !meta_track_angle_vec_ ||
         meta_speed_vec_->isNull(index) || 
         meta_track_angle_vec_->isNull(index))
        return {};

    return targetReport::Velocity(meta_track_angle_vec_->get(index), meta_speed_vec_->get(index));
}

/**
*/
boost::optional<targetReport::VelocityAccuracy> TargetReportAccessor::velocityAccuracy(unsigned int index) const
{
    if (is_adsb_)
    {
        if (!cat021_nucv_nacv_vec_ || cat021_nucv_nacv_vec_->isNull(index))
            return {};

        auto nuc_r = cat021_nucv_nacv_vec_->get(index);

        if (!targetReport::AccuracyTables::adsb_nucr_nacv_accuracies.count(nuc_r))
            return {}; // no info

        double vx_stddev = targetReport::AccuracyTables::adsb_nucr_nacv_accuracies.at(nuc_r);
        double vy_stddev = vx_stddev;

        return targetReport::VelocityAccuracy(vx_stddev, vy_stddev);
    }
    else if (is_tracker_)
    {
        if (!cat062_vx_stddev_vec_ ||
            !cat062_vy_stddev_vec_ ||
             cat062_vx_stddev_vec_->isNull(index) ||
             cat062_vy_stddev_vec_->isNull(index))
            return {};

        return targetReport::VelocityAccuracy(cat062_vx_stddev_vec_->get(index),
                                                cat062_vy_stddev_vec_->get(index));
    }

    //not implemented for dbcontent
    return {};
}

/**
*/
boost::optional<double> TargetReportAccessor::trackAngle(unsigned int index) const
{
    return getOptional<double>(meta_track_angle_vec_, index);
}

/**
*/
boost::optional<bool> TargetReportAccessor::groundBit(unsigned int index) const
{
    return getOptional<bool>(meta_ground_bit_vec_, index);
}

/**
*/
boost::optional<unsigned int> TargetReportAccessor::modeA(unsigned int index) const
{
    return getOptional<unsigned int>(meta_mode_a_vec_, index);
}

/**
*/
boost::optional<bool> TargetReportAccessor::modeAValid(unsigned int index) const
{
    return getOptional<bool>(meta_mode_a_valid_vec_, index);
}

/**
*/
boost::optional<bool> TargetReportAccessor::modeAGarbled(unsigned int index) const
{
    return getOptional<bool>(meta_mode_a_garbled_vec_, index);
}

/**
*/
boost::optional<bool> TargetReportAccessor::modeASmoothed(unsigned int index) const
{
    return getOptional<bool>(meta_mode_a_smoothed_vec_, index);
}

/**
*/
boost::optional<float> TargetReportAccessor::modeC(unsigned int index) const
{
    return getOptional<float>(meta_mode_c_vec_, index);
}

/**
*/
boost::optional<bool> TargetReportAccessor::modeCValid(unsigned int index) const
{
    return getOptional<bool>(meta_mode_c_valid_vec_, index);
}

/**
*/
boost::optional<bool> TargetReportAccessor::modeCGarbled(unsigned int index) const
{
    return getOptional<bool>(meta_mode_c_garbled_vec_, index);
}

/**
*/
boost::optional<unsigned int> TargetReportAccessor::trackNumber(unsigned int index) const
{
    return getOptional<unsigned int>(meta_track_num_vec_, index);
}

/**
*/
boost::optional<bool> TargetReportAccessor::trackStart(unsigned int index) const
{
    return getOptional<bool>(meta_track_begin_vec_, index);
}

/**
*/
boost::optional<bool> TargetReportAccessor::trackEnd(unsigned int index) const
{
    return getOptional<bool>(meta_track_end_vec_, index);
}

} // namespace dbContent
