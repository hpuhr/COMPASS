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

#ifndef BUFFERCSVEXPORTJOB_H
#define BUFFERCSVEXPORTJOB_H

#include "boost/date_time/posix_time/posix_time.hpp"
#include <memory>

#include "job.h"
#include "buffer.h"
#include "dbovariableset.h"

class BufferCSVExportJob : public Job
{
public:
    BufferCSVExportJob(std::shared_ptr<Buffer> buffer, const DBOVariableSet& read_set, const std::string& file_name,
                       bool overwrite, bool use_presentation);
    virtual ~BufferCSVExportJob();

    virtual void run ();

protected:
    std::shared_ptr<Buffer> buffer_;
    DBOVariableSet read_set_;

    std::string file_name_;
    bool overwrite_;
    bool use_presentation_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;
};

#endif // BUFFERCSVEXPORTJOB_H
