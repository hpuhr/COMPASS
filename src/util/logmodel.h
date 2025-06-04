#pragma once

#include "json.hpp"
#include "property.h"
#include "propertylist.h"

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

    LogStream(const LogStream&) = delete;
    LogStream& operator=(const LogStream&) = delete;

    LogStream(LogStream&& other) noexcept
        : buffer_(std::move(other.buffer_)),          // Move the stream buffer
        commit_func_(std::move(other.commit_func_)) // Move the commit function
    {
        // After moving, 'other' is left in a valid but unspecified state.
    }

    // Move assignment operator
    LogStream& operator=(LogStream&& other) noexcept {
        if (this != &other) {
            // Move assign the stream buffer.
            buffer_ = std::move(other.buffer_);
            // Move assign the commit function.
            commit_func_ = std::move(other.commit_func_);
            // 'other' is now in a moved-from state, and its destructor
            // will not perform the commit action if commit_func_ is empty.
        }
        return *this;
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

public slots:
    void databaseOpenedSlot();
    void databaseClosedSlot();

public:
    struct LogEntry {
        LogEntry(unsigned int msg_id, bool accepted, const std::string timestamp, LogStreamType type,
                 const std::string& component,
                 const std::string& message, boost::optional<unsigned int> error_code, const nlohmann::json& json)
            : msg_id_(msg_id),accepted_(accepted),
            timestamp_(timestamp),
            type_(type),
            component_(component),
            message_(message),
            error_code_(error_code),
            json_(json)
        {
        }

        LogEntry(const nlohmann::json& info); // from database

        const unsigned int msg_id_;
        bool accepted_;
        std::string timestamp_;
        LogStreamType type_;
        std::string component_; // A, A:B
        std::string message_;
        boost::optional<unsigned int> error_code_;
        nlohmann::json json_;
        unsigned int message_count_ {0};

        nlohmann::json asJSON() const;

        static std::string logStreamTypeStr (LogStreamType type);
        static LogStreamType logStreamTypeFromStr (const std::string& type_str);

        static const Property     DBColumnID;
        static const Property     DBColumnInfo;
        static const PropertyList DBPropertyList;
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

    void clearMessages();
    void loadMessagesFromDB();

protected:
    QStringList table_columns_;

    std::vector<LogEntry> log_entries_;

    QIcon checked_icon_;
};

