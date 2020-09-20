#include "evaluationdata.h"
#include "evaluationdatawidget.h"
#include "dbobject.h"
#include "buffer.h"
#include "stringconv.h"

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

    const DBOAssociationCollection& associations = object.associations();

    unsigned int buffer_size = buffer->size();
    NullableVector<int>& rec_nums = buffer->get<int>("rec_num");
    NullableVector<float>& tods = buffer->get<float>("tod");

    unsigned int rec_num;
    float tod;

    loginf << "EvaluationData: addReferenceData: adding target data";

    for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
    {
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
           << " ref associated cnt " << associated_ref_cnt_ << " unassoc " << unassociated_ref_cnt_;
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

    const DBOAssociationCollection& associations = object.associations();

    unsigned int buffer_size = buffer->size();
    NullableVector<int>& rec_nums = buffer->get<int>("rec_num");
    NullableVector<float>& tods = buffer->get<float>("tod");

    unsigned int rec_num;
    float tod;

    loginf << "EvaluationData: addTestData: adding target data";

    for (unsigned int cnt=0; cnt < buffer_size; ++cnt)
    {
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
           << " tst associated cnt " << associated_tst_cnt_ << " unassoc " << unassociated_tst_cnt_;
}

void EvaluationData::finalize ()
{
    loginf << "EvaluationData: finalize";

    beginResetModel();

    for (auto target_it = target_data_.begin(); target_it != target_data_.end(); ++target_it)
        target_data_.modify(target_it, [&](EvaluationTargetData& t) { t.finalize(); });

    finalized_ = true;

    endResetModel();

    if (widget_)
        widget_->resizeColumnsToContents();
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

                if (col_name == "UTN")
                {
                    return target.utn_;
                }
                else if (col_name == "Begin")
                {
                    if (target.hasData())
                        return String::timeStringFromDouble(target.timeBegin()).c_str();
                    else
                        return QVariant();
                }
                else if (col_name == "End")
                {
                    if (target.hasData())
                        return String::timeStringFromDouble(target.timeEnd()).c_str();
                    else
                        return QVariant();
                }
                else if (col_name == "# All")
                {
                    return target.numUpdates();
                }
                else if (col_name == "# Ref")
                {
                    return target.numRefUpdates();
                }
                else if (col_name == "# Tst")
                {
                    return target.numTstUpdates();
                }
                else if (col_name == "Mode 3/A")
                {
                    std::vector<unsigned int> codes = target.modeACodes();

                    std::ostringstream out;

                    for (unsigned int cnt=0; cnt < codes.size(); ++cnt)
                    {
                        if (cnt != 0)
                            out << ",";
                        out << String::octStringFromInt(codes.at(cnt), 4, '0');
                    }

                    return out.str().c_str();
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
        default:
            {
                return QVariant();
            }
    }
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
    return QAbstractItemModel::flags(index);
}

EvaluationDataWidget* EvaluationData::widget()
{
    if (!widget_)
        widget_.reset(new EvaluationDataWidget(*this));

    return widget_.get();
}
