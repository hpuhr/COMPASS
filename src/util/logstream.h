#pragma once

#include <string>
#include <sstream>
#include <functional>

#include <QObject>
#include <QDateTime>

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

class LogStore : public QObject
{
    Q_OBJECT

signals:
    void messagesChangedSignal();

public:
    struct LogEntry {
        QString timestamp;
        std::string message;
        LogStreamType type;
        bool accepted;
    };

    LogStream logInfo() {
        return LogStream([this](const std::string& msg) { this->addLogMessage(msg, LogStreamType::Info); });
    }
    LogStream logWarn() {
        return LogStream([this](const std::string& msg) { this->addLogMessage(msg, LogStreamType::Warning); });
    }
    LogStream logError() {
        return LogStream([this](const std::string& msg) { this->addLogMessage(msg, LogStreamType::Error); });
    }

    void addLogMessage(const std::string& message, LogStreamType type)
    {
        log_entries_.push_back({QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"), message, type, false});

        emit messagesChangedSignal();
    }
    void acceptMessages()
    {
        for (auto& entry : log_entries_)
            entry.accepted = true;

        emit messagesChangedSignal();
    }

    const std::vector<LogEntry>& logEntries() const { return log_entries_; }

protected:
    std::vector<LogEntry> log_entries_;
};

