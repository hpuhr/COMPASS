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

#pragma once

#include "configurable.h"
#include "singleton.h"
#include "job.h"

#ifndef Q_MOC_RUN
#include <boost/date_time/posix_time/posix_time.hpp>
#endif

#include <QMutex>
#include <QThread>

#include "util/tbbhack.h"

#include <list>
#include <memory>

class WorkerThread;

/**
 */
class JobManagerBase : public QThread
{
public:
    JobManagerBase();
    virtual ~JobManagerBase();

    // blocks started of later ones
    void addBlockingJob(std::shared_ptr<Job> job);
    // does not block start of later ones
    void addNonBlockingJob(std::shared_ptr<Job> job);
    // only one db job can be active
    void addDBJob(std::shared_ptr<Job> job);

    void cancelJob(std::shared_ptr<Job> job);

    bool hasAnyJobs() const;

    virtual bool hasBlockingJobs() const = 0;
    virtual bool hasNonBlockingJobs() const = 0;
    virtual bool hasDBJobs() const = 0;

    unsigned int numJobs() const;

    virtual unsigned int numBlockingJobs() const = 0;
    virtual unsigned int numNonBlockingJobs() const = 0;
    virtual unsigned int numDBJobs() const = 0;

    virtual int numThreads() const = 0;

    void shutdown();

protected:
    virtual void addBlockingJob_impl(std::shared_ptr<Job> job) = 0;
    virtual void addNonBlockingJob_impl(std::shared_ptr<Job> job) = 0;
    virtual void addDBJob_impl(std::shared_ptr<Job> job) = 0;

    virtual void handleBlockingJobs() = 0;
    virtual void handleNonBlockingJobs() = 0;
    virtual void handleDBJobs() = 0;

    virtual void setJobsObsolete() = 0;

    void run();

    volatile bool stop_requested_;
    volatile bool stopped_;

    boost::posix_time::ptime last_update_time_;

private:
    size_t non_blocking_ids_ = 0;
    size_t blocking_ids_     = 0;
    size_t db_ids_           = 0;

    bool set_thread_affinity_ = false;
};

/**
 */
class JobManagerAsync : public JobManagerBase
{
public:
    /**
     */
    struct AsyncJob
    {
        void exec();
        bool done() const;

        std::shared_ptr<Job> job_;
        std::future<bool>    future_;
        bool                 is_running_ = false;
    };

    typedef std::shared_ptr<AsyncJob> AsyncJobPtr;

    JobManagerAsync();
    virtual ~JobManagerAsync();

    bool hasBlockingJobs() const override;
    bool hasNonBlockingJobs() const override;
    bool hasDBJobs() const override;

    unsigned int numBlockingJobs() const override;
    unsigned int numNonBlockingJobs() const override;
    unsigned int numDBJobs() const override;

    int numThreads() const override;

protected:
    void addBlockingJob_impl(std::shared_ptr<Job> job) override;
    void addNonBlockingJob_impl(std::shared_ptr<Job> job) override;
    void addDBJob_impl(std::shared_ptr<Job> job) override;

    void handleBlockingJobs() override;
    void handleNonBlockingJobs() override;
    void handleDBJobs() override;

    void setJobsObsolete() override;

private:
    AsyncJobPtr active_blocking_job_;
    tbb::concurrent_queue<AsyncJobPtr> blocking_jobs_;

    AsyncJobPtr active_non_blocking_job_;
    tbb::concurrent_queue<AsyncJobPtr> non_blocking_jobs_;

    AsyncJobPtr active_db_job_;
    tbb::concurrent_queue<AsyncJobPtr> queued_db_jobs_;
};

/**
 */
class JobManagerThreadPool : public JobManagerBase
{
public:
    JobManagerThreadPool();
    virtual ~JobManagerThreadPool();

    bool hasBlockingJobs() const override;
    bool hasNonBlockingJobs() const override;
    bool hasDBJobs() const override;

    unsigned int numBlockingJobs() const override;
    unsigned int numNonBlockingJobs() const override;
    unsigned int numDBJobs() const override;

    int numThreads() const override;

protected:
    void addBlockingJob_impl(std::shared_ptr<Job> job) override;
    void addNonBlockingJob_impl(std::shared_ptr<Job> job) override;
    void addDBJob_impl(std::shared_ptr<Job> job) override;

    void handleBlockingJobs() override;
    void handleNonBlockingJobs() override;
    void handleDBJobs() override;

    void setJobsObsolete() override;

private:
    std::shared_ptr<Job> active_blocking_job_;
    tbb::concurrent_queue<std::shared_ptr<Job>> blocking_jobs_;

    std::shared_ptr<Job> active_non_blocking_job_;
    tbb::concurrent_queue<std::shared_ptr<Job>> non_blocking_jobs_;

    std::shared_ptr<Job> active_db_job_;
    tbb::concurrent_queue<std::shared_ptr<Job>> queued_db_jobs_;
};

/**
 */
#ifdef USE_ASYNC_JOBS
class JobManager : public JobManagerAsync, public Singleton, public Configurable
#else 
class JobManager : public JobManagerThreadPool, public Singleton, public Configurable
{
//    Q_OBJECT
//  signals:
//    void databaseBusy();
//    void databaseIdle();

public:
    virtual ~JobManager();

    static JobManager& instance()
    {
        static JobManager instance;
        return instance;
    }

protected:
    JobManager();
};

#endif
