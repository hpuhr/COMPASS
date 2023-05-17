#include "calculatereferencestarget.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "buffer.h"
#include "timeconv.h"
#include "viewpointgenerator.h"
#include "logger.h"
#include "calculatereferencestask.h"

#include "reconstruction/reconstructor_umkalman2d.h"
#include "reconstruction/reconstructor_interp.h"
#if USE_EXPERIMENTAL_SOURCE
    #include "reconstructor_amkalman2d.h"
#endif

using namespace dbContent;
using namespace dbContent::TargetReport;

using namespace std;
using namespace Utils;
using namespace boost::posix_time;
using namespace nlohmann;

namespace CalculateReferences {

Target::Target(unsigned int utn, 
               std::shared_ptr<dbContent::Cache> cache)
    : utn_(utn), cache_(cache)
{
}

void Target::addTargetReport(const std::string& dbcontent_name, 
                             unsigned int ds_id,
                             unsigned int line_id, 
                             boost::posix_time::ptime timestamp, 
                             unsigned int index)
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

std::shared_ptr<Buffer> Target::calculateReference(const CalculateReferencesTaskSettings& settings,
                                                   ViewPointGenVP* gen_view_point)
{
    string dbcontent_name = "RefTraj";

    boost::posix_time::ptime ts_begin, ts_end;
    boost::optional<double>  lat_min, lon_min, lat_max, lon_max;

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

            //keep track of bounds
            if (!lat_min.has_value() || chain_it.second->latitudeMin() < lat_min.value())
                lat_min = chain_it.second->latitudeMin();
            if (!lon_min.has_value() || chain_it.second->longitudeMin() < lon_min.value())
                lon_min = chain_it.second->longitudeMin();
            if (!lat_max.has_value() || chain_it.second->latitudeMax() > lat_max.value())
                lat_max = chain_it.second->latitudeMax();
            if (!lon_max.has_value() || chain_it.second->longitudeMax() > lon_max.value())
                lon_max = chain_it.second->longitudeMax();
        }
    }

    //region of interest
    QRectF roi;
    if (lat_min.has_value() && lon_min.has_value() && lat_max.has_value() && lon_max.has_value())
        roi = QRectF(lat_min.value(), lon_min.value(), lat_max.value() - lat_min.value(), lon_max.value() - lon_min.value());

    if (gen_view_point)
        gen_view_point->setROI(roi);

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

    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_utn_));

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

    NullableVector<unsigned int>& utn_vec = buffer->get<unsigned int> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_utn_).name());

    DataMapping mapping;

    //unsigned int cnt=0;
    //bool data_written=false;

    unsigned int sac = 0;
    unsigned int sic = 0;
    unsigned int ds_id = 0;
    unsigned int line_id = 0;
    //std::vector<unsigned int> assoc_val ({utn_});

    double speed_ms, bearing_rad, xy_cov;

    if (!ts_begin.is_not_a_date_time())
    {
        //boost::posix_time::time_duration update_interval = Time::partialSeconds(1.0);

        std::map<const Chain*, TargetKey> chain_targets;

        std::string dinfo = "UTN " + std::to_string(utn_);

        auto storeReferences = [ & ] (const std::vector<reconstruction::Reference>& references,
                                      const reconstruction::Reconstructor& rec)
        {
            unsigned int buffer_cnt = 0;

            for (size_t i = 0; i < references.size(); ++i)
            {
                const reconstruction::Reference& ref = references[ i ];
                const Chain* chain = rec.chainOfReference(ref);
                assert(chain);

                auto it = chain_targets.find(chain);
                assert(it != chain_targets.end());

                // hack to skip no mode c

                bool has_any_mode_c = false;

                for (auto& chain_it : chains_) // iterate over both chains
                {
                    mapping = chain_it.second->calculateDataMapping(ref.t);

                    if (mapping.pos_ref_.has_altitude_)
                    {
                        has_any_mode_c = true;
                        break;
                    }
                }

                if (!has_any_mode_c)
                    continue;

                ds_id_vec.set(buffer_cnt, ds_id);
                sac_vec.set(buffer_cnt, sac);
                sic_vec.set(buffer_cnt, sic);
                line_vec.set(buffer_cnt, line_id);

                ts_vec.set(buffer_cnt, ref.t);
                tod_vec.set(buffer_cnt, ref.t.time_of_day().total_milliseconds() / 1000.0);

                lat_vec.set(buffer_cnt, ref.lat);
                lon_vec.set(buffer_cnt, ref.lon);

                utn_vec.set(buffer_cnt, utn_);

                // set speed

                if (ref.vx.has_value() && ref.vy.has_value())
                {
                    vx_vec.set(buffer_cnt, *ref.vx);
                    vy_vec.set(buffer_cnt, *ref.vy);

                    speed_ms = sqrt(pow(*ref.vx, 2)+pow(*ref.vy, 2)) ; // for 1s
                    bearing_rad = atan2(*ref.vx, *ref.vy);

                    speed_vec.set(buffer_cnt, speed_ms * M_S2KNOTS);
                    track_angle_vec.set(buffer_cnt, bearing_rad * RAD2DEG);
                }

                // set stddevs

                if (ref.x_stddev.has_value() && ref.y_stddev.has_value() && ref.xy_cov.has_value())
                {
                    x_stddev_vec.set(buffer_cnt, *ref.x_stddev);
                    y_stddev_vec.set(buffer_cnt, *ref.y_stddev);

                    xy_cov = *ref.xy_cov;

                    // to inverse of this asterix rep
                    // if (xy_cov < 0)
                    //     xy_cov = - pow(xy_cov, 2);
                    // else
                    //     xy_cov = pow(xy_cov, 2);

                    if (xy_cov < 0)
                        xy_cov_vec.set(buffer_cnt, -sqrt(-xy_cov));
                    else
                        xy_cov_vec.set(buffer_cnt, sqrt(xy_cov));
                }

                // set other data

                for (auto& chain_it : chains_) // iterate over both chains
                {
                    mapping = chain_it.second->calculateDataMapping(ref.t);

                    if (mapping.pos_ref_.has_altitude_)
                    {
                        if (mc_vec.isNull(i))
                            mc_vec.set(buffer_cnt, mapping.pos_ref_.altitude_);
                        else
                            mc_vec.set(buffer_cnt,
                                       (mapping.pos_ref_.altitude_ + mc_vec.get(buffer_cnt))/2.0);
                    }

                    if (mapping.has_ref1_)
                    {
                        boost::optional<unsigned int> m3a = chain_it.second->modeA(mapping.dataid_ref1_);
                        boost::optional<std::string> acid = chain_it.second->acid(mapping.dataid_ref1_);
                        boost::optional<unsigned int> acad = chain_it.second->acad(mapping.dataid_ref1_);

                        if (m3a_vec.isNull(i) && m3a.has_value())
                            m3a_vec.set(buffer_cnt, *m3a);

                        if (acad_vec.isNull(i) && acad.has_value())
                            acad_vec.set(buffer_cnt, *acad);

                        if (acid_vec.isNull(i) && acid.has_value())
                            acid_vec.set(buffer_cnt, *acid);

                        if (gb_vec.isNull(i) || (!gb_vec.isNull(i) && !gb_vec.get(i)))
                        {
                            boost::optional<bool> gbs = chain_it.second->groundBit(mapping.dataid_ref1_);

                            if (gbs.has_value())
                                gb_vec.set(buffer_cnt, *gbs);
                        }
                    }

                    if (mapping.has_ref2_)
                    {
                        boost::optional<unsigned int> m3a = chain_it.second->modeA(mapping.dataid_ref2_);
                        boost::optional<std::string> acid = chain_it.second->acid(mapping.dataid_ref2_);
                        boost::optional<unsigned int> acad = chain_it.second->acad(mapping.dataid_ref2_);

                        if (m3a_vec.isNull(i) && m3a.has_value())
                            m3a_vec.set(buffer_cnt, *m3a);

                        if (acad_vec.isNull(i) && acad.has_value())
                            acad_vec.set(buffer_cnt, *acad);

                        if (acid_vec.isNull(i) && acid.has_value())
                            acid_vec.set(buffer_cnt, *acid);

                        if (gb_vec.isNull(i) || (!gb_vec.isNull(i) && !gb_vec.get(i)))
                        {
                            boost::optional<bool> gbs = chain_it.second->groundBit(mapping.dataid_ref2_);

                            if (gbs.has_value())
                                gb_vec.set(buffer_cnt, *gbs);
                        }
                    }
                }

                ++ buffer_cnt;
            }
        };

        // auto reconstructInterp = [ & ] ()
        // {
        //     reconstruction::ReconstructorInterp rec;
        //     rec.setViewPoint(gen_view_point);
        //     rec.setCoordConversion(reconstruction::CoordConversion::NoConversion);
        //     rec.config().interp_config.sample_dt            = 1.0;
        //     rec.config().interp_config.max_dt               = 30.0;
        //     rec.config().interp_config.check_fishy_segments = true;

        //     for (auto& chain_it : chains_)
        //     {
        //         if (std::get<0>(chain_it.first) != "CAT062")
        //             continue;
                
        //         chain_targets[chain_it.second.get()] = chain_it.first;
        //         rec.addChain(chain_it.second.get(), std::get<0>(chain_it.first));
        //     }

        //     auto references = rec.reconstruct(dinfo);

        //     if (references.has_value())
        //         storeReferences(references.value(), rec);
        // };

        auto reconstructKalman2D = [ & ] ()
        {
            CalculateReferencesTaskSettings s = settings;

            bool   add_sensor_uncert = true;   // add sensor default uncertainties
            double vel_std_cat021    = 50.0;   // default velocity stdddev CAT021
            double vel_std_cat062    = 50.0;   // default velocity stdddev CAT062
            double acc_std_cat021    = 50.0;   // default acceleration stdddev CAT021
            double acc_std_cat062    = 50.0;   // default acceleration stdddev CAT062

            bool python_compatibility_mode = false; //python reconstructor comparison mode

            if (python_compatibility_mode)
            {
                // apply python code compatible override settings
                s.use_vel_mm         = false;
                s.resample_result    = false;
                s.resample_systracks = false;

                add_sensor_uncert = false;
            }

            std::unique_ptr<reconstruction::ReconstructorKalman> rec;

        #if USE_EXPERIMENTAL_SOURCE
            if (s.rec_type == CalculateReferencesTaskSettings::ReconstructorType::AMKalman2D)
            {
                rec.reset(new reconstruction::Reconstructor_AMKalman2D);
            }
            else
        #endif
            {
                rec.reset(new reconstruction::Reconstructor_UMKalman2D(s.use_vel_mm));
            }
        
            assert(rec);

            rec->setViewPoint(gen_view_point);
            rec->setVerbosity(s.verbose ? 1 : 0);

            //configure kalman reconstructor
            rec->baseConfig().R_std          = s.R_std;
            rec->baseConfig().R_std_high     = s.R_std_high;
            rec->baseConfig().Q_std          = s.Q_std;
            rec->baseConfig().P_std          = s.P_std;
            rec->baseConfig().P_std_high     = s.P_std_high;

            rec->baseConfig().min_dt         = s.min_dt;
            rec->baseConfig().max_dt         = s.max_dt;
            rec->baseConfig().min_chain_size = s.min_chain_size;

            rec->baseConfig().smooth         = s.smooth_rts;
            rec->baseConfig().smooth_scale   = 1.0;

            rec->baseConfig().resample_result = s.resample_result;
            rec->baseConfig().resample_dt     = s.resample_result_dt;

            //configure sensor default noise?
            if (add_sensor_uncert)
            {
                reconstruction::Uncertainty uncert_cat021;
                uncert_cat021.pos_var   = s.R_std * s.R_std;
                uncert_cat021.speed_var = vel_std_cat021 * vel_std_cat021;
                uncert_cat021.acc_var   = acc_std_cat021 * acc_std_cat021;

                reconstruction::Uncertainty uncert_cat062;
                uncert_cat062.pos_var   = s.R_std * s.R_std;
                uncert_cat062.speed_var = vel_std_cat062 * vel_std_cat062;
                uncert_cat062.acc_var   = acc_std_cat062 * acc_std_cat062;

                rec->setSensorUncertainty("CAT021", uncert_cat021);
                rec->setSensorUncertainty("CAT062", uncert_cat062);
            }

            //resample system tracks?
            if (s.resample_systracks)
            {
                reconstruction::InterpOptions options;
                options.sample_dt = s.resample_systracks_dt;
                options.max_dt    = s.max_dt;

                rec->setSensorInterpolation("CAT062", options);
            }

            //add chains to reconstructor
            for (auto& chain_it : chains_)
            {
                chain_targets[chain_it.second.get()] = chain_it.first;
                rec->addChain(chain_it.second.get(), std::get<0>(chain_it.first));
            }

            //reconstruct
            auto references = rec->reconstruct(dinfo);
            if (references.has_value())
                storeReferences(references.value(), *rec);
        };

        //reconstructInterp();
        reconstructKalman2D();
    }

    loginf << "Target: calculateReference: buffer size " << buffer->size();

    return buffer;
}

unsigned int Target::utn() const
{
    return utn_;
}

} // namespace CalculateReferences
