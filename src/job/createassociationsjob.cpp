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
#include "dbobject.h"
#include "dbobjectmanager.h"
#include "metadbovariable.h"
#include "dbovariable.h"
#include "stringconv.h"
#include "projection/transformation.h"

//#include <ogr_spatialref.h>

#include <tbb/tbb.h>

#include <boost/thread/mutex.hpp>

using namespace std;
using namespace Utils;

bool CreateAssociationsJob::in_appimage_ {getenv("APPDIR")};

CreateAssociationsJob::CreateAssociationsJob(CreateAssociationsTask& task, DBInterface& db_interface,
                                             std::map<std::string, std::shared_ptr<Buffer>> buffers)
    : Job("CreateAssociationsJob"), task_(task), db_interface_(db_interface), buffers_(buffers)
{
}

CreateAssociationsJob::~CreateAssociationsJob() {}

void CreateAssociationsJob::run()
{
    logdbg << "CreateAssociationsJob: run: start";

    Association::Target::max_time_diff_ = max_time_diff_;
    Association::Target::max_altitude_diff_ = max_altitude_diff_;

    started_ = true;

    boost::posix_time::ptime start_time;
    boost::posix_time::ptime stop_time;

    start_time = boost::posix_time::microsec_clock::local_time();

    loginf << "CreateARTASAssociationsJob: run: clearing associations";

    DBObjectManager& object_man = COMPASS::instance().objectManager();

    object_man.removeAssociations();

    // create target reports
    emit statusSignal("Creating Target Reports");
    createTargetReports();

    // create tracker utns
    emit statusSignal("Creating Tracker UTNs");
    createTrackerUTNS();

    unsigned int multiple_associated {0};
    unsigned int single_associated {0};

    for (auto& target_it : targets_)
    {
        if (target_it.second.ds_ids_.size() > 1)
            ++multiple_associated;
        else
            ++single_associated;
    }

    loginf << "CreateARTASAssociationsJob: run: tracker targets " << targets_.size()
           << " multiple " << multiple_associated << " single " << single_associated;

    // create non-tracker utns

    // prepare lookup map
    ta_2_utn_.clear();
    for (auto& target_it : targets_)
    {
        if (!target_it.second.hasTA())
            continue;

        assert (target_it.second.tas_.size() == 1);

        assert (!ta_2_utn_.count(*target_it.second.tas_.begin()));

        ta_2_utn_[*target_it.second.tas_.begin()] = target_it.second.utn_;
    }

    emit statusSignal("Creating non-Tracker UTNS");
    createNonTrackerUTNS();

    multiple_associated = 0;
    single_associated = 0;

    for (auto& target_it : targets_)
    {
        if (target_it.second.ds_ids_.size() > 1)
            ++multiple_associated;
        else
            ++single_associated;
    }

    loginf << "CreateARTASAssociationsJob: run: after non-tracker targets " << targets_.size()
           << " multiple " << multiple_associated << " single " << single_associated;

    // create associations
    emit statusSignal("Creating Associations");
    createAssociations();

    // save associations
    emit statusSignal("Saving Associations");
    for (auto& dbo_it : object_man)
    {
        loginf << "CreateARTASAssociationsJob: run: processing object " << dbo_it.first
               << " associated " << dbo_it.second->associations().size() << " of "
               << dbo_it.second->count();
        dbo_it.second->saveAssociations();
    }

    object_man.setAssociationsByAll(); // no specific dbo or data source

    stop_time = boost::posix_time::microsec_clock::local_time();

    double load_time;
    boost::posix_time::time_duration diff = stop_time - start_time;
    load_time = diff.total_milliseconds() / 1000.0;

    loginf << "CreateARTASAssociationsJob: run: done ("
           << String::doubleToStringPrecision(load_time, 2) << " s).";
    done_ = true;
}

void CreateAssociationsJob::createTargetReports()
{
    loginf << "CreateAssociationsJob: createTargetReports";

    MetaDBOVariable* meta_key_var = task_.keyVar();
    MetaDBOVariable* meta_ds_id_var = task_.dsIdVar();
    MetaDBOVariable* meta_tod_var = task_.todVar();
    MetaDBOVariable* meta_ta_var = task_.targetAddrVar();
    MetaDBOVariable* meta_ti_var = task_.targetIdVar();
    MetaDBOVariable* meta_tn_var = task_.trackNumVar();
    MetaDBOVariable* meta_mode_3a_var = task_.mode3AVar();
    MetaDBOVariable* meta_mode_c_var = task_.modeCVar();
    MetaDBOVariable* meta_latitude_var = task_.latitudeVar();
    MetaDBOVariable* meta_longitude_var = task_.longitudeVar();

    assert (meta_key_var);
    assert (meta_ds_id_var);
    assert (meta_tod_var);
    assert (meta_ta_var);
    assert (meta_ti_var);
    assert (meta_tn_var);
    assert (meta_mode_3a_var);
    assert (meta_mode_c_var);
    assert (meta_latitude_var);
    assert (meta_longitude_var);

    DBObjectManager& object_man = COMPASS::instance().objectManager();

    Association::TargetReport tr;

    for (auto& buf_it : buffers_) // dbo name, buffer
    {
        string dbo_name = buf_it.first;
        DBObject& dbo = object_man.object(dbo_name);

        shared_ptr<Buffer> buffer = buf_it.second;
        size_t buffer_size = buffer->size();

        assert (meta_key_var->existsIn(dbo_name));
        DBOVariable& key_var = meta_key_var->getFor(dbo_name);

        assert (meta_ds_id_var->existsIn(dbo_name));
        DBOVariable& ds_id_var = meta_ds_id_var->getFor(dbo_name);

        assert (meta_tod_var->existsIn(dbo_name));
        DBOVariable& tod_var = meta_tod_var->getFor(dbo_name);

        assert (meta_ta_var->existsIn(dbo_name));
        DBOVariable& ta_var = meta_ta_var->getFor(dbo_name);

        assert (meta_ti_var->existsIn(dbo_name));
        DBOVariable& ti_var = meta_ti_var->getFor(dbo_name);

        DBOVariable* tn_var {nullptr}; // not in ads-b
        if (meta_tn_var->existsIn(dbo_name))
            tn_var = &meta_tn_var->getFor(dbo_name);

        assert (meta_mode_3a_var->existsIn(dbo_name));
        DBOVariable& mode_3a_var = meta_mode_3a_var->getFor(dbo_name);

        assert (meta_mode_c_var->existsIn(dbo_name));
        DBOVariable& mode_c_var = meta_mode_c_var->getFor(dbo_name);

        assert (meta_latitude_var->existsIn(dbo_name));
        DBOVariable& latitude_var = meta_latitude_var->getFor(dbo_name);

        assert (meta_longitude_var->existsIn(dbo_name));
        DBOVariable& longitude_var = meta_longitude_var->getFor(dbo_name);


        assert (buffer->has<int>(key_var.name()));
        NullableVector<int>& rec_nums = buffer->get<int>(key_var.name());

        assert (buffer->has<int>(ds_id_var.name()));
        NullableVector<int>& ds_ids = buffer->get<int>(ds_id_var.name());

        assert (buffer->has<float>(tod_var.name()));
        NullableVector<float>& tods = buffer->get<float>(tod_var.name());

        assert (buffer->has<int>(ta_var.name()));
        NullableVector<int>& tas = buffer->get<int>(ta_var.name());

        assert (buffer->has<string>(ti_var.name()));
        NullableVector<string>& tis = buffer->get<string>(ti_var.name());

        NullableVector<int>* tns {nullptr};
        if (tn_var)
        {
            assert (buffer->has<int>(tn_var->name()));
            tns = &buffer->get<int>(tn_var->name());
        }

        assert (buffer->has<int>(mode_3a_var.name()));
        NullableVector<int>& m3as = buffer->get<int>(mode_3a_var.name());

        assert (buffer->has<int>(mode_c_var.name()));
        NullableVector<int>& mcs = buffer->get<int>(mode_c_var.name());

        assert (buffer->has<double>(latitude_var.name()));
        NullableVector<double>& lats = buffer->get<double>(latitude_var.name());

        assert (buffer->has<double>(longitude_var.name()));
        NullableVector<double>& longs = buffer->get<double>(longitude_var.name());

        for (size_t cnt = 0; cnt < buffer_size; ++cnt)
        {
            assert (!rec_nums.isNull(cnt));
            assert (!ds_ids.isNull(cnt));

            tr.dbo_name_ = dbo_name;
            tr.rec_num_ = rec_nums.get(cnt);
            tr.ds_id_ = ds_ids.get(cnt);

            if (tods.isNull(cnt))
            {
                logwrn << "CreateAssociationsJob: createTargetReports: target report w/o time: dbo "
                       << dbo_name << " rec_num " << tr.rec_num_  << " ds_id " << tr.ds_id_;
                continue;
            }

            if (tods.isNull(cnt))
            {
                logwrn << "CreateAssociationsJob: createTargetReports: target report w/o time: dbo "
                       << dbo_name << " rec_num " << tr.rec_num_  << " ds_id " << tr.ds_id_;
                continue;
            }

            if (lats.isNull(cnt))
            {
                logwrn << "CreateAssociationsJob: createTargetReports: target report w/o latitude: dbo "
                       << dbo_name << " rec_num " << tr.rec_num_  << " ds_id " << tr.ds_id_;
                continue;
            }
            if (longs.isNull(cnt))
            {
                logwrn << "CreateAssociationsJob: createTargetReports: target report w/o longitude: dbo "
                       << dbo_name << " rec_num " << tr.rec_num_  << " ds_id " << tr.ds_id_;
                continue;
            }

            tr.tod_ = tods.get(cnt);

            tr.has_ta_ = !tas.isNull(cnt);
            tr.ta_ = tr.has_ta_ ? tas.get(cnt) : 0;

            tr.has_ti_ = !tis.isNull(cnt);
            tr.ti_ = tr.has_ti_ ? tis.get(cnt) : "";

            tr.has_tn_ = tns && !tns->isNull(cnt);
            tr.tn_ = tr.has_tn_ ? tns->get(cnt) : 0;

            tr.has_ma_ = !m3as.isNull(cnt);
            tr.ma_ = tr.has_ma_ ? m3as.get(cnt) : 0;

            tr.has_ma_v_ = false; // TODO
            tr.has_ma_g_ = false; // TODO

            tr.has_mc_ = !mcs.isNull(cnt);
            tr.mc_ = tr.has_mc_ ? mcs.get(cnt) : 0;

            tr.has_mc_v_ = false; // TODO

            tr.latitude_ = lats.get(cnt);
            tr.longitude_ = longs.get(cnt);

            target_reports_[dbo_name][tr.ds_id_].push_back(tr);
        }
    }
}

void CreateAssociationsJob::createTrackerUTNS()
{
    loginf << "CreateAssociationsJob: createTrackerUTNS";

    if (target_reports_.count("Tracker"))
    {
        std::map<unsigned int, std::vector<Association::TargetReport>>& ds_id_trs = target_reports_.at("Tracker");

        unsigned int utn;

        map<unsigned int, Association::Target> tracker_targets;
        map<unsigned int, pair<unsigned int, float>> tn2utn; // track num -> utn, last tod

        DBObjectManager& object_man = COMPASS::instance().objectManager();

        // create utn for all tracks
        for (auto& ds_it : ds_id_trs) // ds_id->trs
        {
            loginf << "CreateAssociationsJob: createTrackerUTNS: processing ds_id " << ds_it.first;

            tracker_targets.clear();
            tn2utn.clear();

            string ds_name = object_man.object("Tracker").dataSources().at(ds_it.first).name();

            unsigned int tmp_utn_cnt {0};

            loginf << "CreateAssociationsJob: createTrackerUTNS: creating tmp targets for ds_id " << ds_it.first;

            // create temporary targets
            for (auto& tr_it : ds_it.second)
            {
                if (tr_it.has_tn_)
                {
                    if (!tn2utn.count(tr_it.tn_)) // first track update exists
                    {
                        logdbg << "CreateAssociationsJob: createTrackerUTNS: registering new tmp target "
                               << tmp_utn_cnt << " for tn " << tr_it.tn_;

                        tn2utn[tr_it.tn_] = {tmp_utn_cnt, tr_it.tod_};
                        ++tmp_utn_cnt;
                    }

                    //loginf << "UGA1";
                    if (tracker_targets.count(tn2utn.at(tr_it.tn_).first)) // additional checks if already exists
                    {
                        Association::Target& existing_target = tracker_targets.at(tn2utn.at(tr_it.tn_).first);

                        if (tr_it.has_ta_ && existing_target.hasTA() // new target part if ta change
                                && !existing_target.hasTA(tr_it.ta_))
                        {
                            logdbg << "CreateAssociationsJob: createTrackerUTNS: registering new tmp target "
                                   << tmp_utn_cnt << " for tn " << tr_it.tn_ << " because of ta switch "
                                   << " at " << String::timeStringFromDouble(tr_it.tod_)
                                   << " existing " << existing_target.asStr()
                                   << " tr " << tr_it.asStr();

                            tn2utn[tr_it.tn_] = {tmp_utn_cnt, tr_it.tod_};
                            ++tmp_utn_cnt;
                        }
                    }
                    //loginf << "UGA2";

                    if (tn2utn.at(tr_it.tn_).second > tr_it.tod_)
                    {
                        logwrn << "CreateAssociationsJob: createTrackerUTNS: tod backjump -"
                               << String::timeStringFromDouble(tn2utn.at(tr_it.tn_).second-tr_it.tod_)
                               << " tmp target " << tmp_utn_cnt << " at tr " << tr_it.asStr();
                    }
                    assert (tn2utn.at(tr_it.tn_).second <= tr_it.tod_);

                    //loginf << "UGA3";

                    if (tr_it.tod_ - tn2utn.at(tr_it.tn_).second > 60.0) // gap, new track // TODO parameter
                    {
                        logdbg << "CreateAssociationsJob: createTrackerUTNS: registering new tmp target "
                               << tmp_utn_cnt << " for tn " << tr_it.tn_ << " because of gap "
                               << String::timeStringFromDouble(tr_it.tod_ - tn2utn.at(tr_it.tn_).second)
                               << " at " << String::timeStringFromDouble(tr_it.tod_);

                        tn2utn[tr_it.tn_] = {tmp_utn_cnt, tr_it.tod_};
                        ++tmp_utn_cnt;
                    }

                    //loginf << "UGA4";

                    assert (tn2utn.count(tr_it.tn_));
                    utn = tn2utn.at(tr_it.tn_).first;
                    tn2utn.at(tr_it.tn_).second = tr_it.tod_;

                    if (!tracker_targets.count(utn)) // add new target if not existing
                    {
                        logdbg << "CreateAssociationsJob: createTrackerUTNS: creating new tmp target " << utn;

                        tracker_targets.emplace(
                                    std::piecewise_construct,
                                    std::forward_as_tuple(utn),   // args for key
                                    std::forward_as_tuple(utn, true));  // args for mapped value
                    }

                    tracker_targets.at(utn).addAssociated(&tr_it);
                }
                else
                {
                    logwrn << "CreateAssociationsJob: createTrackerUTNS: tracker target report w/o track num in ds_id "
                           << tr_it.ds_id_ << " at tod " << String::timeStringFromDouble(tr_it.tod_);
                }
            }

            if (!tracker_targets.size())
            {
                logwrn << "CreateAssociationsJob: createTrackerUTNS: tracker ds_id " << ds_it.first
                       << " created no utns";
                continue;
            }

            loginf << "CreateAssociationsJob: createTrackerUTNS: cleaning new utns for ds_id " << ds_it.first;

            emit statusSignal(("Cleaning new "+ds_name+" UTNs").c_str());

            vector<unsigned int> index_to_utn;
            unsigned int targets_size = tracker_targets.size();
            for (auto& t_it : tracker_targets)
                index_to_utn.push_back(t_it.first);

            tbb::parallel_for(uint(0), targets_size, [&](unsigned int cnt)
            {
                tracker_targets.at(index_to_utn.at(cnt)).calculateSpeeds();
            });

            for (auto& target_it : tracker_targets)
            {
                if (target_it.second.has_speed_ && target_it.second.speed_max_ > max_speed_kts_)
                {
                    loginf << "CreateAssociationsJob: createTrackerUTNS: new target dubious "
                           << target_it.second.utn_ << ": calculateSpeeds: min "
                           << String::doubleToStringPrecision(target_it.second.speed_min_,2)
                           << " avg " << String::doubleToStringPrecision(target_it.second.speed_avg_,2)
                           << " max " << String::doubleToStringPrecision(target_it.second.speed_max_,2) << " kts";

                    loginf << "CreateAssociationsJob: createTrackerUTNS: new target removing non-mode s target reports";
                    target_it.second.removeNonModeSTRs();

                    target_it.second.calculateSpeeds();

                    if (target_it.second.has_speed_)
                        loginf << "CreateAssociationsJob: createTrackerUTNS: cleaned new target "
                               << target_it.second.utn_ << ": calculateSpeeds: min "
                               << String::doubleToStringPrecision(target_it.second.speed_min_,2)
                               << " avg " << String::doubleToStringPrecision(target_it.second.speed_avg_,2)
                               << " max " << String::doubleToStringPrecision(target_it.second.speed_max_,2) << " kts";
                }
            }

            loginf << "CreateAssociationsJob: createTrackerUTNS: creating new utns for ds_id " << ds_it.first;

            emit statusSignal(("Creating new "+ds_name+" UTNs").c_str());

            // tracker_targets exist, tie them together by mode s address

            int tmp_utn;

            float done_ratio;

            unsigned int target_cnt = 0;

            while (tracker_targets.size())
            {
                done_ratio = (float)target_cnt / (float)targets_size;
                emit statusSignal(("Creating "+ds_name+" UTNs ("
                                   +String::percentToString(100.0*done_ratio)+"%)").c_str());

                ++target_cnt;

                auto tmp_target = tracker_targets.begin();
                assert (tmp_target != tracker_targets.end());

                if (tmp_target->second.has_tod_)
                {
                    logdbg << "CreateAssociationsJob: createTrackerUTNS: creating utn for tmp utn " << tmp_target->first;

                    tmp_utn = findUTNForTarget(tmp_target->second);

                    logdbg << "CreateAssociationsJob: createTrackerUTNS: tmp utn " << tmp_target->first
                           << " tmp_utn " << tmp_utn;

                    if (tmp_utn == -1) // none found, create new target
                        addTarget(tmp_target->second);
                    else // attach to existing target
                    {
                        assert (targets_.count(tmp_utn));
                        targets_.at(tmp_utn).addAssociated(tmp_target->second.assoc_trs_);
                    }
                }

                tracker_targets.erase(tmp_target);
            }

            loginf << "CreateAssociationsJob: createTrackerUTNS: processing ds_id " << ds_it.first << " done";

            emit statusSignal(("Checking "+ds_name+" UTNs").c_str());

            tbb::parallel_for(uint(0), utn_cnt_, [&](unsigned int cnt)
            {
                targets_.at(cnt).calculateSpeeds();
            });

            vector <unsigned int> still_dubious;

            for (auto& target_it : targets_)
            {
                if (target_it.second.has_speed_ && target_it.second.speed_max_ > max_speed_kts_)
                {
                    loginf << "CreateAssociationsJob: createTrackerUTNS: target dubious "
                           << target_it.second.utn_ << ": calculateSpeeds: min "
                           << String::doubleToStringPrecision(target_it.second.speed_min_,2)
                           << " avg " << String::doubleToStringPrecision(target_it.second.speed_avg_,2)
                           << " max " << String::doubleToStringPrecision(target_it.second.speed_max_,2) << " kts";

                    loginf << "CreateAssociationsJob: createTrackerUTNS: removing non-mode s target reports";
                    target_it.second.removeNonModeSTRs();

                    if (!target_it.second.has_tod_)
                    {
                        loginf << "CreateAssociationsJob: createTrackerUTNS: empty target utn "
                               << target_it.second.utn_;
                        continue;
                    }

                    target_it.second.calculateSpeeds();

                    if (target_it.second.has_speed_)
                        loginf << "CreateAssociationsJob: createTrackerUTNS: cleaned target "
                               << target_it.second.utn_ << ": calculateSpeeds: min "
                               << String::doubleToStringPrecision(target_it.second.speed_min_,2)
                               << " avg " << String::doubleToStringPrecision(target_it.second.speed_avg_,2)
                               << " max " << String::doubleToStringPrecision(target_it.second.speed_max_,2) << " kts";

                    if (target_it.second.has_speed_ && target_it.second.speed_max_ > max_speed_kts_)
                        still_dubious.push_back(target_it.second.utn_);
                }
            }

            for (unsigned int utn : still_dubious)
                loginf << "CreateAssociationsJob: createTrackerUTNS: target " << utn << " still dubious"
                       <<  " speed min " << String::doubleToStringPrecision(targets_.at(utn).speed_min_,2)
                        << " avg " << String::doubleToStringPrecision(targets_.at(utn).speed_avg_,2)
                        << " max " << String::doubleToStringPrecision(targets_.at(utn).speed_max_,2) << " kts";

//            for (unsigned int utn : to_be_erased)
//            {
//                loginf << "CreateAssociationsJob: createTrackerUTNS: erasing empty utn " << utn;
//                targets_.erase(utn);

//                for (auto& ta_it : ta_2_utn_)
//                {
//                    if (ta_it.second == utn)
//                    {
//                        ta_2_utn_.erase(ta_it.first);
//                        break;
//                    }
//                }
//            }
        }
    }
    else
        loginf << "CreateAssociationsJob: createTrackerUTNS: no tracker data";

}

void CreateAssociationsJob::createNonTrackerUTNS()
{
    loginf << "CreateAssociationsJob: createNonTrackerUTNS";

    unsigned int num_data_sources = 0;

    for (auto& dbo_it : target_reports_)
    {
        if (dbo_it.first == "Tracker") // already associated
            continue;

        for (auto& ds_it : dbo_it.second) // ds_id -> trs
            ++num_data_sources;
    }

    DBObjectManager& object_man = COMPASS::instance().objectManager();

    unsigned int ds_cnt = 0;
    unsigned int done_perc;
    for (auto& dbo_it : target_reports_)
    {
        if (dbo_it.first == "Tracker") // already associated
            continue;

        for (auto& ds_it : dbo_it.second) // ds_id -> trs
        {
            assert (num_data_sources);
            done_perc = (unsigned int)(100.0 * (float)ds_cnt/(float)num_data_sources);

            string ds_name = object_man.object(dbo_it.first).dataSources().at(ds_it.first).name();
            emit statusSignal(("Creating "+dbo_it.first+" "+ds_name+" UTNS ("+to_string(done_perc)+"%)").c_str());

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

                int tmp_utn;

                tmp_utn = findUTNForTargetReport(tr_it);

                if (tmp_utn != -1) // existing target found
                {
                    assert (targets_.count(tmp_utn));
                    //association_todos.push_back({tmp_utn, &tr_it});
                    tmp_assoc_utns[tr_cnt] = tmp_utn;
                    return;
                }

                if (tr_it.has_ta_)
                {
                    //addTargetByTargetReport(tr_it);

                    boost::mutex::scoped_lock lock(create_todos_mutex);
                    create_todos[tr_it.ta_].push_back(&tr_it);

                    return;
                }

                // tr non mode s

                if (!associate_ac_non_trackers_)
                    return;

                float tod;
                vector<tuple<bool, unsigned int, double>> results;
                // usable, utn, distance

                results.resize(utn_cnt_);

                tod = tr_it.tod_;

                EvaluationTargetPosition tst_pos;

                tst_pos.latitude_ = tr_it.latitude_;
                tst_pos.longitude_ = tr_it.longitude_;
                tst_pos.has_altitude_ = tr_it.has_mc_;
                tst_pos.altitude_ = tr_it.mc_;

                FixedTransformation trafo (tst_pos.latitude_, tst_pos.longitude_);

                //loginf << "UGA: checking tr a/c/pos";

                double x_pos, y_pos;
                double distance;

                EvaluationTargetPosition ref_pos;
                bool ok;

                for (unsigned int target_cnt=0; target_cnt < utn_cnt_; ++target_cnt)
                {
                    Association::Target& other = targets_.at(target_cnt);

                    results[target_cnt] = tuple<bool, unsigned int, double>(false, other.utn_, 0);

                    if ((tr_it.has_ta_ && other.hasTA())) // only try if not both mode s
                        continue;

                    if (!other.isTimeInside(tod))
                        continue;

                    // check mode a code
                    Association::CompareResult ma_res = other.compareModeACode(tr_it.has_ma_, tr_it.ma_, tod);

                    if (ma_res != Association::CompareResult::SAME)
                        continue;
                    //loginf << "UGA3 same mode a";

                    // check mode c code
                    Association::CompareResult mc_res = other.compareModeCCode(tr_it.has_mc_, tr_it.mc_, tod);

                    if (mc_res != Association::CompareResult::SAME)
                        continue;

                    // check positions

                    tie(ref_pos, ok) = other.interpolatedPosForTimeFast(tod, max_time_diff_);

                    if (ok &&
                            (sqrt(pow(ref_pos.latitude_-tst_pos.latitude_, 2)
                                  +pow(ref_pos.longitude_-tst_pos.longitude_, 2))
                             <= max_distance_acceptable_sensors_wgs_))
                    {

                        tie(ok, x_pos, y_pos) = trafo.distanceCart(ref_pos.latitude_, ref_pos.longitude_);

                        if (!ok)
                            continue;

                        distance = sqrt(pow(x_pos,2)+pow(y_pos,2));

                        //loginf << "UGA3 distance " << distance;

                        if (distance < max_distance_acceptable_sensors_)
                            results[target_cnt] = tuple<bool, unsigned int, double>(true, other.utn_, distance);
                    }
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
                    //targets_.at(best_other_utn).addAssociated(&tr_it);
                    //association_todos.push_back({best_other_utn, &tr_it});
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
                    targets_.at(tmp_utn).addAssociated(&target_reports.at(tr_cnt));
            }

            // create new targets
            for (auto& todo_it : create_todos) // ta -> trs
            {
                vector<Association::TargetReport*>& trs = todo_it.second;
                assert (trs.size());

                unsigned int new_utn = utn_cnt_;
                addTargetByTargetReport(*trs.at(0));

                for (unsigned int tr_cnt=1; tr_cnt < trs.size(); ++tr_cnt)
                    targets_.at(new_utn).addAssociated(trs.at(tr_cnt));
            }

            ++ds_cnt;
        }
    }

    //    emit statusSignal("Adding Associations");

    //    for (auto& assoc_it : association_todos)
    //        targets_.at(assoc_it.first).addAssociated(assoc_it.second);
}

void CreateAssociationsJob::createAssociations()
{
    loginf << "CreateAssociationsJob: createAssociations";

    DBObjectManager& object_man = COMPASS::instance().objectManager();

    for (auto& dbo_it : target_reports_)
    {
        assert (object_man.existsObject(dbo_it.first));
        DBObject& dbo = object_man.object(dbo_it.first);

        for (auto& ds_it : dbo_it.second) // ds_id -> trs
        {
            for (auto& tr_it : ds_it.second)
            {
                for (auto utn_ptr_it : tr_it.assoc_targets_)
                    dbo.addAssociation(tr_it.rec_num_, utn_ptr_it->utn_, false, 0);
            }
        }
    }
}

int CreateAssociationsJob::findUTNForTarget (const Association::Target& target)
// tries to find existing utn for target, -1 if failed
{
    int tmp_utn = findUTNForTargetByTA(target);

    if (tmp_utn != -1) // either mode s, so
        return tmp_utn;

    // try to find by m a/c/pos
    //if (target.hasMA() && target.hasMA(3599))
    logdbg << "CreateAssociationsJob: findUTNForTarget: checking target " << target.utn_ << " by mode a/c, pos";

    //    OGRSpatialReference wgs84;
    //    wgs84.SetWellKnownGeogCS("WGS84");

    vector<tuple<bool, unsigned int, unsigned int, double>> results;
    // usable, other utn, num updates, avg distance
    results.resize(utn_cnt_);

    tbb::parallel_for(uint(0), utn_cnt_, [&](unsigned int cnt)
    {
        Association::Target& other = targets_.at(cnt);
        Transformation trafo;

        results[cnt] = tuple<bool, unsigned int, unsigned int, double>(false, other.utn_, 0, 0);

        if (!(target.hasTA() && other.hasTA())) // only try if not both mode s
        {
            //if (target.hasMA() && target.hasMA(3599) && other.hasMA() && other.hasMA(3599))
            logdbg << "CreateAssociationsJob: findUTNForTarget: checking target " << target.utn_
                   << " other " << other.utn_
                   << " overlaps " << target.timeOverlaps(other) << " prob " << target.probTimeOverlaps(other);

            if (target.timeOverlaps(other) && target.probTimeOverlaps(other) >= prob_min_time_overlap_)
            {
                vector<float> ma_unknown;
                vector<float> ma_same;
                vector<float> ma_different;

                tie (ma_unknown, ma_same, ma_different) = target.compareModeACodes(other);

                //if (target.hasMA() && target.hasMA(3599) && other.hasMA() && other.hasMA(3599))
                logdbg << "CreateAssociationsJob: findUTNForTarget: target " << target.utn_
                       << " other " << other.utn_
                       << " ma same " << ma_same.size() << " diff " << ma_different.size();

                if (ma_same.size() > ma_different.size() &&  ma_same.size() >= min_updates_)
                {
                    // check mode c codes

                    vector<float> mc_unknown;
                    vector<float> mc_same;
                    vector<float> mc_different;

                    tie (mc_unknown, mc_same, mc_different) = target.compareModeCCodes(other, ma_same);

                    //if (target.hasMA() && target.hasMA(3599) && other.hasMA() && other.hasMA(3599))
                    logdbg << "CreateAssociationsJob: findUTNForTarget: target " << target.utn_
                           << " other " << other.utn_
                           << " ma same " << ma_same.size() << " diff " << ma_different.size()
                           << " mc same " << mc_same.size() << " diff " << mc_different.size();

                    if (mc_same.size() > mc_different.size() && mc_same.size() >= min_updates_)
                    {
                        // check positions

                        vector<pair<float, double>> same_distances;
                        double distances_sum {0};

                        unsigned int pos_dubious_cnt {0};

                        //                        OGRSpatialReference local;

                        //                        std::unique_ptr<OGRCoordinateTransformation> ogr_geo2cart;

                        EvaluationTargetPosition tst_pos;

                        double x_pos, y_pos;
                        double distance;

                        EvaluationTargetPosition ref_pos;
                        bool ok;

                        for (auto tod_it : mc_same)
                        {
                            assert (target.hasDataForExactTime(tod_it));
                            tst_pos = target.posForExactTime(tod_it);

                            tie(ref_pos, ok) = other.interpolatedPosForTimeFast(tod_it, max_time_diff_);

                            if (!ok)
                                continue;

                            //                            local.SetStereographic(ref_pos.latitude_, ref_pos.longitude_, 1.0, 0.0, 0.0);

                            //                            ogr_geo2cart.reset(OGRCreateCoordinateTransformation(&wgs84, &local));

                            //                            if (in_appimage_) // inside appimage
                            //                            {
                            //                                x_pos = tst_pos.longitude_;
                            //                                y_pos = tst_pos.latitude_;
                            //                            }
                            //                            else
                            //                            {
                            //                                x_pos = tst_pos.latitude_;
                            //                                y_pos = tst_pos.longitude_;
                            //                            }

                            //                            ok = ogr_geo2cart->Transform(1, &x_pos, &y_pos); // wgs84 to cartesian offsets

                            tie(ok, x_pos, y_pos) = trafo.distanceCart(
                                        ref_pos.latitude_, ref_pos.longitude_,
                                        tst_pos.latitude_, tst_pos.longitude_);

                            if (!ok)
                                continue;

                            distance = sqrt(pow(x_pos,2)+pow(y_pos,2));

                            if (distance > max_distance_dubious_)
                                ++pos_dubious_cnt;

                            if (distance > max_distance_quit_ || pos_dubious_cnt > max_positions_dubious_)
                                // too far or dubious, quit
                            {
                                same_distances.clear();
                                break;
                            }

                            //loginf << "\tdist " << distance;

                            same_distances.push_back({tod_it, distance});
                            distances_sum += distance;
                        }

                        if (same_distances.size() >= min_updates_)
                        {
                            double distance_avg = distances_sum / (float) same_distances.size();

                            if (distance_avg < max_distance_acceptable_trackers_)
                            {
                                //if (target.hasMA() && target.hasMA(3599) && other.hasMA() && other.hasMA(3599))
                                logdbg << "\ttarget " << target.utn_ << " other " << other.utn_
                                       << " next utn " << utn_cnt_ << " dist avg " << distance_avg
                                       << " num " << same_distances.size();
                                results[cnt] = tuple<bool, unsigned int, unsigned int, double>(
                                            true, other.utn_, same_distances.size(), distance_avg);
                            }
                        }
                    }
                }
            }
            else
            {
                //if (target.hasMA() && target.hasMA(3599) && other.hasMA() && other.hasMA(3599))
                //    logdbg << "\tno overlap";
            }
        }
    });

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

    for (auto& res_it : results) // usable, other utn, num updates, avg distance
    {
        tie(usable, other_utn, num_updates, distance_avg) = res_it;

        if (!usable)
            continue;

        score = (double)num_updates*(max_distance_acceptable_trackers_-distance_avg);

        if (first || score > best_score)
        {
            best_other_utn = other_utn;
            best_num_updates = num_updates;
            best_distance_avg = distance_avg;
            best_score = score;

            first = false;
        }
    }

    if (first)
    {
        //if (target.hasMA() && target.hasMA(3599))
        logdbg << "CreateAssociationsJob: findUTNForTarget: checking target " << target.utn_
               << " no match found";
        return -1;
    }
    else
    {
        //if (target.hasMA() && target.hasMA(3599))
        logdbg << "CreateAssociationsJob: findUTNForTarget: target " << target.utn_
               << " best other " << best_other_utn
               << " best score " << fixed << best_score << " dist avg " << best_distance_avg
               << " num " << best_num_updates;

        return best_other_utn;
    }
}

int CreateAssociationsJob::findUTNForTargetByTA (const Association::Target& target)
{
    if (!target.hasTA()) // cant be found
        return -1;

    for (auto& target_it : targets_)
    {
        if (!target_it.second.hasTA()) // cant be checked
            continue;

        if (target_it.second.hasAnyOfTAs(target.tas_))
            return target_it.first;
    }

    return -1;
}

int CreateAssociationsJob::findUTNForTargetReport (const Association::TargetReport& tr)
{
    // check in ta lookup map
    if (tr.has_ta_ && ta_2_utn_.count(tr.ta_))
        return ta_2_utn_.at(tr.ta_);

    return -1;
}


void CreateAssociationsJob::addTarget (const Association::Target& target) // creates new utn, adds to targets_
{
    assert (findUTNForTargetByTA(target) == -1); // should have been added

    targets_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(utn_cnt_),   // args for key
                std::forward_as_tuple(utn_cnt_, false));  // args for mapped value

    targets_.at(utn_cnt_).addAssociated(target.assoc_trs_);

    ++utn_cnt_;
}

void CreateAssociationsJob::addTargetByTargetReport (Association::TargetReport& tr)
{
    targets_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(utn_cnt_),   // args for key
                std::forward_as_tuple(utn_cnt_, false));  // args for mapped value

    if (tr.has_ta_)
        ta_2_utn_[tr.ta_] = {utn_cnt_};

    targets_.at(utn_cnt_).addAssociated(&tr);

    ++utn_cnt_;
}
