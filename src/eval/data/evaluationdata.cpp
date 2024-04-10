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
#include "dbcontentmanager.h"
#include "dbcontent/dbcontent.h"
//#include "dbcontent/variable/variable.h"
//#include "dbcontent/variable/metavariable.h"
#include "buffer.h"
//#include "stringconv.h"
//#include "compass.h"
#include "dbcontent/dbcontentmanager.h"
#include "util/async.h"

#include <QApplication>
#include <QThread>
#include <QProgressDialog>
#include <QLabel>

#include "util/tbbhack.h"

#include "boost/date_time/posix_time/posix_time.hpp"

#include <sstream>
#include <future>

using namespace std;
using namespace Utils;
using namespace nlohmann;
using namespace boost::posix_time;

EvaluationData::EvaluationData(EvaluationManager& eval_man, DBContentManager& dbcont_man)
    : eval_man_(eval_man), dbcont_man_(dbcont_man)
{
    accessor_ = make_shared<dbContent::DBContentAccessor>();

    connect(&dbcont_man, &DBContentManager::targetChangedSignal, this, &EvaluationData::targetChangedSlot);
    connect(&dbcont_man, &DBContentManager::allTargetsChangedSignal, this, &EvaluationData::allTargetsChangedSlot);
}

void EvaluationData::setBuffers(std::map<std::string, std::shared_ptr<Buffer>> buffers)
{
    loginf << "EvaluationData: setBuffers";

    accessor_->clear();
    accessor_->add(buffers);
}

void EvaluationData::addReferenceData (const std::string& dbcontent_name, unsigned int line_id)
{
    loginf << "EvaluationData: addReferenceData: dbcontent " << dbcontent_name;

    if (!dbcont_man_.hasAssociations())
    {
        logwrn << "EvaluationData: addReferenceData: dbcontent has no associations";

        return;
    }

    ref_line_id_ = line_id;
    assert (ref_line_id_ <= 3);

    set<unsigned int> active_srcs = eval_man_.activeDataSourcesRef();
    bool use_active_srcs = (eval_man_.dbContentNameRef() == eval_man_.dbContentNameTst());
    unsigned int num_skipped {0};

    assert (accessor_->hasMetaVar<ptime>(dbcontent_name, DBContent::meta_var_timestamp_));
    NullableVector<ptime>& ts_vec = accessor_->getMetaVar<ptime>(
                dbcontent_name, DBContent::meta_var_timestamp_);

    unsigned int buffer_size = ts_vec.size();

    assert (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_ds_id_));
    NullableVector<unsigned int>& ds_ids = accessor_->getMetaVar<unsigned int>(
                dbcontent_name, DBContent::meta_var_ds_id_);

    assert (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_line_id_));
    NullableVector<unsigned int>& line_ids = accessor_->getMetaVar<unsigned int>(
                dbcontent_name, DBContent::meta_var_line_id_);

    assert (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_utn_));
    NullableVector<unsigned int>& utn_vec = accessor_->getMetaVar<unsigned int>(
                dbcontent_name, DBContent::meta_var_utn_);

    ptime timestamp;
    //vector<unsigned int> utn_vec;

    unsigned int utn;

    loginf << "EvaluationData: addReferenceData: adding target data";

    loginf << "EvaluationData: addReferenceData: use_active_srcs " << use_active_srcs;

    for (auto ds_id : active_srcs)
        loginf << "EvaluationData: addReferenceData: " << ds_id;

    for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
    {
        assert (!ds_ids.isNull(cnt));

        if (use_active_srcs && !active_srcs.count(ds_ids.get(cnt))) // skip those entries not for tst src
        {
            ++num_skipped;
            continue;
        }

        assert (!line_ids.isNull(cnt));

        if (line_ids.get(cnt) != ref_line_id_)
        {
            ++num_skipped;
            continue;
        }

        if (ts_vec.isNull(cnt))
        {
            ++num_skipped;
            continue;
        }

        timestamp = ts_vec.get(cnt);

        if (utn_vec.isNull(cnt))
        {
            ++unassociated_ref_cnt_;
            continue;
        }

        utn = utn_vec.get(cnt);

        if (!hasTargetData(utn))
            target_data_.emplace_back(utn, *this, accessor_, eval_man_, dbcont_man_);

        assert (hasTargetData(utn));

        auto tr_tag_it = target_data_.get<target_tag>().find(utn);
        auto index_it = target_data_.project<0>(tr_tag_it); // get iterator for random access

        target_data_.modify(index_it, [timestamp, cnt](EvaluationTargetData& t) { t.addRefIndex(timestamp, cnt); });

        ++associated_ref_cnt_;
    }

    loginf << "EvaluationData: addReferenceData: num targets " << target_data_.size()
           << " ref associated cnt " << associated_ref_cnt_ << " unassoc " << unassociated_ref_cnt_
           << " num_skipped " << num_skipped;
}

void EvaluationData::addTestData (const std::string& dbcontent_name, unsigned int line_id)
{
    loginf << "EvaluationData: addTestData: dbcontent " << dbcontent_name;

    if (!dbcont_man_.hasAssociations())
    {
        logwrn << "EvaluationData: addTestData: dbcontent has no associations";
        return;
    }

    tst_line_id_ = line_id;
    assert (tst_line_id_ <= 3);

    set<unsigned int> active_srcs = eval_man_.activeDataSourcesTst();
    bool use_active_srcs = (eval_man_.dbContentNameRef() == eval_man_.dbContentNameTst());
    unsigned int num_skipped {0};

    assert (accessor_->hasMetaVar<ptime>(dbcontent_name, DBContent::meta_var_timestamp_));
    NullableVector<ptime>& ts_vec = accessor_->getMetaVar<ptime>(
                dbcontent_name, DBContent::meta_var_timestamp_);

    unsigned int buffer_size = ts_vec.size();

    assert (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_ds_id_));
    NullableVector<unsigned int>& ds_ids = accessor_->getMetaVar<unsigned int>(
                dbcontent_name, DBContent::meta_var_ds_id_);

    assert (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_line_id_));
    NullableVector<unsigned int>& line_ids = accessor_->getMetaVar<unsigned int>(
                dbcontent_name, DBContent::meta_var_line_id_);

    assert (accessor_->hasMetaVar<unsigned int>(dbcontent_name, DBContent::meta_var_utn_));
    NullableVector<unsigned int>& utn_vec = accessor_->getMetaVar<unsigned int>(
                dbcontent_name, DBContent::meta_var_utn_);

    boost::posix_time::ptime timestamp;
    //vector<unsigned int> utn_vec;
    unsigned int utn;

    loginf << "EvaluationData: addTestData: adding target data";

    loginf << "EvaluationData: addReferenceData: use_active_srcs " << use_active_srcs;

    for (auto ds_id : active_srcs)
        loginf << "EvaluationData: addReferenceData: " << ds_id;

    for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
    {
        assert (!ds_ids.isNull(cnt));

        if (use_active_srcs && !active_srcs.count(ds_ids.get(cnt))) // skip those entries not for tst src
        {
            ++num_skipped;
            continue;
        }

        assert (!line_ids.isNull(cnt));

        if (line_ids.get(cnt) != tst_line_id_)
        {
            ++num_skipped;
            continue;
        }

        if (ts_vec.isNull(cnt))
        {
            ++num_skipped;
            continue;
        }

        timestamp = ts_vec.get(cnt);

        if (utn_vec.isNull(cnt))
        {
            ++unassociated_tst_cnt_;
            continue;
        }

        utn = utn_vec.get(cnt);

        if (!hasTargetData(utn))
            target_data_.emplace_back(utn, *this, accessor_, eval_man_, dbcont_man_);

        assert (hasTargetData(utn));

        auto tr_tag_it = target_data_.get<target_tag>().find(utn);
        auto index_it = target_data_.project<0>(tr_tag_it); // get iterator for random access

        target_data_.modify(index_it, [timestamp, cnt](EvaluationTargetData& t) { t.addTstIndex(timestamp, cnt); });

        ++associated_tst_cnt_;
    }

    loginf << "EvaluationData: addTestData: num targets " << target_data_.size()
           << " tst associated cnt " << associated_tst_cnt_ << " unassoc " << unassociated_tst_cnt_
           << " num_skipped " << num_skipped;
}

void EvaluationData::finalize ()
{
    loginf << "EvaluationData: finalize";

    assert (!finalized_);

    unsigned int num_targets = target_data_.size();

    QApplication::setOverrideCursor(Qt::WaitCursor);
    {
        eval_man_.updateSectorLayers();
        beginResetModel();
    }
    QApplication::restoreOverrideCursor();

    if (!num_targets)
    {
        logerr << "EvaluationData: finalize: no targets loaded";
    }
    else
    {
        auto task = [&] (int cnt) { target_data_[cnt].finalize(); return true; };

        Utils::Async::waitDialogAsyncArray(task, (int)num_targets, "Finalizing data");
    }

    finalized_ = true;

    QApplication::setOverrideCursor(Qt::WaitCursor);
    {
        endResetModel();

        if (widget_)
            widget_->resizeColumnsToContents();
    }
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

    accessor_->clear();

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

            if (dbcont_man_.utnUseEval(target.utn_))
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

        if (!dbcont_man_.utnUseEval(target.utn_))
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
            return dbcont_man_.utnComment(target.utn_).c_str();
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
            return target.acidsStr().c_str();
        }
        else if (col_name == "TA")
        {
            return target.acadsStr().c_str();
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

        dbcont_man_.utnUseEval(it->utn_, checked);

        emit dataChanged(index, EvaluationData::index(index.row(), columnCount()-1));
        return true;
    }
    else if (role == Qt::EditRole && index.column() == 2) // comment
    {
        assert (index.row() >= 0);
        assert (index.row() < target_data_.size());

        auto it = target_data_.begin()+index.row();

        dbcont_man_.utnComment(it->utn_, value.toString().toStdString());
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

    if (index.column() == 0) // Use
    {
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
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

EvaluationDataWidget* EvaluationData::widget()
{
    if (!widget_)
        widget_.reset(new EvaluationDataWidget(*this, eval_man_));

    return widget_.get();
}

void EvaluationData::targetChangedSlot(unsigned int utn) // for one utn
{
    loginf << "EvaluationData: targetChangedSlot: utn " << utn;

    // check if checkbox utn thingi is found
    QModelIndexList items = match(
                index(0, 0),
                Qt::UserRole,
                QVariant(utn),
                1, // look *
                Qt::MatchExactly); // look *

    loginf << "EvaluationData: targetChangedSlot: utn " << utn << " matches " << items.size();

    if (items.size() == 1)
    {
        emit dataChanged(index(items.at(0).row(), 0), index(items.at(0).row(), columnCount()-1));

        eval_man_.updateResultsToChanges();
    }
}

void EvaluationData::allTargetsChangedSlot() // for more than 1 utn
{
    loginf << "EvaluationData: allTargetsChangedSlot";

    beginResetModel();
    endResetModel();

    eval_man_.updateResultsToChanges();
}
