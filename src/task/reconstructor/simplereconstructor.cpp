#include "simplereconstructor.h"
#include "simplereconstructorwidget.h"
#include "compass.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/dbcontentmanager.h"
#include "dbcontent/variable/variableset.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "targetreportaccessor.h"
#include "datasourcemanager.h"
#include "dbinterface.h"
#include "number.h"

#include "timeconv.h"

using namespace std;
using namespace Utils;

SimpleReconstructor::SimpleReconstructor(const std::string& class_id, const std::string& instance_id,
                                         ReconstructorTask& task)
    : ReconstructorBase(class_id, instance_id, task), associatior_(*this),
      acc_estimator_(*this)
{
    // common
    registerParameter("associate_non_mode_s", &settings_.associate_non_mode_s_, true);
    registerParameter("clean_dubious_utns", &settings_.clean_dubious_utns_, true);
    registerParameter("mark_dubious_utns_unused", &settings_.mark_dubious_utns_unused_, false);
    registerParameter("comment_dubious_utns", &settings_.comment_dubious_utns_, true);

            // tracker stuff
    registerParameter("max_time_diff_tracker", &settings_.max_time_diff_tracker_, 15.0);

    registerParameter("max_distance_quit_tracker", &settings_.max_distance_quit_tracker_, 10*NM2M); // kb 5nm
    registerParameter("max_distance_dubious_tracker", &settings_.max_distance_dubious_tracker_, 3*NM2M);
    //kb 2.5? 2.5 lowest
    registerParameter("max_positions_dubious_tracker", &settings_.max_positions_dubious_tracker_, 5u);

    registerParameter("max_distance_acceptable_tracker", &settings_.max_distance_acceptable_tracker_, NM2M/2.0);
    registerParameter("max_altitude_diff_tracker", &settings_.max_altitude_diff_tracker_, 300.0);

    registerParameter("min_updates_tracker", &settings_.min_updates_tracker_, 2u); // kb 3!!!
    registerParameter("prob_min_time_overlap_tracker", &settings_.prob_min_time_overlap_tracker_, 0.5); //kb 0.7
    registerParameter("max_speed_tracker_kts", &settings_.max_speed_tracker_kts_, 100000.0);

    registerParameter("cont_max_time_diff_tracker", &settings_.cont_max_time_diff_tracker_, 30.0);
    registerParameter("cont_max_distance_acceptable_tracker", &settings_.cont_max_distance_acceptable_tracker_, 1852.0);

            // sensor
    registerParameter("max_time_diff_sensor", &settings_.max_time_diff_sensor_, 15.0);
    registerParameter("max_distance_acceptable_sensor", &settings_.max_distance_acceptable_sensor_, 2*NM2M);
    registerParameter("max_altitude_diff_sensor", &settings_.max_altitude_diff_sensor_, 300.0);

            // target id? kb: nope
            // kb: TODO ma 1bit hamming distance, especially g (1bit wrong)/v (!->at least 1bit wrong)
}

SimpleReconstructor::~SimpleReconstructor() {}

dbContent::VariableSet SimpleReconstructor::getReadSetFor(const std::string& dbcontent_name) const
{
    dbContent::VariableSet read_set;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

            // ds id
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_ds_id_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_ds_id_));

            // line id
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_line_id_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_line_id_));

            // timestamp
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_timestamp_));

            // aircraft address
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_acad_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acad_));

            // aircraft id
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_acid_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_acid_));

            // track num
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_num_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_num_));

            // track end
    if (dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_track_end_))
        read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_track_end_));

            // mode 3a
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_m3a_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_m3a_));

            // mode c
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_mc_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_mc_));

    if (dbcontent_name == "CAT062")
    {
        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat062_fl_measured_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat062_fl_measured_));
    }

            // latitude
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_latitude_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_latitude_));

            // longitude
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_longitude_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_longitude_));

            // assoc
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_utn_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_utn_));

            // rec num, must be last for update process
    assert(dbcont_man.metaCanGetVariable(dbcontent_name, DBContent::meta_var_rec_num_));
    read_set.add(dbcont_man.metaGetVariable(dbcontent_name, DBContent::meta_var_rec_num_));

            // adsb mops
    if (dbcontent_name == "CAT021")
    {
        assert(dbcont_man.canGetVariable(dbcontent_name, DBContent::var_cat021_mops_version_));
        read_set.add(dbcont_man.getVariable(dbcontent_name, DBContent::var_cat021_mops_version_));
    }

    return read_set;
}

void SimpleReconstructor::reset()
{
    loginf << "SimpleReconstructor: reset";

    target_reports_.clear();
    tr_timestamps_.clear();
    tr_ds_.clear();

    ReconstructorBase::reset();
}

SimpleReconstructorSettings& SimpleReconstructor::settings()
{
    return settings_;
}

SimpleReconstructorWidget* SimpleReconstructor::widget() // ownage by caller
{
    SimpleReconstructorWidget* widget = new SimpleReconstructorWidget(*this);

    connect (this, &SimpleReconstructor::updateWidgetsSignal,
            widget, &SimpleReconstructorWidget::updateSlot);

    return widget;
}

void SimpleReconstructor::updateWidgets()
{
    emit updateWidgetsSignal();
}

bool SimpleReconstructor::processSlice_impl()
{
    loginf << "SimpleReconstructor: processSlice_impl: current_slice_begin " << Time::toString(current_slice_begin_)
           << " end " << Time::toString(current_slice_begin_ + slice_duration_)
           << " has next " << hasNextTimeSlice();

            // remove_before_time_, new data >= current_slice_begin_

    bool is_last_slice = !hasNextTimeSlice();

    clearOldTargetReports();

    createTargetReports();

    associatior_.associateNewData();

    auto associations = createAssociations();
    saveAssociations(associations);

    if (is_last_slice)
        saveTargets();

    return true;
}


void SimpleReconstructor::clearOldTargetReports()
{
    loginf << "SimpleReconstructor: clearOldTargetReports: remove_before_time " << Time::toString(remove_before_time_)
           << " size " << target_reports_.size();

    tr_timestamps_.clear();
    tr_ds_.clear();

    for (auto ts_it = target_reports_.begin(); ts_it != target_reports_.end() /* not hoisted */; /* no increment */)
    {
        if (ts_it->second.timestamp_ < remove_before_time_)
        {
            //loginf << "SimpleReconstructor: clearOldTargetReports: removing " << Time::toString(ts_it->second.timestamp_);
            ts_it = target_reports_.erase(ts_it);
        }
        else
        {
            //loginf << "SimpleReconstructor: clearOldTargetReports: keeping " << Time::toString(ts_it->second.timestamp_);

            ts_it->second.in_current_slice_ = false;

                    // add to lookup structures
            tr_timestamps_.insert({ts_it->second.timestamp_, ts_it->second.record_num_});
            tr_ds_[Number::recNumGetDBContId(ts_it->second.record_num_)]
                  [ts_it->second.ds_id_][ts_it->second.line_id_].push_back(
                          ts_it->second.record_num_);

            ++ts_it;
        }
    }

    loginf << "SimpleReconstructor: clearOldTargetReports: size after " << target_reports_.size();

    //    for (auto ts_it = tr_timestamps_.cbegin(); ts_it != tr_timestamps_.cend() /* not hoisted */; /* no increment */)
    //    {
    //        if (ts_it->first < remove_before_time_)
    //            ts_it = tr_timestamps_.erase(ts_it);
    //        else
    //            ++ts_it;
    //    }

    //    // dbcontent -> ds_id -> record_num
    //    //std::map<unsigned int, std::map<unsigned int,std::vector<unsigned long>>>

    //    unsigned int removed_cnt {0}, not_removed_cnt{0};

    //    for (auto& dbcont_it : tr_ds_)
    //    {
    //        for (auto& ds_it : dbcont_it.second)
    //        {
    //            for (auto ts_it = ds_it.second.cbegin(); ts_it != ds_it.second.cend() /* not hoisted */; /* no increment */)
    //            {
    //                if (!target_reports_.count(*ts_it)) // TODO could be made faster
    //                {
    //                    ts_it = ds_it.second.erase(ts_it);
    //                    ++removed_cnt;
    //                }
    //                else
    //                {
    //                    ++ts_it;
    //                    ++not_removed_cnt;
    //                }
    //            }
    //        }
    //    }

    //    loginf << "SimpleReconstructor: clearOldTargetReports: per ds removed_cnt " << removed_cnt
    //           << " not_removed_cnt " << not_removed_cnt;

            // clear old data from targets
    for (auto& tgt_it : targets_)
        tgt_it.second.removeOutdatedTargetReports();

}

void SimpleReconstructor::createTargetReports()
{
    loginf << "SimpleReconstructor: createTargetReports: current_slice_begin " << Time::toString(current_slice_begin_);

    boost::posix_time::ptime ts;
    unsigned long record_num;

    dbContent::targetReport::ReconstructorInfo info;

    DBContentManager& dbcont_man = COMPASS::instance().dbContentManager();

    for (auto& buf_it : *accessor_)
    {
        dbContent::TargetReportAccessor tgt_acc = accessor_->targetReportAccessor(buf_it.first);
        unsigned int buffer_size = tgt_acc.size();

        assert (dbcont_man.existsDBContent(buf_it.first));
        unsigned int dbcont_id = dbcont_man.dbContent(buf_it.first).id();

        for (unsigned int cnt=0; cnt < buffer_size; cnt++)
        {
            record_num = tgt_acc.recordNumber(cnt);
            ts = tgt_acc.timestamp(cnt);

                    //loginf << "SimpleReconstructor: createTargetReports: ts " << Time::toString(ts);

            if (ts >= current_slice_begin_) // insert
            {
                // base info
                info.buffer_index_ = cnt;
                info.record_num_ = record_num;
                info.ds_id_ = tgt_acc.dsID(cnt);
                info.line_id_ = tgt_acc.lineID(cnt);
                info.timestamp_ = ts;

                        // reconstructor info
                info.in_current_slice_ = true;
                info.acad_ = tgt_acc.acad(cnt);
                info.acid_ = tgt_acc.acid(cnt);

                info.mode_a_code_ = tgt_acc.modeACode(cnt);

                info.track_number_ = tgt_acc.trackNumber(cnt);
                info.track_begin_ = tgt_acc.trackBegin(cnt);
                info.track_end_ = tgt_acc.trackEnd(cnt);

                info.position_ = tgt_acc.position(cnt);
                info.position_accuracy_ = tgt_acc.positionAccuracy(cnt);

                info.barometric_altitude_ = tgt_acc.barometricAltitude(cnt);

                info.velocity_ = tgt_acc.velocity(cnt);
                info.velocity_accuracy_ = tgt_acc.velocityAccuracy(cnt);

                info.track_angle_ = tgt_acc.trackAngle(cnt);
                info.ground_bit_ = tgt_acc.groundBit(cnt);

                        // insert info
                assert (!target_reports_.count(record_num));
                target_reports_[record_num] = info;

                        // insert into lookups
                tr_timestamps_.insert({ts, record_num});
                // dbcontent id -> ds_id -> ts ->  record_num

                tr_ds_[dbcont_id][info.ds_id_][info.line_id_].push_back(record_num);
            }
            else // update buffer_index_
            {
                assert (ts >= remove_before_time_);

                if (!target_reports_.count(record_num))
                    logerr << "SimpleReconstructor: createTargetReports: missing prev ts " << Time::toString(ts);

                assert (target_reports_.count(record_num));

                target_reports_.at(record_num).buffer_index_ = cnt;
                target_reports_.at(record_num).in_current_slice_ = false;

                assert (target_reports_.at(record_num).timestamp_ == ts); // just to be sure
            }
        }
    }
}

std::map<unsigned int, std::map<unsigned long, unsigned int>> SimpleReconstructor::createAssociations()
{
    loginf << "SimpleReconstructor: createAssociations";

    std::map<unsigned int, std::map<unsigned long, unsigned int>> associations;
    unsigned int num_assoc {0};

    for (auto& tgt_it : targets_)
    {
        for (auto rn_it : tgt_it.second.target_reports_)
        {
            assert (target_reports_.count(rn_it));

            dbContent::targetReport::ReconstructorInfo& tr = target_reports_.at(rn_it);

            if (tr.in_current_slice_)
            {
                associations[Number::recNumGetDBContId(rn_it)][rn_it] = tgt_it.first;
                ++num_assoc;
            }
        }
        tgt_it.second.associations_written_ = true;

        tgt_it.second.updateCounts();
    }

    loginf << "SimpleReconstructor: createAssociations: done with " << num_assoc << " associated";

    return associations;
}

void SimpleReconstructor::saveAssociations(
    std::map<unsigned int, std::map<unsigned long,unsigned int>> associations)
{
    loginf << "SimpleReconstructor: saveAssociations";

    DBContentManager& dbcontent_man = COMPASS::instance().dbContentManager();

    DBInterface& db_interface = COMPASS::instance().interface();

            // write association info to buffers

    for (auto& cont_assoc_it : associations) // dbcontent -> rec_nums
    {
        unsigned int num_associated {0};
        unsigned int num_not_associated {0};

        unsigned int dbcontent_id = cont_assoc_it.first;
        string dbcontent_name = dbcontent_man.dbContentWithId(cont_assoc_it.first);
        DBContent& dbcontent = dbcontent_man.dbContent(dbcontent_name);

        std::map<unsigned long, unsigned int>& tr_associations = cont_assoc_it.second;

        loginf << "SimpleReconstructor: saveAssociations: db content " << dbcontent_name;

        string rec_num_col_name =
            dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name).dbColumnName();

        string utn_col_name =
            dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).getFor(dbcontent_name).dbColumnName();

        PropertyList properties;
        properties.addProperty(utn_col_name,  DBContent::meta_var_utn_.dataType());
        properties.addProperty(rec_num_col_name,  DBContent::meta_var_rec_num_.dataType());

        shared_ptr <Buffer> buffer {new Buffer(properties)};

        NullableVector<unsigned int>& utn_col_vec = buffer->get<unsigned int>(utn_col_name);
        NullableVector<unsigned long>& rec_num_col_vec = buffer->get<unsigned long>(rec_num_col_name);

        assert (tr_ds_.count(dbcontent_id));

        unsigned int buf_cnt = 0;
        for (auto& ds_it : tr_ds_.at(dbcontent_id))  // iterate over all rec nums
        {
            for (auto& line_it : ds_it.second)
            {
                for (auto& rn_it : line_it.second)
                {
                    assert (target_reports_.count(rn_it));

                    if (!target_reports_.at(rn_it).in_current_slice_)
                        continue;

                    rec_num_col_vec.set(buf_cnt, rn_it);

                    if (tr_associations.count(rn_it))
                    {
                        utn_col_vec.set(buf_cnt, tr_associations.at(rn_it));
                        ++num_associated;
                    }
                    else
                        ++num_not_associated;
                    // else null

                    ++buf_cnt;
                }
            }
        }

        //        assert (accessor_->hasMetaVar<unsigned long>(dbcontent_name, DBContent::meta_var_rec_num_));
        //        NullableVector<unsigned long>& rec_num_vec = accessor_->getMetaVar<unsigned long>(
        //            dbcontent_name, DBContent::meta_var_rec_num_);

        //        assert (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_utn_));
        //        NullableVector<unsigned int>& assoc_vec = accessor_->getMetaVar<unsigned int>(
        //            dbcontent_name, DBContent::meta_var_utn_);

        //        assert (accessor_->has(dbcontent_name));
        //        unsigned int buffer_size = accessor_->get(dbcontent_name)->size();

        //        for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
        //        {
        //            assert (!rec_num_vec.isNull(cnt));

        //            rec_num = rec_num_vec.get(cnt);

        //            if (associations.count(rec_num))
        //            {
        //               //if (assoc_vec.isNull(cnt))
        //                assoc_vec.set(cnt, get<0>(associations.at(rec_num)));
        //                //else
        //                //assoc_vec.getRef(cnt).push_back(get<0>(associations.at(rec_num)));

        //                ++num_associated;
        //            }
        //            else
        //                ++num_not_associated;
        //        }

        //        association_counts_[dbcontent_name] = {buffer_size, num_associated};

        loginf << "SimpleReconstructor: saveAssociations: dcontent " << dbcontent_name
               <<  " assoc " << num_associated << " not assoc " << num_not_associated
               << " buffer size " << buffer->size();

        db_interface.updateBuffer(dbcontent.dbTableName(), rec_num_col_name, buffer);

        loginf << "SimpleReconstructor: saveAssociations: dcontent " << dbcontent_name << " done";
    }

            // delete all data from buffer except rec_nums and associations, rename to db column names
    //    for (auto& buf_it : *accessor_)
    //    {
    //        string dbcontent_name = buf_it.first;

    //        string rec_num_var_name =
    //            dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name).name();
    //        string rec_num_col_name =
    //            dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name).dbColumnName();

    //        string utn_var_name =
    //            dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).getFor(dbcontent_name).name();
    //        string utn_col_name =
    //            dbcontent_man.metaVariable(DBContent::meta_var_utn_.name()).getFor(dbcontent_name).dbColumnName();

    //        PropertyList properties = buf_it.second->properties();

    //        for (auto& prop_it : properties.properties())
    //        {
    //            if (prop_it.name() == rec_num_var_name)
    //                buf_it.second->rename<unsigned long>(rec_num_var_name, rec_num_col_name);
    //            else if (prop_it.name() == utn_var_name)
    //                buf_it.second->rename<unsigned int>(utn_var_name, utn_col_name);
    //            else
    //                buf_it.second->deleteProperty(prop_it);
    //        }
    //    }

    //            // actually save data, ok since DB job
    //    for (auto& buf_it : *accessor_)
    //    {
    //        string dbcontent_name = buf_it.first;

    //        loginf << "CreateAssociationsJob: saveAssociations: saving for " << dbcontent_name;

    //        DBContent& dbcontent = dbcontent_man.dbContent(buf_it.first);
    //        dbContent::Variable& key_var =
    //            dbcontent_man.metaVariable(DBContent::meta_var_rec_num_.name()).getFor(dbcontent_name);

    //        unsigned int chunk_size = 50000;

    //        unsigned int steps = buf_it.second->size() / chunk_size;

    //        unsigned int index_from = 0;
    //        unsigned int index_to = 0;

    //        for (unsigned int cnt = 0; cnt <= steps; cnt++)
    //        {
    //            index_from = cnt * chunk_size;
    //            index_to = index_from + chunk_size;

    //            if (index_to > buf_it.second->size() - 1)
    //                index_to = buf_it.second->size() - 1;

    //            loginf << "CreateAssociationsJob: saveAssociations: step " << cnt << " steps " << steps << " from "
    //                   << index_from << " to " << index_to;

    //            db_interface_.updateBuffer(dbcontent.dbTableName(), key_var.dbColumnName(),
    //                                       buf_it.second, index_from, index_to);

    //        }
    //    }

    loginf << "SimpleReconstructor: saveAssociations: done";
}

void SimpleReconstructor::saveTargets()
{
    loginf << "SimpleReconstructor: saveTargets: num " << targets_.size();

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();

    for (auto& tgt_it : targets_)
    {
        cont_man.createNewTarget(tgt_it.first);

        dbContent::Target& target = cont_man.target(tgt_it.first);

        //target.useInEval(tgt_it.second.use_in_eval_);

        //if (tgt_it.second.comment_.size())
        //    target.comment(tgt_it.second.comment_);

        target.aircraftAddresses(tgt_it.second.acads_);
        target.aircraftIdentifications(tgt_it.second.acids_);
        target.modeACodes(tgt_it.second.mode_as_);

        if (tgt_it.second.hasTimestamps())
        {
            target.timeBegin(tgt_it.second.timestamp_min_);
            target.timeEnd(tgt_it.second.timestamp_max_);
        }

        if (tgt_it.second.hasModeC())
            target.modeCMinMax(*tgt_it.second.mode_c_min_, *tgt_it.second.mode_c_max_);

                // set counts
        for (auto& count_it : tgt_it.second.getDBContentCounts())
            target.dbContentCount(count_it.first, count_it.second);

                // set adsb stuff
//        if (tgt_it.second.hasADSBMOPSVersion() && tgt_it.second.getADSBMOPSVersions().size())
//            target.adsbMOPSVersions(tgt_it.second.getADSBMOPSVersions());
    }

    cont_man.saveTargets();

    loginf << "SimpleReconstructor: saveTargets: done";
}
