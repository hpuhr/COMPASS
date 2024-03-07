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

#include "createassociationsjob.h"
#include "compass.h"
#include "buffer.h"
#include "createassociationstask.h"
#include "dbinterface.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/target/target.h"
#include "datasourcemanager.h"
#include "dbcontent/variable/metavariable.h"
#include "dbcontent/variable/variable.h"
#include "stringconv.h"
#include "projection/transformation.h"
//#include "evaluationmanager.h"
#include "util/timeconv.h"

#include "util/tbbhack.h"

#include <boost/thread/mutex.hpp>

#include <cassert>

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace boost::posix_time;

//bool CreateAssociationsJob::in_appimage_ = COMPASS::isAppImage();

CreateAssociationsJob::CreateAssociationsJob(CreateAssociationsTask& task, DBInterface& db_interface,
                                             std::shared_ptr<dbContent::DBContentAccessor> accessor)
    : Job("CreateAssociationsJob"), task_(task), db_interface_(db_interface), accessor_(accessor)
{
}

CreateAssociationsJob::~CreateAssociationsJob()
{
    logdbg << "CreateAssociationsJob: dtor";

    target_reports_.clear();

    logdbg << "CreateAssociationsJob: dtor: done";
}

void CreateAssociationsJob::run()
{
    logdbg << "CreateAssociationsJob: run: start";

    started_ = true;

    ptime start_time;
    ptime stop_time;

    start_time = microsec_clock::local_time();

    loginf << "CreateAssociationsJob: run: clearing associations";

    emit statusSignal("Clearing Previous Associations");
    removePreviousAssociations();

    // create target reports
    emit statusSignal("Creating Target Reports");
    createTargetReports();

    // create reference utns
    emit statusSignal("Creating Reference UTNs");
    std::map<unsigned int, Association::Target> targets = createReferenceUTNs();

    // create tracker utns
    emit statusSignal("Creating Tracker UTNs");
    createTrackerUTNs(targets);

    unsigned int multiple_associated {0};
    unsigned int single_associated {0};

    for (auto& target_it : targets)
    {
        if (target_it.second.ds_ids_.size() > 1)
            ++multiple_associated;
        else
            ++single_associated;
    }

    loginf << "CreateAssociationsJob: run: tracker targets " << targets.size()
           << " multiple " << multiple_associated << " single " << single_associated;

    // create non-tracker utns

    emit statusSignal("Creating non-Tracker UTNs");
    createNonTrackerUTNS(targets);

    multiple_associated = 0;
    single_associated = 0;

    for (auto& target_it : targets)
    {
        if (target_it.second.ds_ids_.size() > 1)
            ++multiple_associated;
        else
            ++single_associated;
    }

    loginf << "CreateAssociationsJob: run: after non-tracker targets " << targets.size()
           << " multiple " << multiple_associated << " single " << single_associated;

    // create associations
    emit statusSignal("Creating Associations");
    createAssociations();

    // save associations
    emit statusSignal("Saving Associations");

    saveAssociations();

    // save targets
    emit statusSignal("Saving Targets");

    saveTargets(targets);

    //    object_man.setAssociationsByAll(); // no specific dbo or data source

    loginf << "CreateAssociationsJob: run: clearing tmp data";
    targets.clear(); // removes from assoc target reports

    stop_time = microsec_clock::local_time();

    double load_time;
    time_duration diff = stop_time - start_time;
    load_time = diff.total_milliseconds() / 1000.0;

    loginf << "CreateAssociationsJob: run: done ("
           << String::doubleToStringPrecision(load_time, 2) << " s).";

    done_ = true;
}

std::map<std::string, std::pair<unsigned int, unsigned int> > CreateAssociationsJob::associationCounts() const
{
    return association_counts_;
}

void CreateAssociationsJob::removePreviousAssociations()
{
    loginf << "CreateAssociationsJob: removePreviousAssociations";

    for (auto& buf_it : *accessor_)
    {
        assert(accessor_->hasMetaVar<unsigned int>(buf_it.first, DBContent::meta_var_utn_));
        NullableVector<unsigned int>& utn_vec = accessor_->getMetaVar<unsigned int>(buf_it.first, DBContent::meta_var_utn_);

        utn_vec.setAllNull();
    }
}

void CreateAssociationsJob::createTargetReports()
{
    loginf << "CreateAssociationsJob: createTargetReports";

    using namespace dbContent;

    Association::TargetReport tr;

    for (auto& buf_it : *accessor_) // dbo name, buffer
    {
        string dbcontent_name = buf_it.first;

        shared_ptr<Buffer> buffer = buf_it.second;
        size_t buffer_size = buffer->size();

        assert (accessor_->hasMetaVar<unsigned long>(dbcontent_name, DBContent::meta_var_rec_num_));
        NullableVector<unsigned long>& rec_nums = accessor_->getMetaVar<unsigned long>(
                    dbcontent_name, DBContent::meta_var_rec_num_);

        assert (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_ds_id_));
        NullableVector<unsigned int>& ds_ids = accessor_->getMetaVar<unsigned int>(
                    dbcontent_name, DBContent::meta_var_ds_id_);

        assert (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_line_id_));
        NullableVector<unsigned int>& line_ids = accessor_->getMetaVar<unsigned int>(
                    dbcontent_name, DBContent::meta_var_line_id_);

        assert (accessor_->hasMetaVar<ptime>(dbcontent_name, DBContent::meta_var_timestamp_));
        NullableVector<ptime>& ts_vec = accessor_->getMetaVar<ptime>(
                    dbcontent_name, DBContent::meta_var_timestamp_);

        NullableVector<unsigned int>* tas {nullptr};
        if (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_acad_))
            tas = &accessor_->getMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_acad_);

        NullableVector<string>* tis {nullptr};
        if (accessor_->hasMetaVar<string>(dbcontent_name, DBContent::meta_var_acid_))
            tis = &accessor_->getMetaVar<string>(dbcontent_name, DBContent::meta_var_acid_);

        NullableVector<unsigned int>* tns {nullptr};
        if (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_track_num_))
            tns = &accessor_->getMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_track_num_);

        NullableVector<bool>* tr_ends {nullptr};
        if (accessor_->hasMetaVar<bool>(dbcontent_name, DBContent::meta_var_track_end_))
            tr_ends = &accessor_->getMetaVar<bool>(dbcontent_name, DBContent::meta_var_track_end_);

        assert (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_m3a_));
        NullableVector<unsigned int>& m3as = accessor_->getMetaVar<unsigned int>(
                    dbcontent_name, DBContent::meta_var_m3a_);

        assert (accessor_->hasMetaVar<float>(dbcontent_name, DBContent::meta_var_mc_));
        NullableVector<float>& mcs = accessor_->getMetaVar<float>(
                    dbcontent_name, DBContent::meta_var_mc_);

        NullableVector<float>* mcs_valid {nullptr};

        if (dbcontent_name == "CAT062")
        {
            assert (accessor_->hasVar<float>(dbcontent_name, DBContent::var_cat062_fl_measured_));
            mcs_valid = &accessor_->getVar<float>(
                        dbcontent_name, DBContent::var_cat062_fl_measured_);
        }

        assert (accessor_->hasMetaVar<double>(dbcontent_name, DBContent::meta_var_latitude_));
        NullableVector<double>& lats = accessor_->getMetaVar<double>(
                    dbcontent_name, DBContent::meta_var_latitude_);

        assert (accessor_->hasMetaVar<double>(dbcontent_name, DBContent::meta_var_longitude_));
        NullableVector<double>& longs = accessor_->getMetaVar<double>(
                    dbcontent_name, DBContent::meta_var_longitude_);

        NullableVector<unsigned char>* adsb_mops {nullptr};
        if (dbcontent_name == "CAT021")
        {
            assert (accessor_->hasVar<unsigned char>(dbcontent_name, DBContent::var_cat021_mops_version_));
            adsb_mops = &accessor_->getVar<unsigned char>(dbcontent_name, DBContent::var_cat021_mops_version_);
        }

        for (size_t cnt = 0; cnt < buffer_size; ++cnt)
        {
            assert (!rec_nums.isNull(cnt));
            assert (!ds_ids.isNull(cnt));
            assert (!line_ids.isNull(cnt));

            tr.dbcontent_name_ = dbcontent_name;
            tr.rec_num_ = rec_nums.get(cnt);
            tr.ds_id_ = ds_ids.get(cnt);
            tr.line_id_ = line_ids.get(cnt);

            if (ts_vec.isNull(cnt))
            {
                logwrn << "CreateAssociationsJob: createTargetReports: target report w/o time: dbcont "
                       << dbcontent_name << " rec_num " << tr.rec_num_  << " ds_id " << tr.ds_id_;
                continue;
            }

            if (ts_vec.isNull(cnt))
            {
                logwrn << "CreateAssociationsJob: createTargetReports: target report w/o time: dbcont "
                       << dbcontent_name << " rec_num " << tr.rec_num_  << " ds_id " << tr.ds_id_;
                continue;
            }

            if (lats.isNull(cnt))
            {
                logwrn << "CreateAssociationsJob: createTargetReports: target report w/o latitude: dbcont "
                       << dbcontent_name << " rec_num " << tr.rec_num_  << " ds_id " << tr.ds_id_;
                continue;
            }
            if (longs.isNull(cnt))
            {
                logwrn << "CreateAssociationsJob: createTargetReports: target report w/o longitude: dbcont "
                       << dbcontent_name << " rec_num " << tr.rec_num_  << " ds_id " << tr.ds_id_;
                continue;
            }

            tr.timestamp_ = ts_vec.get(cnt);

            tr.has_ta_ = tas && !tas->isNull(cnt);
            tr.ta_ = tr.has_ta_ ? tas->get(cnt) : 0;

            tr.has_ti_ = tis && !tis->isNull(cnt);
            tr.ti_ = tr.has_ti_ ? tis->get(cnt) : "";

            tr.has_tn_ = tns && !tns->isNull(cnt);
            tr.tn_ = tr.has_tn_ ? tns->get(cnt) : 0;

            tr.has_track_end_ = tr_ends && !tr_ends->isNull(cnt);
            tr.track_end_ = tr.has_track_end_ ? tr_ends->get(cnt) : false;

            tr.has_ma_ = !m3as.isNull(cnt);
            tr.ma_ = tr.has_ma_ ? m3as.get(cnt) : 0;

            tr.has_ma_v_ = false; // TODO
            tr.has_ma_g_ = false; // TODO

            if (mcs_valid && !mcs_valid->isNull(cnt))
            {
                tr.has_mc_ = true;
                tr.mc_ = mcs_valid->get(cnt);
            }
            else
            {
                tr.has_mc_ = !mcs.isNull(cnt);
                tr.mc_ = tr.has_mc_ ? mcs.get(cnt) : 0;
            }

            tr.has_mc_v_ = false; // TODO

            tr.latitude_ = lats.get(cnt);
            tr.longitude_ = longs.get(cnt);

            if (adsb_mops && !adsb_mops->isNull(cnt))
            {
                tr.has_adsb_info_ = true;
                tr.has_mops_version_ = true;
                tr.mops_version_ = adsb_mops->get(cnt);
            }
            else
            {
                tr.has_adsb_info_ = false;
                tr.has_mops_version_ = false;
                tr.mops_version_ = 0;
            }

            target_reports_[dbcontent_name][tr.ds_id_].push_back(tr);
        }
    }
}

std::map<unsigned int, Association::Target> CreateAssociationsJob::createReferenceUTNs()
{
    loginf << "CreateAssociationsJob: createReferenceUTNs";

    std::map<unsigned int, Association::Target> sum_targets;

    if (!target_reports_.count("RefTraj"))
    {
        loginf << "CreateAssociationsJob: createReferenceUTNs: no tracker data";
        return sum_targets;
    }

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    // create utn for all tracks
    for (auto& ds_it : target_reports_.at("RefTraj")) // ds_id->trs
    {
        loginf << "CreateAssociationsJob: createReferenceUTNs: processing ds_id " << ds_it.first;

        assert (ds_man.hasDBDataSource(ds_it.first));
        string ds_name = ds_man.dbDataSource(ds_it.first).name();

        loginf << "CreateAssociationsJob: createReferenceUTNs: creating tmp targets for ds_id " << ds_it.first;

        emit statusSignal(("Creating new "+ds_name+" UTNs").c_str());

        map<unsigned int, Association::Target> tracker_targets = createTrackedTargets("RefTraj", ds_it.first);

        if (!tracker_targets.size())
        {
            logwrn << "CreateAssociationsJob: createReferenceUTNs: ref ds_id " << ds_it.first
                   << " created no utns";
            continue;
        }

        loginf << "CreateAssociationsJob: createReferenceUTNs: cleaning new utns for ds_id " << ds_it.first;

        emit statusSignal(("Cleaning new "+ds_name+" Targets").c_str());

        cleanTrackerUTNs (tracker_targets);

        loginf << "CreateAssociationsJob: createReferenceUTNs: creating new utns for ds_id " << ds_it.first;

        emit statusSignal(("Creating new "+ds_name+" Targets").c_str());

        addTrackerUTNs (ds_name, move(tracker_targets), sum_targets);

        // try to associate targets to each other

        loginf << "CreateAssociationsJob: createReferenceUTNs: processing ds_id " << ds_it.first << " done";

        emit statusSignal("Checking Sum Targets");
        cleanTrackerUTNs(sum_targets);
    }

    emit statusSignal("Self-associating Sum Reference Targets");
    map<unsigned int, Association::Target> final_targets = selfAssociateTrackerUTNs(sum_targets);

    emit statusSignal("Checking Final Reference Targets");
    cleanTrackerUTNs(final_targets);

    markDubiousUTNs (final_targets);

    return final_targets;
}


void CreateAssociationsJob::createTrackerUTNs(std::map<unsigned int, Association::Target>& sum_targets)
{
    loginf << "CreateAssociationsJob: createTrackerUTNs";

    //std::map<unsigned int, Association::Target> sum_targets;

    if (!target_reports_.count("CAT062"))
    {
        loginf << "CreateAssociationsJob: createTrackerUTNs: no tracker data";
        return;
    }

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    // create utn for all tracks
    for (auto& ds_it : target_reports_.at("CAT062")) // ds_id->trs
    {
        loginf << "CreateAssociationsJob: createTrackerUTNs: processing ds_id " << ds_it.first;

        assert (ds_man.hasDBDataSource(ds_it.first));
        string ds_name = ds_man.dbDataSource(ds_it.first).name();

        loginf << "CreateAssociationsJob: createTrackerUTNs: creating tmp targets for ds_id " << ds_it.first;

        emit statusSignal(("Creating new "+ds_name+" UTNs").c_str());

        map<unsigned int, Association::Target> tracker_targets = createTrackedTargets("CAT062", ds_it.first);

        if (!tracker_targets.size())
        {
            logwrn << "CreateAssociationsJob: createTrackerUTNs: tracker ds_id " << ds_it.first
                   << " created no utns";
            continue;
        }

        loginf << "CreateAssociationsJob: createTrackerUTNs: cleaning new utns for ds_id " << ds_it.first;

        emit statusSignal(("Cleaning new "+ds_name+" Targets").c_str());

        cleanTrackerUTNs (tracker_targets);

        loginf << "CreateAssociationsJob: createTrackerUTNs: creating new utns for ds_id " << ds_it.first;

        emit statusSignal(("Creating new "+ds_name+" Targets").c_str());

        addTrackerUTNs (ds_name, move(tracker_targets), sum_targets);

        // try to associate targets to each other

        loginf << "CreateAssociationsJob: createTrackerUTNs: processing ds_id " << ds_it.first << " done";

        emit statusSignal("Checking Sum Targets");
        cleanTrackerUTNs(sum_targets);
    }

    emit statusSignal("Self-associating Sum Targets");
    sum_targets = selfAssociateTrackerUTNs(sum_targets);

    emit statusSignal("Checking Final Targets");
    cleanTrackerUTNs(sum_targets);

    markDubiousUTNs (sum_targets);

    return;
}

void CreateAssociationsJob::createNonTrackerUTNS(std::map<unsigned int, Association::Target>& targets)
{
    loginf << "CreateAssociationsJob: createNonTrackerUTNS";

    unsigned int num_data_sources = 0;

    for (auto& dbo_it : target_reports_)
    {
        if (dbo_it.first == "RefTraj" || dbo_it.first == "CAT062") // already associated
            continue;

        num_data_sources += dbo_it.second.size();
    }

    loginf << "CreateAssociationsJob: createNonTrackerUTNS: num_data_sources " << num_data_sources;

    // get ta lookup map
    std::map<unsigned int, unsigned int> ta_2_utn = getTALookupMap(targets);

    //DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    const bool associate_non_mode_s = task_.associateNonModeS();
    const time_duration max_time_diff_sensor = Time::partialSeconds(task_.maxTimeDiffSensor());
    const double max_altitude_diff_sensor = task_.maxAltitudeDiffSensor();
    const double max_distance_acceptable_sensor = task_.maxDistanceAcceptableSensor();

    unsigned int ds_cnt = 0;
    unsigned int done_perc;

    const std::set<unsigned int> mode_a_conspic = task_.modeAConspicuityCodes();

    for (auto& dbo_it : target_reports_)
    {
        if (dbo_it.first == "RefTraj" || dbo_it.first == "CAT062") // already associated
            continue;

        for (auto& ds_it : dbo_it.second) // ds_id -> trs
        {
            loginf << "CreateAssociationsJob: createNonTrackerUTNS: ds " << ds_it.first;

            assert (num_data_sources);
            done_perc = (unsigned int)(100.0 * (float)ds_cnt/(float)num_data_sources);
            assert (ds_man.hasDBDataSource(ds_it.first));

            string ds_name = ds_man.dbDataSource(ds_it.first).name();

            emit statusSignal(("Creating "+dbo_it.first+" "+ds_name+" UTNs ("+to_string(done_perc)+"%)").c_str());

            std::vector<Association::TargetReport>& target_reports = ds_it.second;
            unsigned int num_target_reports = target_reports.size();
            vector<int> tmp_assoc_utns; // tr_cnt -> utn
            tmp_assoc_utns.resize(num_target_reports);

            map<unsigned int, vector<Association::TargetReport*>> create_todos; // ta -> trs
            boost::mutex create_todos_mutex;

            //for (unsigned int tr_cnt=0; tr_cnt < num_target_reports; ++tr_cnt)
            tbb::parallel_for(uint(0), num_target_reports, [&](unsigned int tr_cnt)
            {
                Association::TargetReport& tr_it = target_reports[tr_cnt];

                tmp_assoc_utns[tr_cnt] = -1; // set as not associated

                if (tr_it.has_ta_ && ta_2_utn.count(tr_it.ta_)) // check ta with lookup
                {
                    unsigned int tmp_utn = ta_2_utn.at(tr_it.ta_);

                    assert (targets.count(tmp_utn));
                    tmp_assoc_utns[tr_cnt] = tmp_utn;
                    return;
                }

                // lookup by mode s failed

                if (tr_it.has_ta_) // create new utn if tr has ta
                    //  && (!tr_it.has_ma_ || mode_a_conspic.count(tr_it.ma_))  and can not be associated using mode a
                {
                    boost::mutex::scoped_lock lock(create_todos_mutex);
                    create_todos[tr_it.ta_].push_back(&tr_it);

                    return;
                }

                // tr non mode s

                if (!associate_non_mode_s)
                    return;

                ptime timestamp;
                vector<tuple<bool, unsigned int, double>> results;
                // usable, utn, distance

                results.resize(targets.size());

                timestamp = tr_it.timestamp_;

                dbContent::TargetPosition tst_pos;

                tst_pos.latitude_ = tr_it.latitude_;
                tst_pos.longitude_ = tr_it.longitude_;
                tst_pos.has_altitude_ = tr_it.has_mc_;
                tst_pos.altitude_ = tr_it.mc_;

                FixedTransformation trafo (tst_pos.latitude_, tst_pos.longitude_);

                //loginf << "UGA: checking tr a/c/pos";

                double x_pos, y_pos;
                double distance;

                dbContent::TargetPosition ref_pos;
                bool ok;

                unsigned int target_cnt=0;
                for (auto& target_it : targets)
                {
                    Association::Target& other = target_it.second;

                    results[target_cnt] = tuple<bool, unsigned int, double>(false, other.utn_, 0);

                    if ((tr_it.has_ta_ && other.hasTA())) // only try if not both mode s
                    {
                        ++target_cnt;
                        continue;
                    }

                    if (!other.isTimeInside(timestamp))
                    {
                        ++target_cnt;
                        continue;
                    }

                    if (tr_it.has_ma_ || tr_it.has_mc_) // mode a/c based
                    {
                        // check mode a code

                        if (tr_it.has_ma_)
                        {
                            Association::CompareResult ma_res = other.compareModeACode(
                                        tr_it.has_ma_, tr_it.ma_, timestamp, max_time_diff_sensor);

                            if (ma_res == Association::CompareResult::DIFFERENT)
                            {
                                target_cnt++;
                                continue;
                            }
                        }
                        //loginf << "UGA3 same mode a";

                        // check mode c code
                        if (tr_it.has_mc_)
                        {
                            Association::CompareResult mc_res = other.compareModeCCode(
                                        tr_it.has_mc_, tr_it.mc_, timestamp,
                                        max_time_diff_sensor, max_altitude_diff_sensor, false);

                            if (mc_res == Association::CompareResult::DIFFERENT)
                            {
                                target_cnt++;
                                continue;
                            }
                        }
                    }

                    // check positions

                    tie(ref_pos, ok) = other.interpolatedPosForTimeFast(timestamp, max_time_diff_sensor);

                    tie(ok, x_pos, y_pos) = trafo.distanceCart(ref_pos.latitude_, ref_pos.longitude_);

                    if (!ok)
                    {

                        loginf << "UGA3 NOT OK";
                        ++target_cnt;
                        continue;
                    }

                    distance = sqrt(pow(x_pos,2)+pow(y_pos,2));

                    //loginf << "UGA3 distance " << distance;

                    if (distance < max_distance_acceptable_sensor)
                        results[target_cnt] = tuple<bool, unsigned int, double>(true, other.utn_, distance);

                    ++target_cnt;
                }

                // find best match
                bool usable;
                unsigned int other_utn;

                bool first = true;
                unsigned int best_other_utn;
                double best_distance;

                for (auto& res_it : results) // usable, other utn, num updates, avg distance
                {
                    tie(usable, other_utn, distance) = res_it;

                    if (!usable)
                        continue;

                    if (first || distance < best_distance)
                    {
                        best_other_utn = other_utn;
                        best_distance = distance;

                        first = false;
                    }
                }

                if (!first)
                {
                    tmp_assoc_utns[tr_cnt] = best_other_utn;
                }
            });

            emit statusSignal(("Creating "+dbo_it.first+" "+ds_name+" Associations ("
                               +to_string(done_perc)+"%)").c_str());

            // create associations
            int tmp_utn;
            for (unsigned int tr_cnt=0; tr_cnt < num_target_reports; ++tr_cnt) // tr_cnt -> utn
            {
                tmp_utn = tmp_assoc_utns.at(tr_cnt);
                if (tmp_utn != -1)
                {
                    assert (targets.count(tmp_utn));
                    targets.at(tmp_utn).addAssociated(&target_reports.at(tr_cnt));
                }
            }

            // create new targets
            for (auto& todo_it : create_todos) // ta -> trs
            {
                vector<Association::TargetReport*>& trs = todo_it.second;
                assert (trs.size());

                unsigned int new_utn;

                if (targets.size())
                    new_utn = targets.rbegin()->first + 1;
                else
                    new_utn = 0;

                //addTargetByTargetReport(*trs.at(0));

                targets.emplace(
                            std::piecewise_construct,
                            std::forward_as_tuple(new_utn),   // args for key
                            std::forward_as_tuple(new_utn, false));  // args for mapped value

                if (trs.at(0)->has_ta_)
                    ta_2_utn[trs.at(0)->ta_] = {new_utn};

                targets.at(new_utn).addAssociated(trs.at(0));

                for (unsigned int tr_cnt=1; tr_cnt < trs.size(); ++tr_cnt)
                {
                    assert (targets.count(new_utn));
                    targets.at(new_utn).addAssociated(trs.at(tr_cnt));
                }
            }

            ++ds_cnt;
        }
    }

    loginf << "CreateAssociationsJob: createNonTrackerUTNS: done";
}

void CreateAssociationsJob::createAssociations()
{
    loginf << "CreateAssociationsJob: createAssociations";

    for (auto& dbo_it : target_reports_)
    {
        for (auto& ds_it : dbo_it.second) // ds_id -> trs
        {
            for (auto& tr_it : ds_it.second)
            {
                for (auto& utn_ptr_it : tr_it.assoc_targets_)
                {
                    //dbo.addAssociation(tr_it.rec_num_, utn_ptr_it->utn_, false, 0);

                    associations_[dbo_it.first][tr_it.rec_num_] =
                            std::make_tuple(utn_ptr_it->utn_, std::vector<std::pair<std::string, unsigned long>>());
                }
            }
        }
    }
}

void CreateAssociationsJob::saveAssociations()
{
    loginf << "CreateAssociationsJob: saveAssociations";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    // write association info to buffers

    unsigned long rec_num;

    for (auto& cont_assoc_it : associations_) // dbcontent -> rec_nums
    {
        unsigned int num_associated {0};
        unsigned int num_not_associated {0};

        string dbcontent_name = cont_assoc_it.first;
        std::map<unsigned long,
                std::tuple<unsigned int, std::vector<std::pair<std::string, unsigned long>>>>& associations
                = cont_assoc_it.second;

        loginf << "CreateAssociationsJob: saveAssociations: db content " << dbcontent_name;

        assert (accessor_->hasMetaVar<unsigned long>(dbcontent_name, DBContent::meta_var_rec_num_));
        NullableVector<unsigned long>& rec_num_vec = accessor_->getMetaVar<unsigned long>(
                    dbcontent_name, DBContent::meta_var_rec_num_);

        assert (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_utn_));
        NullableVector<unsigned int>& assoc_vec = accessor_->getMetaVar<unsigned int>(
                    dbcontent_name, DBContent::meta_var_utn_);

        assert (accessor_->has(dbcontent_name));
        unsigned int buffer_size = accessor_->get(dbcontent_name)->size();

        for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
        {
            assert (!rec_num_vec.isNull(cnt));

            rec_num = rec_num_vec.get(cnt);

            if (associations.count(rec_num))
            {
                //if (assoc_vec.isNull(cnt))
                    assoc_vec.set(cnt, get<0>(associations.at(rec_num)));
                //else
                    //assoc_vec.getRef(cnt).push_back(get<0>(associations.at(rec_num)));

                ++num_associated;
            }
            else
                ++num_not_associated;
        }

        association_counts_[dbcontent_name] = {buffer_size, num_associated};

        loginf << "CreateAssociationsJob: saveAssociations: dcontent " << dbcontent_name
               <<  " assoc " << num_associated << " not assoc " << num_not_associated;
    }

    // delete all data from buffer except rec_nums and associations, rename to db column names
    for (auto& buf_it : *accessor_)
    {
        string dbcontent_name = buf_it.first;

        string rec_num_var_name =
                dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name).name();
        string rec_num_col_name =
                dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name).dbColumnName();

        string utn_var_name =
                dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).getFor(dbcontent_name).name();
        string utn_col_name =
                dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).getFor(dbcontent_name).dbColumnName();

        PropertyList properties = buf_it.second->properties();

        for (auto& prop_it : properties.properties())
        {
            if (prop_it.name() == rec_num_var_name)
                buf_it.second->rename<unsigned long>(rec_num_var_name, rec_num_col_name);
            else if (prop_it.name() == utn_var_name)
                buf_it.second->rename<unsigned int>(utn_var_name, utn_col_name);
            else
                buf_it.second->deleteProperty(prop_it);
        }
    }

    // actually save data, ok since DB job
    for (auto& buf_it : *accessor_)
    {
        string dbcontent_name = buf_it.first;

        loginf << "CreateAssociationsJob: saveAssociations: saving for " << dbcontent_name;

        DBContent& dbcontent = dbcontent_man.dbContent(buf_it.first);
        dbContent::Variable& key_var =
                dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name);

        unsigned int chunk_size = 50000;

        unsigned int steps = buf_it.second->size() / chunk_size;

        unsigned int index_from = 0;
        unsigned int index_to = 0;

        for (unsigned int cnt = 0; cnt <= steps; cnt++)
        {
            index_from = cnt * chunk_size;
            index_to = index_from + chunk_size;

            if (index_to > buf_it.second->size() - 1)
                index_to = buf_it.second->size() - 1;

            loginf << "CreateAssociationsJob: saveAssociations: step " << cnt << " steps " << steps << " from "
                   << index_from << " to " << index_to;

            db_interface_.updateBuffer(dbcontent.dbTableName(), key_var.dbColumnName(),
                                       buf_it.second, index_from, index_to);

        }
    }

    loginf << "CreateAssociationsJob: saveAssociations: done";
}

void CreateAssociationsJob::saveTargets(std::map<unsigned int, Association::Target>& targets)
{
    loginf << "CreateAssociationsJob: saveTargets";

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();

    cont_man.clearTargetsInfo();

    for (auto& tgt_it : targets)
    {
        cont_man.createNewTarget(tgt_it.first);

        dbContent::Target& target = cont_man.target(tgt_it.first);

        target.useInEval(tgt_it.second.use_in_eval_);

        if (tgt_it.second.comment_.size())
            target.comment(tgt_it.second.comment_);

        target.aircraftAddresses(tgt_it.second.tas_);
        target.aircraftIdentifications(tgt_it.second.ids_);
        target.modeACodes(tgt_it.second.mas_);

        if (tgt_it.second.has_timestamps_)
        {
            target.timeBegin(tgt_it.second.timestamp_min_);
            target.timeEnd(tgt_it.second.timestamp_max_);
        }

        if (tgt_it.second.has_mode_c_)
            target.modeCMinMax(tgt_it.second.mode_c_min_, tgt_it.second.mode_c_max_);

        // set counts
        for (auto& count_it : tgt_it.second.getDBContentCounts())
            target.dbContentCount(count_it.first, count_it.second);

        // set adsb stuff
        if (tgt_it.second.hasADSBMOPSVersion() && tgt_it.second.getADSBMOPSVersions().size())
            target.adsbMOPSVersions(tgt_it.second.getADSBMOPSVersions());
    }

    cont_man.saveTargets();

    loginf << "CreateAssociationsJob: saveTargets: done";
}

std::map<unsigned int, Association::Target> CreateAssociationsJob::createTrackedTargets(
        const std::string& dbcontent_name, unsigned int ds_id)
{
    map<unsigned int, Association::Target> tracker_targets; // utn -> target

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    assert (ds_man.hasDBDataSource(ds_id));
    string ds_name = ds_man.dbDataSource(ds_id).name();

    std::map<unsigned int, std::vector<Association::TargetReport>>& ds_id_trs = target_reports_.at(dbcontent_name);

    if (!ds_id_trs.count(ds_id))
    {
        loginf << "CreateAssociationsJob: createPerTrackerTargets: ds " << ds_name << " has not target reports";
        return tracker_targets;
    }

    bool attached_to_existing_utn;
    unsigned int tmp_utn_cnt {0};
    unsigned int utn;

    bool use_non_mode_s = task_.associateNonModeS();

    // iterate over lines
    for (unsigned int line_cnt = 0; line_cnt < 4; line_cnt++)
    {
        map<unsigned int, pair<unsigned int, ptime>> tn2utn; // track num -> utn, last tod

        // create temporary targets
        for (auto& tr_it : ds_id_trs.at(ds_id))
        {
            if (tr_it.line_id_ != line_cnt) // check for current line
                continue;

            if (tr_it.has_tn_) // has track number
            {
                if (!tn2utn.count(tr_it.tn_)) // if not yet mapped to utn
                {
                    attached_to_existing_utn = false;

                    // check if can be attached to already existing utn
                    if (!tr_it.has_ta_ && use_non_mode_s) // not for mode-s targets
                    {
                        int cont_utn = findContinuationUTNForTrackerUpdate(tr_it, tracker_targets);

                        if (cont_utn != -1)
                        {
                            logdbg << "CreateAssociationsJob: createPerTrackerTargets: continuing target "
                                   << cont_utn << " with tn " << tr_it.tn_ << " at time "
                                   << Time::toString(tr_it.timestamp_);
                            tn2utn[tr_it.tn_] = {cont_utn, tr_it.timestamp_};
                            attached_to_existing_utn = true;
                        }
                    }

                    if (!attached_to_existing_utn)
                    {
                        logdbg << "CreateAssociationsJob: createPerTrackerTargets: registering new tmp target "
                               << tmp_utn_cnt << " for tn " << tr_it.tn_;

                        tn2utn[tr_it.tn_] = {tmp_utn_cnt, tr_it.timestamp_};
                        ++tmp_utn_cnt;
                    }
                }

                //loginf << "UGA1";
                if (tracker_targets.count(tn2utn.at(tr_it.tn_).first)) // additional checks if already exists
                {
                    Association::Target& existing_target = tracker_targets.at(tn2utn.at(tr_it.tn_).first);

                    if (tr_it.has_ta_ && existing_target.hasTA() // new target part if ta change
                            && !existing_target.hasTA(tr_it.ta_))
                    {
                        logdbg << "CreateAssociationsJob: createPerTrackerTargets: registering new tmp target "
                               << tmp_utn_cnt << " for tn " << tr_it.tn_ << " because of ta switch "
                               << " at " << Time::toString(tr_it.timestamp_)
                               << " existing " << existing_target.asStr()
                               << " tr " << tr_it.asStr();

                        tn2utn[tr_it.tn_] = {tmp_utn_cnt, tr_it.timestamp_};
                        ++tmp_utn_cnt;
                    }
                }
                //loginf << "UGA2";

                if (tn2utn.at(tr_it.tn_).second > tr_it.timestamp_)
                {
                    logwrn << "CreateAssociationsJob: createPerTrackerTargets: tod backjump -"
                           << Time::toString(tn2utn.at(tr_it.tn_).second - tr_it.timestamp_)
                           << " tmp target " << tmp_utn_cnt << " at tr " << tr_it.asStr() << " tn " << tr_it.tn_;
                }
                assert (tn2utn.at(tr_it.tn_).second <= tr_it.timestamp_);

                //loginf << "UGA3";

                if ((tr_it.timestamp_ - tn2utn.at(tr_it.tn_).second).total_seconds() > 60.0) // gap, new track // TODO parameter
                {
                    logdbg << "CreateAssociationsJob: createPerTrackerTargets: registering new tmp target "
                           << tmp_utn_cnt << " for tn " << tr_it.tn_ << " because of gap "
                           << Time::toString(tr_it.timestamp_ - tn2utn.at(tr_it.tn_).second)
                           << " at " << Time::toString(tr_it.timestamp_);

                    tn2utn[tr_it.tn_] = {tmp_utn_cnt, tr_it.timestamp_};
                    ++tmp_utn_cnt;
                }

                //loginf << "UGA4";

                assert (tn2utn.count(tr_it.tn_));
                utn = tn2utn.at(tr_it.tn_).first;
                tn2utn.at(tr_it.tn_).second = tr_it.timestamp_;

                if (!tracker_targets.count(utn)) // add new target if not existing
                {
                    logdbg << "CreateAssociationsJob: createPerTrackerTargets: creating new tmp target " << utn;

                    tracker_targets.emplace(
                                std::piecewise_construct,
                                std::forward_as_tuple(utn),   // args for key
                                std::forward_as_tuple(utn, true));  // args for mapped value
                }

                tracker_targets.at(utn).addAssociated(&tr_it);
            }
            else
            {
                logwrn << "CreateAssociationsJob: createPerTrackerTargets: tracker target report w/o track num in ds_id "
                       << tr_it.ds_id_ << " at tod " << Time::toString(tr_it.timestamp_);
            }
        }

    }

    return tracker_targets;
}

void CreateAssociationsJob::cleanTrackerUTNs(std::map<unsigned int, Association::Target>& targets)
{
    bool clean_dubious_utns = task_.cleanDubiousUtns();
    float max_speed_kts = task_.maxSpeedTrackerKts();

    unsigned int targets_size = targets.size();

    vector<unsigned int> index_to_utn;

    for (auto& t_it : targets)
        index_to_utn.push_back(t_it.first);

    tbb::parallel_for(uint(0), targets_size, [&](unsigned int cnt)
    {
        targets.at(index_to_utn.at(cnt)).calculateSpeeds();
    });

    for (auto& target_it : targets)
    {
        if (target_it.second.has_speed_ && target_it.second.speed_max_ > max_speed_kts)
        {
            loginf << "CreateAssociationsJob: cleanTrackerUTNs: new target dubious "
                   << target_it.second.utn_ << ": calculateSpeeds: min "
                   << String::doubleToStringPrecision(target_it.second.speed_min_,2)
                   << " avg " << String::doubleToStringPrecision(target_it.second.speed_avg_,2)
                   << " max " << String::doubleToStringPrecision(target_it.second.speed_max_,2) << " kts";

            if (clean_dubious_utns)
            {
                loginf << "CreateAssociationsJob: cleanTrackerUTNs: new target removing non-mode s"
                          " target reports";
                target_it.second.removeNonModeSTRs();

                target_it.second.calculateSpeeds();

                if (target_it.second.has_speed_)
                    loginf << "CreateAssociationsJob: cleanTrackerUTNs: cleaned new target "
                           << target_it.second.utn_ << ": calculateSpeeds: min "
                           << String::doubleToStringPrecision(target_it.second.speed_min_,2)
                           << " avg " << String::doubleToStringPrecision(target_it.second.speed_avg_,2)
                           << " max " << String::doubleToStringPrecision(target_it.second.speed_max_,2)
                           << " kts";
            }
        }
    }
}

std::map<unsigned int, Association::Target> CreateAssociationsJob::selfAssociateTrackerUTNs(
        std::map<unsigned int, Association::Target>& targets)
{
    loginf << "CreateAssociationsJob: selfAssociateTrackerUTNs: num targets " << targets.size();

    std::map<unsigned int, Association::Target> new_targets;

    while (targets.size())
    {
        pair<const unsigned int, Association::Target>& tgt_it = *targets.begin();

        loginf << "CreateAssociationsJob: selfAssociateTrackerUTNs: processing target utn " << tgt_it.first;

        int tmp_utn = findUTNForTrackerTarget(tgt_it.second, new_targets);

        if (tmp_utn == -1)
        {

            if (new_targets.size())
                tmp_utn = new_targets.rbegin()->first + 1;
            else
                tmp_utn = 0;

            loginf << "CreateAssociationsJob: selfAssociateTrackerUTNs: no associatble utn found,"
                      " keeping target as utn " << tmp_utn;

            new_targets.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(tmp_utn),   // args for key
                        std::forward_as_tuple(tmp_utn, false));  // args for mapped value
        }
        else
        {
            loginf << "CreateAssociationsJob: selfAssociateTrackerUTNs: associatble utn " << tmp_utn
                   << " found, associating";
        }

        // move to other map
        new_targets.at(tmp_utn).addAssociated(tgt_it.second.assoc_trs_);
        targets.erase(tgt_it.first);
    }


    loginf << "CreateAssociationsJob: selfAssociateTrackerUTNs: done with num targets " << targets.size();

    return new_targets;
}

void CreateAssociationsJob::markDubiousUTNs(std::map<unsigned int, Association::Target>& targets)
// only for final utns, must have calculated speeds
{
    bool mark_dubious_utns_unused = task_.markDubiousUtnsUnused();
    bool comment_dubious_utns = task_.commentDubiousUtns();
    float max_speed_kts = task_.maxSpeedTrackerKts();

    vector <unsigned int> still_dubious;

    for (auto& target_it : targets)
    {
        if (target_it.second.has_speed_ && target_it.second.speed_max_ > max_speed_kts)
            still_dubious.push_back(target_it.second.utn_);
    }

    for (unsigned int utn : still_dubious)
    {
        loginf << "CreateAssociationsJob: markDubiousUTNs: target " << utn << " still dubious"
               <<  " speed min " << String::doubleToStringPrecision(targets.at(utn).speed_min_,2)
                << " avg " << String::doubleToStringPrecision(targets.at(utn).speed_avg_,2)
                << " max " << String::doubleToStringPrecision(targets.at(utn).speed_max_,2) << " kts";

        if (mark_dubious_utns_unused)
            targets.at(utn).use_in_eval_ = false;

        if (comment_dubious_utns)
            targets.at(utn).comment_ =  "Dubious Association";
    }
}

void CreateAssociationsJob::addTrackerUTNs(const std::string& ds_name,
                                           std::map<unsigned int, Association::Target> from_targets,
                                           std::map<unsigned int, Association::Target>& to_targets)
{
    loginf << "CreateAssociationsJob: addTrackerUTNs: src " << ds_name
           << " from_targets size " << from_targets.size() << " to_targets size " << to_targets.size();

    int tmp_utn;

    float done_ratio;

    unsigned int target_cnt = 0;
    unsigned int from_targets_size = from_targets.size();

    while (from_targets.size())
    {
        done_ratio = (float)target_cnt / (float)from_targets_size;
        emit statusSignal(("Creating "+ds_name+" UTNs ("
                           +String::percentToString(100.0*done_ratio)+"%)").c_str());

        ++target_cnt;

        auto tmp_target = from_targets.begin();
        assert (tmp_target != from_targets.end());

        if (tmp_target->second.has_timestamps_)
        {
            logdbg << "CreateAssociationsJob: addTrackerUTNs: creating utn for tmp utn " << tmp_target->first;

            tmp_utn = findUTNForTrackerTarget(tmp_target->second, to_targets);

            logdbg << "CreateAssociationsJob: addTrackerUTNs: tmp utn " << tmp_target->first
                   << " tmp_utn " << tmp_utn;

            if (tmp_utn == -1) // none found, create new target
            {
                if (to_targets.size())
                    tmp_utn  = to_targets.rbegin()->first + 1;
                else
                    tmp_utn = 0;

                logdbg << "CreateAssociationsJob: addTrackerUTNs: tmp utn " << tmp_target->first
                       << " as new " << tmp_utn;

                // add the target
                to_targets.emplace(
                            std::piecewise_construct,
                            std::forward_as_tuple(tmp_utn),   // args for key
                            std::forward_as_tuple(tmp_utn, false));  // args for mapped value

                // add associated target reports
                to_targets.at(tmp_utn).addAssociated(tmp_target->second.assoc_trs_);
            }
            else // attach to existing target
            {
                //                        if (tmp_target->second.hasMA() && tmp_target->second.hasMA(396))
                //                            loginf << "CreateAssociationsJob: createTrackerUTNs: attaching utn " << tmp_target->first
                //                                   << " to tmp_utn " << tmp_utn;

                logdbg << "CreateAssociationsJob: addTrackerUTNs: tmp utn " << tmp_target->first
                       << " as existing " << tmp_utn;

                assert (to_targets.count(tmp_utn));
                to_targets.at(tmp_utn).addAssociated(tmp_target->second.assoc_trs_);
            }
        }

        // remove target
        from_targets.erase(tmp_target);
    }


    loginf << "CreateAssociationsJob: addTrackerUTNs: done with src " << ds_name
           << " to_targets size " << to_targets.size();
}

int CreateAssociationsJob::findContinuationUTNForTrackerUpdate (
        const Association::TargetReport& tr, const std::map<unsigned int, Association::Target>& targets)
// tries to find existing utn for tracker update, -1 if failed
{
    if (tr.has_ta_)
        return -1;

    const time_duration max_time_diff_tracker = Time::partialSeconds(task_.contMaxTimeDiffTracker());
    const double max_altitude_diff_tracker =
            task_.maxAltitudeDiffTracker();
    const double max_distance_acceptable_tracker = task_.contMaxDistanceAcceptableTracker();

    unsigned int num_targets = targets.size();

    vector<tuple<bool, unsigned int, double>> results;
    // usable, other utn, distance
    results.resize(num_targets);

    tbb::parallel_for(uint(0), num_targets, [&](unsigned int cnt)
    {
        const Association::Target& other = targets.at(cnt);
        Transformation trafo;

        results[cnt] = tuple<bool, unsigned int, double>(false, other.utn_, 0);

        if (!other.numAssociated()) // check if target has associated target reports
            return;

        if (other.hasTA()) // not for mode-s targets
            return;

        if (tr.timestamp_ <= other.timestamp_max_) // check if not recently updated
            return;

        // tr.tod_ > other.tod_max_
        if (tr.timestamp_ - other.timestamp_max_ > max_time_diff_tracker) // check if last updated longer ago than threshold
            return;

        const Association::TargetReport& other_last_tr = other.lastAssociated();

        if (!other_last_tr.has_track_end_ || !other_last_tr.track_end_) // check if other track was ended
            return;

        if (!other_last_tr.has_ma_ || !tr.has_ma_) // check mode a codes exist
            return;

        if (other_last_tr.ma_ != tr.ma_) // check mode-a
            return;

        // mode a codes the same

        if (other_last_tr.has_mc_ && tr.has_mc_
                && fabs(other_last_tr.mc_ - tr.mc_) > max_altitude_diff_tracker) // check mode c codes if existing
            return;

        bool ok;
        double x_pos, y_pos;
        double distance;

        tie(ok, x_pos, y_pos) = trafo.distanceCart(
                    other_last_tr.latitude_, other_last_tr.longitude_,
                    tr.latitude_, tr.longitude_);

        if (!ok)
            return;

        distance = sqrt(pow(x_pos,2)+pow(y_pos,2));

        if (distance > max_distance_acceptable_tracker)
            return;

        results[cnt] = tuple<bool, unsigned int, double>(
                    true, other.utn_, distance);
    });

    // find best match
    unsigned int num_matches = 0;

    bool usable;
    unsigned int other_utn;
    double distance;

    bool first = true;
    unsigned int best_other_utn;
    double best_distance;

    for (auto& res_it : results) // usable, other utn, distance
    {
        tie(usable, other_utn, distance) = res_it;

        if (!usable)
            continue;

        ++num_matches;

        if (first || distance < best_distance)
        {
            best_other_utn = other_utn;
            best_distance = distance;

            first = false;
        }
    }

    if (first)
        return -1;

    if (num_matches > 1)
    {
        logdbg << "CreateAssociationsJob: findContinuationUTNForTrackerUpdate: " << num_matches << " found";
        return -1;
    }

    logdbg << "CreateAssociationsJob: findContinuationUTNForTrackerUpdate: continuation match utn "
           << best_other_utn << " found, distance " << best_distance;

    return best_other_utn;
}

int CreateAssociationsJob::findUTNForTrackerTarget (const Association::Target& target,
                                                    const std::map<unsigned int, Association::Target>& targets)
// tries to find existing utn for target, -1 if failed
{
    if (!targets.size()) // check if targets exist
        return -1;

    int tmp_utn = findUTNForTargetByTA(target, targets);

    if (tmp_utn != -1) // either mode s, so
        return tmp_utn;

    if (!task_.associateNonModeS())
        return -1;

    // try to find by m a/c/pos
    bool print_debug_target = target.hasMA() && target.hasMA(3824);
    if (print_debug_target)
        loginf << "CreateAssociationsJob: findUTNForTrackerTarget: checking target " << target.utn_
               << " by mode a/c, pos";

    vector<tuple<bool, unsigned int, unsigned int, double>> results;
    // usable, other utn, num updates, avg distance

    unsigned int num_utns = targets.size();
    results.resize(num_utns);

    const double prob_min_time_overlap_tracker = task_.probMinTimeOverlapTracker();
    const time_duration max_time_diff_tracker = Time::partialSeconds(task_.maxTimeDiffTracker());
    const unsigned int min_updates_tracker = task_.minUpdatesTracker();
    const double max_altitude_diff_tracker = task_.maxAltitudeDiffTracker();
    const unsigned int max_positions_dubious_tracker = task_.maxPositionsDubiousTracker();
    const double max_distance_quit_tracker = task_.maxDistanceQuitTracker();
    const double max_distance_dubious_tracker = task_.maxDistanceDubiousTracker();
    const double max_distance_acceptable_tracker = task_.maxDistanceAcceptableTracker();

    tbb::parallel_for(uint(0), num_utns, [&](unsigned int cnt)
                      //for (unsigned int cnt=0; cnt < utn_cnt_; ++cnt)
    {
        const Association::Target& other = targets.at(cnt);
        Transformation trafo;

        results[cnt] = tuple<bool, unsigned int, unsigned int, double>(false, other.utn_, 0, 0);

        bool print_debug = target.hasMA() && target.hasMA(3824) && other.hasMA() && other.hasMA(3824);

        if (!(target.hasTA() && other.hasTA())) // only try if not both mode s
        {
            if (print_debug)
            {
                loginf << "\ttarget " << target.utn_ << " " << target.timeStr()
                       << " checking other " << other.utn_ << " " << other.timeStr()
                       << " overlaps " << target.timeOverlaps(other) << " prob " << target.probTimeOverlaps(other);
            }

            if (target.timeOverlaps(other) && target.probTimeOverlaps(other) >= prob_min_time_overlap_tracker)
            {
                if (print_debug)
                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_ << " overlap passed";

                vector<ptime> ma_unknown;
                vector<ptime> ma_same;
                vector<ptime> ma_different;

                tie (ma_unknown, ma_same, ma_different) = target.compareModeACodes(other, max_time_diff_tracker);

                if (print_debug)
                {
                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                           << " ma unknown " << ma_unknown.size()
                           << " same " << ma_same.size() << " diff " << ma_different.size();
                }

                if (ma_same.size() > ma_different.size() && ma_same.size() >= min_updates_tracker)
                {
                    if (print_debug)
                        loginf << "\ttarget " << target.utn_ << " other " << other.utn_ << " mode a check passed";

                    // check mode c codes

                    vector<ptime> mc_unknown;
                    vector<ptime> mc_same;
                    vector<ptime> mc_different;

                    tie (mc_unknown, mc_same, mc_different) = target.compareModeCCodes(
                                other, ma_same, max_time_diff_tracker, max_altitude_diff_tracker, print_debug);

                    if (print_debug)
                    {
                        loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                               << " ma same " << ma_same.size() << " diff " << ma_different.size()
                               << " mc same " << mc_same.size() << " diff " << mc_different.size();
                    }

                    if (mc_same.size() > mc_different.size() && mc_same.size() >= min_updates_tracker)
                    {
                        if (print_debug)
                            loginf << "\ttarget " << target.utn_ << " other " << other.utn_ << " mode c check passed";
                        // check positions

                        vector<pair<ptime, double>> same_distances;
                        double distances_sum {0};

                        unsigned int pos_dubious_cnt {0};

                        dbContent::TargetPosition tst_pos;

                        double x_pos, y_pos;
                        double distance;

                        dbContent::TargetPosition ref_pos;
                        bool ok;

                        for (auto tod_it : mc_same)
                        {
                            assert (target.hasDataForExactTime(tod_it));
                            tst_pos = target.posForExactTime(tod_it);

                            tie(ref_pos, ok) = other.interpolatedPosForTimeFast(tod_it, max_time_diff_tracker);

                            if (!ok)
                            {
                                if (print_debug)
                                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                           << " pos calc failed ";

                                continue;
                            }

                            tie(ok, x_pos, y_pos) = trafo.distanceCart(
                                        ref_pos.latitude_, ref_pos.longitude_,
                                        tst_pos.latitude_, tst_pos.longitude_);

                            if (!ok)
                            {
                                if (print_debug)
                                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                           << " pos calc failed ";

                                continue;
                            }

                            distance = sqrt(pow(x_pos,2)+pow(y_pos,2));

                            if (distance > max_distance_dubious_tracker)
                                ++pos_dubious_cnt;

                            if (distance > max_distance_quit_tracker)
                                // too far or dubious, quit
                            {
                                same_distances.clear();

                                if (print_debug)
                                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                           << " max distance failed "
                                           << distance << " > " << max_distance_quit_tracker;

                                break;
                            }

                            if (pos_dubious_cnt > max_positions_dubious_tracker)
                            {
                                same_distances.clear();

                                if (print_debug)
                                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                           << " pos dubious failed "
                                           << pos_dubious_cnt << " > " << max_positions_dubious_tracker;

                                break;
                            }

                            //loginf << "\tdist " << distance;

                            same_distances.push_back({tod_it, distance});
                            distances_sum += distance;
                        }

                        if (same_distances.size() >= min_updates_tracker)
                        {
                            double distance_avg = distances_sum / (float) same_distances.size();

                            if (distance_avg < max_distance_acceptable_tracker)
                            {
                                if (print_debug)
                                {
                                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                           << " next utn " << num_utns << " dist avg " << distance_avg
                                           << " num " << same_distances.size();
                                }

                                results[cnt] = tuple<bool, unsigned int, unsigned int, double>(
                                            true, other.utn_, same_distances.size(), distance_avg);
                            }
                            else
                            {
                                if (print_debug)
                                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                           << " distance_avg failed "
                                           << distance_avg << " < " << max_distance_acceptable_tracker;
                            }
                        }
                        else
                        {
                            if (print_debug)
                                loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                       << " same distances failed "
                                       << same_distances.size() << " < " << min_updates_tracker;
                        }
                    }
                    else
                    {
                        if (print_debug)
                            loginf << "\ttarget " << target.utn_ << " other " << other.utn_ << " mode c check failed";
                    }
                }
                else
                {
                    if (print_debug)
                        loginf << "\ttarget " << target.utn_ << " other " << other.utn_ << " mode a check failed";
                }
            }
            else
            {
                if (print_debug)
                    loginf << "\ttarget " << target.utn_ << " other " << other.utn_ << " no overlap";
            }
        }
    });
    //}

    // find best match
    bool usable;
    unsigned int other_utn;
    unsigned int num_updates;
    double distance_avg;
    double score;

    bool first = true;
    unsigned int best_other_utn;
    unsigned int best_num_updates;
    double best_distance_avg;
    double best_score;

    if (print_debug_target)
        loginf << "\ttarget " << target.utn_ << " checking results";

    for (auto& res_it : results) // usable, other utn, num updates, avg distance
    {
        tie(usable, other_utn, num_updates, distance_avg) = res_it;

        if (!usable)
        {
            //            if (print_debug_target)
            //                loginf << "\ttarget " << target.utn_ << " result utn " << other_utn << " not usable";
            continue;
        }

        score = (double)num_updates*(max_distance_acceptable_tracker-distance_avg);

        if (first || score > best_score)
        {
            if (print_debug_target)
                loginf << "\ttarget " << target.utn_ << " result utn " << other_utn
                       << " marked as best, score " << score;

            best_other_utn = other_utn;
            best_num_updates = num_updates;
            best_distance_avg = distance_avg;
            best_score = score;

            first = false;
        }
    }

    if (first)
    {
        if (print_debug_target)
            loginf << "\ttarget " << target.utn_ << " no match found";
        return -1;
    }
    else
    {
        if (print_debug_target)
            loginf << "\ttarget " << target.utn_ << " match found best other " << best_other_utn
                   << " best score " << fixed << best_score << " dist avg " << best_distance_avg
                   << " num " << best_num_updates;

        return best_other_utn;
    }
}

int CreateAssociationsJob::findUTNForTargetByTA (const Association::Target& target,
                                                 const std::map<unsigned int, Association::Target>& targets)
{
    if (!target.hasTA()) // cant be found
        return -1;

    for (auto& target_it : targets)
    {
        if (!target_it.second.hasTA()) // cant be checked
            continue;

        if (target_it.second.hasAnyOfTAs(target.tas_))
            return target_it.first;
    }

    return -1;
}

//void CreateAssociationsJob::addTarget (const Association::Target& target) // creates new utn, adds to targets_
//{
//    assert (findUTNForTargetByTA(target) == -1); // should have been added

//    targets_.emplace(
//                std::piecewise_construct,
//                std::forward_as_tuple(utn_cnt_),   // args for key
//                std::forward_as_tuple(utn_cnt_, false));  // args for mapped value

//    targets_.at(utn_cnt_).addAssociated(target.assoc_trs_);

//    ++utn_cnt_;
//}

//void CreateAssociationsJob::addTargetByTargetReport (Association::TargetReport& tr)
//{
//    targets_.emplace(
//                std::piecewise_construct,
//                std::forward_as_tuple(utn_cnt_),   // args for key
//                std::forward_as_tuple(utn_cnt_, false));  // args for mapped value

//    if (tr.has_ta_)
//        ta_2_utn_[tr.ta_] = {utn_cnt_};

//    targets_.at(utn_cnt_).addAssociated(&tr);

//    ++utn_cnt_;
//}

std::map<unsigned int, unsigned int> CreateAssociationsJob::getTALookupMap (
        const std::map<unsigned int, Association::Target>& targets)
{
    logdbg << "CreateAssociationsJob: getTALookupMap";

    std::map<unsigned int, unsigned int> ta_2_utn;

    for (auto& target_it : targets)
    {
        if (!target_it.second.hasTA())
            continue;

        assert (target_it.second.tas_.size() == 1);

        assert (!ta_2_utn.count(*target_it.second.tas_.begin()));

        ta_2_utn[*target_it.second.tas_.begin()] = target_it.second.utn_;
    }

    logdbg << "CreateAssociationsJob: getTALookupMap: done";

    return ta_2_utn;
}

