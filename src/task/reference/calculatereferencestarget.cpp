#include "calculatereferencestarget.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "buffer.h"
#include "timeconv.h"
#include "logger.h"

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
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
    buffer_list.addProperty(dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_));
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
    NullableVector<ptime>& ts_vec = buffer->get<ptime> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_).name());
    NullableVector<double>& lat_vec = buffer->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_).name());
    NullableVector<double>& lon_vec = buffer->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_).name());
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

        for (boost::posix_time::ptime ts_current = ts_begin; ts_current <= ts_end; ts_current += update_interval)
        {
            data_written = false;

            for (auto& chain_it : chains_)
            {
                mapping = chain_it.second->calculateDataMapping(ts_current);

                if (mapping.has_ref_pos_) // only set values if at least position exists
                {
                    data_written = true;

                    ds_id_vec.set(cnt, ds_id);
                    sac_vec.set(cnt, sac);
                    sic_vec.set(cnt, sic);
                    line_vec.set(cnt, line_id);
                    ts_vec.set(cnt, ts_current);

                    if (!lat_vec.isNull(cnt)) // already set
                    {
                        assert (!lon_vec.isNull(cnt));

                        lat_vec.set(cnt, (mapping.pos_ref_.latitude_ + lat_vec.get(cnt))/2.0);
                        lon_vec.set(cnt, (mapping.pos_ref_.longitude_ + lon_vec.get(cnt))/2.0);
                    }
                    else
                    {
                        lat_vec.set(cnt, mapping.pos_ref_.latitude_);
                        lon_vec.set(cnt, mapping.pos_ref_.longitude_);
                    }

                    assoc_vec.set(cnt, assoc_val);
                }
            }

            if (data_written)
                ++cnt;
        }
    }

    loginf << "Target: calculateReference: buffer size " << buffer->size();

    return buffer;
}

unsigned int Target::utn() const
{
    return utn_;
}

} // namespace CalculateReferences
