#ifndef RTCOMMANDRECEIVER_H
#define RTCOMMANDRECEIVER_H

#include "configurable.h"
#include "singleton.h"
#include "logger.h"
#include "rtcommand_defs.h"


#include <QThread>

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

class RTCommandManager : public QThread, public Singleton, public Configurable
{
public:
    virtual ~RTCommandManager();

    void shutdown();

    static RTCommandManager& instance()
    {
        static RTCommandManager instance;
        return instance;
    }

    bool injectCommand(const std::string& cmd_str); // true on success, false on failed

protected:
    volatile bool stop_requested_;
    volatile bool stopped_;

    unsigned int port_num_ {27960};

    std::queue<std::unique_ptr<rtcommand::RTCommand>> command_queue_;

//    bool command_active_ {false};
//    std::future<rtcommand::RTCommandResult> current_result_;

    RTCommandManager();

private:
    void run();
};

#endif // RTCOMMANDRECEIVER_H
