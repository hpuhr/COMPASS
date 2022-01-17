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

#include "evaluationdata.h"
#include "evaluationdatawidget.h"
#include "evaluationmanager.h"
#include "dbcontent/dbcontent.h"
#include "dbcontent/variable/variable.h"
#include "dbcontent/variable/metavariable.h"
#include "buffer.h"
#include "stringconv.h"
#include "compass.h"
#include "dbcontent/dbcontentmanager.h"

#include <QApplication>
#include <QThread>
#include <QProgressDialog>
#include <QLabel>

#include "boost/date_time/posix_time/posix_time.hpp"

#include <sstream>

using namespace std;
using namespace Utils;

EvaluationData::EvaluationData(EvaluationManager& eval_man)
    : eval_man_(eval_man)
{

}

void EvaluationData::addReferenceData (DBContent& object, std::shared_ptr<Buffer> buffer)
{
    loginf << "EvaluationData: addReferenceData: dbo " << object.name() << " size " << buffer->size();

    if (!object.hasAssociations())
    {
        logwrn << "EvaluationData: addReferenceData: object " << object.name() << " has no associations";
        unassociated_ref_cnt_ = buffer->size();

        return;
    }

    assert (!ref_buffer_);
    ref_buffer_ = buffer;

    // preset variable names
    DBContentManager& object_manager = COMPASS::instance().dbContentManager();

    string dbo_name = ref_buffer_->dboName();

    ref_latitude_name_ = object_manager.metaVariable("pos_lat_deg").getFor(dbo_name).name();
    ref_longitude_name_ = object_manager.metaVariable("pos_long_deg").getFor(dbo_name).name();
    ref_target_address_name_ = object_manager.metaVariable("target_addr").getFor(dbo_name).name();
    ref_callsign_name_ = object_manager.metaVariable("callsign").getFor(dbo_name).name();

    // mc
    ref_modec_name_ = object_manager.metaVariable("modec_code_ft").getFor(dbo_name).name();

    if (object_manager.metaVariable("modec_g").existsIn(dbo_name))
        ref_modec_g_name_ = object_manager.metaVariable("modec_g").getFor(dbo_name).name();

    if (object_manager.metaVariable("modec_v").existsIn(dbo_name))
        ref_modec_v_name_ = object_manager.metaVariable("modec_v").getFor(dbo_name).name();


    if (dbo_name == "Tracker")
    {
        has_ref_altitude_secondary_ = true;
        ref_altitude_secondary_name_ = "tracked_alt_baro_ft";
    }

    // m3a
    ref_modea_name_ = object_manager.metaVariable("mode3a_code").getFor(dbo_name).name();

    if (object_manager.metaVariable("mode3a_g").existsIn(dbo_name))
        ref_modea_g_name_ = object_manager.metaVariable("mode3a_g").getFor(dbo_name).name();

    if (object_manager.metaVariable("mode3a_v").existsIn(dbo_name))
        ref_modea_v_name_ = object_manager.metaVariable("mode3a_v").getFor(dbo_name).name();

    // ground bit
    if (object_manager.metaVariable("ground_bit").existsIn(dbo_name))
        ref_ground_bit_name_ = object_manager.metaVariable("ground_bit").getFor(dbo_name).name();

    // speed & track_angle
    if (dbo_name == "ADSB")
    {
        ref_spd_ground_speed_kts_name_ = "groundspeed_kt";
        ref_spd_track_angle_deg_name_ = "track_angle_deg";
    }
    else if (dbo_name == "MLAT")
    {
        ref_spd_x_ms_name_ = "velocity_vx_ms";
        ref_spd_y_ms_name_ = "velocity_vy_ms";
    }
    else if (dbo_name == "Radar")
    {
        ref_spd_ground_speed_kts_name_ = "track_groundspeed_kt";
        ref_spd_track_angle_deg_name_ = "track_heading_deg";
    }
    else if (dbo_name == "Tracker" || dbo_name == "RefTraj")
    {
        ref_spd_ground_speed_kts_name_ = "groundspeed_kt";
        ref_spd_track_angle_deg_name_ = "heading_deg";
    }

    set<int> active_srcs = eval_man_.activeDataSourcesRef();
    bool use_active_srcs = (eval_man_.dboNameRef() == eval_man_.dboNameTst());
    unsigned int num_skipped {0};

    const DBOAssociationCollection& associations = object.associations();

    unsigned int buffer_size = buffer->size();
    NullableVector<int>& rec_nums = buffer->get<int>("rec_num");
    NullableVector<float>& tods = buffer->get<float>("tod");
    NullableVector<int>& ds_ids = buffer->get<int>("ds_id");

    unsigned int rec_num;
    float tod;

    loginf << "EvaluationData: addReferenceData: adding target data";

    for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
    {
        assert (!ds_ids.isNull(cnt));

        if (use_active_srcs && !active_srcs.count(ds_ids.get(cnt))) // skip those entries not for tst src
        {
            ++num_skipped;
            continue;
        }

        assert (!rec_nums.isNull(cnt));
        assert (!tods.isNull(cnt));

        rec_num = rec_nums.get(cnt);
        tod = tods.get(cnt);

        std::vector<unsigned int> utn_vec = associations.getUTNsFor(rec_num);

        if (!utn_vec.size())
        {
            ++unassociated_ref_cnt_;
            continue;
        }

        for (auto utn_it : utn_vec)
        {
            if (!hasTargetData(utn_it))
                //target_data_.emplace(target_data_.end(), utn_it, *this, eval_man_);
                target_data_.push_back({utn_it, *this, eval_man_});

            assert (hasTargetData(utn_it));

            auto tr_tag_it = target_data_.get<target_tag>().find(utn_it);
            auto index_it = target_data_.project<0>(tr_tag_it); // get iterator for random access

            //            if (!targetData(utn_it).hasRefBuffer())
            //                target_data_.modify(index_it, [buffer](EvaluationTargetData& t) { t.setRefBuffer(buffer); });

            target_data_.modify(index_it, [tod, cnt](EvaluationTargetData& t) { t.addRefIndex(tod, cnt); });

            ++associated_ref_cnt_;
        }
    }

    loginf << "EvaluationData: addReferenceData: num targets " << target_data_.size()
           << " ref associated cnt " << associated_ref_cnt_ << " unassoc " << unassociated_ref_cnt_
           << " num_skipped " << num_skipped;
}

void EvaluationData::addTestData (DBContent& object, std::shared_ptr<Buffer> buffer)
{
    loginf << "EvaluationData: addTestData: dbo " << object.name() << " size " << buffer->size();

    if (!object.hasAssociations())
    {
        logwrn << "EvaluationData: addTestData: object " << object.name() << " has no associations";
        unassociated_tst_cnt_ = buffer->size();

        return;
    }

    assert (!tst_buffer_);
    tst_buffer_ = buffer;

    DBContentManager& object_manager = COMPASS::instance().dbContentManager();

    string dbo_name = tst_buffer_->dboName();

    tst_latitude_name_ = object_manager.metaVariable("pos_lat_deg").getFor(dbo_name).name();
    tst_longitude_name_ = object_manager.metaVariable("pos_long_deg").getFor(dbo_name).name();
    tst_target_address_name_ = object_manager.metaVariable("target_addr").getFor(dbo_name).name();
    tst_callsign_name_ = object_manager.metaVariable("callsign").getFor(dbo_name).name();

    // m3a
    tst_modea_name_ = object_manager.metaVariable("mode3a_code").getFor(dbo_name).name();

    if (object_manager.metaVariable("mode3a_g").existsIn(dbo_name))
        tst_modea_g_name_ = object_manager.metaVariable("mode3a_g").getFor(dbo_name).name();

    if (object_manager.metaVariable("mode3a_v").existsIn(dbo_name))
        tst_modea_v_name_ = object_manager.metaVariable("mode3a_v").getFor(dbo_name).name();

    // mc
    tst_modec_name_ = object_manager.metaVariable("modec_code_ft").getFor(dbo_name).name();
    if (object_manager.metaVariable("modec_g").existsIn(dbo_name))
        tst_modec_g_name_ = object_manager.metaVariable("modec_g").getFor(dbo_name).name();

    if (object_manager.metaVariable("modec_v").existsIn(dbo_name))
        tst_modec_v_name_ = object_manager.metaVariable("modec_v").getFor(dbo_name).name();

    // ground bit
    if (object_manager.metaVariable("ground_bit").existsIn(dbo_name))
        tst_ground_bit_name_ = object_manager.metaVariable("ground_bit").getFor(dbo_name).name();

    // track num
    if (object_manager.metaVariable("track_num").existsIn(dbo_name) && dbo_name != "ADSB" ) // HACK
        tst_track_num_name_ = object_manager.metaVariable("track_num").getFor(dbo_name).name();

    // speed & track_angle
    if (dbo_name == "ADSB")
    {
        tst_spd_ground_speed_kts_name_ = "groundspeed_kt";
        tst_spd_track_angle_deg_name_ = "track_angle_deg";
    }
    else if (dbo_name == "MLAT")
    {
        tst_spd_x_ms_name_ = "velocity_vx_ms";
        tst_spd_y_ms_name_ = "velocity_vy_ms";
    }
    else if (dbo_name == "Radar")
    {
        tst_spd_ground_speed_kts_name_ = "track_groundspeed_kt";
        tst_spd_track_angle_deg_name_ = "track_heading_deg";
    }
    else if (dbo_name == "Tracker" || dbo_name == "RefTraj")
    {
        tst_spd_ground_speed_kts_name_ = "groundspeed_kt";
        tst_spd_track_angle_deg_name_ = "heading_deg";

        tst_multiple_srcs_name_ = "multiple_sources";
        tst_track_lu_ds_id_name_ = "track_lu_ds_id";
    }

    set<int> active_srcs = eval_man_.activeDataSourcesTst();
    bool use_active_srcs = (eval_man_.dboNameRef() == eval_man_.dboNameTst());
    unsigned int num_skipped {0};

    const DBOAssociationCollection& associations = object.associations();

    unsigned int buffer_size = buffer->size();
    NullableVector<int>& rec_nums = buffer->get<int>("rec_num");
    NullableVector<float>& tods = buffer->get<float>("tod");
    NullableVector<int>& ds_ids = buffer->get<int>("ds_id");

    unsigned int rec_num;
    float tod;

    loginf << "EvaluationData: addTestData: adding target data";

    for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
    {
        assert (!ds_ids.isNull(cnt));

        if (use_active_srcs && !active_srcs.count(ds_ids.get(cnt))) // skip those entries not for tst src
        {
            ++num_skipped;
            continue;
        }

        assert (!rec_nums.isNull(cnt));
        assert (!tods.isNull(cnt));

        rec_num = rec_nums.get(cnt);
        tod = tods.get(cnt);

        std::vector<unsigned int> utn_vec = associations.getUTNsFor(rec_num);

        if (!utn_vec.size())
        {
            ++unassociated_tst_cnt_;
            continue;
        }

        for (auto utn_it : utn_vec)
        {
            if (!hasTargetData(utn_it))
                //target_data_.emplace(target_data_.end(), utn_it, *this, eval_man_);
                target_data_.push_back({utn_it, *this, eval_man_});
                //target_data_.emplace_back(utn_it, *this, eval_man_);

            assert (hasTargetData(utn_it));

            auto tr_tag_it = target_data_.get<target_tag>().find(utn_it);
            auto index_it = target_data_.project<0>(tr_tag_it);  // get iterator for random access

            //            if (!targetData(utn_it).hasTstBuffer())
            //                target_data_.modify(index_it, [buffer](EvaluationTargetData& t) { t.setTstBuffer(buffer); });

            target_data_.modify(index_it, [tod, cnt](EvaluationTargetData& t) { t.addTstIndex(tod, cnt); });

            ++associated_tst_cnt_;
        }
    }

    loginf << "EvaluationData: addTestData: num targets " << target_data_.size()
           << " tst associated cnt " << associated_tst_cnt_ << " unassoc " << unassociated_tst_cnt_
           << " num_skipped " << num_skipped;
}

void EvaluationData::finalize ()
{
    loginf << "EvaluationData: finalize";

    assert (!finalized_);

    boost::posix_time::ptime start_time;
    boost::posix_time::ptime elapsed_time;

    start_time = boost::posix_time::microsec_clock::local_time();

    unsigned int num_targets = target_data_.size();

    beginResetModel();

    QProgressDialog postprocess_dialog_ ("", "", 0, num_targets);
    postprocess_dialog_.setWindowTitle("Finalizing Evaluation Data");
    postprocess_dialog_.setCancelButton(nullptr);
    postprocess_dialog_.setWindowModality(Qt::ApplicationModal);

    QLabel* progress_label = new QLabel("", &postprocess_dialog_);
    progress_label->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    postprocess_dialog_.setLabel(progress_label);

    postprocess_dialog_.show();

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    vector<bool> done_flags;
    done_flags.resize(target_data_.size());

    bool done = false;
    unsigned int tmp_done_cnt;

    boost::posix_time::time_duration time_diff;
    double elapsed_time_s;
    double time_per_eval, remaining_time_s;

    string remaining_time_str;

    EvaluateTargetsFinalizeTask* t = new (tbb::task::allocate_root()) EvaluateTargetsFinalizeTask(
                target_data_, done_flags, done);
    tbb::task::enqueue(*t);

    postprocess_dialog_.setValue(0);

    boost::posix_time::ptime last_elapsed_time = boost::posix_time::microsec_clock::local_time();
    elapsed_time = boost::posix_time::microsec_clock::local_time();
    unsigned int last_tmp_done_cnt = 0;

    boost::posix_time::time_duration tmp_time_diff;
    double tmp_elapsed_time_s;

    while (!done)
    {
        tmp_done_cnt = 0;

        for (auto done_it : done_flags)
        {
            if (done_it)
                tmp_done_cnt++;
        }

        assert (tmp_done_cnt <= num_targets);

        if (tmp_done_cnt && tmp_done_cnt != last_tmp_done_cnt)
        {
            elapsed_time = boost::posix_time::microsec_clock::local_time();

            time_diff = elapsed_time - start_time;
            elapsed_time_s = time_diff.total_milliseconds() / 1000.0;

            tmp_time_diff = elapsed_time - last_elapsed_time;
            tmp_elapsed_time_s = tmp_time_diff.total_milliseconds() / 1000.0;

            time_per_eval = 0.95*time_per_eval + 0.05*(tmp_elapsed_time_s/(double)(tmp_done_cnt-last_tmp_done_cnt));
            // halfnhalf
            remaining_time_s = (double)(num_targets-tmp_done_cnt)*time_per_eval;

            //        loginf << " UGA num_targets " << num_targets << " tmp_done_cnt " << tmp_done_cnt
            //               << " elapsed_time_s " << elapsed_time_s;

            postprocess_dialog_.setLabelText(
                        ("Elapsed: "+String::timeStringFromDouble(elapsed_time_s, false)
                         +"\nRemaining: "+String::timeStringFromDouble(remaining_time_s, false)
                         +" (estimated)").c_str());

            postprocess_dialog_.setValue(tmp_done_cnt);

            last_tmp_done_cnt = tmp_done_cnt;
            last_elapsed_time = elapsed_time;
        }

        if (!done)
        {
            QCoreApplication::processEvents();
            QThread::msleep(200);
        }
    }

    //    unsigned int num_targets = target_data_.size();

    //    tbb::parallel_for(uint(0), num_targets, [&](unsigned int cnt)
    //    {
    //        target_data_[cnt].finalize();
    //    });

    //    for (auto target_it = target_data_.begin(); target_it != target_data_.end(); ++target_it)
    //        target_data_.modify(target_it, [&](EvaluationTargetData& t) { t.finalize(); });

    finalized_ = true;

    endResetModel();

    if (widget_)
        widget_->resizeColumnsToContents();

    postprocess_dialog_.close();

    QApplication::restoreOverrideCursor();
}

bool EvaluationData::hasTargetData (unsigned int utn)
{
    return target_data_.get<target_tag>().find(utn) != target_data_.get<target_tag>().end();
}

const EvaluationTargetData& EvaluationData::targetData(unsigned int utn)
{
    assert (hasTargetData(utn));

    return *target_data_.get<target_tag>().find(utn);
}

void EvaluationData::clear()
{
    beginResetModel();

    ref_buffer_ = nullptr;
    tst_buffer_ = nullptr;

    target_data_.clear();
    finalized_ = false;

    unassociated_ref_cnt_ = 0;
    associated_ref_cnt_ = 0;

    unassociated_tst_cnt_ = 0;
    associated_tst_cnt_ = 0;

    endResetModel();
}

QVariant EvaluationData::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || !finalized_)
        return QVariant();

    switch (role)
    {
        case Qt::CheckStateRole:
            {
                if (index.column() == 0)  // selected special case
                {
                    assert (index.row() >= 0);
                    assert (index.row() < target_data_.size());

                    const EvaluationTargetData& target = target_data_.at(index.row());

                    if (eval_man_.useUTN(target.utn_))
                        return Qt::Checked;
                    else
                        return Qt::Unchecked;
                }
                else
                    return QVariant();
            }
        case Qt::BackgroundRole:
            {
                assert (index.row() >= 0);
                assert (index.row() < target_data_.size());

                const EvaluationTargetData& target = target_data_.at(index.row());

                if (!target.use())
                    return QBrush(Qt::lightGray);
                else
                    return QVariant();

            }
        case Qt::DisplayRole:
        case Qt::EditRole:
            {
                logdbg << "EvaluationData: data: display role: row " << index.row() << " col " << index.column();

                assert (index.row() >= 0);
                assert (index.row() < target_data_.size());

                const EvaluationTargetData& target = target_data_.at(index.row());

                logdbg << "EvaluationData: data: got utn " << target.utn_;

                assert (index.column() < table_columns_.size());
                std::string col_name = table_columns_.at(index.column()).toStdString();

                if (col_name == "Use")
                {
                    return QVariant();
                }
                else if (col_name == "UTN")
                {
                    return target.utn_;
                }
                else if (col_name == "Comment")
                {
                    return eval_man_.utnComment(target.utn_).c_str();
                }
                else if (col_name == "Begin")
                {
                    return target.timeBeginStr().c_str();
                }
                else if (col_name == "End")
                {
                    return target.timeEndStr().c_str();
                }
                else if (col_name == "#All")
                {
                    return target.numUpdates();
                }
                else if (col_name == "#Ref")
                {
                    return target.numRefUpdates();
                }
                else if (col_name == "#Tst")
                {
                    return target.numTstUpdates();
                }
                else if (col_name == "Callsign")
                {
                    return target.callsignsStr().c_str();
                }
                else if (col_name == "TA")
                {
                    return target.targetAddressesStr().c_str();
                }
                else if (col_name == "M3/A")
                {
                    return target.modeACodesStr().c_str();
                }
                else if (col_name == "MC Min")
                {
                    return target.modeCMinStr().c_str();
                }
                else if (col_name == "MC Max")
                {
                    return target.modeCMaxStr().c_str();
                }

            }
        case Qt::DecorationRole:
            {
                assert (index.column() < table_columns_.size());

                //                if (table_columns_.at(index.column()) == "status")
                //                {
                //                    assert (index.row() >= 0);
                //                    assert (index.row() < view_points_.size());

                //                    const ViewPoint& vp = view_points_.at(index.row());

                //                    const json& data = vp.data().at("status");
                //                    assert (data.is_string());

                //                    std::string status = data;

                //                    if (status == "open")
                //                        return open_icon_;
                //                    else if (status == "closed")
                //                        return closed_icon_;
                //                    else if (status == "todo")
                //                        return todo_icon_;
                //                    else
                //                        return unknown_icon_;
                //                }
                //                else
                return QVariant();
            }
        case Qt::UserRole: // to find the checkboxes
            {
                if (index.column() == 0)
                {
                    assert (index.row() >= 0);
                    assert (index.row() < target_data_.size());

                    const EvaluationTargetData& target = target_data_.at(index.row());
                    return target.utn_;
                }
                else if (index.column() == 2) // comment
                {
                    assert (index.row() >= 0);
                    assert (index.row() < target_data_.size());

                    const EvaluationTargetData& target = target_data_.at(index.row());
                    return ("comment_"+to_string(target.utn_)).c_str();
                }
            }
        default:
            {
                return QVariant();
            }
    }
}

bool EvaluationData::setData(const QModelIndex &index, const QVariant& value, int role)
{
    if (!index.isValid() /*|| role != Qt::EditRole*/)
        return false;

    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        assert (index.row() >= 0);
        assert (index.row() < target_data_.size());

        auto it = target_data_.begin()+index.row();

        bool checked = (Qt::CheckState)value.toInt() == Qt::Checked;
        loginf << "EvaluationData: setData: utn " << it->utn_ <<" check state " << checked;

        eval_man_.useUTN(it->utn_, checked, false);
        target_data_.modify(it, [value,checked](EvaluationTargetData& p) { p.use(checked); });

        emit dataChanged(index, EvaluationData::index(index.row(), columnCount()-1));
        return true;
    }
    else if (role == Qt::EditRole && index.column() == 2) // comment
    {
        assert (index.row() >= 0);
        assert (index.row() < target_data_.size());

        auto it = target_data_.begin()+index.row();
        eval_man_.utnComment(it->utn_, value.toString().toStdString(), false);
        //target_data_.modify(it, [value](EvaluationTargetData& p) { p.use(false); });
        return true;
    }

    return false;
}


QVariant EvaluationData::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        assert (section < table_columns_.size());
        return table_columns_.at(section);
    }

    return QVariant();
}

QModelIndex EvaluationData::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

int EvaluationData::rowCount(const QModelIndex& parent) const
{
    if (!finalized_)
        return 0;

    return target_data_.size();
}

int EvaluationData::columnCount(const QModelIndex& parent) const
{
    return table_columns_.size();
}

QModelIndex EvaluationData::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

Qt::ItemFlags EvaluationData::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    assert (index.column() < table_columns_.size());

    //    if (table_columns_.at(index.column()) == "comment")
    //        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    //    else
    if (index.column() == 0) // Use
    {
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
        //        flags |= Qt::ItemIsEnabled;
        //        flags |= Qt::ItemIsUserCheckable;
        //        flags |= Qt::ItemIsEditable;
        // flags |= Qt::ItemIsSelectable;
    }
    else if (index.column() == 2)
    {
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    else
        return QAbstractItemModel::flags(index);
}

const EvaluationTargetData& EvaluationData::getTargetOf (const QModelIndex& index)
{
    assert (index.isValid());

    assert (index.row() >= 0);
    assert (index.row() < target_data_.size());

    const EvaluationTargetData& target = target_data_.at(index.row());

    return target;
}


void EvaluationData::setUseTargetData (unsigned int utn, bool value)
{
    loginf << "EvaluationData: setUseTargetData: utn " << utn << " value " << value;

    assert (hasTargetData(utn));

    QModelIndexList items = match(
                index(0, 0),
                Qt::UserRole,
                QVariant(utn),
                1, // look *
                Qt::MatchExactly); // look *

    assert (items.size() == 1);

    setData(items.at(0), {value ? Qt::Checked: Qt::Unchecked}, Qt::CheckStateRole);
}

void EvaluationData::setUseAllTargetData (bool value)
{
    loginf << "EvaluationData: setUseAllTargetData: value " << value;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    beginResetModel();

    eval_man_.useAllUTNs(value);

    endResetModel();

    QApplication::restoreOverrideCursor();
}

void EvaluationData::clearComments ()
{
    loginf << "EvaluationData: clearComments";

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    beginResetModel();

    eval_man_.clearUTNComments();

    endResetModel();

    QApplication::restoreOverrideCursor();
}

void EvaluationData::setUseByFilter ()
{
    loginf << "EvaluationData: setUseByFilter";

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    beginResetModel();

    eval_man_.filterUTNs();

    endResetModel();

    QApplication::restoreOverrideCursor();
}

void EvaluationData::setTargetDataComment (unsigned int utn, std::string comment)
{
    loginf << "EvaluationData: setTargetDataComment: utn " << utn << " comment '" << comment << "'";

    assert (hasTargetData(utn));

    QModelIndexList items = match(
                index(0, 0),
                Qt::UserRole,
                QVariant(("comment_"+to_string(utn)).c_str()),
                1, // look *
                Qt::MatchExactly); // look *

    if (items.size() == 1)
        setData(items.at(0), comment.c_str(), Qt::CheckStateRole);
}

EvaluationDataWidget* EvaluationData::widget()
{
    if (!widget_)
        widget_.reset(new EvaluationDataWidget(*this, eval_man_));

    return widget_.get();
}

EvaluationDataFilterDialog& EvaluationData::dialog()
{
    if (!dialog_)
        dialog_.reset(new EvaluationDataFilterDialog(*this, eval_man_));

    return *dialog_;
}
