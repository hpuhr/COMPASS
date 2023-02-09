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
}

/**
 * Class for listening for and processing string commands at runtime,
 * either coming externally from a TCP port or internally from the application itself.
 */
class RTCommandManager : public QThread, public Singleton, public Configurable
{
public:
    static bool open_port_;

    virtual ~RTCommandManager();

    void shutdown();

    static RTCommandManager& instance()
    {
        static RTCommandManager instance;
        return instance;
    }

    bool addCommand(const std::string& cmd_str); // true on success, false on failed

    std::vector<std::string> commandBacklog() const;
    void clearBacklog();

protected:
    volatile bool stop_requested_;
    volatile bool stopped_;

    unsigned int port_num_ {27960};

    std::queue<std::unique_ptr<rtcommand::RTCommand>> command_queue_;
    boost::mutex command_queue_mutex_;

//    bool command_active_ {false};
//    std::future<rtcommand::RTCommandResult> current_result_;

    RTCommandManager();

private:
    static const size_t BacklogSize = 100;

    void run();

    void addToBacklog(const std::string& cmd);

    nlohmann::json command_backlog_;
};

#endif // RTCOMMANDRECEIVER_H
