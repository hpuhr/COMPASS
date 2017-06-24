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
 * Job.h
 *
 *  Created on: Feb 4, 2013
 *      Author: sk
 */

#ifndef JOB_H_
#define JOB_H_

#include <QObject>
#include <memory>

/**
 * @brief Encapsulates a work-package
 *
 * When created, is automatically added to the JobOrderer. Callbacks are performed either when the Job was
 * canceled (set as obsolete) or was completed (done). The work itself is defined in the execute function, which
 * must be overridden (and MUST set the done_ flag to true).
 *
 * Important: The Job and the contained data must be deleted in the callback functions.
 */
class Job : public QObject
{
    Q_OBJECT
signals:
    void doneSignal (std::shared_ptr <Job> job);
    void obsoleteSignal(std::shared_ptr <Job> job);

public:
    /// @brief Constructor
    Job() : done_ (false), obsolete_(false) {}
    /// @brief Destructor
    virtual ~Job() {}

    // @brief Main operation function
    virtual void execute ()=0;

    // @brief Returns done flag
    bool done () { return done_; }
    void emitDone () { emit obsoleteSignal(std::shared_ptr<Job>(this)); }
    // @brief Sets obsolete flag
    void setObsolete () { obsolete_=true; }
    // @brief Returns obsolete flag
    bool obsolete () { return obsolete_; }
    void emitObsolete () { emit doneSignal(std::shared_ptr<Job>(this)); }

protected:
    /// Done flag
    bool done_;
    /// Obsolete flag
    bool obsolete_;

    virtual void setDone () { done_=true; }
};

class DBInterface;

/**
 * @brief Job specialization for database operations
 *
 * Was created to ensure that database jobs are performed in the correct order in one thread (performance gain
 * of multiple threads dubious).
 *
 * Requires a DBInterface instance, calls the done_function when completed or obsolete_function when aborted.
 */
class DBJob : public Job
{
    Q_OBJECT
public:
    /// @brief Constructor
    DBJob(DBInterface &db_interface) : Job(), db_interface_(db_interface) {}
    /// @brief Destructor
    virtual ~DBJob() {}

protected:
    /// Database interface
    DBInterface &db_interface_;
};

#endif /* JOB_H_ */
