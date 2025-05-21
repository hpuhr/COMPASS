#include "targetmodel.h"
#include "compass.h"
#include "dbinterface.h"
#include "dbcontentmanager.h"
#include "evaluationmanager.h"
#include "evaluationtargetfilter.h"
#include "logger.h"
#include "reconstructortarget.h"
#include "task/result/report/reportdefs.h"
#include "util/files.h"

#include <QApplication>
#include <QThread>
#include <QProgressDialog>
#include <QLabel>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace dbContent {

const QColor very_light_gray (230, 230, 230);

/**
 */
TargetModel::TargetModel(DBContentManager& dbcont_manager)
    :dbcont_manager_(dbcont_manager)
{
}

/**
 */
TargetModel::~TargetModel() = default;

/**
 */
void TargetModel::clear()
{
    beginResetModel();

    target_data_.clear();

    endResetModel();
}

/**
 */
QVariant TargetModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    assert (index.row() >= 0);
    assert (index.row() < target_data_.size());

    const Target& target = target_data_.at(index.row());

    switch (role)
    {
        case Qt::BackgroundRole:
        {
            if (!target.useInEval())
                return QBrush(Qt::lightGray);
            if (COMPASS::instance().evaluationManager().useTimestampFilter()
                    || target.evalExcludedTimeWindows().size()
                    || target.evalExcludedRequirements().size())
                return QBrush(very_light_gray);
            else
                return QVariant();

        }
        case Qt::DisplayRole:
        {
            logdbg << "TargetModel: data: display role: row " << index.row() << " col " << index.column();

            assert (index.column() < table_columns_.size());
            int col = index.column();
            assert (col >= 0);

            return getCellContent(target, (Columns) col);
        }
        case Qt::UserRole: // to find the checkboxes
        {
            if (index.column() == ColUseEval)
            {
                return target.utn_;
            }
            else if (index.column() == ColComment) // comment
            {
                return ("comment_"+to_string(target.utn_)).c_str();
            }
        }
        case Qt::DecorationRole:
        {
            if (index.column() == ColUseEval)  // selected special case
            {
                if (!target.useInEval())
                    return Utils::Files::IconProvider::getIcon("delete.png");

                // could be used

                if (COMPASS::instance().evaluationManager().useTimestampFilter()
                    || target.evalExcludedTimeWindows().size()
                    || target.evalExcludedRequirements().size())
                        return Utils::Files::IconProvider::getIcon("partial_done.png");

                return Utils::Files::IconProvider::getIcon("done.png");
            }
            else
                return QVariant();
        }
        case Qt::ToolTipRole:
        {
            QString tooltip_html;
            tooltip_html += "<html><body><table>";

            // Helper lambda to append any (label,value) pair as a right-aligned row
            auto append_row = [&](const QString& label, const QString& value, bool bold_label = true)
            {
                QString tmp_value = value;
                tmp_value.replace("\n", "<br/>");

                tooltip_html += "<tr>"
                                "<td>" + (bold_label ? "<b>" + label + "</b>" : label) + ":</td>"
                                + "<td align=\"left\">" + tmp_value + "</td>"
                                + "</tr>";
            };

            if (!show_eval_columns_)
            {
                QString eval_value;
                if (!target.useInEval())
                    eval_value = "No";
                else if (COMPASS::instance().evaluationManager().useTimestampFilter()
                           || target.evalExcludedTimeWindows().size()
                           || target.evalExcludedRequirements().size())
                    eval_value = "Partial";
                else
                    eval_value = "Yes";

                // first row, label in bold
                append_row("Eval", eval_value);

                append_row(table_columns_.at(ColUseEvalDetails),
                           getCellContent(target, ColUseEvalDetails).toString());
            }

            if (!show_duration_columns_)
            {
                for (auto column : duration_columns_)
                    append_row(table_columns_.at(column), getCellContent(target, (Columns)column).toString());
            }

            if (!show_mode_s_columns_)
            {
                for (auto column : mode_s_columns_)
                    append_row(table_columns_.at(column), getCellContent(target, (Columns)column).toString());
            }

            if (!show_mode_ac_columns_)
            {
                for (auto column : mode_ac_columns_)
                    append_row(table_columns_.at(column), getCellContent(target, (Columns)column).toString());
            }

            tooltip_html += "</table></body></html>";
            return tooltip_html;
        }
        default:
        {
            return QVariant();
        }
    }
}

/**
 */
bool TargetModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
    if (!index.isValid() /*|| role != Qt::EditRole*/)
        return false;

    if (role == Qt::EditRole && index.column() == ColComment) // comment
    {
        assert (index.row() >= 0);
        assert (index.row() < target_data_.size());

        auto it = target_data_.begin() + index.row();

        loginf << "TargetModel: setData: utn " << it->utn_ <<" comment '" << value.toString().toStdString() << "'";

        target_data_.modify(it, [value](Target& p) { p.comment(value.toString().toStdString()); });

        saveToDB(it->utn_);

        emit dbcont_manager_.targetChangedSignal(it->utn_);

        return true;
    }

    return false;
}

/**
 */
QVariant TargetModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        assert (section < table_columns_.size());
        return table_columns_.at(section);
    }

    return QVariant();
}

/**
 */
QModelIndex TargetModel::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

/**
 */
int TargetModel::rowCount(const QModelIndex& parent) const
{
    return target_data_.size();
}

/**
 */
int TargetModel::columnCount(const QModelIndex& parent) const
{
    return table_columns_.size();
}

/**
 */
QModelIndex TargetModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

/**
 */
Qt::ItemFlags TargetModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    assert (index.column() < table_columns_.size());

    // if (index.column() == ColUseEval) // Use
    // {
    //     return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    // }
    // else
    if (index.column() == ColComment) // comment
    {
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    else
        return QAbstractItemModel::flags(index);
}

QVariant TargetModel::getCellContent(const Target& target, Columns col) const
{
    switch(col)
    {
    case ColUTN:
        return target.utn_;
    case ColComment:
        return target.comment().c_str();
    case ColCategory:
        return target.emitterCategoryStr().c_str();
    case ColUseEval:
        return QVariant();
    case ColUseEvalDetails:
    {
        QString tmp;

        if (target.evalExcludedTimeWindows().size())
            tmp = target.evalExcludedTimeWindows().asString().c_str();

        if (target.evalExcludedRequirements().size())
        {
            if (tmp.size())
                tmp += "\n";

            tmp += String::compress(target.evalExcludedRequirements(),',').c_str();
        }

        return tmp;
    }
    case ColNumUpdates:
        return target.numUpdates();
    case ColBegin:
        return target.timeBeginStr().c_str();
    case ColEnd:
        return target.timeEndStr().c_str();
    case ColDuration:
        return target.timeDurationStr().c_str();
    case ColACIDs:
        return target.aircraftIdentificationsStr().c_str();
    case ColACADs:
        return target.aircraftAddressesStr().c_str();
    case ColMode3A:
        return target.modeACodesStr().c_str();
    case ColModeCMin:
        if (target.hasModeC())
            return target.modeCMin();
        else
            return "";
    case ColModeCMax:
        if (target.hasModeC())
            return target.modeCMax();
        else
            return "";
    }

    return QVariant();
}

/**
 */
const dbContent::Target& TargetModel::getTargetOf (const QModelIndex& index)
{
    assert (index.isValid());

    assert (index.row() >= 0);
    assert (index.row() < target_data_.size());

    const dbContent::Target& target = target_data_.at(index.row());

    return target;
}

/**
 */
void TargetModel::setUseTargetData (unsigned int utn, bool value)
{
    loginf << "TargetModel: setUseTargetData: utn " << utn << " value " << value;

    assert (existsTarget(utn));

    // search if checkbox can be found
    QModelIndexList items = match(
                index(0, ColUseEval),
                Qt::UserRole,
                QVariant(utn),
                1, // look *
                Qt::MatchExactly); // look *

    assert (items.size() == 1);

    setData(items.at(0), {value ? Qt::Checked: Qt::Unchecked}, Qt::CheckStateRole);

    // already emitted in setData
    //emit dbcont_manager_.targetChangedSignal(utn);
}

/**
 */
void TargetModel::setTargetDataComment (unsigned int utn, std::string comment)
{
    loginf << "TargetModel: setTargetDataComment: utn " << utn << " comment '" << comment << "'";

    assert (existsTarget(utn));

    // search if comment can be found can be found, check in COLUMN 2!
    QModelIndexList items = match(
                index(0, ColComment),
                Qt::UserRole,
                QVariant(("comment_"+to_string(utn)).c_str()),
                1, // look *
                Qt::MatchExactly); // look *

     loginf << "TargetModel: setTargetDataComment: size " << items.size();

    assert (items.size() == 1);
    setData(items.at(0), comment.c_str(), Qt::EditRole);

    // already emitted in setData
    //emit dbcont_manager_.targetChangedSignal(utn);
}

/**
 */
void TargetModel::setUseAllTargetData (bool value)
{
    loginf << "TargetModel: setUseAllTargetData: value " << value;

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    beginResetModel();

    for (auto target_it = target_data_.begin(); target_it != target_data_.end(); ++target_it)
        target_data_.modify(target_it, [value](Target& p) { p.useInEval(value); });

    saveToDB();

    endResetModel();

    dbcont_manager_.storeTargetsEvalInfo();
    emit dbcont_manager_.allTargetsChangedSignal();

    QApplication::restoreOverrideCursor();
}

/**
 */
void TargetModel::clearComments ()
{
    loginf << "TargetModel: clearComments";

    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    beginResetModel();

    for (auto target_it = target_data_.begin(); target_it != target_data_.end(); ++target_it)
        target_data_.modify(target_it, [](Target& p) { p.comment(""); });

    saveToDB();

    endResetModel();

    emit dbcont_manager_.allTargetsChangedSignal();

    QApplication::restoreOverrideCursor();
}

/**
 */
void TargetModel::setUseByFilter ()
{
    loginf << "TargetModel: setUseByFilter";

    beginResetModel();

    COMPASS::instance().evaluationManager().targetFilter().setUse(target_data_);

    saveToDB();

    endResetModel();

    emit dbcont_manager_.allTargetsChangedSignal();
}

/**
 */
bool TargetModel::hasTargetsInfo()
{
    return target_data_.size();
}

/**
 */
void TargetModel::clearTargetsInfo()
{
    clear();

    COMPASS::instance().dbInterface().clearTargetsTable();
}

/**
 */
bool TargetModel::existsTarget(unsigned int utn) const
{
    auto tr_tag_it = target_data_.get<target_tag>().find(utn);

    return tr_tag_it != target_data_.get<target_tag>().end();
}

/**
 */
void TargetModel::createNewTargets(const std::map<unsigned int, dbContent::ReconstructorTarget>& targets)
{
    beginResetModel();

    assert (!target_data_.size());

    unsigned int utn = 0;

    for (auto& tgt_it : targets)
    {
        //cont_man.createNewTarget(tgt_it.first);
        utn = tgt_it.first;

        target_data_.push_back({utn, nlohmann::json::object()});

        assert (existsTarget(utn));

        dbContent::Target& tgt = target(utn);

                //target.useInEval(tgt_it.second.use_in_eval_);

                //if (tgt_it.second.comment_.size())
                //    target.comment(tgt_it.second.comment_);

        tgt.aircraftAddresses(tgt_it.second.acads_);
        tgt.aircraftIdentifications(tgt_it.second.acids_);
        tgt.modeACodes(tgt_it.second.mode_as_);

        if (tgt_it.second.ecat_ && *tgt_it.second.ecat_ != (unsigned int) dbContent::Target::Category::Unknown)
            tgt.targetCategory(dbContent::Target::fromECAT(tgt_it.second.ecat_));

        if (tgt_it.second.hasTimestamps())
        {
            tgt.timeBegin(tgt_it.second.total_timestamp_min_);
            tgt.timeEnd(tgt_it.second.total_timestamp_max_);
        }

        if (tgt_it.second.hasModeC())
            tgt.modeCMinMax(*tgt_it.second.mode_c_min_, *tgt_it.second.mode_c_max_);

        if (tgt_it.second.latitude_min_ && tgt_it.second.latitude_max_
            && tgt_it.second.longitude_min_ && tgt_it.second.longitude_max_)
            tgt.setPositionBounds(*tgt_it.second.latitude_min_, *tgt_it.second.latitude_max_,
                                  *tgt_it.second.longitude_min_, *tgt_it.second.longitude_max_);

                // set counts
        for (auto& count_it : tgt_it.second.getDBContentCounts())
            tgt.dbContentCount(count_it.first, count_it.second);

                // set adsb stuff
        //        if (tgt_it.second.hasADSBMOPSVersion() && tgt_it.second.getADSBMOPSVersions().size())
        //            target.adsbMOPSVersions(tgt_it.second.getADSBMOPSVersions());

        //++utn;
    }

    endResetModel();
}

/**
 */
dbContent::Target& TargetModel::target(unsigned int utn)
{
    auto tr_tag_it = target_data_.get<target_tag>().find(utn);

    assert (tr_tag_it != target_data_.get<target_tag>().end());

    return const_cast<dbContent::Target&> (*tr_tag_it); // ok since key utn_ can not be modified, still const
}

/**
 */
const dbContent::Target& TargetModel::target(unsigned int utn) const
{
    auto tr_tag_it = target_data_.get<target_tag>().find(utn);

    assert (tr_tag_it != target_data_.get<target_tag>().end());

    return const_cast<const dbContent::Target&> (*tr_tag_it);
}

/**
 */
unsigned int TargetModel::size() const
{
    return target_data_.size();
}

/**
 */
void TargetModel::removeDBContentFromTargets(const std::string& dbcont_name)
{
    for (auto target_it = target_data_.begin(); target_it != target_data_.end(); ++target_it)
        target_data_.modify(target_it, [dbcont_name](Target& p) { p.clearDBContentCount(dbcont_name); });
}

void TargetModel::storeTargetsEvalInfo()
{
    for (auto target_it = target_data_.begin(); target_it != target_data_.end(); ++target_it)
        target_data_.modify(target_it, [](Target& p) { p.storeEvalutionInfo(); });
}

/**
 */
nlohmann::json TargetModel::asJSON() const
{
    nlohmann::json data;

    for (auto& target : target_data_)
    {
        data[to_string(target.utn_)] = target.info();
        data[to_string(target.utn_)]["utn"] = target.utn_;
    }

    return data;
}

/**
 */
nlohmann::json TargetModel::targetAsJSON(unsigned int utn) const
{
    if (!existsTarget(utn))
        return {};

    const auto& t = target(utn);

    nlohmann::json data;
    data["utn" ] = utn;
    data["info"] = t.info();

    return data;
}

/**
 */
nlohmann::json TargetModel::targetStatsAsJSON() const
{
    nlohmann::json data;

    size_t num_targets = target_data_.size();

    size_t associated = 0;
    size_t min_size   = std::numeric_limits<size_t>::max();
    size_t max_size   = 0;
    size_t mean_size  = 0;

    for (const auto& target : target_data_)
    {
        size_t nu = target.numUpdates();

        associated += nu;
        mean_size  += nu;

        if (nu > max_size) max_size = nu;
        if (nu < min_size) min_size = nu;
    }

    if (num_targets == 0)
        min_size = 0;
    else
        mean_size /= num_targets;

    data["num_targets"  ] = num_targets;
    data["min_updates"  ] = min_size;
    data["max_updates"  ] = max_size;
    data["avg_updates"  ] = mean_size;
    data["total_updates"] = associated;

    return data;
}

/**
 */
nlohmann::json TargetModel::utnsAsJSON() const
{
    nlohmann::json data;

    auto utn_arr = nlohmann::json::array();

    for (auto& target : target_data_)
    {
        utn_arr.push_back(target.utn_);
    }

    data["utns"] = utn_arr;

    return data;
}

/**
 */
void TargetModel::loadFromDB()
{
    loginf << "TargetModel: loadFromDB";

    beginResetModel();

    for (auto& target : COMPASS::instance().dbInterface().loadTargets())
    {
        target_data_.push_back({target->utn_, target->info()});
    }

    for (auto& target : target_data_)
        assert (existsTarget(target.utn_));

    endResetModel();

    loginf << "TargetModel: loadFromDB: loaded " << target_data_.size() << " targets";
}

/**
 */
void TargetModel::saveToDB()
{
    loginf << "TargetModel: saveToDB: saving " << target_data_.size() << " targets";

    std::vector<std::unique_ptr<dbContent::Target>> targets;

    for (auto& target : target_data_)
        targets.emplace_back(new dbContent::Target(target.utn_, target.info()));

    COMPASS::instance().dbInterface().saveTargets(targets);
}

/**
 */
void TargetModel::saveToDB(unsigned int utn)
{
    loginf << "TargetModel: saveToDB: saving utn " << utn;

    auto tr_tag_it = target_data_.get<target_tag>().find(utn);

    assert (tr_tag_it != target_data_.get<target_tag>().end());

    std::unique_ptr<dbContent::Target> tgt_copy {new dbContent::Target(tr_tag_it->utn_, tr_tag_it->info())};

    COMPASS::instance().dbInterface().saveTarget(tgt_copy);
}


/**
 */
void TargetModel::showMainColumns(bool show)
{
    show_main_columns_ = show;
}

/**
 */
void TargetModel::showEvalColumns(bool show)
{
    show_eval_columns_ = show;
}

/**
 */
void TargetModel::showDurationColumns(bool show)
{
    show_duration_columns_ = show;
}

/**
 */
void TargetModel::showModeSColumns(bool show)
{
    show_mode_s_columns_ = show;
}

/**
 */
void TargetModel::showModeACColumns(bool show)
{
    show_mode_ac_columns_ = show;
}

void TargetModel::updateEvalItems()
{
    if (show_eval_columns_)
    {
        int row_count = this->rowCount();

        emit dataChanged(index(0, ColUseEvalDetails), index(row_count, ColUseEvalDetails), {Qt::DisplayRole});
        emit dataChanged(index(0, ColUseEval), index(row_count, ColUseEval), {Qt::DecorationRole});
    }
}

}
