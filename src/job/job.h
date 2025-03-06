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

#include "logger.h"

#include <memory>

//use std::async instead of QThreadPool
//#define USE_ASYNC_JOBS

#include <QObject>
#include <QThread>

#ifndef USE_ASYNC_JOBS
#include <QRunnable>
#endif

#include <boost/optional.hpp>

/**
 * @brief Encapsulates a work-package
 *
 * When created, is automatically added to the JobOrderer. Callbacks are performed either when the
 * Job was canceled (set as obsolete) or was completed (done). The work itself is defined in the
 * execute function, which must be overridden (and MUST set the done_ flag to true).
 *
 * Important: The Job and the contained data must be deleted in the callback functions.
 */
#ifdef USE_ASYNC_JOBS
class Job : public QObject
#else
class Job : public QObject, public QRunnable
#endif
{
    Q_OBJECT
signals:
    void doneSignal();
    void obsoleteSignal();

public:
#ifdef USE_ASYNC_JOBS
    /// @brief Constructor
    Job(const std::string& name) : name_(name) {}
#else
    /// @brief Constructor
    Job(const std::string& name) : name_(name) { setAutoDelete(false); }
#endif
    /// @brief Destructor
    virtual ~Job() {}
  
    // @brief Main operation function
#ifdef USE_ASYNC_JOBS
    void run()
#else
    void run() override final
#endif
    {
        //set thread affinity?
        if (set_thread_affinity_ && job_id_.has_value())
        {
            //evenly distribute over cpus
            int cpu = (int)(job_id_.value() % (size_t)QThread::idealThreadCount());

            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(cpu, &cpuset);

            pthread_t nativeThread = pthread_self();
            if (pthread_setaffinity_np(nativeThread, sizeof(cpu_set_t), &cpuset) != 0)
                logerr << "Job: run: failed to set thread affinity of job " << job_id_.value() << " to cpu" << cpu;
        }

        //invoke derived
        run_impl();
    }

    void setJobID(size_t id, bool set_thread_affinity = false)
    {
        job_id_ = id;
        set_thread_affinity_ = set_thread_affinity;
    }

    bool started() { return started_; }
    // @brief Returns done flag
    bool done() { return done_; }
    void emitDone() { emit doneSignal(); }
    // @brief Sets obsolete flag
    virtual void setObsolete() 
    {
        logdbg << "Job: " << name_ << ": setObsolete";
        obsolete_ = true;
    }
    // @brief Returns obsolete flag
    bool obsolete() { return obsolete_; }
    //void emitObsolete() { emit doneSignal(); }

    const std::string& name() { return name_; }

protected:
    virtual void run_impl() = 0;

    std::string name_;
    ///
    bool started_{false};
    /// Done flag
    bool done_{false};
    /// Obsolete flag
    volatile bool obsolete_{false};

    //virtual void setDone() { done_ = true; }

    boost::optional<size_t> job_id_;
    bool set_thread_affinity_ = false;
};
