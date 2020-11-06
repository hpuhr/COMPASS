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

using namespace std;
using namespace Utils;

CreateAssociationsJob::CreateAssociationsJob(CreateAssociationsTask& task, DBInterface& db_interface,
                                             std::map<std::string, std::shared_ptr<Buffer>> buffers)
    : Job("CreateAssociationsJob"), task_(task), db_interface_(db_interface), buffers_(buffers)
{
}

CreateAssociationsJob::~CreateAssociationsJob() {}

void CreateAssociationsJob::run()
{
    logdbg << "CreateAssociationsJob: run: start";

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

    // create non-tracker utns
    emit statusSignal("Creating non-Tracker UTNS");
    createNonTrackerUTNS();

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

        // create utn for all tracks
        for (auto& ds_it : ds_id_trs) // ds_id->trs
        {
            loginf << "CreateAssociationsJob: createTrackerUTNS: processing ds_id " << ds_it.first;

            tracker_targets.clear();
            tn2utn.clear();

            unsigned int tmp_utn_cnt {0};

            for (auto& tr_it : ds_it.second)
            {
                if (tr_it.has_tn_)
                {
                    if (!tn2utn.count(tr_it.tn_)) // first track update exists
                    {
                        loginf << "CreateAssociationsJob: createTrackerUTNS: creating new tmp utn " << tmp_utn_cnt
                               << " for tn " << tr_it.tn_;

                        tn2utn[tr_it.tn_] = {tmp_utn_cnt, tr_it.tod_};
                        ++tmp_utn_cnt;
                    }

                    if (tn2utn.at(tr_it.tn_).second > tr_it.tod_)
                    {
                        logwrn << "CreateAssociationsJob: createTrackerUTNS: tod backjump -"
                               << String::timeStringFromDouble(tn2utn.at(tr_it.tn_).second-tr_it.tod_)
                               << " tmp utn " << tmp_utn_cnt << " at tr " << tr_it.asStr();
                    }
                    assert (tn2utn.at(tr_it.tn_).second <= tr_it.tod_);

                    if (tr_it.tod_ - tn2utn.at(tr_it.tn_).second > 60.0) // gap, new track
                    {
                        loginf << "CreateAssociationsJob: createTrackerUTNS: creating new tmp utn " << tmp_utn_cnt
                               << " for tn " << tr_it.tn_ << " because of gap "
                               << String::timeStringFromDouble(tr_it.tod_ - tn2utn.at(tr_it.tn_).second);

                        tn2utn[tr_it.tn_] = {tmp_utn_cnt, tr_it.tod_};
                        ++tmp_utn_cnt;
                    }

                    assert (tn2utn.count(tr_it.tn_));
                    utn = tn2utn.at(tr_it.tn_).first;
                    tn2utn.at(tr_it.tn_).second = tr_it.tod_;

                    if (!tracker_targets.count(utn)) // add new target if not existing
                        tracker_targets.emplace(
                                    std::piecewise_construct,
                                    std::forward_as_tuple(utn),   // args for key
                                    std::forward_as_tuple(utn, true));  // args for mapped value

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

            loginf << "CreateAssociationsJob: createTrackerUTNS: creating utns for ds_id " << ds_it.first;

            // tracker_targets exist, tie them together by mode s address

            int tmp_utn;

            while (tracker_targets.size())
            {
                auto tmp_target = tracker_targets.begin();
                assert (tmp_target != tracker_targets.end());

                loginf << "CreateAssociationsJob: createTrackerUTNS: creating utn for tmp utn " << tmp_target->first;

                tmp_utn = findUTNForTarget(tmp_target->second);

                if (tmp_utn == -1) // none found, create new target
                    addTarget(tmp_target->second);
                else // attach to existing target
                {
                    assert (targets_.count(tmp_utn));
                    targets_.at(tmp_utn).addAssociated(tmp_target->second.assoc_trs_);
                }

                tracker_targets.erase(tmp_target);
            }

            loginf << "CreateAssociationsJob: createTrackerUTNS: processing ds_id " << ds_it.first << " done";
        }
    }
    else
        loginf << "CreateAssociationsJob: createTrackerUTNS: no tracker data";

}

void CreateAssociationsJob::createNonTrackerUTNS()
{
    loginf << "CreateAssociationsJob: createNonTrackerUTNS";

    int tmp_utn;

    for (auto& dbo_it : target_reports_)
    {
        if (dbo_it.first == "Tracker") // already associated
            continue;

        for (auto& ds_it : dbo_it.second) // ds_id -> trs
        {
            for (auto& tr_it : ds_it.second)
            {
                tmp_utn = findUTNForTargetReport(tr_it);

                if (tmp_utn != -1) // existing target found
                {
                    assert (targets_.count(tmp_utn));
                    targets_.at(tmp_utn).addAssociated(&tr_it);
                }
                else if (tr_it.has_ta_)
                {
                    addTargetByTargetReport(tr_it);
                }
                else
                    ; // nothing can be done at the momemt
            }
        }
    }
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
    if (target.has_ta_ && ta_2_utn_.count(target.ta_))
        return ta_2_utn_.at(target.ta_);
    else
        return -1;
}

int CreateAssociationsJob::findUTNForTargetReport (const Association::TargetReport& tr)
{
    if (tr.has_ta_ && ta_2_utn_.count(tr.ta_))
        return ta_2_utn_.at(tr.ta_);
    else
        return -1;
}


void CreateAssociationsJob::addTarget (const Association::Target& target) // creates new utn, adds to targets_
{
    if (target.has_ta_)
    {
        assert (!ta_2_utn_.count(target.ta_));
        ta_2_utn_[target.ta_] = utn_cnt_;
    }

    //targets_[utn_cnt_] = {utn_cnt_};

    targets_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(utn_cnt_),   // args for key
                std::forward_as_tuple(utn_cnt_, false));  // args for mapped value

    targets_.at(utn_cnt_).addAssociated(target.assoc_trs_);

    ++utn_cnt_;
}

void CreateAssociationsJob::addTargetByTargetReport (Association::TargetReport& tr)
{
    if (tr.has_ta_)
    {
        assert (!ta_2_utn_.count(tr.ta_));
        ta_2_utn_[tr.ta_] = utn_cnt_;
    }

    //targets_[utn_cnt_] = {utn_cnt_};
    targets_.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(utn_cnt_),   // args for key
                std::forward_as_tuple(utn_cnt_, false));  // args for mapped value

    targets_.at(utn_cnt_).addAssociated(&tr);

    ++utn_cnt_;
}

//void CreateAssociationsJob::createUTNS()
//{
//    loginf << "CreateAssociationsJob: createUTNS";

//    MetaDBOVariable* meta_key_var = task_.keyVar();
//    MetaDBOVariable* meta_tod_var = task_.todVar();
//    MetaDBOVariable* meta_ta_var = task_.targetAddrVar();

//    assert (meta_key_var);
//    assert (meta_tod_var);
//    assert (meta_ta_var);

//    DBObjectManager& object_man = COMPASS::instance().objectManager();

//    for (auto& buf_it : buffers_) // dbo name, buffer
//    {
//        string dbo_name = buf_it.first;
//        DBObject& dbo = object_man.object(dbo_name);

//        shared_ptr<Buffer> buffer = buf_it.second;
//        size_t buffer_size = buffer->size();

//        assert (meta_key_var->existsIn(dbo_name));
//        DBOVariable& key_var = meta_key_var->getFor(dbo_name);

//        assert (meta_tod_var->existsIn(dbo_name));
//        DBOVariable& tod_var = meta_tod_var->getFor(dbo_name);

//        assert (meta_ta_var->existsIn(dbo_name));
//        DBOVariable& ta_var = meta_ta_var->getFor(dbo_name);

//        assert (buffer->has<int>(key_var.name()));
//        assert (buffer->has<float>(tod_var.name()));
//        assert (buffer->has<int>(ta_var.name()));

//        NullableVector<int>& rec_nums = buffer->get<int>(key_var.name());
//        NullableVector<float>& tods = buffer->get<float>(tod_var.name());
//        NullableVector<int>& tas = buffer->get<int>(ta_var.name());

//        unsigned int rec_num;
//        unsigned int target_addr;
//        unsigned int utn;

//        for (size_t cnt = 0; cnt < buffer_size; ++cnt)
//        {
//            assert (!rec_nums.isNull(cnt));

//            if (tas.isNull(cnt))
//                continue;

//            rec_num = rec_nums.get(cnt);
//            target_addr = tas.get(cnt);

//            if (ta_2_utn_.count(target_addr) == 0)
//            {
//                ta_2_utn_[target_addr] = utn_cnt_;
//                ++utn_cnt_;
//            }

//            utn = ta_2_utn_[target_addr];

//            dbo.addAssociation(rec_num, utn, false, 0);
//        }

//        dbo.saveAssociations();
//    }
//}
