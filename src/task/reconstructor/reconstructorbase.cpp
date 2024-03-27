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

#include "reconstructorbase.h"
#include "reconstructortask.h"
#include "compass.h"
#include "dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
#include "logger.h"
#include "timeconv.h"

#include "dbcontent/variable/metavariable.h"
#include "targetreportaccessor.h"
#include "dbinterface.h"
#include "number.h"

using namespace std;
using namespace Utils;

/**
 */
ReconstructorBase::ReconstructorBase(const std::string& class_id, const std::string& instance_id,
                                     ReconstructorTask& task)
    : Configurable (class_id, instance_id, &task)
{
    accessor_ = make_shared<dbContent::DBContentAccessor>();
}

/**
 */
ReconstructorBase::~ReconstructorBase() = default;

bool ReconstructorBase::hasNextTimeSlice()
{
    if (current_slice_begin_.is_not_a_date_time())
    {
        assert (COMPASS::instance().dbContentManager().hasMinMaxTimestamp());
        std::tie(timestamp_min_, timestamp_max_) = COMPASS::instance().dbContentManager().minMaxTimestamp();

        current_slice_begin_ = timestamp_min_;
        next_slice_begin_ = timestamp_min_; // first slice

        loginf << "ReconstructorBase: hasNextTimeSlice: new min " << Time::toString(current_slice_begin_)
               << " max " << Time::toString(timestamp_max_) << " first_slice " << first_slice_;
    }

    assert (!current_slice_begin_.is_not_a_date_time());
    assert (!timestamp_max_.is_not_a_date_time());

    first_slice_ = current_slice_begin_ == timestamp_min_;

    return next_slice_begin_ < timestamp_max_;
}

TimeWindow ReconstructorBase::getNextTimeSlice()
{
    assert (hasNextTimeSlice());

    current_slice_begin_ = next_slice_begin_;

    assert (!current_slice_begin_.is_not_a_date_time());
    assert (!timestamp_max_.is_not_a_date_time());

    assert (current_slice_begin_ <= timestamp_max_);

    boost::posix_time::ptime current_slice_end = current_slice_begin_ + slice_duration_;

    TimeWindow window {current_slice_begin_, current_slice_end};

    loginf << "ReconstructorBase: getNextTimeSlice: current_slice_begin " << Time::toString(current_slice_begin_)
           << " current_slice_end " << Time::toString(current_slice_end);

    first_slice_ = current_slice_begin_ == timestamp_min_;

    remove_before_time_ = current_slice_begin_ - outdated_duration_;
    write_before_time_ = current_slice_end - outdated_duration_;

    next_slice_begin_ = current_slice_end; // for next iteration

            //assert (current_slice_begin_ <= timestamp_max_); can be bigger

    return window;
}

/**
 */
bool ReconstructorBase::processSlice(Buffers&& buffers)
{
    loginf << "ReconstructorBase: processSlice: first_slice " << first_slice_;

    if (!first_slice_)
        accessor_->removeContentBeforeTimestamp(remove_before_time_);

    accessor_->add(buffers);

    return processSlice_impl();
}

void ReconstructorBase::clearOldTargetReports()
{
    loginf << "ReconstructorBase: clearOldTargetReports: remove_before_time " << Time::toString(remove_before_time_)
           << " size " << target_reports_.size();

    tr_timestamps_.clear();
    tr_ds_.clear();

    for (auto ts_it = target_reports_.begin(); ts_it != target_reports_.end() /* not hoisted */; /* no increment */)
    {
        if (ts_it->second.timestamp_ < remove_before_time_)
        {
            //loginf << "ReconstructorBase: clearOldTargetReports: removing " << Time::toString(ts_it->second.timestamp_);
            ts_it = target_reports_.erase(ts_it);
        }
        else
        {
            //loginf << "ReconstructorBase: clearOldTargetReports: keeping " << Time::toString(ts_it->second.timestamp_);

            ts_it->second.in_current_slice_ = false;

                    // add to lookup structures
            tr_timestamps_.insert({ts_it->second.timestamp_, ts_it->second.record_num_});
            tr_ds_[Number::recNumGetDBContId(ts_it->second.record_num_)]
                  [ts_it->second.ds_id_][ts_it->second.line_id_].push_back(
                          ts_it->second.record_num_);

            ++ts_it;
        }
    }

    loginf << "ReconstructorBase: clearOldTargetReports: size after " << target_reports_.size();

            // clear old data from targets
    for (auto& tgt_it : targets_)
        tgt_it.second.removeOutdatedTargetReports();

}

void ReconstructorBase::createTargetReports()
{
    loginf << "ReconstructorBase: createTargetReports: current_slice_begin " << Time::toString(current_slice_begin_);

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

                    //loginf << "ReconstructorBase: createTargetReports: ts " << Time::toString(ts);

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
                    logerr << "ReconstructorBase: createTargetReports: missing prev ts " << Time::toString(ts);

                assert (target_reports_.count(record_num));

                target_reports_.at(record_num).buffer_index_ = cnt;
                target_reports_.at(record_num).in_current_slice_ = false;

                assert (target_reports_.at(record_num).timestamp_ == ts); // just to be sure
            }
        }
    }
}

std::map<unsigned int, std::map<unsigned long, unsigned int>> ReconstructorBase::createAssociations()
{
    loginf << "ReconstructorBase: createAssociations";

    std::map<unsigned int, std::map<unsigned long, unsigned int>> associations;
    unsigned int num_assoc {0};

    for (auto& tgt_it : targets_)
    {
        for (auto rn_it : tgt_it.second.target_reports_)
        {
            assert (target_reports_.count(rn_it));

            dbContent::targetReport::ReconstructorInfo& tr = target_reports_.at(rn_it);

            if (tr.timestamp_ < write_before_time_) // tr.in_current_slice_
            {
                associations[Number::recNumGetDBContId(rn_it)][rn_it] = tgt_it.first;
                ++num_assoc;
            }
        }
        tgt_it.second.associations_written_ = true;

        tgt_it.second.updateCounts();
    }

    loginf << "ReconstructorBase: createAssociations: done with " << num_assoc << " associated";

    return associations;
}

void ReconstructorBase::saveAssociations(
    std::map<unsigned int, std::map<unsigned long,unsigned int>> associations)
{
    loginf << "ReconstructorBase: saveAssociations";

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

        loginf << "ReconstructorBase: saveAssociations: db content " << dbcontent_name;

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

                    if (target_reports_.at(rn_it).timestamp_ >= write_before_time_)
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

        loginf << "ReconstructorBase: saveAssociations: dcontent " << dbcontent_name
               <<  " assoc " << num_associated << " not assoc " << num_not_associated
               << " buffer size " << buffer->size();

        // TODO move to DB job
        db_interface.updateBuffer(dbcontent.dbTableName(), rec_num_col_name, buffer);

        loginf << "ReconstructorBase: saveAssociations: dcontent " << dbcontent_name << " done";
    }

    loginf << "ReconstructorBase: saveAssociations: done";
}

void ReconstructorBase::saveReferences()
{
    loginf << "ReconstructorBase: saveReferences: num " << targets_.size();

    DBContentManager& cont_man = COMPASS::instance().dbContentManager();

    std::shared_ptr<Buffer> buffer;

    for (auto& tgt_it : targets_)
    {
        if (!buffer)
            buffer = tgt_it.second.getReferenceBuffer(); // also updates count
        else
        {
            auto tmp = tgt_it.second.getReferenceBuffer();
            buffer->seizeBuffer(*tmp);
        }
    }

    if (buffer && buffer->size())
    {
        loginf << "ReconstructorBase: saveReferences: buffer size " << buffer->size();

        cont_man.insertData({{buffer->dbContentName(), buffer}});
    }
    else
        loginf << "ReconstructorBase: saveReferences: empty buffer";
}

void ReconstructorBase::saveTargets()
{
    loginf << "ReconstructorBase: saveTargets: num " << targets_.size();

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

    loginf << "ReconstructorBase: saveTargets: done";
}



void ReconstructorBase::reset()
{
    current_slice_begin_ = {};
    next_slice_begin_ = {};
    timestamp_min_ = {};
    timestamp_max_ = {};
}


