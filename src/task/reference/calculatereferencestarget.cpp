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

    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_datasource_id_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_sac_id_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_sic_id_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_));

    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_time_of_day_));

    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_));

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

    NullableVector<unsigned int>& m3a_vec = buffer->get<unsigned int> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_).name());
    NullableVector<unsigned int>& acad_vec = buffer->get<unsigned int> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ta_).name());
    NullableVector<string>& acid_vec = buffer->get<string> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ti_).name());

    NullableVector<json>& assoc_vec = buffer->get<json> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_associations_).name());

    DataMapping mapping;

    unsigned int cnt=0;
    bool data_written=false;

    unsigned int sac = 0;
    unsigned int sic = 0;
    unsigned int ds_id = 0;
    unsigned int line_id = 0;
    std::vector<unsigned int> assoc_val ({utn_});

    if (!ts_begin.is_not_a_date_time())
    {
        boost::posix_time::time_duration update_interval = Time::partialSeconds(1.0);

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
            reconstruction::Reconstructor_UMKalman2D rec;
            rec.baseConfig().R_std          = 30.0; // observation noise
            rec.baseConfig().Q_std          = 10.0; // process noise
            rec.baseConfig().P_std          = 30.0; // system noise (standard)
            rec.baseConfig().P_std_high     = 1000.0; // system noise (high)
            rec.baseConfig().smooth         = true;
            rec.baseConfig().min_dt         = 0.0;
            rec.baseConfig().max_dt         = 60.0;
            rec.baseConfig().min_chain_size = 2;

            rec.config().simple_init = false;

            for (auto& chain_it : chains_)
            {
                chain_targets[chain_it.second.get()] = chain_it.first;
                rec.addChain(chain_it.second.get());
            }

            auto references = rec.reconstruct(dinfo);
            if (references.has_value())
                storeReferences(references.value(), rec);
        };

        //        auto reconstructMidpoint = [ & ] ()
        //        {
        //            for (boost::posix_time::ptime ts_current = ts_begin; ts_current <= ts_end; ts_current += update_interval)
        //            {
        //                data_written = false;

        //                for (auto& chain_it : chains_)
        //                {
        //                    mapping = chain_it.second->calculateDataMapping(ts_current);

        //                    if (mapping.has_ref_pos_) // only set values if at least position exists
        //                    {
        //                        data_written = true;

        //                        ds_id_vec.set(cnt, ds_id);
        //                        sac_vec.set(cnt, sac);
        //                        sic_vec.set(cnt, sic);
        //                        line_vec.set(cnt, line_id);

        //                        ts_vec.set(cnt, ts_current);
        //                        tod_vec.set(cnt, ts_current.time_of_day().total_milliseconds() / 1000.0);

        //                        if (!lat_vec.isNull(cnt)) // already set
        //                        {
        //                            assert (!lon_vec.isNull(cnt));

        //                            lat_vec.set(cnt, (mapping.pos_ref_.latitude_ + lat_vec.get(cnt))/2.0);
        //                            lon_vec.set(cnt, (mapping.pos_ref_.longitude_ + lon_vec.get(cnt))/2.0);
        //                        }
        //                        else
        //                        {
        //                            lat_vec.set(cnt, mapping.pos_ref_.latitude_);
        //                            lon_vec.set(cnt, mapping.pos_ref_.longitude_);
        //                        }

        //                        assoc_vec.set(cnt, assoc_val);
        //                    }

        //                    if (mc_vec.isNull(cnt) &&mapping.pos_ref_.has_altitude_)
        //                        mc_vec.set(cnt, mapping.pos_ref_.altitude_);

        //                    if (mapping.has_ref1_)
        //                    {
        //                        boost::optional<unsigned int> m3a = chain_it.second->modeA(mapping.dataid_ref1_);
        //                        boost::optional<std::string> acid = chain_it.second->acid(mapping.dataid_ref1_);
        //                        boost::optional<unsigned int> acad = chain_it.second->acad(mapping.dataid_ref1_);

        //                        if (m3a_vec.isNull(cnt) && m3a.has_value())
        //                            m3a_vec.set(cnt, *m3a);

        //                        if (acad_vec.isNull(cnt) && acad.has_value())
        //                            acad_vec.set(cnt, *acad);

        //                        if (acid_vec.isNull(cnt) && acid.has_value())
        //                            acid_vec.set(cnt, *acid);
        //                    }

        //                    if (mapping.has_ref2_)
        //                    {
        //                        boost::optional<unsigned int> m3a = chain_it.second->modeA(mapping.dataid_ref2_);
        //                        boost::optional<std::string> acid = chain_it.second->acid(mapping.dataid_ref2_);
        //                        boost::optional<unsigned int> acad = chain_it.second->acad(mapping.dataid_ref2_);

        //                        if (m3a_vec.isNull(cnt) && m3a.has_value())
        //                            m3a_vec.set(cnt, *m3a);

        //                        if (acad_vec.isNull(cnt) && acad.has_value())
        //                            acad_vec.set(cnt, *acad);

        //                        if (acid_vec.isNull(cnt) && acid.has_value())
        //                            acid_vec.set(cnt, *acid);
        //                    }
        //                }

        //                if (data_written)
        //                    ++cnt;
        //            }
        //        };

        //reconstructInterp();
        reconstructUMKalman2D();
        //reconstructMidpoint();
    }

    loginf << "Target: calculateReference: buffer size " << buffer->size();

    return buffer;
}

unsigned int Target::utn() const
{
    return utn_;
}

} // namespace CalculateReferences
