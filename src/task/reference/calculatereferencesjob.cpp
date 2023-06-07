#include "calculatereferencesjob.h"
#include "calculatereferencestask.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/target/target.h"
#include "stringconv.h"
#include "viewpointgenerator.h"

#include "util/tbbhack.h"

#include <QThread>

#include <future>
#include <cmath>

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace boost::posix_time;

string tracker_only_confirmed_positions_reason {"I062/080 CNF: Tentative or unkown"}; // non-tentative
string tracker_only_noncoasting_positions_reason {"I062/080 CST: Coasting or unkown"};
string tracker_only_report_detection_positions_reason {"I062/340 TYP: No detection or unkown"};
string tracker_only_report_detection_nonpsronly_positions_reason {
    "I062/080 MON: Mono sensor or unknown + I062/340 TYP: Single PSR detection or unkown"};
string tracker_only_high_accuracy_postions_reason {"I062/500 APC too high or unknown"};

CalculateReferencesJob::CalculateReferencesJob(CalculateReferencesTask& task, 
                                               std::shared_ptr<dbContent::Cache> cache)
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

    //write references to db if desired
    if (task_.writeReferences())
    {
        emit statusSignal("Writing References");
        writeReferences();

        while (!insert_done_)
            QThread::msleep(10);
    }

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
    unsigned int utn;

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

        assert (cache_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_utn_));
        NullableVector<unsigned int>& utn_vec = cache_->getMetaVar<unsigned int>(
                    dbcontent_name, DBContent::meta_var_utn_);

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

            if (utn_vec.isNull(cnt))
            {
                ++num_unassoc;
                continue;
            }

            utn = utn_vec.get(cnt);

            if (!target_map.count(utn))
                target_map[utn].reset(new CalculateReferences::Target(utn, cache_));

            target_map.at(utn)->addTargetReport(dbcontent_name, ds_ids.get(cnt), line_ids.get(cnt), timestamp, cnt);

            ++num_assoc;
        }
    }

    // filter pos accuracy

    loginf << "CalculateReferencesJob: createTargets: assessing position data usage";

    const CalculateReferencesTaskSettings& settings = task_.settings();

    unsigned int ignored_positions {0}, used_positions {0};
    map<string, unsigned int> tracker_ignored_reasons;

    string dbcontent_name = "CAT062";

    //    I062/080 Track Status (PFT)
    //        Mono/Multi track, CNF (confirmed/tentative), Primary-tracks
    //    I062/500 Estimated Accuracies (APC, ATV)

    if (cache_->has(dbcontent_name))
    {
        vector<bool> ignore_positions;
        bool ignore_current_position;

        unsigned int buffer_size = cache_->get(dbcontent_name)->size();

        assert (cache_->hasMetaVar<bool>(dbcontent_name, DBContent::meta_var_track_confirmed_));
        NullableVector<bool>& confirmed_vec = cache_->getMetaVar<bool>(
                    dbcontent_name, DBContent::meta_var_track_confirmed_);

        assert (cache_->hasMetaVar<unsigned char>(dbcontent_name, DBContent::meta_var_track_coasting_));
        NullableVector<unsigned char>& coasting_vec = cache_->getMetaVar<unsigned char>(
                    dbcontent_name, DBContent::meta_var_track_coasting_);

        assert (cache_->hasVar<bool>(dbcontent_name, DBContent::var_cat062_mono_sensor_));
        NullableVector<bool>& mono_vec = cache_->getVar<bool>(
                    dbcontent_name, DBContent::var_cat062_mono_sensor_);

        assert (cache_->hasVar<unsigned char>(dbcontent_name, DBContent::var_cat062_type_lm_));
        NullableVector<unsigned char>& type_lm_vec = cache_->getVar<unsigned char>(
                    dbcontent_name, DBContent::var_cat062_type_lm_);

        assert (cache_->hasMetaVar<double>(dbcontent_name, DBContent::meta_var_x_stddev_));
        NullableVector<double>& x_stddev_vec = cache_->getMetaVar<double>(
                    dbcontent_name, DBContent::meta_var_x_stddev_);

        assert (cache_->hasMetaVar<double>(dbcontent_name, DBContent::meta_var_y_stddev_));
        NullableVector<double>& y_stddev_vec = cache_->getMetaVar<double>(
                    dbcontent_name, DBContent::meta_var_y_stddev_);

        unsigned int index;

        for (auto& target_it : target_map)
        {
            for (auto& chain_it : target_it.second->chains())
            {
                if (get<0>(chain_it.first) != dbcontent_name) // skip non-tracker chains
                    continue;

                ignore_positions.clear();

                for (auto& ts_index_it : chain_it.second->timestampIndexes())
                {
                    ignore_current_position = false;

                    index = ts_index_it.second.idx_external; // index into buffer
                    assert (index <= buffer_size);

                    if (!ignore_current_position && settings.tracker_only_confirmed_positions)
                    {
                        if (confirmed_vec.isNull(index) || !confirmed_vec.get(index))
                        {
                            ignore_current_position = true;
                            tracker_ignored_reasons[tracker_only_confirmed_positions_reason] += 1;
                        }
                    }

                    if (!ignore_current_position && settings.tracker_only_noncoasting_positions)
                    {
                        if (coasting_vec.isNull(index) || coasting_vec.get(index))
                        {
                            ignore_current_position = true;
                            tracker_ignored_reasons[tracker_only_noncoasting_positions_reason] += 1;
                        }
                    }

                    if (!ignore_current_position && settings.tracker_only_report_detection_positions)
                    {
                        if (type_lm_vec.isNull(index) || type_lm_vec.get(index) == 0) // no detection
                        {
                            ignore_current_position = true;
                            tracker_ignored_reasons[tracker_only_report_detection_positions_reason] += 1;
                        }
                    }

                    if (!ignore_current_position && settings.tracker_only_report_detection_nonpsronly_positions)
                    {
                        if (mono_vec.isNull(index) || type_lm_vec.isNull(index)
                                || (mono_vec.get(index) && type_lm_vec.get(index) == 1 )) // mono + single psr
                        {
                            ignore_current_position = true;
                            tracker_ignored_reasons[tracker_only_report_detection_nonpsronly_positions_reason] += 1;
                        }
                    }

                    if (!ignore_current_position && settings.tracker_only_high_accuracy_postions)
                    {
                        if (x_stddev_vec.isNull(index) || y_stddev_vec.isNull(index)
                                || (sqrt(x_stddev_vec.get(index) + y_stddev_vec.get(index))
                                    > settings.tracker_minium_accuracy))
                        {
                            ignore_current_position = true;
                            tracker_ignored_reasons[tracker_only_high_accuracy_postions_reason] += 1;
                        }
                    }

                    ignore_positions.push_back(ignore_current_position);

                    if (ignore_current_position)
                        ++ignored_positions;
                    else
                        ++used_positions;
                }

                // set ignore positions flag
                chain_it.second->setIgnoredPositions(ignore_positions);
            }
        }
    }

    loginf << "CalculateReferencesJob: createTargets: tracker data ignored pos " << ignored_positions
           << " used " << used_positions;

    for (auto& reason_it : tracker_ignored_reasons)
        loginf << "CalculateReferencesJob: createTargets: ignored " << reason_it.second
               << " '" << reason_it.first << "'";


    dbcontent_name = "ADSB";

    // MOPS version, NACp, NIC, PIC, SIL

    if (cache_->has(dbcontent_name))
    {
        for (auto& target_it : target_map)
        {
            for (auto& chain_it : target_it.second->chains())
            {
                if (get<0>(chain_it.first) != dbcontent_name) // skip non-tracker chains
                    continue;
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

//#include <valgrind/callgrind.h>

void CalculateReferencesJob::calculateReferences()
{
    loginf << "CalculateReferencesJob: calculateReferences";

    viewpoint_json_ = {};

    ViewPointGenerator viewpoint_gen;

    unsigned int num_targets = targets_.size();
    //num_targets = 50; // only calculate part

    std::vector<std::shared_ptr<Buffer>> results;

    results.resize(num_targets);

    //init viewpoints
    std::vector<ViewPointGenVP*> viewpoints(num_targets, nullptr);
    if (task_.generateViewPoints())
    {
        for (unsigned int i = 0; i < num_targets; ++i)
        {
            auto utn = targets_.at(i)->utn();
            std::string vp_name = "Reconstructor Result - UTN " + std::to_string(utn);

            //add viewpoint for target
            viewpoints[ i ] = viewpoint_gen.addViewPoint(vp_name, i, "Reconstructor", QRectF());

            //setup utn filter
            std::unique_ptr<ViewPointGenFilterUTN> utn_filter(new ViewPointGenFilterUTN(utn));
            viewpoints[ i ]->filters().addFilter(std::move(utn_filter));
        }
    }

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
            results[tgt_cnt] = targets_.at(tgt_cnt)->calculateReference(task_.settings(), viewpoints[tgt_cnt]);

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
        if (!buf_it->size())
            continue;

        if (!result_)
            result_ = move(buf_it);
        else
            result_->seizeBuffer(*buf_it);
    }

    //create viewpoint json
    viewpoint_json_ = viewpoint_gen.toJSON(true, true);

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
