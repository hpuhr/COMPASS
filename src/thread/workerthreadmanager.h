/*
 * This file is part of ATSDB.
 *
 * ATSDB is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ATSDB is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with ATSDB.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * WorkerThreadManager.h
 *
 *  Created on: Mar 26, 2012
 *      Author: sk
 */

#ifndef WORKERTHREADMANAGER_H_
#define WORKERTHREADMANAGER_H_

#ifndef Q_MOC_RUN
#include <boost/thread/mutex.hpp>
#endif
//#include <boost/asio.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/thread.hpp>
#include <list>
#include <memory>
#include <qobject.h>
#include <qthread.h>

#include "singleton.h"
#include "configurable.h"

class WorkerThread;
class DBJob;
class Job;
class QTimer;
//class WorkerThreadWidget;

/**
 * @brief Manages execution of TransformationJobs
 *
 * Allows addition of TransformationJobs, which are held in a list and assigned to any active TransformationWorkers.
 * A number of such TransformationWorkers are generated and managed.
 * Using a timer event, jobs are checked if earlier jobs are done and flushed in the order of addition. Jobs may be
 * done, but can be blocked by unfinished jobs which were added earlier.
 *
 */
class WorkerThreadManager: public QThread, public Singleton, public Configurable
{
    Q_OBJECT
//private slots:
//    void timerEventSlot();

public:
    /// @brief Constructor
    WorkerThreadManager();
    virtual ~WorkerThreadManager();

    void addJob (std::shared_ptr<Job> job);
    void addDBJob (std::shared_ptr<DBJob> job);

    void shutdown ();

    bool noJobs ();

    static WorkerThreadManager& getInstance()
    {
        static WorkerThreadManager instance;
        return instance;
    }

    WorkerThread *getWorker (unsigned int index);
    unsigned int getJobNumber ();
    unsigned int getNumWorkers ();

protected:
    /// Flag indicating if thread should stop.
    volatile bool stop_requested_;
    /// Boost thread
    //boost::shared_ptr<boost::thread> thread_;

    boost::mutex mutex_;
    unsigned int num_workers_;

    unsigned int update_time_;

    std::list <std::shared_ptr<Job>> todos_signal_;
    std::list <std::shared_ptr<DBJob>> db_todos_signal_;

    std::vector <WorkerThread *> workers_;
    unsigned int cnt_;

//    QTimer *timer_;
    //    boost::asio::io_service io_;
//    boost::asio::deadline_timer timer_;

//    ///@brief Starts the thread.
//    void go();
//    ///@brief Stops the thread.
//    void stop();
//    ///@brief Thread main function doing all the work.
//    void do_work();

    void flushFinishedJobs ();

private:
    void run ();

};

#endif /* WorkerThreadManager_H_ */
