#pragma once

#include "json.hpp"

#include <string>
#include <sstream>
#include <functional>

#include "boost/optional.hpp"

#include <QDateTime>
#include <QIcon>
#include <QAbstractItemModel>

enum class LogStreamType { Info, Warning, Error };

class LogStream
{
public:
    using CommitFunc = std::function<void(const std::string&)>;

    LogStream(CommitFunc commit)
        : commit_func_(std::move(commit)) {}

    ~LogStream() {
        if (commit_func_)
            commit_func_(buffer_.str());
    }

    template <typename T>
    LogStream& operator<<(const T& val) {
        buffer_ << val;
        return *this;
    }

private:
    std::ostringstream buffer_;
    CommitFunc commit_func_;
};

class LogStore : public QAbstractItemModel
{
    Q_OBJECT

signals:
    void messagesChangedSignal();

public:
    struct LogEntry {
        bool accepted;
        QString timestamp;
        LogStreamType type;
        std::string component_; // A, A:B
        std::string message;
        boost::optional<unsigned int> error_code_;
        nlohmann::json json_;
        unsigned int message_count_ {0};
    };

    LogStore(bool show_everything);
    virtual ~LogStore() = default;

    LogStream logInfo(const std::string& component,
                      boost::optional<unsigned int> error_code, nlohmann::json json_blob);
    LogStream logWarn(const std::string& component,
                      boost::optional<unsigned int> error_code, nlohmann::json json_blob);
    LogStream logError(const std::string& component,
                       boost::optional<unsigned int> error_code, nlohmann::json json_blob);

    void addLogMessage(const std::string& message, LogStreamType type, const std::string& component,
                       boost::optional<unsigned int> error_code, nlohmann::json json_blob);

    void acceptMessages();

    const std::vector<LogEntry>& logEntries() const { return log_entries_; }

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant& value, int role) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
    QStringList table_columns_;

    std::vector<LogEntry> log_entries_;

    QIcon checked_icon_;
};

