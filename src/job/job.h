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

#ifndef JOB_H_
#define JOB_H_

#include <QObject>
#include <QRunnable>
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
class Job : public QObject, public QRunnable
{
    Q_OBJECT
signals:
    void doneSignal ();
    void obsoleteSignal();

public:
    /// @brief Constructor
    Job(const std::string& name) : name_(name), started_(false), done_ (false), obsolete_(false)
    { setAutoDelete(false); }
    /// @brief Destructor
    virtual ~Job() {}

    // @brief Main operation function
    virtual void run() = 0;

    bool started () { return started_; }
    // @brief Returns done flag
    bool done () { return done_; }
    void emitDone () { emit doneSignal(); }
    // @brief Sets obsolete flag
    void setObsolete () { obsolete_=true; }
    // @brief Returns obsolete flag
    bool obsolete () { return obsolete_; }
    void emitObsolete () { emit doneSignal(); }

    const std::string &name() { return name_; }

protected:
    std::string name_;
    ///
    bool started_;
    /// Done flag
    bool done_;
    /// Obsolete flag
    bool obsolete_;

    virtual void setDone () { done_=true; }
};

#endif /* JOB_H_ */
