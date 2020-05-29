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

#ifndef JOBMANAGER_H_
#define JOBMANAGER_H_

#ifndef Q_MOC_RUN
#include <boost/date_time/posix_time/posix_time.hpp>
#endif

#include <tbb/concurrent_queue.h>

#include <QMutex>
#include <QThread>
#include <list>
#include <memory>

#include "configurable.h"
#include "singleton.h"

class WorkerThread;
// class DBJob;
class Job;
class JobManagerWidget;

/**
 * @brief Manages execution of TransformationJobs
 *
 * Allows addition of TransformationJobs, which are held in a list and assigned to any active
 * TransformationWorkers. A number of such TransformationWorkers are generated and managed. Using a
 * timer event, jobs are checked if earlier jobs are done and flushed in the order of addition. Jobs
 * may be done, but can be blocked by unfinished jobs which were added earlier.
 *
 */
class JobManager : public QThread, public Singleton, public Configurable
{
    Q_OBJECT
  signals:
    void databaseBusy();
    void databaseIdle();

  public:
    virtual ~JobManager();

    // all job's done signal order is maintained in the call order

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

    JobManagerWidget* widget();

  protected:
    /// Flag indicating if thread should stop.
    volatile bool stop_requested_;
    volatile bool stopped_;

    bool changed_{false};
    bool really_update_widget_{false};

    std::shared_ptr<Job> active_blocking_job_;
    tbb::concurrent_queue<std::shared_ptr<Job>> blocking_jobs_;

    std::shared_ptr<Job> active_non_blocking_job_;
    tbb::concurrent_queue<std::shared_ptr<Job>> non_blocking_jobs_;

    std::shared_ptr<Job> active_db_job_;
    tbb::concurrent_queue<std::shared_ptr<Job>> queued_db_jobs_;

    JobManagerWidget* widget_;

    boost::posix_time::ptime last_update_time_;

    /// @brief Constructor
    JobManager();

    void updateWidget(bool really = false);

  private:
    void run();

    // set change flags as appropriate
    void handleBlockingJobs();
    void handleNonBlockingJobs();
    void handleDBJobs();
};

#endif /* JOBMANAGER_H_ */
