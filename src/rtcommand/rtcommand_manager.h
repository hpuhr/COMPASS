#ifndef RTCOMMANDRECEIVER_H
#define RTCOMMANDRECEIVER_H

#include "configurable.h"
#include "singleton.h"
#include "logger.h"
#include "rtcommand_defs.h"
#include "json.h"

#include <QThread>

#include <boost/thread/mutex.hpp>

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <queue>
#include <future>

// netcat call for console
// netcat 127.0.0.1 27960

namespace rtcommand
{
    struct RTCommand;
    struct RTCommandResponse;
    class  RTCommandShell;
}

/**
 * Class for listening for and processing string commands at runtime,
 * either coming externally from a TCP port or internally from the application itself.
 */
class RTCommandManager : public QThread, public Singleton, public Configurable
{
    Q_OBJECT
public:
    typedef std::unique_ptr<rtcommand::RTCommand>     CommandPtr;
    typedef uint64_t                                  CommandId;
    typedef std::pair<CommandId,rtcommand::ErrorInfo> AddInfo;

    enum class Source
    {
        Server = 0,
        Application,
        Shell
    };

    struct QueuedCommand
    {
        Source     source;
        CommandId  id;
        CommandPtr command;
    };

    static bool open_port_;

    virtual ~RTCommandManager();

    void shutdown();

    static RTCommandManager& instance()
    {
        static RTCommandManager instance;
        return instance;
    }

    rtcommand::IssueInfo addCommand(const std::string& cmd_str, CommandId* id = nullptr);

    void clearBacklog();
    std::vector<std::string> commandBacklog() const;

signals:
    void commandProcessed(CommandId id, std::string msg, std::string data, bool is_error);
    void shellCommandProcessed(const QString& msg, const QString& data, bool is_error);

protected:
    volatile bool stop_requested_;
    volatile bool stopped_;

    unsigned int port_num_ {27960};

    std::queue<QueuedCommand> command_queue_;
    boost::mutex command_queue_mutex_;

//    bool command_active_ {false};
//    std::future<rtcommand::RTCommandResult> current_result_;

    RTCommandManager();

private:
    friend class rtcommand::RTCommandShell;

    static const size_t BacklogSize = 100;

    static CommandId command_count_;

    void run();

    void addToBacklog(const std::string& cmd);
    rtcommand::IssueInfo addCommand(const std::string& cmd_str, Source source, CommandId* id = nullptr);

    nlohmann::json command_backlog_;
};

#endif // RTCOMMANDRECEIVER_H
