#include "rtcommand_manager.h"
#include "rtcommand_string.h"
#include "rtcommand_runner.h"
#include "tcpserver.h"
#include "stringconv.h"
#include "rtcommand.h"
#include "compass.h"

#include "logger.h"

#include <QCoreApplication>

#include <boost/bind.hpp>
#include <boost/thread.hpp>

using namespace std;
using namespace Utils;

RTCommandManager::RTCommandManager()
    : Configurable("RTCommandManager", "RTCommandManager0", 0, "rtcommand.json"),
      stop_requested_(false), stopped_(false)
{
    loginf << "JobManager: constructor";

    registerParameter("port_num", &port_num_, 27960);
}


RTCommandManager::~RTCommandManager() { loginf << "JobManager: destructor"; }


void RTCommandManager::run()
{
    loginf<< "RTCommandManager: run: start";

    loginf<< "RTCommandManager: run: io context";
    boost::asio::io_context io_context;

    TCPServer server (io_context, port_num_);

    loginf<< "RTCommandManager: run: running io context";

    boost::thread t(boost::bind(&boost::asio::io_context::run, &io_context));
    t.detach();

    loginf<< "RTCommandManager: run: starting loop";

    while (1)
    {
        if (stop_requested_) //  && !hasAnyJobs()
            break;

        if (server.hasSession() && server.hasStrData())
        {
            logdbg << "RTCommandManager: run: getting commands";

            std::vector<std::string> cmds = server.getStrData();

            loginf<< "RTCommandManager: run: got " << cmds.size() << " commands '"
                  << String::compress(cmds, ';') << "'";

            // create commands from strings
            for (const auto& cmd_str : cmds)
            {
                if (injectCommand(cmd_str)) // success
                {
                    server.sendStrData("accepted '"+cmd_str+"'");

                    loginf<< "RTCommandManager: run: added commend " << cmd_str << " to queue, size "
                          << command_queue_.size();
                }
                else
                    server.sendStrData("refused '"+cmd_str+"'");
            }
        }

        // do commands

        if (command_queue_.size())
        {
            loginf<< "RTCommandManager: run: issuing command";

            rtcommand::RTCommandRunner& cmd_runner = COMPASS::instance().rtCmdRunner();

            std::future<std::vector<rtcommand::RTCommandResult>> current_result = cmd_runner.runCommand(
                        move(command_queue_.front()));
            command_queue_.pop();

            loginf<< "RTCommandManager: run: waiting for result";

            current_result.wait();

            std::vector<rtcommand::RTCommandResult> results = current_result.get();
            assert (results.size() == 1);

            rtcommand::RTCommandResult cmd_result = results.at(0);

            loginf<< "RTCommandManager: run: result wait done, success " << cmd_result.success();

            if (server.hasSession())
            {
                if (cmd_result.success())
                    server.sendStrData("successfully run command, result '"+cmd_result.toString().toStdString()+"'");
                else
                    server.sendStrData("failed command, result '"+cmd_result.toString().toStdString()+"'");
            }
        }

        msleep(1);
    }

    io_context.stop();
    assert (io_context.stopped());

    t.timed_join(100);

    stopped_ = true;
    loginf << "RTCommandManager: run: stopped";
}


void RTCommandManager::shutdown()
{
    loginf << "RTCommandManager: shutdown";

    stop_requested_ = true;

    //    if (active_db_job_)
    //        active_db_job_->setObsolete();

    //    for (auto job_it = queued_db_jobs_.unsafe_begin(); job_it != queued_db_jobs_.unsafe_end();
    //         ++job_it)
    //        (*job_it)->setObsolete();

    //    while (hasAnyJobs())
    //    {
    //        loginf << "JobManager: shutdown: waiting on jobs to finish: db " << hasDBJobs()
    //               << " blocking " << hasBlockingJobs() << " non-locking " << hasNonBlockingJobs();

    //        msleep(1000);
    //    }

    while (!stopped_)
    {
        loginf << "RTCommandManager: shutdown: waiting on run stop";
        msleep(1000);
    }

    //    assert(!active_blocking_job_);
    //    assert(blocking_jobs_.empty());

    loginf << "RTCommandManager: shutdown: done";
}

bool RTCommandManager::injectCommand(const std::string& cmd_str)
{
    rtcommand::RTCommandString cmd_inst (QString(cmd_str.c_str())); // todo change to std str

    auto rtcmd_inst = cmd_inst.issue();

    if (!rtcmd_inst)
        return false;

    command_queue_.push(move(rtcmd_inst));

    return true;
}
