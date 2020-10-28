#include "evaluationdata.h"
#include "evaluationdatawidget.h"
#include "evaluationmanager.h"
#include "dbobject.h"
#include "buffer.h"
#include "stringconv.h"

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

void EvaluationData::addReferenceData (DBObject& object, std::shared_ptr<Buffer> buffer)
{
    loginf << "EvaluationData: addReferenceData: dbo " << object.name() << " size " << buffer->size();

    if (!object.hasAssociations())
    {
        logwrn << "EvaluationData: addReferenceData: object " << object.name() << " has no associations";
        unassociated_ref_cnt_ = buffer->size();

        return;
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
                target_data_.push_back({utn_it});

            assert (hasTargetData(utn_it));

            auto tr_tag_it = target_data_.get<target_tag>().find(utn_it);
            auto index_it = target_data_.project<0>(tr_tag_it); // get iterator for random access

            if (!targetData(utn_it).hasRefBuffer())
                target_data_.modify(index_it, [buffer](EvaluationTargetData& t) { t.setRefBuffer(buffer); });

            target_data_.modify(index_it, [tod, cnt](EvaluationTargetData& t) { t.addRefIndex(tod, cnt); });

            ++associated_ref_cnt_;
        }
    }

    loginf << "EvaluationData: addReferenceData: num targets " << target_data_.size()
           << " ref associated cnt " << associated_ref_cnt_ << " unassoc " << unassociated_ref_cnt_
              << " num_skipped " << num_skipped;
}

void EvaluationData::addTestData (DBObject& object, std::shared_ptr<Buffer> buffer)
{
    loginf << "EvaluationData: addTestData: dbo " << object.name() << " size " << buffer->size();

    if (!object.hasAssociations())
    {
        logwrn << "EvaluationData: addTestData: object " << object.name() << " has no associations";
        unassociated_tst_cnt_ = buffer->size();

        return;
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
                target_data_.push_back({utn_it});

            assert (hasTargetData(utn_it));

            auto tr_tag_it = target_data_.get<target_tag>().find(utn_it);
            auto index_it = target_data_.project<0>(tr_tag_it);  // get iterator for random access

            if (!targetData(utn_it).hasTstBuffer())
                target_data_.modify(index_it, [buffer](EvaluationTargetData& t) { t.setTstBuffer(buffer); });

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
                target_data_, done_flags);
    tbb::task::enqueue(*t);

    while (!done)
    {
        done = true;
        tmp_done_cnt = 0;

        for (auto done_it : done_flags)
        {
            if (!done_it)
                done = false;
            else
                tmp_done_cnt++;
        }

        assert (tmp_done_cnt <= num_targets);

        elapsed_time = boost::posix_time::microsec_clock::local_time();

        time_diff = elapsed_time - start_time;
        elapsed_time_s = time_diff.total_milliseconds() / 1000.0;

        time_per_eval = elapsed_time_s/(double)(tmp_done_cnt);
        remaining_time_s = (double)(num_targets-tmp_done_cnt)*time_per_eval;

//        loginf << " UGA num_targets " << num_targets << " tmp_done_cnt " << tmp_done_cnt
//               << " elapsed_time_s " << elapsed_time_s;

        postprocess_dialog_.setLabelText(
                    ("Elapsed: "+String::timeStringFromDouble(elapsed_time_s, false)
                     +"\nRemaining: "+String::timeStringFromDouble(remaining_time_s, false)
                     +" (estimated)").c_str());

        postprocess_dialog_.setValue(tmp_done_cnt);

        if (!done)
        {
            QCoreApplication::processEvents();
            QThread::msleep(100);
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

                    if (target.use())
                        return Qt::Checked;
                    else
                        return Qt::Unchecked;
                }
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

        if ((Qt::CheckState)value.toInt() == Qt::Checked)
        {
            loginf << "EvaluationData: setData: utn " << it->utn_ <<" check state " << true;

            target_data_.modify(it, [value](EvaluationTargetData& p) { p.use(true); });
            return true;
        }
        else
        {
            loginf << "EvaluationData: setData: utn " << it->utn_ <<" check state " << false;

            target_data_.modify(it, [value](EvaluationTargetData& p) { p.use(false); });
            return true;
        }
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

EvaluationDataWidget* EvaluationData::widget()
{
    if (!widget_)
        widget_.reset(new EvaluationDataWidget(*this, eval_man_));

    return widget_.get();
}
