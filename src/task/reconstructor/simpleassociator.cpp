#include "simpleassociator.h"
#include "simplereconstructor.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "datasourcemanager.h"
#include "timeconv.h"

#include "util/tbbhack.h"

#include <boost/thread/mutex.hpp>
#include <boost/optional/optional_io.hpp>

#include <osgEarth/GeoMath>

#include <tuple>

using namespace std;
using namespace Utils;
using namespace dbContent;
using namespace boost::posix_time;


SimpleAssociator::SimpleAssociator(SimpleReconstructor& reconstructor)
    : reconstructor_(reconstructor)
{

}


void SimpleAssociator::associateNewData()
{
    // create reference targets
    //emit statusSignal("Creating Reference UTNs");

            // create reference targets
    createReferenceUTNs();

            // create tracker targets
    //emit statusSignal("Creating Tracker UTNs");
    createTrackerUTNs();

    unsigned int multiple_associated {0};
    unsigned int single_associated {0};

    for (auto& target_it : reconstructor_.targets_)
    {
        if (target_it.second.ds_ids_.size() > 1)
            ++multiple_associated;
        else
            ++single_associated;
    }

    loginf << "SimpleAssociator: associateNewData: tracker targets " << reconstructor_.targets_.size()
           << " multiple " << multiple_associated << " single " << single_associated;

            // create non-tracker utns

            //emit statusSignal("Creating non-Tracker UTNs");
    createNonTrackerUTNS();

    multiple_associated = 0;
    single_associated = 0;

    for (auto& target_it : reconstructor_.targets_)
    {
        if (target_it.second.ds_ids_.size() > 1)
            ++multiple_associated;
        else
            ++single_associated;
    }

    loginf << "SimpleAssociator: associateNewData: after non-tracker targets " << reconstructor_.targets_.size()
           << " multiple " << multiple_associated << " single " << single_associated;

            // create associations
    //    emit statusSignal("Creating Associations");
    //    createAssociations();
}

void SimpleAssociator::createReferenceUTNs()
{
    loginf << "SimpleAssociator: createReferenceUTNs";

            //std::map<unsigned int, dbContent::ReconstructorTarget> sum_targets;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();
    unsigned int reftraj_id = dbcont_man.dbContent("RefTraj").id();

    if (!reconstructor_.tr_ds_.count(reftraj_id))
    {
        loginf << "SimpleAssociator: createReferenceUTNs: no tracker data";
        return;
    }

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

            // create utn for all tracks
    for (auto& ds_it : reconstructor_.tr_ds_.at(reftraj_id)) // ds_id->trs
    {
        loginf << "SimpleAssociator: createReferenceUTNs: processing ds_id " << ds_it.first;

        assert (ds_man.hasDBDataSource(ds_it.first));
        string ds_name = ds_man.dbDataSource(ds_it.first).name();

        loginf << "SimpleAssociator: createReferenceUTNs: creating tmp targets for ds_id " << ds_it.first;

                //emit statusSignal(("Creating new "+ds_name+" UTNs").c_str());

        std::map<unsigned int, ReconstructorTarget> tracker_targets = createTrackedTargets(
            reftraj_id, ds_it.first);

        if (!tracker_targets.size())
        {
            logwrn << "SimpleAssociator: createReferenceUTNs: ref ds_id " << ds_it.first
                   << " created no utns";
            continue;
        }

        loginf << "SimpleAssociator: createReferenceUTNs: cleaning new utns for ds_id " << ds_it.first;

                //emit statusSignal(("Cleaning new "+ds_name+" Targets").c_str());

        loginf << "SimpleAssociator: createReferenceUTNs: creating new utns for ds_id " << ds_it.first;

                //emit statusSignal(("Creating new "+ds_name+" Targets").c_str());

        addTrackerUTNs (ds_name, std::move(tracker_targets), reconstructor_.targets_);

                // try to associate targets to each other

        loginf << "SimpleAssociator: createReferenceUTNs: processing ds_id " << ds_it.first << " done";
    }

            //emit statusSignal("Self-associating Sum Reference Targets");
    //selfAssociateTrackerUTNs(); TODO

    return;
}

void SimpleAssociator::createTrackerUTNs()
{
    loginf << "SimpleAssociator: createTrackerUTNs";

            //std::map<unsigned int, ReconstructorTarget> sum_targets;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();
    unsigned int cat062_id = dbcont_man.dbContent("CAT062").id();

    if (!reconstructor_.tr_ds_.count(cat062_id))
    {
        loginf << "SimpleAssociator: createTrackerUTNs: no tracker data";
        return;
    }

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

            // create utn for all tracks
    for (auto& ds_it : reconstructor_.tr_ds_.at(cat062_id)) // ds_id->trs
    {
        loginf << "SimpleAssociator: createTrackerUTNs: processing ds_id " << ds_it.first;

        assert (ds_man.hasDBDataSource(ds_it.first));
        string ds_name = ds_man.dbDataSource(ds_it.first).name();

        loginf << "SimpleAssociator: createTrackerUTNs: creating tmp targets for ds_id " << ds_it.first;

                //emit statusSignal(("Creating new "+ds_name+" UTNs").c_str());

        map<unsigned int, ReconstructorTarget> tracker_targets = createTrackedTargets(cat062_id, ds_it.first);

        if (!tracker_targets.size())
        {
            logwrn << "SimpleAssociator: createTrackerUTNs: tracker ds_id " << ds_it.first
                   << " created no utns";
            continue;
        }

                //loginf << "SimpleAssociator: createTrackerUTNs: cleaning new utns for ds_id " << ds_it.first;

                //emit statusSignal(("Cleaning new "+ds_name+" Targets").c_str());

                //cleanTrackerUTNs (tracker_targets);

        loginf << "SimpleAssociator: createTrackerUTNs: creating new utns for ds_id " << ds_it.first;

                //emit statusSignal(("Creating new "+ds_name+" Targets").c_str());

        addTrackerUTNs (ds_name, std::move(tracker_targets), reconstructor_.targets_);

                // try to associate targets to each other

        loginf << "SimpleAssociator: createTrackerUTNs: processing ds_id " << ds_it.first << " done";

                //emit statusSignal("Checking Sum Targets");
                //cleanTrackerUTNs(sum_targets);
    }

            //emit statusSignal("Self-associating Sum Targets");
    //selfAssociateTrackerUTNs();

            //emit statusSignal("Checking Final Targets");
            //cleanTrackerUTNs(sum_targets);

            //markDubiousUTNs (sum_targets);

    return;
}

void SimpleAssociator::createNonTrackerUTNS()
{
    loginf << "SimpleAssociator: createNonTrackerUTNS";

    unsigned int num_data_sources = 0;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    for (auto& dbcont_it : reconstructor_.tr_ds_)
    {
        const std::string& dbcont_name = dbcont_man.dbContentWithId(dbcont_it.first);

        if (dbcont_name == "RefTraj" || dbcont_name == "CAT062") // already associated
            continue;

        num_data_sources += dbcont_it.second.size();
    }

    logdbg << "SimpleAssociator: createNonTrackerUTNS: num_data_sources " << num_data_sources;

            // get ta lookup map
    std::map<unsigned int, unsigned int> ta_2_utn = getTALookupMap(reconstructor_.targets_);

            //DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();
    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    const bool associate_non_mode_s = reconstructor_.settings_.associate_non_mode_s_;
    const time_duration max_time_diff_sensor =
        Time::partialSeconds(reconstructor_.settings_.max_time_diff_sensor_);
    const double max_altitude_diff_sensor = reconstructor_.settings_.max_altitude_diff_sensor_;
    const double max_distance_acceptable_sensor = reconstructor_.settings_.max_distance_acceptable_sensor_;

    unsigned int ds_cnt = 0;
    unsigned int done_perc;

    const std::set<unsigned int> mode_a_conspic = reconstructor_.settings_.mode_a_conspicuity_codes_;

    for (auto& dbcont_it : reconstructor_.tr_ds_)
    {
        const std::string& dbcont_name = dbcont_man.dbContentWithId(dbcont_it.first);

        if (dbcont_name == "RefTraj" || dbcont_name == "CAT062") // already associated
            continue;

        for (auto& ds_it : dbcont_it.second) // ds_id -> ts ->  record_num
        {
            loginf << "SimpleAssociator: createNonTrackerUTNS: ds " << ds_it.first;

            assert (num_data_sources);
            done_perc = (unsigned int)(100.0 * (float)ds_cnt/(float)num_data_sources);
            assert (ds_man.hasDBDataSource(ds_it.first));

            string ds_name = ds_man.dbDataSource(ds_it.first).name();

                    //emit statusSignal(("Creating "+dbcont_it.first+" "+ds_name+" UTNs ("+to_string(done_perc)+"%)").c_str());

            for (auto& line_it : ds_it.second)
            {

                std::vector<unsigned long>& rec_nums = line_it.second;
                unsigned int num_target_reports = rec_nums.size();
                vector<int> tmp_assoc_utns; // tr_cnt -> utn
                tmp_assoc_utns.resize(num_target_reports);

                map<unsigned int, vector<unsigned long>> create_todos; // ta -> tr rec_nums
                boost::mutex create_todos_mutex;

                        //for (unsigned int tr_cnt=0; tr_cnt < num_target_reports; ++tr_cnt)
                tbb::parallel_for(uint(0), num_target_reports, [&](unsigned int tr_cnt)
                                  {
                                      unsigned int rec_num = rec_nums.at(tr_cnt);

                                      targetReport::ReconstructorInfo& tr =
                                          reconstructor_.target_reports_.at(rec_num);

                                      assert (tr.timestamp_ >= reconstructor_.remove_before_time_);

                                      tmp_assoc_utns[tr_cnt] = -1; // set as not associated

                                      if(!tr.in_current_slice_) // already processed
                                          return;

                                      if (tr.acad_ && ta_2_utn.count(*tr.acad_)) // check ta with lookup
                                      {
                                          unsigned int tmp_utn = ta_2_utn.at(*tr.acad_);

                                          assert (reconstructor_.targets_.count(tmp_utn));
                                          tmp_assoc_utns[tr_cnt] = tmp_utn;
                                          return;
                                      }

                                              // lookup by mode s failed

                                      if (tr.acad_) // create new utn if tr has ta
                                                     //  && (!tr_it.has_ma_ || mode_a_conspic.count(tr_it.ma_))  and can not be associated using mode a
                                      {
                                          boost::mutex::scoped_lock lock(create_todos_mutex);
                                          create_todos[*tr.acad_].push_back(rec_num);

                                          return;
                                      }

                                              // tr non mode s

                                      if (!associate_non_mode_s)
                                          return;

                                      ptime timestamp;
                                      vector<tuple<bool, unsigned int, double>> results;
                                      // usable, utn, distance

                                      results.resize(reconstructor_.targets_.size());

                                      timestamp = tr.timestamp_;

                                      if (!tr.position_)
                                          return;

                                              //                                  TargetPosition tst_pos;

                                              //                                  tst_pos.latitude_ = tr_it.latitude_;
                                              //                                  tst_pos.longitude_ = tr_it.longitude_;
                                              //                                  tst_pos.has_altitude_ = tr_it.has_mc_;
                                              //                                  tst_pos.altitude_ = tr_it.mc_;

                                      FixedTransformation trafo (tr.position_->latitude_,
                                                                tr.position_->longitude_);

                                              //loginf << "UGA: checking tr a/c/pos";

                                      double x_pos, y_pos;
                                      double distance;

                                      targetReport::Position ref_pos;
                                      bool ok;

                                      unsigned int target_cnt=0;
                                      for (auto& target_it : reconstructor_.targets_)
                                      {
                                          ReconstructorTarget& other = target_it.second;

                                          results[target_cnt] = tuple<bool, unsigned int, double>(false, other.utn_, 0);

                                          if ((tr.acad_ && other.hasACAD())) // only try if not both mode s
                                          {
                                              ++target_cnt;
                                              continue;
                                          }

                                          if (!other.isTimeInside(timestamp))
                                          {
                                              ++target_cnt;
                                              continue;
                                          }

                                          if (tr.mode_a_code_ || tr.barometric_altitude_) // mode a/c based
                                          {
                                              // check mode a code

                                              if (tr.mode_a_code_)
                                              {
                                                  ComparisonResult ma_res = other.compareModeACode(
                                                      tr, max_time_diff_sensor);

                                                  if (ma_res == ComparisonResult::DIFFERENT)
                                                  {
                                                      target_cnt++;
                                                      continue;
                                                  }
                                              }
                                              //loginf << "UGA3 same mode a";

                                                      // check mode c code
                                              if (tr.barometric_altitude_)
                                              {
                                                  ComparisonResult mc_res = other.compareModeCCode(
                                                      tr, max_time_diff_sensor, max_altitude_diff_sensor, false);

                                                  if (mc_res == ComparisonResult::DIFFERENT)
                                                  {
                                                      target_cnt++;
                                                      continue;
                                                  }
                                              }
                                          }

                                                  // check positions

                                          tie(ref_pos, ok) = other.interpolatedPosForTimeFast(
                                              timestamp, max_time_diff_sensor);

                                          tie(ok, x_pos, y_pos) = trafo.distanceCart(ref_pos.latitude_, ref_pos.longitude_);

                                          if (!ok)
                                          {

                                              //loginf << "UGA3 NOT OK";
                                              ++target_cnt;
                                              continue;
                                          }

                                          distance = sqrt(pow(x_pos,2)+pow(y_pos,2));

                                                  //loginf << "UGA3 distance " << distance;

                                          if (distance < max_distance_acceptable_sensor)
                                              results[target_cnt] = tuple<bool, unsigned int, double>(
                                                  true, other.utn_, distance);

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

                        //            emit statusSignal(("Creating "+dbcont_it.first+" "+ds_name+" Associations ("
                        //                               +to_string(done_perc)+"%)").c_str());

                        // create associations
                int tmp_utn;
                for (unsigned int tr_cnt=0; tr_cnt < num_target_reports; ++tr_cnt) // tr_cnt -> utn
                {
                    tmp_utn = tmp_assoc_utns.at(tr_cnt);
                    if (tmp_utn != -1)
                    {
                        assert (reconstructor_.targets_.count(tmp_utn));
                        reconstructor_.targets_.at(tmp_utn).addTargetReport(rec_nums.at(tr_cnt));
                    }
                }

                        // create new targets
                for (auto& todo_it : create_todos) // ta -> trs
                {
                    vector<unsigned long>& trs = todo_it.second;
                    assert (trs.size());

                    unsigned int new_utn;

                    if (reconstructor_.targets_.size())
                        new_utn = reconstructor_.targets_.rbegin()->first + 1;
                    else
                        new_utn = 0;

                            //addTargetByTargetReport(*trs.at(0));

                    reconstructor_.targets_.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(new_utn),   // args for key
                        std::forward_as_tuple(reconstructor_, new_utn, false));  // args for mapped value, new_utn, tmp_utn false

                    reconstructor_.targets_.at(new_utn).addTargetReports(trs);

                            // add to mode s lookup

                    for (auto acad : reconstructor_.targets_.at(new_utn).acads_)
                    {
                        if (ta_2_utn.count(acad))
                        {
                            logwrn << "SimpleAssociator: createNonTrackerUTNS: acad "
                                   << String::hexStringFromInt(acad, 6, '0') << " multiple usage";
                        }
                        else
                            ta_2_utn[acad] = {new_utn};
                    }

                            //                if (trs.at(0)->has_ta_)
                            //                    ta_2_utn[trs.at(0)->ta_] = {new_utn};

                            //                targets.at(new_utn).addAssociated(trs.at(0));


                            //                for (unsigned int tr_cnt=0; tr_cnt < trs.size(); ++tr_cnt)
                            //                {
                            //                    assert (targets.count(new_utn));
                            //                    targets.at(new_utn).addAssociated(trs.at(tr_cnt));
                            //                }
                }

            }
            ++ds_cnt;
        }
    }

    loginf << "SimpleAssociator: createNonTrackerUTNS: done";
}

std::map<unsigned int, ReconstructorTarget> SimpleAssociator::createTrackedTargets(
    unsigned int dbcont_id, unsigned int ds_id)
{
    loginf << "SimpleAssociator: createTrackedTargets: dbcont_id " << dbcont_id << " ds_id " << ds_id;

    map<unsigned int, ReconstructorTarget> tracker_targets; // utn -> target
    vector<ReconstructorTarget*> tracker_targets_vec;

            //DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    DataSourceManager& ds_man = COMPASS::instance().dataSourceManager();

    assert (ds_man.hasDBDataSource(ds_id));
    string ds_name = ds_man.dbDataSource(ds_id).name();



    std::map<unsigned int, std::map<unsigned int, std::vector<unsigned long>>>& ds_id_trs =
        reconstructor_.tr_ds_.at(dbcont_id);
    // ds_id -> line_id -> ts ->  record_num

    if (!ds_id_trs.count(ds_id))
    {
        loginf << "SimpleAssociator: createTrackedTargets: ds " << ds_name << " has not target reports";
        return tracker_targets;
    }

    std::map<unsigned int, std::vector<unsigned long>>& line_id_trs = ds_id_trs.at(ds_id);

    unsigned int tmp_utn_cnt {0};
    bool attached_to_existing_utn;
    unsigned int utn;

            //    std::map<unsigned int, std::map<unsigned int,
            //                                    std::map<unsigned int,
            //                                             std::pair<unsigned int, boost::posix_time::ptime>>>> tmp_tn2utn_;

            // iterate over lines
    for (auto& line_it : line_id_trs)
    {
        logdbg << "SimpleAssociator: createTrackedTargets: iterating line " << line_it.first;

                //track num -> tmp_utn, last tod

        std::map<unsigned int, std::pair<unsigned int, boost::posix_time::ptime>> tmp_tn2utn_;

                // create temporary targets
        for (auto& tr_it : line_it.second)
        {
            assert (reconstructor_.target_reports_.count(tr_it));
            targetReport::ReconstructorInfo& tr = reconstructor_.target_reports_.at(tr_it);

            assert (tr.timestamp_ >= reconstructor_.remove_before_time_);

            if(!tr.in_current_slice_) // already processed
                continue;

            assert (tr.line_id_ == line_it.first); // check for current line

            if (tr.track_number_) // has track number
            {
                if (!tmp_tn2utn_.count(*tr.track_number_)) // if not yet mapped to utn
                {
                    attached_to_existing_utn = false;

                            // check if can be attached to already existing utn
                    if (!tr.acad_ && reconstructor_.settings_.associate_non_mode_s_) // not for mode-s targets
                    {
                        int cont_utn = findContinuationUTNForTrackerUpdate(tr, tracker_targets);

                        if (cont_utn != -1)
                        {
                            logdbg << "SimpleAssociator: createPerTrackerTargets: continuing target "
                                   << cont_utn << " with tn " << *tr.track_number_ << " at time "
                                   << Time::toString(tr.timestamp_);
                            tmp_tn2utn_[*tr.track_number_] = {cont_utn, tr.timestamp_};
                            attached_to_existing_utn = true;
                        }
                    }

                    if (!attached_to_existing_utn)
                    {
                        logdbg << "SimpleAssociator: createPerTrackerTargets: registering new tmp target "
                               << tmp_utn_cnt << " for tn " << *tr.track_number_;

                        tmp_tn2utn_[*tr.track_number_] = {tmp_utn_cnt, tr.timestamp_};
                        ++tmp_utn_cnt;
                    }
                }

                if (tracker_targets.count(tmp_tn2utn_.at(*tr.track_number_).first)) // additional checks if already exists
                {
                    ReconstructorTarget& existing_target = tracker_targets.at(
                        tmp_tn2utn_.at(*tr.track_number_).first);

                    if (tr.acad_ && existing_target.hasACAD() // new target part if ta change
                        && !existing_target.hasACAD(*tr.acad_))
                    {
                        logdbg << "SimpleAssociator: createPerTrackerTargets: registering new tmp target "
                               << tmp_utn_cnt << " for tn " << *tr.track_number_ << " because of ta switch "
                               << " at " << Time::toString(tr.timestamp_)
                               << " existing " << existing_target.asStr()
                               << " tr " << tr.asStr();

                        tmp_tn2utn_[*tr.track_number_] = {tmp_utn_cnt, tr.timestamp_};
                        ++tmp_utn_cnt;
                    }
                }

                if (tmp_tn2utn_.at(*tr.track_number_).second > tr.timestamp_)
                {
                    logwrn << "SimpleAssociator: createPerTrackerTargets: tod backjump -"
                           << " tmp_tn2utn_ ts "
                           << Time::toString(tmp_tn2utn_.at(*tr.track_number_).second)
                           << " tr " << Time::toString(tr.timestamp_)
                           << " delta "
                           << Time::toString(tmp_tn2utn_.at(*tr.track_number_).second - tr.timestamp_)
                           << " tmp target " << tmp_utn_cnt << " at tr " << tr.asStr() << " tn " << *tr.track_number_;

                    unsigned int tmp_utn = tmp_tn2utn_.at(*tr.track_number_).first;
                    assert (tracker_targets.count(tmp_utn));
                    logwrn << tracker_targets.at(tmp_utn).asStr();
                }
                assert (tmp_tn2utn_.at(*tr.track_number_).second <= tr.timestamp_);

                if ((tr.timestamp_ - tmp_tn2utn_.at(*tr.track_number_).second).total_seconds() > 60.0) // gap, new track // TODO parameter
                {
                    logdbg << "SimpleAssociator: createPerTrackerTargets: registering new tmp target "
                           << tmp_utn_cnt << " for tn " << *tr.track_number_ << " because of gap "
                           << Time::toString(tr.timestamp_ - tmp_tn2utn_.at(*tr.track_number_).second)
                           << " at " << Time::toString(tr.timestamp_);

                    tmp_tn2utn_[*tr.track_number_] = {tmp_utn_cnt, tr.timestamp_};
                    ++tmp_utn_cnt;
                }

                assert (tmp_tn2utn_.count(*tr.track_number_));
                utn = tmp_tn2utn_.at(*tr.track_number_).first;
                tmp_tn2utn_.at(*tr.track_number_).second = tr.timestamp_;

                if (!tracker_targets.count(utn)) // add new target if not existing
                {
                    logdbg << "SimpleAssociator: createPerTrackerTargets: creating new tmp target " << utn;

                    tracker_targets.emplace(
                        std::piecewise_construct,
                        std::forward_as_tuple(utn),   // args for key
                        std::forward_as_tuple(reconstructor_, utn, true));  // args for mapped value utn, tmp_utn true

                    tracker_targets_vec.push_back(&tracker_targets.at(utn));
                }

                tracker_targets.at(utn).addTargetReport(tr.record_num_);
            }
            else
            {
                logwrn << "SimpleAssociator: createPerTrackerTargets: tracker target report w/o track num in ds_id "
                       << tr.ds_id_ << " at tod " << Time::toString(tr.timestamp_);
            }
        }
    }

    return tracker_targets;
}

void SimpleAssociator::selfAssociateTrackerUTNs()
{
    loginf << "SimpleAssociator: selfAssociateTrackerUTNs: num targets " << reconstructor_.targets_.size();

    for (auto tgt_it = reconstructor_.targets_.cbegin(); tgt_it != reconstructor_.targets_.cend() /* not hoisted */; /* no increment */)
    {
        if (!tgt_it->second.associations_written_)
        {
            loginf << "SimpleAssociator: selfAssociateTrackerUTNs: processing target utn " << tgt_it->first;

                    // only check for targets before
            int tmp_utn = findUTNForTrackerTarget(tgt_it->second, reconstructor_.targets_, tgt_it->first - 1);

            if (tmp_utn != -1) // found match
            {
                reconstructor_.targets_.at(tmp_utn).addTargetReports(tgt_it->second.target_reports_);

                tgt_it = reconstructor_.targets_.erase(tgt_it);
            }
            else
                ++tgt_it;
        }
        else // skip, can't be changed anymore
            ++tgt_it;
    }


            //tgt_it.second.associations_written_

            //    std::map<unsigned int, dbContent::ReconstructorTarget> new_targets;

            //    while (targets.size())
            //    {
            //        pair<const unsigned int, dbContent::ReconstructorTarget>& tgt_it = *targets.begin();

            //        loginf << "SimpleAssociator: selfAssociateTrackerUTNs: processing target utn " << tgt_it.first;

            //        int tmp_utn = findUTNForTrackerTarget(tgt_it.second, new_targets);

            //        if (tmp_utn == -1)
            //        {

            //            if (new_targets.size())
            //                tmp_utn = new_targets.rbegin()->first + 1;
            //            else
            //                tmp_utn = 0;

            //            loginf << "SimpleAssociator: selfAssociateTrackerUTNs: no related utn found,"
            //                      " keeping target as utn " << tmp_utn;

            //            new_targets.emplace(
            //                std::piecewise_construct,
            //                std::forward_as_tuple(tmp_utn),   // args for key
            //                std::forward_as_tuple(reconstructor_, tmp_utn, false));  // args for mapped value tmp_utn, false
            //        }
            //        else
            //        {
            //            loginf << "SimpleAssociator: selfAssociateTrackerUTNs: related utn " << tmp_utn
            //                   << " found, associating";
            //        }

            //                // move to other map
            //        new_targets.at(tmp_utn).addTargetReports(tgt_it.second.target_reports_);
            //        targets.erase(tgt_it.first);
            //    }


    loginf << "SimpleAssociator: selfAssociateTrackerUTNs: done with num targets " << reconstructor_.targets_.size();

            //    return new_targets;
}

void SimpleAssociator::addTrackerUTNs(const std::string& ds_name,
                                      std::map<unsigned int, dbContent::ReconstructorTarget> from_targets,
                                      std::map<unsigned int, dbContent::ReconstructorTarget>& to_targets)
{
    loginf << "SimpleAssociator: addTrackerUTNs: src " << ds_name
           << " from_targets size " << from_targets.size() << " to_targets size " << to_targets.size();

    int tmp_utn;

    float done_ratio;

    unsigned int target_cnt = 0;
    unsigned int from_targets_size = from_targets.size();

    while (from_targets.size())
    {
        done_ratio = (float)target_cnt / (float)from_targets_size;
        //        emit statusSignal(("Creating "+ds_name+" UTNs ("
        //                           +String::percentToString(100.0*done_ratio)+"%)").c_str());

        ++target_cnt;

        auto tmp_target = from_targets.begin();
        assert (tmp_target != from_targets.end());

        if (tmp_target->second.hasTimestamps())
        {
            logdbg << "SimpleAssociator: addTrackerUTNs: creating utn for tmp utn " << tmp_target->first;

            tmp_utn = findUTNForTrackerTarget(tmp_target->second, to_targets);

            logdbg << "SimpleAssociator: addTrackerUTNs: tmp utn " << tmp_target->first
                   << " tmp_utn " << tmp_utn;

            if (tmp_utn == -1) // none found, create new target
            {
                if (to_targets.size())
                    tmp_utn  = to_targets.rbegin()->first + 1;
                else
                    tmp_utn = 0;

                logdbg << "SimpleAssociator: addTrackerUTNs: tmp utn " << tmp_target->first
                       << " as new " << tmp_utn;

                        // add the target
                to_targets.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(tmp_utn),   // args for key
                    std::forward_as_tuple(reconstructor_, tmp_utn, false));  // args for mapped value tmp_utn, false

                        // add associated target reports
                to_targets.at(tmp_utn).addTargetReports(tmp_target->second.target_reports_);
            }
            else // attach to existing target
            {
                //                        if (tmp_target->second.hasMA() && tmp_target->second.hasMA(396))
                //                            loginf << "SimpleAssociator: createTrackerUTNs: attaching utn " << tmp_target->first
                //                                   << " to tmp_utn " << tmp_utn;

                logdbg << "SimpleAssociator: addTrackerUTNs: tmp utn " << tmp_target->first
                       << " as existing " << tmp_utn;

                assert (to_targets.count(tmp_utn));
                to_targets.at(tmp_utn).addTargetReports(tmp_target->second.target_reports_);
            }
        }

                // remove target
        from_targets.erase(tmp_target);
    }


    loginf << "SimpleAssociator: addTrackerUTNs: done with src " << ds_name
           << " to_targets size " << to_targets.size();
}

int SimpleAssociator::findContinuationUTNForTrackerUpdate (
    const targetReport::ReconstructorInfo& tr,
    const std::map<unsigned int, ReconstructorTarget>& targets)
// tries to find existing utn for tracker update, -1 if failed
{
    logdbg << "SimpleAssociator: findContinuationUTNForTrackerUpdate";

    if (tr.acad_)
        return -1;

    const time_duration max_time_diff_tracker = Time::partialSeconds(
        reconstructor_.settings_.cont_max_time_diff_tracker_);
    const double max_altitude_diff_tracker = reconstructor_.settings_.max_altitude_diff_tracker_;
    const double max_distance_acceptable_tracker = reconstructor_.settings_.cont_max_distance_acceptable_tracker_;

    unsigned int num_targets = targets.size();

    vector<tuple<bool, unsigned int, double>> results;
    // usable, other utn, distance
    results.resize(num_targets);

            //    for (auto& tgt_it : targets)
            //        loginf << "UGA " << tgt_it.first;

    tbb::parallel_for(uint(0), num_targets, [&](unsigned int cnt)
                      {
                         //loginf << "SimpleAssociator: findContinuationUTNForTrackerUpdate: 1";
                          //                          assert (tracker_reconstructor_.targets_vec.at(cnt));
                          //                          unsigned int utn = tracker_reconstructor_.targets_vec.at(cnt)->utn_;
                          assert (targets.count(cnt));

                          const ReconstructorTarget& other = targets.at(cnt);

                          //Transformation trafo;

                          results[cnt] = tuple<bool, unsigned int, double>(false, other.utn_, 0);

                          if (!other.numAssociated()) // check if target has associated target reports
                              return;

                          if (other.hasACAD()) // not for mode-s targets
                              return;

                          if (tr.timestamp_ <= other.timestamp_max_) // check if not recently updated
                              return;

                                  // tr.tod_ > other.tod_max_
                          if (tr.timestamp_ - other.timestamp_max_ > max_time_diff_tracker) // check if last updated longer ago than threshold
                              return;

                                  //loginf << "SimpleAssociator: findContinuationUTNForTrackerUpdate: 2";

                          assert (reconstructor_.target_reports_.count(other.lastAssociated()));

                          const targetReport::ReconstructorInfo& other_last_tr =
                              reconstructor_.target_reports_.at(other.lastAssociated());

                          assert (other_last_tr.track_end_ && other_last_tr.track_end_);
                          if (!*other_last_tr.track_end_ || !*other_last_tr.track_end_) // check if other track was ended
                              return;

                          if (!other_last_tr.mode_a_code_ || !tr.mode_a_code_) // check mode a codes exist
                              return;

                          if (!other_last_tr.mode_a_code_->hasReliableValue() || !tr.mode_a_code_->hasReliableValue())
                              return;  // check reliable mode a codes exist

                          if (other_last_tr.mode_a_code_->code_ != tr.mode_a_code_->code_) // check mode-a
                              return;

                                  // mode a codes the same

                                  //loginf << "SimpleAssociator: findContinuationUTNForTrackerUpdate: 3";

                          if (other_last_tr.barometric_altitude_ && tr.barometric_altitude_
                              && other_last_tr.barometric_altitude_->hasReliableValue()
                              && tr.barometric_altitude_ ->hasReliableValue()
                              && fabs(other_last_tr.barometric_altitude_->altitude_
                                      - tr.barometric_altitude_->altitude_) > max_altitude_diff_tracker)
                              return; // check mode c codes if existing

                          bool ok;
                          //double x_pos, y_pos;
                          double distance;

                          assert (tr.position_ && other_last_tr.position_);

//                          tie(ok, x_pos, y_pos) = trafo.distanceCart(
//                              other_last_tr.position_->latitude_, other_last_tr.position_->longitude_,
//                              tr.position_->latitude_, tr.position_->longitude_);

//                          if (!ok)
//                              return;

//                          distance = sqrt(pow(x_pos,2)+pow(y_pos,2));

                          distance = osgEarth::GeoMath::distance(
                              other_last_tr.position_->latitude_ * DEG2RAD,
                              other_last_tr.position_->longitude_  * DEG2RAD,
                              tr.position_->latitude_ * DEG2RAD, tr.position_->longitude_ * DEG2RAD);

                          if (distance > max_distance_acceptable_tracker)
                              return;

                                  //loginf << "SimpleAssociator: findContinuationUTNForTrackerUpdate: 4";

                          results[cnt] = tuple<bool, unsigned int, double>(
                              true, other.utn_, distance);
                      });

    logdbg << "SimpleAssociator: findContinuationUTNForTrackerUpdate: finding best matches";

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
        logdbg << "SimpleAssociator: findContinuationUTNForTrackerUpdate: " << num_matches << " found";
        return -1;
    }

    logdbg << "SimpleAssociator: findContinuationUTNForTrackerUpdate: continuation match utn "
           << best_other_utn << " found, distance " << best_distance;

    return best_other_utn;
}

int SimpleAssociator::findUTNForTrackerTarget (const ReconstructorTarget& target,
                                              const std::map<unsigned int, ReconstructorTarget>& targets,
                                              int max_utn)
// tries to find existing utn for target, -1 if failed
{
    if (!targets.size()) // check if targets exist
        return -1;

    int tmp_utn = findUTNForTargetByTA(target, targets, max_utn);

    if (tmp_utn != -1) // either mode s, so
        return tmp_utn;

    if (!reconstructor_.settings_.associate_non_mode_s_)
        return -1;

            // try to find by m a/c/pos
    bool print_debug_target = false; //target.hasModeA() && target.hasMA(3824);
    if (print_debug_target)
        loginf << "SimpleAssociator: findUTNForTrackerTarget: checking target " << target.utn_
               << " by mode a/c, pos";

    vector<tuple<bool, unsigned int, unsigned int, double>> results;
    // usable, other utn, num updates, avg distance

    unsigned int num_utns = targets.size();
    results.resize(num_utns);

    const double prob_min_time_overlap_tracker = reconstructor_.settings_.prob_min_time_overlap_tracker_;
    const time_duration max_time_diff_tracker = Time::partialSeconds(reconstructor_.settings_.cont_max_time_diff_tracker_);
    const unsigned int min_updates_tracker = reconstructor_.settings_.min_updates_tracker_;
    const double max_altitude_diff_tracker = reconstructor_.settings_.max_altitude_diff_tracker_;
    const unsigned int max_positions_dubious_tracker = reconstructor_.settings_.max_positions_dubious_tracker_;
    const double max_distance_quit_tracker = reconstructor_.settings_.max_distance_quit_tracker_;
    const double max_distance_dubious_tracker = reconstructor_.settings_.max_distance_dubious_tracker_;
    const double max_distance_acceptable_tracker = reconstructor_.settings_.cont_max_distance_acceptable_tracker_;

            //    for (auto& tgt_it : targets)
            //        loginf << "UGA " << tgt_it.first;

    tbb::parallel_for(uint(0), num_utns, [&](unsigned int cnt)
                                                                //for (unsigned int cnt=0; cnt < utn_cnt_; ++cnt)
                      {
                          assert (targets.count(cnt));

                          results[cnt] = tuple<bool, unsigned int, unsigned int, double>(false, cnt, 0, 0);

                          if (max_utn != -100 && cnt > max_utn) // skip, used to check only targets before current one
                              return;

                          const ReconstructorTarget& other = targets.at(cnt);
                          assert (other.utn_ == cnt);

                          Transformation trafo;

                          bool print_debug = false;
                          //target.hasMA() && target.hasMA(3824) && other.hasMA() && other.hasMA(3824);

                          if (!(target.hasACAD() && other.hasACAD())) // only try if not both mode s
                          {
                              if (print_debug)
                              {
                                  loginf << "\ttarget " << target.utn_ << " " << target.timeStr()
                                         << " checking other " << other.utn_ << " " << other.timeStr()
                                         << " overlaps " << target.timeOverlaps(other) << " prob " << target.probTimeOverlaps(other);
                              }

                              if (target.timeOverlaps(other)
                                  && target.probTimeOverlaps(other) >= prob_min_time_overlap_tracker)
                              {
                                  if (print_debug)
                                      loginf << "\ttarget " << target.utn_ << " other " << other.utn_ << " overlap passed";

                                  vector<unsigned long> ma_unknown;
                                  vector<unsigned long> ma_same;
                                  vector<unsigned long> ma_different;

                                  tie (ma_unknown, ma_same, ma_different) =
                                      target.compareModeACodes(other, max_time_diff_tracker);

                                  if (print_debug)
                                  {
                                      loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                             << " ma unknown " << ma_unknown.size()
                                             << " same " << ma_same.size() << " diff " << ma_different.size();
                                  }

                                  if (ma_same.size() > ma_different.size() && ma_same.size() >= min_updates_tracker)
                                  {
                                      if (print_debug)
                                          loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                                 << " mode a check passed";

                                              // check mode c codes

                                      vector<unsigned long> mc_unknown;
                                      vector<unsigned long> mc_same;
                                      vector<unsigned long> mc_different;

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
                                              loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                                     << " mode c check passed";
                                          // check positions

                                          vector<pair<unsigned long, double>> same_distances;
                                          double distances_sum {0};

                                          unsigned int pos_dubious_cnt {0};

                                          dbContent::targetReport::Position tst_pos;

                                          double x_pos, y_pos;
                                          double distance;

                                          dbContent::targetReport::Position ref_pos;
                                          bool ok;

                                          ptime timestamp;

                                          for (auto rec_num_it : mc_same)
                                          {
                                              assert (target.hasDataFor(rec_num_it));
                                              timestamp = target.dataFor(rec_num_it).timestamp_;

                                              assert (target.hasPositionFor(rec_num_it));
                                              tst_pos = target.positionFor(rec_num_it);

                                              tie(ref_pos, ok) = other.interpolatedPosForTimeFast(
                                                  timestamp, max_time_diff_tracker);

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

                                              same_distances.push_back({rec_num_it, distance});
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
                                              loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                                     << " mode c check failed";
                                      }
                                  }
                                  else
                                  {
                                      if (print_debug)
                                          loginf << "\ttarget " << target.utn_ << " other " << other.utn_
                                                 << " mode a check failed";
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

int SimpleAssociator::findUTNForTargetByTA (const ReconstructorTarget& target,
                                           const std::map<unsigned int, ReconstructorTarget>& targets,
                                           int max_utn)
{
    if (!target.hasACAD()) // cant be found
        return -1;

    for (auto& target_it : targets)
    {
        if (target_it.first > max_utn)
            continue;

        if (!target_it.second.hasACAD()) // cant be checked
            continue;

        if (target_it.second.hasAnyOfACADs(target.acads_))
            return target_it.first;
    }

    return -1;
}

std::map<unsigned int, unsigned int> SimpleAssociator::getTALookupMap (
    const std::map<unsigned int, ReconstructorTarget>& targets)
{
    logdbg << "SimpleAssociator: getTALookupMap";

    std::map<unsigned int, unsigned int> ta_2_utn;

    for (auto& target_it : targets)
    {
        if (!target_it.second.hasACAD())
            continue;

        assert (target_it.second.acads_.size() == 1);

        assert (!ta_2_utn.count(*target_it.second.acads_.begin()));

        ta_2_utn[*target_it.second.acads_.begin()] = target_it.second.utn_;
    }

    logdbg << "SimpleAssociator: getTALookupMap: done";

    return ta_2_utn;
}
