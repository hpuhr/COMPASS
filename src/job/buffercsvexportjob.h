#ifndef BUFFERCSVEXPORTJOB_H
#define BUFFERCSVEXPORTJOB_H

#include "boost/date_time/posix_time/posix_time.hpp"
#include <memory>

#include "job.h"
#include "buffer.h"

class BufferCSVExportJob : public Job
{
public:
    BufferCSVExportJob(std::shared_ptr<Buffer> buffer, const std::string& file_name, bool overwrite, bool use_presentation);
    virtual ~BufferCSVExportJob();

    virtual void run ();

protected:
    std::shared_ptr<Buffer> buffer_;
    std::string file_name_;
    bool overwrite_;
    bool use_presentation_;

    boost::posix_time::ptime start_time_;
    boost::posix_time::ptime stop_time_;
};

#endif // BUFFERCSVEXPORTJOB_H
