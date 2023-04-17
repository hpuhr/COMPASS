#include "calculatereferencesjob.h"
#include "calculatereferencestask.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/target/target.h"
#include "stringconv.h"

#include "util/tbbhack.h"

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace boost::posix_time;

CalculateReferencesJob::CalculateReferencesJob(CalculateReferencesTask& task, std::shared_ptr<dbContent::Cache> cache)
    : Job("CalculateReferencesJob"), task_(task), cache_(cache)
{

}

CalculateReferencesJob::~CalculateReferencesJob()
{

}

void CalculateReferencesJob::run()
{
    logdbg << "CalculateReferencesJob: run: start";

    started_ = true;

    ptime start_time;
    ptime stop_time;

    start_time = microsec_clock::local_time();

    emit statusSignal("Creating Targets");

    assert (cache_);

    unsigned int num_skipped {0};
    unsigned int num_unassoc {0};
    unsigned int num_assoc {0};

    ptime timestamp;
    vector<unsigned int> utn_vec;

    for (auto& buf_it : *cache_)
    {
        string dbcontent_name = buf_it.first;

        assert (cache_->hasMetaVar<ptime>(dbcontent_name, DBContent::meta_var_timestamp_));
        NullableVector<ptime>& ts_vec = cache_->getMetaVar<ptime>(
                    dbcontent_name, DBContent::meta_var_timestamp_);


        unsigned int buffer_size = ts_vec.size();

        assert (cache_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_datasource_id_));
        NullableVector<unsigned int>& ds_ids = cache_->getMetaVar<unsigned int>(
                    dbcontent_name, DBContent::meta_var_datasource_id_);


        assert (cache_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_line_id_));
        NullableVector<unsigned int>& line_ids = cache_->getMetaVar<unsigned int>(
                    dbcontent_name, DBContent::meta_var_line_id_);

        assert (cache_->hasMetaVar<json>(dbcontent_name, DBContent::meta_var_associations_));
        NullableVector<json>& assoc_vec = cache_->getMetaVar<json>(
                    dbcontent_name, DBContent::meta_var_associations_);

        for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
        {
            assert (!ds_ids.isNull(cnt));
            assert (!line_ids.isNull(cnt));

            if (ts_vec.isNull(cnt))
            {
                ++num_skipped;
                continue;
            }

            timestamp = ts_vec.get(cnt);

            if (assoc_vec.isNull(cnt))
                utn_vec.clear();
            else
                utn_vec = assoc_vec.get(cnt).get<std::vector<unsigned int>>();

            if (!utn_vec.size())
            {
                ++num_unassoc;
                continue;
            }

            for (auto utn_it : utn_vec)
            {
                if (!targets_.count(utn_it))
                    targets_[utn_it].reset(new CalculateReferences::Target(utn_it, cache_));
                    //target_data_.emplace_back(utn_it, *this, cache_, eval_man_, dbcont_man_);

                targets_.at(utn_it)->addTargetReport(dbcontent_name, ds_ids.get(cnt), line_ids.get(cnt),
                                                     timestamp, cnt);

                ++num_assoc;
            }
        }
    }

    for (auto& tgt_it : targets_)
        tgt_it.second->finalizeChains();

    stop_time = microsec_clock::local_time();

    loginf << "CalculateReferencesJob: run: created " << targets_.size() << " targets";

    double load_time;
    time_duration diff = stop_time - start_time;
    load_time = diff.total_milliseconds() / 1000.0;

    loginf << "CalculateReferencesJob: run: done ("
           << String::doubleToStringPrecision(load_time, 2) << " s).";

    done_ = true;
}
