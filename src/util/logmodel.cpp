#include "logmodel.h"
#include "compass.h"
#include "logger.h"
#include "stringconv.h"
#include "files.h"

#include <QBrush>
#include <QFont>

using namespace Utils;

LogStore::LogStore(bool show_everything)
{
    table_columns_ = QStringList::fromVector({"", "Time", "Type", "Component", "Message"});

    if (show_everything)
        table_columns_.append(QStringList::fromVector({"Error", "JSON", "Count"}));

    checked_icon_ = QIcon(Files::getIconFilepath("done.png").c_str());

    logInfo("Test", {}, {}) << "Test Info";
    logWarn("Test", {}, {}) << "Test Warning";
    logError("Test", {}, {}) << "Test Error";

    acceptMessages();

    logInfo("Test", {}, {}) << "Test Info2";
    logWarn("Test", {}, {}) << "Test Warning2";
    logError("Test", {}, {}) << "Test Error2";
}

LogStream LogStore::logInfo(const std::string& component,
                            boost::optional<unsigned int> error_code, nlohmann::json json_blob) {
    return LogStream([this, component, error_code, json_blob](const std::string& msg) {
        this->addLogMessage(msg, LogStreamType::Info, component, error_code, json_blob); });
}
LogStream LogStore::logWarn(const std::string& component,
                            boost::optional<unsigned int> error_code, nlohmann::json json_blob) {
    return LogStream([this, component, error_code, json_blob](const std::string& msg) {
        this->addLogMessage(msg, LogStreamType::Warning, component, error_code, json_blob); });
}
LogStream LogStore::logError(const std::string& component,
                             boost::optional<unsigned int> error_code, nlohmann::json json_blob) {
    return LogStream([this, component, error_code, json_blob](const std::string& msg) {
        this->addLogMessage(msg, LogStreamType::Error, component, error_code, json_blob); });
}

void LogStore::addLogMessage(const std::string& message, LogStreamType type, const std::string& component,
                             boost::optional<unsigned int> error_code, nlohmann::json json_blob)
{
    beginResetModel();

    log_entries_.push_back({false, QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"), type,
                            component, message, error_code, json_blob});

    endResetModel();

    emit messagesChangedSignal();
}

void LogStore::acceptMessages()
{
    beginResetModel();

    for (auto& entry : log_entries_)
        entry.accepted = true;

    endResetModel();

    emit messagesChangedSignal();
}

QVariant LogStore::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    assert (index.row() >= 0);
    assert (index.row() < log_entries_.size());

    const LogEntry& entry = log_entries_.at(index.row());

    switch (role)
    {
    // case Qt::CheckStateRole:
    // {
    //     if (index.column() == 0)  // selected special case
    //     {
    //         if (entry.accepted)
    //             return Qt::Checked;
    //         else
    //             return Qt::Unchecked;
    //     }
    //     else
    //         return QVariant();
    // }
    case Qt::ForegroundRole:
    {
        if (!entry.accepted)
        {
            switch(entry.type)
            {
            case LogStreamType::Error:
                return QBrush(Qt::red);
            case LogStreamType::Warning:
                return QBrush(QColor("orange"));
            case LogStreamType::Info:
            default:
                break;
            }
        }

        return QVariant();
    }
    case Qt::BackgroundRole:
    {
        if (entry.accepted)
            return QBrush(QColor("gainsboro"));
        else
            return QVariant();

    }
    case Qt::FontRole:
    {
        QFont font;

        if (!entry.accepted)
        {
            switch(entry.type)
            {
            case LogStreamType::Error:
                font.setBold(true);
                break;
            case LogStreamType::Warning:
                font.setItalic(true);
                break;
            case LogStreamType::Info:
            default:
                break;
            }
        }

        return font;
    }
    case Qt::DisplayRole:
    //case Qt::EditRole:
    {
        logdbg << "LogStore: data: display role: row " << index.row() << " col " << index.column();

        assert (index.column() < table_columns_.size());
        std::string col_name = table_columns_.at(index.column()).toStdString();

        if (col_name == "")
        {
            return QVariant();
        }
        else if (col_name == "Time")
        {
            return entry.timestamp;
        }
        else if (col_name == "Type")
        {
            switch(entry.type)
            {
            case LogStreamType::Info:
                return "Info";
            case LogStreamType::Warning:
                return "Warning";
            case LogStreamType::Error:
                return "Error";
            default:
                return "Unknown";;
            }
        }
        else if (col_name == "Component")
        {
            return entry.component_.c_str();
        }
        else if (col_name == "Message")
        {
            return entry.message.c_str();
        }
        else if (col_name == "Error")
        {
            return entry.error_code_ ?
                       ("0x"+String::hexStringFromInt((unsigned int) *entry.error_code_)).c_str() : QVariant();
        }
        else if (col_name == "JSON")
        {
            return entry.json_.empty() ? QVariant() : entry.json_.dump(2).c_str();
        }
        else if (col_name == "Count")
        {
            return entry.message_count_ ? entry.message_count_ : QVariant();
        }

    }
    case Qt::DecorationRole:
    {
        if (index.column() > 0)  // only col 0 have icons
            return QVariant();

        if (entry.accepted)
            return checked_icon_;
        else
            return QVariant();;
    }
    // case Qt::UserRole: // to find the checkboxes
    // {
    //     if (index.column() == 0)
    //     {
    //         assert (index.row() >= 0);
    //         assert (index.row() < target_data_.size());

    //         const Target& target = target_data_.at(index.row());
    //         return target.utn_;
    //     }
    //     else if (index.column() == 2) // comment
    //     {
    //         assert (index.row() >= 0);
    //         assert (index.row() < target_data_.size());

    //         const Target& target = target_data_.at(index.row());
    //         return ("comment_"+to_string(target.utn_)).c_str();
    //     }
    // }
    default:
    {
        return QVariant();
    }
    }
}

bool LogStore::setData(const QModelIndex &index, const QVariant& value, int role)
{
    // if (!index.isValid() /*|| role != Qt::EditRole*/)
    //     return false;

    // if (role == Qt::CheckStateRole && index.column() == 0)
    // {
    //     assert (index.row() >= 0);
    //     assert (index.row() < target_data_.size());

    //     auto it = target_data_.begin()+index.row();

    //     bool checked = (Qt::CheckState)value.toInt() == Qt::Checked;
    //     loginf << "LogStore: setData: utn " << it->utn_ <<" check state " << checked;

    //     //eval_man_.useUTN(it->utn_, checked, false);
    //     target_data_.modify(it, [value,checked](Target& p) { p.useInEval(checked); });

    //     saveToDB(it->utn_);

    //     emit dataChanged(index, LogStore::index(index.row(), columnCount()-1));
    //     emit dbcont_manager_.targetChangedSignal(it->utn_);

    //     return true;
    // }
    // else if (role == Qt::EditRole && index.column() == 2) // comment
    // {
    //     assert (index.row() >= 0);
    //     assert (index.row() < target_data_.size());

    //     auto it = target_data_.begin()+index.row();

    //     loginf << "LogStore: setData: utn " << it->utn_ <<" comment '" << value.toString().toStdString() << "'";

    //     target_data_.modify(it, [value](Target& p) { p.comment(value.toString().toStdString()); });

    //     saveToDB(it->utn_);

    //     emit dbcont_manager_.targetChangedSignal(it->utn_);

    //     return true;
    // }

    return false;
}


QVariant LogStore::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        assert (section < table_columns_.size());
        return table_columns_.at(section);
    }

    return QVariant();
}

QModelIndex LogStore::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column);
}

int LogStore::rowCount(const QModelIndex& parent) const
{
    return log_entries_.size();
}

int LogStore::columnCount(const QModelIndex& parent) const
{
    return table_columns_.size();
}

QModelIndex LogStore::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

Qt::ItemFlags LogStore::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;

    assert (index.column() < table_columns_.size());

    // if (index.column() == 0) // Use
    // {
    //     return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    // }
    // else if (index.column() == 2) // comment
    // {
    //     return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    // }
    // else
        return QAbstractItemModel::flags(index);
}
