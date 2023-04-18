#include "calculatereferencesjob.h"
#include "calculatereferencestask.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/target/target.h"
#include "stringconv.h"

#include "util/tbbhack.h"

#include <QThread>

#include <future>

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

    createTargets();

    emit statusSignal("Finalizing Targets");

    finalizeTargets();

    emit statusSignal("Calculating References");

    calculateReferences();

    emit statusSignal("Writing References");

    writeReferences();

    while (!insert_done_)
        QThread::msleep(10);

    stop_time = microsec_clock::local_time();

    loginf << "CalculateReferencesJob: run: created " << targets_.size() << " targets";

    double load_time;
    time_duration diff = stop_time - start_time;
    load_time = diff.total_milliseconds() / 1000.0;

    loginf << "CalculateReferencesJob: run: done ("
           << String::doubleToStringPrecision(load_time, 2) << " s).";

    done_ = true;
}


void CalculateReferencesJob::createTargets()
{
    loginf << "CalculateReferencesJob: createTargets";

    assert (cache_);

    unsigned int num_skipped {0};
    unsigned int num_unassoc {0};
    unsigned int num_assoc {0};

    ptime timestamp;
    vector<unsigned int> utn_vec;

    // create map for utn lookup
    map<unsigned int, std::unique_ptr<CalculateReferences::Target>> target_map;

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
                if (!target_map.count(utn_it))
                    target_map[utn_it].reset(new CalculateReferences::Target(utn_it, cache_));
                //target_data_.emplace_back(utn_it, *this, cache_, eval_man_, dbcont_man_);

                target_map.at(utn_it)->addTargetReport(dbcontent_name, ds_ids.get(cnt), line_ids.get(cnt),
                                                       timestamp, cnt);

                ++num_assoc;
            }
        }
    }

    // move targets from map to vec

    for (auto& tgt_it : target_map)
        targets_.emplace_back(move(tgt_it.second));

    loginf << "CalculateReferencesJob: createTargets: done for " << targets_.size()
           << ", reports num_skipped " << num_skipped
           << " num_unassoc " << num_unassoc << " num_assoc " << num_assoc;
}

void CalculateReferencesJob::finalizeTargets()
{
    for (auto& tgt_it : targets_)
        tgt_it->finalizeChains();
}

#include <valgrind/callgrind.h>

void CalculateReferencesJob::calculateReferences()
{
    loginf << "CalculateReferencesJob: calculateReferences";

    unsigned int num_targets = targets_.size();
    //num_targets = 50; // only calculate part

    std::vector<std::shared_ptr<Buffer>> results;

    results.resize(num_targets);

//    CALLGRIND_START_INSTRUMENTATION;

//    CALLGRIND_TOGGLE_COLLECT;

//    for (unsigned int tgt_cnt=0; tgt_cnt < num_targets; ++tgt_cnt)
//    {
//        results[tgt_cnt] = targets_.at(tgt_cnt)->calculateReference();

//        loginf << "CalculateReferencesJob: calculateReferences: utn "
//               << targets_.at(tgt_cnt)->utn() << " done";
//    }

    std::future<void> pending_future = std::async(std::launch::async, [&] {

        tbb::parallel_for(uint(0), num_targets, [&](unsigned int tgt_cnt)
        {
            results[tgt_cnt] = targets_.at(tgt_cnt)->calculateReference();

            reftraj_counts_[targets_.at(tgt_cnt)->utn()] = results.at(tgt_cnt)->size(); // store count

            loginf << "CalculateReferencesJob: calculateReferences: utn "
                   << targets_.at(tgt_cnt)->utn() << " done";
        });
    });

//    CALLGRIND_TOGGLE_COLLECT;

//    CALLGRIND_STOP_INSTRUMENTATION;

    pending_future.wait(); // or do done flags for progress updates

    for (auto& buf_it : results)
    {
        if (!result_)
            result_ = move(buf_it);
        else
            result_->seizeBuffer(*buf_it);
    }

    loginf << "CalculateReferencesJob: calculateReferences: done, buffer size " << result_->size();
}


void CalculateReferencesJob::writeReferences()
{
    string dbcontent_name = "RefTraj";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    connect(&dbcontent_man, &DBContentManager::insertDoneSignal,
            this, &CalculateReferencesJob::insertDoneSlot);

    std::map<std::string, std::shared_ptr<Buffer>> data {{dbcontent_name, result_}};

    dbcontent_man.insertData(data);
}

void CalculateReferencesJob::insertDoneSlot()
{
    loginf << "CalculateReferencesJob: insertDoneSlot";

    // store counts

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    for (auto& cnt_it : reftraj_counts_) // utn -> cnt
    {
        assert (dbcontent_man.existsTarget(cnt_it.first));
        dbContent::Target &target = dbcontent_man.target(cnt_it.first);

        target.dbContentCount("RefTraj", target.dbContentCount("RefTraj") + cnt_it.second);
    }

    dbcontent_man.saveTargets();

    insert_done_ = true;
}

