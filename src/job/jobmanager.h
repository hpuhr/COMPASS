/*
 * This file is part of OpenATS COMPASS.
 *
 * COMPASS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * COMPASS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with COMPASS. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JOBMANAGER_H_
#define JOBMANAGER_H_

#include "configurable.h"
#include "singleton.h"

#ifndef Q_MOC_RUN
#include <boost/date_time/posix_time/posix_time.hpp>
#endif

#include <QMutex>
#include <QThread>

#include "util/tbbhack.h"

#include <list>
#include <memory>

class WorkerThread;
class Job;

class JobManager : public QThread, public Singleton, public Configurable
{
//    Q_OBJECT
//  signals:
//    void databaseBusy();
//    void databaseIdle();

  public:
    virtual ~JobManager();

    // blocks started of later ones
    void addBlockingJob(std::shared_ptr<Job> job);
    // does not block start of later ones
    void addNonBlockingJob(std::shared_ptr<Job> job);
    // only one db job can be active
    void addDBJob(std::shared_ptr<Job> job);
    void cancelJob(std::shared_ptr<Job> job);

    bool hasAnyJobs();
    bool hasBlockingJobs();
    bool hasNonBlockingJobs();
    bool hasDBJobs();

    unsigned int numBlockingJobs();
    unsigned int numNonBlockingJobs();
    unsigned int numJobs();
    unsigned int numDBJobs();
    int numThreads();

    void shutdown();

    static JobManager& instance()
    {
        static JobManager instance;
        return instance;
    }

  protected:
    volatile bool stop_requested_;
    volatile bool stopped_;

    //bool changed_{false};
    //bool really_update_widget_{false};

    std::shared_ptr<Job> active_blocking_job_;
    tbb::concurrent_queue<std::shared_ptr<Job>> blocking_jobs_;

    std::shared_ptr<Job> active_non_blocking_job_;
    tbb::concurrent_queue<std::shared_ptr<Job>> non_blocking_jobs_;

    std::shared_ptr<Job> active_db_job_;
    tbb::concurrent_queue<std::shared_ptr<Job>> queued_db_jobs_;

    boost::posix_time::ptime last_update_time_;

    JobManager();

  private:
    void run();

    // set change flags as appropriate
    void handleBlockingJobs();
    void handleNonBlockingJobs();
    void handleDBJobs();
};

#endif /* JOBMANAGER_H_ */
