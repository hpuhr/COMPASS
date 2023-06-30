#include "calculatereferencesjob.h"
#include "calculatereferencestask.h"
#include "calculatereferencesstatusdialog.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/target/target.h"
#include "stringconv.h"
#include "viewpointgenerator.h"
#include "datasourcemanager.h"
#include "util/number.h"

#include "util/tbbhack.h"

#include <QThread>

#include <future>
#include <cmath>

using namespace std;
using namespace Utils;
//using namespace nlohmann;
using namespace boost::posix_time;

string tracker_only_confirmed_positions_reason {"I062/080 CNF: Tentative or unknown"}; // non-tentative
string tracker_only_noncoasting_positions_reason {"I062/080 CST: Coasting or unknown"};
string tracker_only_report_detection_positions_reason {"I062/340 TYP: No detection or unknown"};
string tracker_only_report_detection_nonpsronly_positions_reason {
    "I062/080 MON: Mono sensor or unknown + I062/340 TYP: Single PSR detection or unknown"};
string tracker_only_high_accuracy_postions_reason {"I062/500 APC too high or unknown"};

string adsb_no_mops_reason {"I021/210 MOPS: unkown"};
string adsb_only_mops12_reason {"I021/210 MOPS: V0"}; // v 1/2 only
string adsb_only_high_nuc_nic_reason {"I021/090 NUCp or NIC: Too low or unknown"};
string adsb_only_high_nacp_reason {"I021/090 NACp: Too low or unknown"};
string adsb_only_high_sil_reason {"I021/090 SIL: Too low or unknown"};

CalculateReferencesJob::CalculateReferencesJob(CalculateReferencesTask& task, 
                                               CalculateReferencesStatusDialog& status_dialog,
                                               std::shared_ptr<dbContent::Cache> cache)
    : Job("CalculateReferencesJob"), task_(task), status_dialog_(status_dialog), cache_(cache)
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

    QMetaObject::invokeMethod(&status_dialog_, "setStatusSlot",
                              Qt::QueuedConnection,
                              Q_ARG(const std::string&, "Creating Targets"));

    //status_dialog_.setStatus("Creating Targets");

    createTargets();

    //status_dialog_.setStatus("Finalizing Targets");

    QMetaObject::invokeMethod(&status_dialog_, "setStatusSlot",
                              Qt::QueuedConnection,
                              Q_ARG(const std::string&, "Finalizing Targets"));

    finalizeTargets();

    //status_dialog_.setStatus("Calculating References");

    QMetaObject::invokeMethod(&status_dialog_, "setStatusSlot",
                              Qt::QueuedConnection,
                              Q_ARG(const std::string&, "Calculating References"));

    calculateReferences();

    //write references to db if desired
    if (task_.writeReferences())
    {
        //status_dialog_.setStatus("Writing References");

        QMetaObject::invokeMethod(&status_dialog_, "setStatusSlot",
                                  Qt::QueuedConnection,
                                  Q_ARG(const std::string&, "Writing References"));

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
    filterPositionUsage(target_map);

    // move targets from map to vec

    for (auto& tgt_it : target_map)
        targets_.emplace_back(move(tgt_it.second));

    loginf << "CalculateReferencesJob: createTargets: done for " << targets_.size()
           << ", reports num_skipped " << num_skipped
           << " num_unassoc " << num_unassoc << " num_assoc " << num_assoc;
}

void CalculateReferencesJob::filterPositionUsage(
        map<unsigned int, std::unique_ptr<CalculateReferences::Target>>& target_map)
{
    const CalculateReferencesTaskSettings& settings = task_.settings();

    if (!settings.filter_position_usage)
    {
        loginf << "CalculateReferencesJob: filterPositionUsage: disabled";
        return;
    }

    loginf << "CalculateReferencesJob: filterPositionUsage: assessing position data usage";

    unsigned int cat062_ignored_positions {0}, cat062_used_positions {0};
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
                                || (sqrt(pow(x_stddev_vec.get(index), 2) + pow(y_stddev_vec.get(index), 2))
                                    > settings.tracker_minimum_accuracy))
                        {
                            ignore_current_position = true;
                            tracker_ignored_reasons[tracker_only_high_accuracy_postions_reason] += 1;
                        }
                    }

                    ignore_positions.push_back(ignore_current_position);

                    if (ignore_current_position)
                        ++cat062_ignored_positions;
                    else
                        ++cat062_used_positions;
                }

                // set ignore positions flag
                chain_it.second->setIgnoredPositions(ignore_positions);
            }
        }
    }

    for (auto& reason_it : tracker_ignored_reasons)
        loginf << "CalculateReferencesJob: filterPositionUsage: ignored " << reason_it.second
               << " '" << reason_it.first << "'";

    loginf << "CalculateReferencesJob: filterPositionUsage: CAT062 data ignored pos " << cat062_ignored_positions
           << " used " << cat062_used_positions;

    used_pos_counts_[dbcontent_name] = {cat062_used_positions, cat062_ignored_positions};

    dbcontent_name = "CAT021";

    unsigned int cat021_ignored_positions {0}, cat021_used_positions {0};
    map<string, unsigned int> adsb_ignored_reasons;

    // MOPS version, NACp, NIC, PIC, SIL

    if (cache_->has(dbcontent_name))
    {
        vector<bool> ignore_positions;
        bool ignore_current_position;

        unsigned int buffer_size = cache_->get(dbcontent_name)->size();

        assert (cache_->hasVar<unsigned char>(dbcontent_name, DBContent::var_cat021_mops_version_));
        NullableVector<unsigned char>& mops_vec = cache_->getVar<unsigned char>(
                    dbcontent_name, DBContent::var_cat021_mops_version_);

        assert (cache_->hasVar<unsigned char>(dbcontent_name, DBContent::var_cat021_nacp_));
        NullableVector<unsigned char>& nacp_vec = cache_->getVar<unsigned char>(
                    dbcontent_name, DBContent::var_cat021_nacp_);

        assert (cache_->hasVar<unsigned char>(dbcontent_name, DBContent::var_cat021_nucp_nic_));
        NullableVector<unsigned char>& nucp_nic_vec = cache_->getVar<unsigned char>(
                    dbcontent_name, DBContent::var_cat021_nucp_nic_);

        assert (cache_->hasVar<unsigned char>(dbcontent_name, DBContent::var_cat021_sil_));
        NullableVector<unsigned char>& sil_vec = cache_->getVar<unsigned char>(
                    dbcontent_name, DBContent::var_cat021_sil_);

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

                    if (mops_vec.isNull(index))
                    {
                        ignore_current_position = true;
                        adsb_ignored_reasons[adsb_no_mops_reason] += 1;
                    }

                    if (!ignore_current_position && settings.adsb_only_v12_positions)
                    {
                        if (mops_vec.get(index) == 0)
                        {
                            ignore_current_position = true;
                            adsb_ignored_reasons[adsb_only_mops12_reason] += 1;
                        }
                    }

                    if (!ignore_current_position && settings.adsb_only_high_nacp_positions)
                    {
                        if (nacp_vec.isNull(index) || nacp_vec.get(index) < settings.adsb_minimum_nacp)
                        {
                            ignore_current_position = true;
                            adsb_ignored_reasons[adsb_only_high_nacp_reason] += 1;
                        }
                    }

                    if (!ignore_current_position && settings.adsb_only_high_nucp_nic_positions)
                    {
                        if (nucp_nic_vec.isNull(index) || nucp_nic_vec.get(index) < settings.adsb_minimum_nucp_nic)
                        {
                            ignore_current_position = true;
                            adsb_ignored_reasons[adsb_only_high_nuc_nic_reason] += 1;
                        }
                    }

                    if (!ignore_current_position && settings.adsb_only_high_sil_positions)
                    {
                        if (sil_vec.isNull(index) || sil_vec.get(index) < settings.adsb_minimum_sil)
                        {
                            ignore_current_position = true;
                            adsb_ignored_reasons[adsb_only_high_sil_reason] += 1;
                        }
                    }

                    ignore_positions.push_back(ignore_current_position);

                    if (ignore_current_position)
                        ++cat021_ignored_positions;
                    else
                        ++cat021_used_positions;
                }

                // set ignore positions flag
                chain_it.second->setIgnoredPositions(ignore_positions);
            }
        }
    }

    for (auto& reason_it : adsb_ignored_reasons)
        loginf << "CalculateReferencesJob: filterPositionUsage: ignored " << reason_it.second
               << " '" << reason_it.first << "'";

    loginf << "CalculateReferencesJob: filterPositionUsage: cat021 data ignored pos " << cat021_ignored_positions
           << " used " << cat021_used_positions;

    used_pos_counts_[dbcontent_name] = {cat021_used_positions, cat021_ignored_positions};


    QMetaObject::invokeMethod(&status_dialog_, "setUsedPositionCountsSlot",
                              Qt::QueuedConnection,
                              Q_ARG(CalculateReferencesStatusDialog::PositionCountsMap, used_pos_counts_));


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

            logdbg << "CalculateReferencesJob: calculateReferences: utn "
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

    // set info

    string dbcontent_name = "RefTraj";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    NullableVector<double>& x_stddev_vec = result_->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_x_stddev_).name());
    NullableVector<double>& y_stddev_vec = result_->get<double> (
                dbcontent_man.metaGetVariable(dbcontent_name, DBContent::meta_var_y_stddev_).name());

    unsigned int num_accs = 0;
    double acc;
    double acc_min = 0, acc_max = 0, acc_sum = 0;

    for (unsigned int cnt=0; cnt < result_->size(); ++cnt)
    {
        if (!x_stddev_vec.isNull(cnt) && !y_stddev_vec.isNull(cnt))
        {
            acc = sqrt(pow(x_stddev_vec.get(cnt),2) + pow(y_stddev_vec.get(cnt),2));
            acc_sum += acc;

            if (num_accs)
            {
                acc_min = min(acc_min, acc);
                acc_max = max(acc_max, acc);
            }
            else
            {
                acc_min = acc;
                acc_max = acc;
            }

            ++num_accs;
        }
    }

    CalculateReferencesStatusDialog::CalcInfoVector info;

    info.push_back({"Number of Targets", to_string(num_targets)});
    info.push_back({"Number of Reference Target Reports", to_string(result_->size())});

    if (num_accs)
    {
        double acc_avg = acc_sum / (double) num_accs;
        double acc_stddev = 0;

        for (unsigned int cnt=0; cnt < result_->size(); ++cnt)
        {
            if (!x_stddev_vec.isNull(cnt) && !y_stddev_vec.isNull(cnt))
            {
                acc = sqrt(pow(x_stddev_vec.get(cnt),2) + pow(y_stddev_vec.get(cnt),2));

                acc_stddev += pow(acc - acc_avg, 2); // store as variance
            }
        }

        acc_stddev /= num_accs;
        acc_stddev = sqrt(acc_stddev); // var to std dev

        info.push_back({"Accuracy Min [m]", String::doubleToStringPrecision(acc_min, 2)});
        info.push_back({"Accuracy Max [m]", String::doubleToStringPrecision(acc_max, 2)});

        info.push_back({"Accuracy Avg [m]", String::doubleToStringPrecision(acc_avg, 2)});
        info.push_back({"Accuracy StdDev [m]", String::doubleToStringPrecision(acc_stddev, 2)});
    }

    QMetaObject::invokeMethod(&status_dialog_, "setCalculateInfoSlot",
                              Qt::QueuedConnection,
                              Q_ARG(CalculateReferencesStatusDialog::CalcInfoVector, info));

    loginf << "CalculateReferencesJob: calculateReferences: done, buffer size " << result_->size();
}

void CalculateReferencesJob::writeReferences()
{
    string dbcontent_name = "RefTraj";

    const CalculateReferencesTaskSettings& settings = task_.settings();

    // config data source
    {
        DataSourceManager& src_man = COMPASS::instance().dataSourceManager();

        unsigned int ds_id = Number::dsIdFrom(settings.ds_sac, settings.ds_sic);

        if (!src_man.hasDBDataSource(ds_id))
            src_man.addNewDataSource(ds_id);

        assert (src_man.hasDBDataSource(ds_id));

        dbContent::DBDataSource& src = src_man.dbDataSource(ds_id);

        src.name(settings.ds_name);
        src.dsType(dbcontent_name); // same as dstype
    }

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

