#include "targetmodel.h"
#include "compass.h"
#include "dbinterface.h"
#include "dbcontentmanager.h"
#include "util/stringconv.h"
#include "util/timeconv.h"
#include "logger.h"
#include "reconstructortarget.h"
#include "task/result/report/reportdefs.h"

#include <QApplication>
#include <QThread>
#include <QProgressDialog>
#include <QLabel>

using namespace std;
using namespace Utils;
using namespace nlohmann;

namespace dbContent {

/**
 */
TargetModel::TargetModel(const std::string& class_id, const std::string& instance_id, DBContentManager& dbcont_manager)
    : Configurable(class_id, instance_id, &dbcont_manager), dbcont_manager_(dbcont_manager)
{
    // remove utn stuff
    // shorts
    registerParameter("remove_short_targets", &remove_short_targets_, true);
    registerParameter("remove_short_targets_min_updates", &remove_short_targets_min_updates_, 10u);
    registerParameter("remove_short_targets_min_duration", &remove_short_targets_min_duration_, 60.0);
    // psr
    registerParameter("remove_psr_only_targets", &remove_psr_only_targets_, true);
    // ma
    registerParameter("remove_modeac_onlys", &remove_modeac_onlys_, false);
    registerParameter("filter_mode_a_codes", &filter_mode_a_codes_, false);
    registerParameter("filter_mode_a_code_blacklist", &filter_mode_a_code_blacklist_, true);
    registerParameter("filter_mode_a_code_values", &filter_mode_a_code_values_, std::string("7000,7777"));
    // mc
    registerParameter("remove_mode_c_values", &remove_mode_c_values_, false);
    registerParameter("remove_mode_c_min_value", &remove_mode_c_min_value_, 11000.0f);
    // ta
    registerParameter("filter_target_addresses", &filter_target_addresses_, false);
    registerParameter("filter_target_addresses_blacklist", &filter_target_addresses_blacklist_, true);
    registerParameter("filter_target_address_values", &filter_target_address_values_, std::string());
    // dbo
    registerParameter("remove_not_detected_dbos", &remove_not_detected_dbos_, false);
    registerParameter("remove_not_detected_dbo_values", &remove_not_detected_dbo_values_, json::object());

    createSubConfigurables();
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

    switch (role)
    {
        case Qt::CheckStateRole:
        {
            if (index.column() == ColUse)  // selected special case
            {
                assert (index.row() >= 0);
                assert (index.row() < target_data_.size());

                const Target& target = target_data_.at(index.row());

                if (target.useInEval())
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

            if (!target.useInEval())
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
            int col = index.column();

            switch(col)
            {
                case ColUTN: 
                    return target.utn_;
                case ColComment: 
                    return target.comment().c_str();
                case ColCategory: 
                    return target.emitterCategoryStr().c_str();
                case ColNumUpdates: 
                    return target.numUpdates();
                case ColBegin:
                    return target.timeBeginStr().c_str();
                    //return QDateTime::fromString(target.timeBeginStr().c_str(), Time::QT_DATETIME_FORMAT.c_str());
                case ColEnd:
                    return target.timeEndStr().c_str();
                    //return QDateTime::fromString(target.timeEndStr().c_str(), Time::QT_DATETIME_FORMAT.c_str());
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
        case Qt::UserRole: // to find the checkboxes
        {
            if (index.column() == ColUse)
            {
                assert (index.row() >= 0);
                assert (index.row() < target_data_.size());

                const Target& target = target_data_.at(index.row());
                return target.utn_;
            }
            else if (index.column() == ColComment) // comment
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

/**
 */
nlohmann::json TargetModel::rawCellData(int row, int column) const
{
    const Target& target = target_data_.at(row);

    switch(column)
    {
        case ColUse:
            return target.useInEval();
        case ColUTN: 
            return target.utn_;
        case ColComment: 
            return target.comment().c_str();
        case ColCategory: 
            return target.emitterCategoryStr().c_str();
        case ColNumUpdates: 
            return target.numUpdates();
        case ColBegin:
            return target.timeBeginStr().c_str();
            //return QDateTime::fromString(target.timeBeginStr().c_str(), Time::QT_DATETIME_FORMAT.c_str());
        case ColEnd:
            return target.timeEndStr().c_str();
            //return QDateTime::fromString(target.timeEndStr().c_str(), Time::QT_DATETIME_FORMAT.c_str());
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
}

/**
 */
unsigned int TargetModel::rowStyle(int row) const
{
    if (!target_data_.at(row).useInEval())
        return ResultReport::CellStyleBGColorGray;

    return 0;
}

/**
 */
unsigned int TargetModel::columnStyle(int column) const
{
    if (column == ColUse)
        return ResultReport::CellStyleCheckable;

    return 0;
}

/**
 */
bool TargetModel::setData(const QModelIndex &index, const QVariant& value, int role)
{
    if (!index.isValid() /*|| role != Qt::EditRole*/)
        return false;

    if (role == Qt::CheckStateRole && index.column() == ColUse)
    {
        assert (index.row() >= 0);
        assert (index.row() < target_data_.size());

        auto it = target_data_.begin() + index.row();

        bool checked = (Qt::CheckState)value.toInt() == Qt::Checked;
        loginf << "TargetModel: setData: utn " << it->utn_ <<" check state " << checked;

        //eval_man_.useUTN(it->utn_, checked, false);
        target_data_.modify(it, [value,checked](Target& p) { p.useInEval(checked); });

        saveToDB(it->utn_);

        emit dataChanged(index, TargetModel::index(index.row(), columnCount()-1));
        emit dbcont_manager_.targetChangedSignal(it->utn_);

        return true;
    }
    else if (role == Qt::EditRole && index.column() == ColComment) // comment
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

    if (index.column() == ColUse) // Use
    {
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }
    else if (index.column() == ColComment) // comment
    {
        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }
    else
        return QAbstractItemModel::flags(index);
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
                index(0, ColUse),
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

    using namespace boost::posix_time;

    bool use;
    string comment;

    std::set<std::pair<int,int>> remove_mode_as = filterModeACodeData();
    std::set<unsigned int> remove_tas = filterTargetAddressData();

    bool tmp_match;

    time_duration short_duration = Time::partialSeconds(remove_short_targets_min_duration_);

    beginResetModel();

    for (auto target_it = target_data_.begin(); target_it != target_data_.end(); ++target_it)
    {
        if (!target_it->useInEval())
            continue;

        use = true; // must be true here
        comment = "";

        if (remove_short_targets_
                && (target_it->numUpdates() < remove_short_targets_min_updates_
                    || target_it->timeDuration() < short_duration))
        {
            use = false;
            comment = "Short track";
        }

        if (use && remove_psr_only_targets_ && target_it->isPrimaryOnly())
        {
            use = false;
            comment = "Primary only";
        }

        if (use && remove_modeac_onlys_ && target_it->isModeACOnly())
        {
            use = false;
            comment = "Mode A/C only";
        }

        if (use && filter_mode_a_codes_)
        {
            tmp_match = false;

            for (auto t_ma : target_it->modeACodes())
            {
                for (auto& r_ma_p : remove_mode_as)
                {
                    if (r_ma_p.second == -1) // single
                        tmp_match |= (t_ma == r_ma_p.first);
                    else // pair
                        tmp_match |= (t_ma >= r_ma_p.first && t_ma <= r_ma_p.second);
                }

                if (tmp_match)
                    break;
            }

            if (filter_mode_a_code_blacklist_)
            {
                if (tmp_match) // disable if match
                {
                    use = false;
                    comment = "Mode A";
                }
            }
            else // whitelist
            {
                if (!tmp_match) // disable if not match
                {
                    use = false;
                    comment = "Mode A";
                }
            }
        }

        if (use && remove_mode_c_values_)
        {
            if (!target_it->hasModeC())
            {
                use = false;
                comment = "Mode C not existing";
            }
            else if (target_it->modeCMax() < remove_mode_c_min_value_)
            {
                use = false;
                comment = "Max Mode C too low";
            }
        }

        if (use && filter_target_addresses_)
        {
            tmp_match = false;

            for (auto ta_it : target_it->aircraftAddresses())
            {
                tmp_match = remove_tas.count(ta_it);

                if (tmp_match)
                    break;
            }

            if (filter_target_addresses_blacklist_)
            {
                if (tmp_match) // disable if match
                {
                    use = false;
                    comment = "Target Address";
                }
            }
            else // whitelist
            {
                if (!tmp_match) // disable if not match
                {
                    use = false;
                    comment = "Target Address";
                }
            }
        }

        if (use && remove_not_detected_dbos_) // prepare associations
        {

            for (auto& dbcont_it : dbcont_manager_)
            {
                if (remove_not_detected_dbo_values_.contains(dbcont_it.first)
                        && remove_not_detected_dbo_values_.at(dbcont_it.first) == true // removed if not detected
                        && target_it->dbContentCount(dbcont_it.first) == 0) // not detected
                {
                    use = false; // remove it
                    comment = "Not Detected in "+dbcont_it.first;
                    break;
                }
            }
        }

        if (!use)
        {
            logdbg << "TargetModel: filterUTNs: removing " << target_it->utn_ << " comment '" << comment << "'";
            //useUTN (target_it->utn_, use, true);
            target_data_.modify(target_it, [use](Target& p) { p.useInEval(use); });
            //utnComment(target_it->utn_, comment, false);
            target_data_.modify(target_it, [comment](Target& p) { p.comment(comment); });
        }
    }

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
//void TargetModel::createNewTarget(unsigned int utn)
//{
//    beginResetModel();

//    assert (!existsTarget(utn));

//    target_data_.push_back({utn, nlohmann::json::object()});

//    assert (existsTarget(utn));

//    endResetModel();
//}

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
bool TargetModel::removeShortTargets() const
{
    return remove_short_targets_;
}

/**
 */
void TargetModel::removeShortTargets(bool value)
{
    loginf << "TargetModel: removeShortTargets: value " << value;

    remove_short_targets_ = value;
}

/**
 */
unsigned int TargetModel::removeShortTargetsMinUpdates() const
{
    return remove_short_targets_min_updates_;
}

/**
 */
void TargetModel::removeShortTargetsMinUpdates(unsigned int value)
{
    loginf << "TargetModel: removeShortTargetsMinUpdates: value " << value;

    remove_short_targets_min_updates_ = value;
}

/**
 */
double TargetModel::removeShortTargetsMinDuration() const
{
    return remove_short_targets_min_duration_;
}

/**
 */
void TargetModel::removeShortTargetsMinDuration(double value)
{
    loginf << "TargetModel: removeShortTargetsMinDuration: value " << value;

    remove_short_targets_min_duration_ = value;
}

/**
 */
bool TargetModel::removePsrOnlyTargets() const
{
    return remove_psr_only_targets_;
}

/**
 */
void TargetModel::removePsrOnlyTargets(bool value)
{
    loginf << "TargetModel: removePsrOnlyTargets: value " << value;

    remove_psr_only_targets_ = value;
}

/**
 */
std::string TargetModel::filterModeACodeValues() const
{
    return filter_mode_a_code_values_;
}

/**
 */
std::set<std::pair<int,int>> TargetModel::filterModeACodeData() const // single ma,-1 or range ma1,ma2
{
    std::set<std::pair<int,int>> data;

    vector<string> parts = String::split(filter_mode_a_code_values_, ',');

    for (auto& part_it : parts)
    {
        if (part_it.find("-") != std::string::npos) // range
        {
            vector<string> sub_parts = String::split(part_it, '-');

            if (sub_parts.size() != 2)
            {
                logwrn << "TargetModel: removeModeACodeData: not able to parse range '" << part_it << "'";
                continue;
            }

            int val1 = String::intFromOctalString(sub_parts.at(0));
            int val2 = String::intFromOctalString(sub_parts.at(1));

            data.insert({val1, val2});
        }
        else // single value
        {
            int val1 = String::intFromOctalString(part_it);
            data.insert({val1, -1});
        }
    }

    return data;
}

/**
 */
void TargetModel::filterModeACodeValues(const std::string& value)
{
    loginf << "TargetModel: removeModeACodeValues: value '" << value << "'";

    filter_mode_a_code_values_ = value;
}

/**
 */
std::string TargetModel::filterTargetAddressValues() const
{
    return filter_target_address_values_;
}

/**
 */
std::set<unsigned int> TargetModel::filterTargetAddressData() const
{
    std::set<unsigned int>  data;

    vector<string> parts = String::split(filter_target_address_values_, ',');

    for (auto& part_it : parts)
    {
        int val1 = String::intFromHexString(part_it);
        data.insert(val1);
    }

    return data;
}

/**
 */
void TargetModel::filterTargetAddressValues(const std::string& value)
{
    loginf << "TargetModel: removeTargetAddressValues: value '" << value << "'";

    filter_target_address_values_ = value;
}

/**
 */
bool TargetModel::removeModeACOnlys() const
{
    return remove_modeac_onlys_;
}

/**
 */
void TargetModel::removeModeACOnlys(bool value)
{
    loginf << "TargetModel: removeModeACOnlys: value " << value;
    remove_modeac_onlys_ = value;
}

/**
 */
bool TargetModel::removeNotDetectedDBContents() const
{
    return remove_not_detected_dbos_;
}

/**
 */
void TargetModel::removeNotDetectedDBContents(bool value)
{
    loginf << "TargetModel: removeNotDetectedDBOs: value " << value;

    remove_not_detected_dbos_ = value;
}

/**
 */
bool TargetModel::removeNotDetectedDBContent(const std::string& dbcontent_name) const
{
    if (!remove_not_detected_dbo_values_.contains(dbcontent_name))
        return false;

    return remove_not_detected_dbo_values_.at(dbcontent_name);
}

/**
 */
void TargetModel::removeNotDetectedDBContents(const std::string& dbcontent_name, bool value)
{
    loginf << "TargetModel: removeNotDetectedDBOs: dbo " << dbcontent_name << " value " << value;

    remove_not_detected_dbo_values_[dbcontent_name] = value;
}

/**
 */
bool TargetModel::filterTargetAddressesBlacklist() const
{
    return filter_target_addresses_blacklist_;
}

/**
 */
void TargetModel::filterTargetAddressesBlacklist(bool value)
{
    loginf << "TargetModel: filterTargetAddressesBlacklist: value " << value;

    filter_target_addresses_blacklist_ = value;
}

/**
 */
bool TargetModel::filterModeACodeBlacklist() const
{
    return filter_mode_a_code_blacklist_;
}

/**
 */
void TargetModel::filterModeACodeBlacklist(bool value)
{
    loginf << "TargetModel: filterModeACodeBlacklist: value " << value;

    filter_mode_a_code_blacklist_ = value;
}

/**
 */
bool TargetModel::removeModeCValues() const
{
    return remove_mode_c_values_;
}

/**
 */
void TargetModel::removeModeCValues(bool value)
{
    loginf << "TargetModel: removeModeCValues: value " << value;

    remove_mode_c_values_ = value;
}

/**
 */
float TargetModel::removeModeCMinValue() const
{
    return remove_mode_c_min_value_;
}

/**
 */
void TargetModel::removeModeCMinValue(float value)
{
    loginf << "TargetModel: removeModeCMinValue: value " << value;
    remove_mode_c_min_value_ = value;
}

/**
 */
bool TargetModel::filterTargetAddresses() const
{
    return filter_target_addresses_;
}

/**
 */
void TargetModel::filterTargetAddresses(bool value)
{
    loginf << "TargetModel: removeTargetAddresses: value " << value;

    filter_target_addresses_ = value;
}

/**
 */
bool TargetModel::filterModeACodes() const
{
    return filter_mode_a_codes_;
}

/**
 */
void TargetModel::filterModeACodes(bool value)
{
    loginf << "TargetModel: removeModeACodes: value " << value;

    filter_mode_a_codes_ = value;
}

/**
 */
void TargetModel::showMainColumns(bool show)
{
    show_main_columns_ = show;
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

}
