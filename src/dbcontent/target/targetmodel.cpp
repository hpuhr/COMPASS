#include "targetmodel.h"
#include "compass.h"
#include "mainwindow.h"
#include "dbinterface.h"
#include "logger.h"

#include <QApplication>
#include <QThread>
#include <QProgressDialog>
#include <QLabel>

using namespace std;

namespace dbContent {


TargetModel::TargetModel()
{

}

TargetModel::~TargetModel()
{

}

void TargetModel::clear()
{
    beginResetModel();

    target_data_.clear();

    endResetModel();
}

QVariant TargetModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch (role)
    {
        case Qt::CheckStateRole:
            {
                if (index.column() == 0)  // selected special case
                {
                    assert (index.row() >= 0);
                    assert (index.row() < target_data_.size());

                    const Target& target = target_data_.at(index.row());

                    if (target.use())
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

                const Target& target = target_data_.at(index.row());

                if (!target.use())
                    return QBrush(Qt::lightGray);
                else
                    return QVariant();

            }
        case Qt::DisplayRole:
        case Qt::EditRole:
            {
                logdbg << "TargetModel: data: display role: row " << index.row() << " col " << index.column();

                assert (index.row() >= 0);
                assert (index.row() < target_data_.size());

                const Target& target = target_data_.at(index.row());

                logdbg << "TargetModel: data: got utn " << target.utn_;

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
                    return target.comment().c_str();
                }
                else if (col_name == "#Updates")
                {
                    return target.numUpdates();
                }
                else if (col_name == "Begin")
                {
                    return target.timeBeginStr().c_str();
                }
                else if (col_name == "End")
                {
                    return target.timeEndStr().c_str();
                }
                else if (col_name == "ACIDs")
                {
                    return target.aircraftIdentificationsStr().c_str();
                }
                else if (col_name == "ACADs")
                {
                    return target.aircraftAddressesStr().c_str();
                }
                else if (col_name == "M3/A")
                {
                    return target.modeACodesStr().c_str();
                }
                else if (col_name == "MC Min")
                {
                    if (target.hasModeC())
                        return target.modeCMin();
                    else
                        return "";
                }
                else if (col_name == "MC Max")
                {
                    if (target.hasModeC())
                        return target.modeCMax();
                    else
                        return "";
                }
                else if (col_name == "MOPS")
                {
                    if (target.hasAdsbMOPSVersions())
                        return target.adsbMOPSVersionsStr().c_str();
                    else
                        return "";
                }

            }
        case Qt::DecorationRole:
            {
                assert (index.column() < table_columns_.size());

                return QVariant();
            }
        case Qt::UserRole: // to find the checkboxes
            {
                if (index.column() == 0)
                {
                    assert (index.row() >= 0);
                    assert (index.row() < target_data_.size());

                    const Target& target = target_data_.at(index.row());
                    return target.utn_;
                }
                else if (index.column() == 2) // comment
                {
                    assert (index.row() >= 0);
                    assert (index.row() < target_data_.size());

                    const Target& target = target_data_.at(index.row());
                    return ("comment_"+to_string(target.utn_)).c_str();
                }
            }
        default:
            {
                return QVariant();
            }
    }
}

bool TargetModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
    if (!index.isValid() /*|| role != Qt::EditRole*/)
        return false;

    if (role == Qt::CheckStateRole && index.column() == 0)
    {
        assert (index.row() >= 0);
        assert (index.row() < target_data_.size());

        auto it = target_data_.begin()+index.row();

        bool checked = (Qt::CheckState)value.toInt() == Qt::Checked;
        loginf << "TargetModel: setData: utn " << it->utn_ <<" check state " << checked;

        //eval_man_.useUTN(it->utn_, checked, false);
        target_data_.modify(it, [value,checked](Target& p) { p.use(checked); });

        saveToDB(it->utn_);

        emit dataChanged(index, TargetModel::index(index.row(), columnCount()-1));
        return true;
    }
    else if (role == Qt::EditRole && index.column() == 2) // comment
    {
        assert (index.row() >= 0);
        assert (index.row() < target_data_.size());

        auto it = target_data_.begin()+index.row();

        target_data_.modify(it, [value](Target& p) { p.comment(value.toString().toStdString()); });

        saveToDB(it->utn_);

        return true;
    }

    return false;
}


QVariant TargetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        assert (section < table_columns_.size());
        return table_columns_.at(section);
    }

    return QVariant();
}

QModelIndex TargetModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

int TargetModel::rowCount(const QModelIndex& parent) const
{
    return target_data_.size();
}

int TargetModel::columnCount(const QModelIndex& parent) const
{
    return table_columns_.size();
}

QModelIndex TargetModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

Qt::ItemFlags TargetModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    assert (index.column() < table_columns_.size());

    if (index.column() == 0) // Use
    {
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }
    else if (index.column() == 2) // comment
    {
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    else
        return QAbstractItemModel::flags(index);
}

const dbContent::Target& TargetModel::getTargetOf (const QModelIndex& index)
{
    assert (index.isValid());

    assert (index.row() >= 0);
    assert (index.row() < target_data_.size());

    const dbContent::Target& target = target_data_.at(index.row());

    return target;
}

void TargetModel::setUseTargetData (unsigned int utn, bool value)
{
    loginf << "TargetModel: setUseTargetData: utn " << utn << " value " << value;

    assert (existsTarget(utn));

    QModelIndexList items = match(
                index(0, 0),
                Qt::UserRole,
                QVariant(utn),
                1, // look *
                Qt::MatchExactly); // look *

    assert (items.size() == 1);

    setData(items.at(0), {value ? Qt::Checked: Qt::Unchecked}, Qt::CheckStateRole);
}
void TargetModel::setUseAllTargetData (bool value)
{
    loginf << "TargetModel: setUseAllTargetData: value " << value;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    beginResetModel();

    for (auto target_it = target_data_.begin(); target_it != target_data_.end(); ++target_it)
        target_data_.modify(target_it, [value](Target& p) { p.use(value); });

    saveToDB();

    endResetModel();

    QApplication::restoreOverrideCursor();
}

void TargetModel::clearComments ()
{
    loginf << "TargetModel: clearComments";

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    beginResetModel();

    for (auto target_it = target_data_.begin(); target_it != target_data_.end(); ++target_it)
        target_data_.modify(target_it, [](Target& p) { p.comment(""); });

    saveToDB();

    endResetModel();

    QApplication::restoreOverrideCursor();
}

void TargetModel::setUseByFilter ()
{
    loginf << "TargetModel: setUseByFilter";
}

void TargetModel::setTargetDataComment (unsigned int utn, std::string comment)
{
    loginf << "TargetModel: setTargetDataComment: utn " << utn << " comment '" << comment << "'";

    assert (existsTarget(utn));

    QModelIndexList items = match(
                index(0, 0),
                Qt::UserRole,
                QVariant(("comment_"+to_string(utn)).c_str()),
                1, // look *
                Qt::MatchExactly); // look *

    if (items.size() == 1)
        setData(items.at(0), comment.c_str(), Qt::EditRole);
}

bool TargetModel::hasTargetsInfo()
{
    return target_data_.size();
}

void TargetModel::clearTargetsInfo()
{
    clear();

    COMPASS::instance().interface().clearTargetsTable();
}

bool TargetModel::existsTarget(unsigned int utn)
{
    auto tr_tag_it = target_data_.get<target_tag>().find(utn);

    return tr_tag_it != target_data_.get<target_tag>().end();
}

void TargetModel::createNewTarget(unsigned int utn)
{
    beginResetModel();

    assert (!existsTarget(utn));

    target_data_.push_back({utn, nlohmann::json::object()});

    assert (existsTarget(utn));

    endResetModel();
}

dbContent::Target& TargetModel::target(unsigned int utn)
{
    auto tr_tag_it = target_data_.get<target_tag>().find(utn);

    assert (tr_tag_it != target_data_.get<target_tag>().end());

    return const_cast<dbContent::Target&> (*tr_tag_it); // ok since key utn_ can not be modified, still const
}

void TargetModel::loadFromDB()
{
    loginf << "TargetModel: loadFromDB";

    beginResetModel();

    for (auto& target : COMPASS::instance().interface().loadTargets())
    {
        target_data_.push_back({target->utn_, target->info()});
    }

    endResetModel();

    loginf << "TargetModel: loadFromDB: loaded " << target_data_.size() << " targets";
}

void TargetModel::saveToDB()
{
    loginf << "TargetModel: saveToDB: saving " << target_data_.size() << " targets";

    std::vector<std::unique_ptr<dbContent::Target>> targets;

    for (auto& target : target_data_)
        targets.emplace_back(new dbContent::Target(target.utn_, target.info()));

    COMPASS::instance().interface().saveTargets(targets);
}

void TargetModel::saveToDB(unsigned int utn)
{
    loginf << "TargetModel: saveToDB: saving utn " << utn;

    auto tr_tag_it = target_data_.get<target_tag>().find(utn);

    assert (tr_tag_it != target_data_.get<target_tag>().end());

    std::unique_ptr<dbContent::Target> tgt_copy {new dbContent::Target(tr_tag_it->utn_, tr_tag_it->info())};

    COMPASS::instance().interface().saveTarget(tgt_copy);
}


}
