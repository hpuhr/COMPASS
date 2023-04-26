#include "calculatereferencestarget.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "buffer.h"
#include "timeconv.h"
#include "logger.h"

#include "reconstruction/reconstructor_umkalman2d.h"
#include "reconstruction/reconstructor_interp.h"

using namespace dbContent;
using namespace dbContent::TargetReport;

using namespace std;
using namespace Utils;
using namespace boost::posix_time;
using namespace nlohmann;

namespace CalculateReferences {

Target::Target(unsigned int utn, std::shared_ptr<dbContent::Cache> cache)
    : utn_(utn), cache_(cache)
{

}

void Target::addTargetReport(const std::string& dbcontent_name, unsigned int ds_id,
                             unsigned int line_id, boost::posix_time::ptime timestamp, unsigned int index)
{
    TargetKey key = TargetKey{dbcontent_name, ds_id, line_id};

    if (!chains_.count(key))
        chains_[key].reset(new Chain(cache_, dbcontent_name));

    chains_.at(key)->addIndex(timestamp, index);
}

void Target::finalizeChains()
{
    for (auto& chain_it : chains_)
        chain_it.second->finalize();
}

std::shared_ptr<Buffer> Target::calculateReference()
{
    string dbcontent_name = "RefTraj";

    boost::posix_time::ptime ts_begin, ts_end;

    for (auto& chain_it : chains_)
    {
        if (chain_it.second->hasData())
        {
            if (ts_begin.is_not_a_date_time())
            {
                ts_begin = chain_it.second->timeBegin();
                ts_end = chain_it.second->timeEnd();

                assert (!ts_begin.is_not_a_date_time());
                assert (!ts_end.is_not_a_date_time());
            }
            else
            {
                ts_begin = min(ts_begin, chain_it.second->timeBegin());
                ts_end = max(ts_end, chain_it.second->timeEnd());
            }
        }
    }

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    PropertyList buffer_list;

    // basics
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_sac_id_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_sic_id_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_));

    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_time_of_day_));

    // pos
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_));

    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_bit_));

    // spd
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vx_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vy_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_angle_));

    // stddevs
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_x_stddev_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_y_stddev_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_xy_cov_));

    // secondary
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ta_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ti_));

    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_associations_));

    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(buffer_list, dbcontent_name);

    NullableVector<unsigned int>& ds_id_vec = buffer->get<unsigned int> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_).name());
    NullableVector<unsigned char>& sac_vec = buffer->get<unsigned char> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_sac_id_).name());
    NullableVector<unsigned char>& sic_vec = buffer->get<unsigned char> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_sic_id_).name());
    NullableVector<unsigned int>& line_vec = buffer->get<unsigned int> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_).name());

    NullableVector<float>& tod_vec = buffer->get<float> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_time_of_day_).name());
    NullableVector<ptime>& ts_vec = buffer->get<ptime> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_).name());

    NullableVector<double>& lat_vec = buffer->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_).name());
    NullableVector<double>& lon_vec = buffer->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_).name());
    NullableVector<float>& mc_vec = buffer->get<float> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_).name());

    // speed, track angle

    NullableVector<double>& vx_vec = buffer->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vx_).name());
    NullableVector<double>& vy_vec = buffer->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_vy_).name());

    NullableVector<double>& speed_vec = buffer->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_speed_).name());
    NullableVector<double>& track_angle_vec = buffer->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_angle_).name());

    // stddevs

    NullableVector<double>& x_stddev_vec = buffer->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_x_stddev_).name());
    NullableVector<double>& y_stddev_vec = buffer->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_y_stddev_).name());
    NullableVector<double>& xy_cov_vec = buffer->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_xy_cov_).name());

    // ground bit
    NullableVector<bool>& gb_vec = buffer->get<bool> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ground_bit_).name());

    NullableVector<unsigned int>& m3a_vec = buffer->get<unsigned int> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_).name());
    NullableVector<unsigned int>& acad_vec = buffer->get<unsigned int> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ta_).name());
    NullableVector<string>& acid_vec = buffer->get<string> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ti_).name());

    NullableVector<json>& assoc_vec = buffer->get<json> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_associations_).name());

    DataMapping mapping;

    //unsigned int cnt=0;
    //bool data_written=false;

    unsigned int sac = 0;
    unsigned int sic = 0;
    unsigned int ds_id = 0;
    unsigned int line_id = 0;
    std::vector<unsigned int> assoc_val ({utn_});

    double speed_ms, bearing_rad, xy_cov;

    if (!ts_begin.is_not_a_date_time())
    {
        //boost::posix_time::time_duration update_interval = Time::partialSeconds(1.0);

        std::map<const Chain*, TargetKey> chain_targets;

        std::string dinfo = "UTN" + std::to_string(utn_);

        auto storeReferences = [ & ] (const std::vector<reconstruction::Reference>& references,
                const reconstruction::Reconstructor& rec)
        {
            for (size_t i = 0; i < references.size(); ++i)
            {
                const reconstruction::Reference& ref = references[ i ];
                const Chain* chain = rec.chainOfReference(ref);
                assert(chain);

                auto it = chain_targets.find(chain);
                assert(it != chain_targets.end());

                ds_id_vec.set(i, ds_id);
                sac_vec.set(i, sac);
                sic_vec.set(i, sic);
                line_vec.set(i, line_id);

                ts_vec.set(i, ref.t);
                tod_vec.set(i, ref.t.time_of_day().total_milliseconds() / 1000.0);

                lat_vec.set(i, ref.lat);
                lon_vec.set(i, ref.lon);

                assoc_vec.set(i, assoc_val);

                // set speed

                if (ref.vx.has_value() && ref.vy.has_value())
                {
                    vx_vec.set(i, *ref.vx);
                    vy_vec.set(i, *ref.vy);

                    speed_ms = sqrt(pow(*ref.vx, 2)+pow(*ref.vy, 2)) ; // for 1s
                    bearing_rad = atan2(*ref.vx, *ref.vy);

                    speed_vec.set(i, speed_ms * M_S2KNOTS);
                    track_angle_vec.set(i, bearing_rad * RAD2DEG);
                }

                // set stddevs

                if (ref.x_stddev.has_value() && ref.y_stddev.has_value() && ref.xy_cov.has_value())
                {
                    x_stddev_vec.set(i, *ref.x_stddev);
                    y_stddev_vec.set(i, *ref.y_stddev);

                    xy_cov = *ref.xy_cov;

                    // to inverse of this asterix rep
                    // if (xy_cov < 0)
                    //     xy_cov = - pow(xy_cov, 2);
                    // else
                    //     xy_cov = pow(xy_cov, 2);

                    if (xy_cov < 0)
                        xy_cov_vec.set(i, -sqrt(-xy_cov));
                    else
                        xy_cov_vec.set(i, sqrt(xy_cov));
                }

                // set other data

                for (auto& chain_it : chains_) // iterate over both chains
                {
                    mapping = chain_it.second->calculateDataMapping(ref.t);

                    if (mc_vec.isNull(i) &&mapping.pos_ref_.has_altitude_)
                        mc_vec.set(i, mapping.pos_ref_.altitude_);

                    if (mapping.has_ref1_)
                    {
                        boost::optional<unsigned int> m3a = chain_it.second->modeA(mapping.dataid_ref1_);
                        boost::optional<std::string> acid = chain_it.second->acid(mapping.dataid_ref1_);
                        boost::optional<unsigned int> acad = chain_it.second->acad(mapping.dataid_ref1_);

                        if (m3a_vec.isNull(i) && m3a.has_value())
                            m3a_vec.set(i, *m3a);

                        if (acad_vec.isNull(i) && acad.has_value())
                            acad_vec.set(i, *acad);

                        if (acid_vec.isNull(i) && acid.has_value())
                            acid_vec.set(i, *acid);

                        if (gb_vec.isNull(i) || (!gb_vec.isNull(i) && !gb_vec.get(i)))
                        {
                            boost::optional<bool> gbs = chain_it.second->groundBit(mapping.dataid_ref1_);

                            if (gbs.has_value())
                                gb_vec.set(i, *gbs);
                        }
                    }

                    if (mapping.has_ref2_)
                    {
                        boost::optional<unsigned int> m3a = chain_it.second->modeA(mapping.dataid_ref2_);
                        boost::optional<std::string> acid = chain_it.second->acid(mapping.dataid_ref2_);
                        boost::optional<unsigned int> acad = chain_it.second->acad(mapping.dataid_ref2_);

                        if (m3a_vec.isNull(i) && m3a.has_value())
                            m3a_vec.set(i, *m3a);

                        if (acad_vec.isNull(i) && acad.has_value())
                            acad_vec.set(i, *acad);

                        if (acid_vec.isNull(i) && acid.has_value())
                            acid_vec.set(i, *acid);

                        if (gb_vec.isNull(i) || (!gb_vec.isNull(i) && !gb_vec.get(i)))
                        {
                            boost::optional<bool> gbs = chain_it.second->groundBit(mapping.dataid_ref2_);

                            if (gbs.has_value())
                                gb_vec.set(i, *gbs);
                        }
                    }
                }
            }
        };

        //        auto reconstructInterp = [ & ] ()
        //        {
        //            reconstruction::ReconstructorInterp rec;
        //            rec.setCoordConversion(reconstruction::Reconstructor::CoordConversion::NoConversion);
        //            rec.config().sample_dt = 1.0;
        //            rec.config().max_dt    = 30.0;

        //            for (auto& chain_it : chains_)
        //            {
        //                if (std::get<0>(chain_it.first) != "CAT062")
        //                    continue;
        //                chain_targets[chain_it.second.get()] = chain_it.first;
        //                rec.addChain(chain_it.second.get());
        //            }

        //            auto references = rec.reconstruct(dinfo);

        //            if (references.has_value())
        //                storeReferences(references.value(), rec);
        //        };

        auto reconstructUMKalman2D = [ & ] ()
        {
            //default uncertainties
            const double R_std          = 30.0;   // observation noise (standard)
            const double R_std_high     = 1000.0; // observation noise (high)
            const double Q_std          = 30.0;   // process noise
            const double P_std          = 30.0;   // system noise (standard)
            const double P_std_high     = 1000.0; // system noise (high)

            const double vel_std_cat021 = 50.0; // default velocity stdddev CAT021
            const double vel_std_cat062 = 50.0;   // default velocity stdddev CAT062

            //other config values
            const double min_dt         = 0.0;    // minimum allowed timestep in seconds
            const double max_dt         = 60.0;   // maximum allowed timestep in seconds
            const size_t min_chain_size = 2;      // minimum kalman chain size

            const bool   track_vel      = true;   // track velocities in measurements
            const bool   smooth_rts     = true;   // enable RTS smoother
            const double smooth_scale   = 1;    // scale factor for RTS smoother

            reconstruction::Reconstructor_UMKalman2D rec(track_vel);

            //configure kalman
            rec.baseConfig().R_std          = R_std;
            rec.baseConfig().R_std_high     = R_std_high;
            rec.baseConfig().Q_std          = Q_std;
            rec.baseConfig().P_std          = P_std;
            rec.baseConfig().P_std_high     = P_std_high;

            rec.baseConfig().min_dt         = min_dt;
            rec.baseConfig().max_dt         = max_dt;
            rec.baseConfig().min_chain_size = min_chain_size;

            rec.baseConfig().smooth         = smooth_rts;
            rec.baseConfig().smooth_scale   = smooth_scale;

            //if (utn_ == 3)
            //    rec.setVerbosity(2);

            //configure sensor default noise
            reconstruction::Uncertainty uncert_cat021;
            uncert_cat021.pos_var   = R_std          * R_std;
            uncert_cat021.speed_var = vel_std_cat021 * vel_std_cat021;
            uncert_cat021.acc_var   = R_std_high     * R_std_high;

            reconstruction::Uncertainty uncert_cat062;
            uncert_cat062.pos_var   = R_std          * R_std;
            uncert_cat062.speed_var = vel_std_cat062 * vel_std_cat062;
            uncert_cat062.acc_var   = R_std_high     * R_std_high;

            rec.setSensorUncertainty("CAT021", uncert_cat021);
            rec.setSensorUncertainty("CAT062", uncert_cat062);

            //add chains to reconstructor
            for (auto& chain_it : chains_)
            {
                chain_targets[chain_it.second.get()] = chain_it.first;
                rec.addChain(chain_it.second.get(), std::get<0>(chain_it.first));
            }

            //reconstruct
            auto references = rec.reconstruct(dinfo);
            if (references.has_value())
                storeReferences(references.value(), rec);
        };

        //reconstructInterp();
        reconstructUMKalman2D();
    }

    loginf << "Target: calculateReference: buffer size " << buffer->size();

    return buffer;
}

unsigned int Target::utn() const
{
    return utn_;
}

} // namespace CalculateReferences
